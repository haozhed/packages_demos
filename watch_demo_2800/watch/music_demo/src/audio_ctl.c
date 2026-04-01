#include "audio_ctl.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <nuttx/audio/audio.h>
#include <audioutils/nxaudio.h>
#include <stdlib.h>

// 假设OpenVela使用NuttX音频驱动
// 这些定义需要根据实际音频驱动进行调整
#define AUDIO_DEVICE "/usr/bin/amixer"

static int audio_fd = -1;
static audio_state_t current_state = AUDIO_STATE_STOPPED;
static int current_position = 0;
static int total_duration = 180000;
static int current_volume = 50;
static char current_filename[128] = "";
static int total_songs_count = 0;

// 从路径中提取文件名
const char* extract_filename(const char* path)
{
    if (path == NULL || strlen(path) == 0) {
        return "Unknown";
    }
    
    const char* filename = strrchr(path, '/');
    if (filename != NULL) {
        return filename + 1;
    }
    
    return path;
}

// 移除文件扩展名
void remove_extension(char* filename)
{
    if (filename == NULL) return;
    
    char* dot = strrchr(filename, '.');
    if (dot != NULL) {
        *dot = '\0'; // 在点号处截断字符串
    }
}

// 检查文件是否为MP3文件
static int is_mp3_file(const char* filename)
{
    if (filename == NULL) return 0;
    
    const char* dot = strrchr(filename, '.');
    if (dot != NULL) {
        return (strcasecmp(dot, ".mp3") == 0);
    }
    
    return 0;
}

// 扫描音乐目录并生成歌单
int audio_ctl_scan_music_directory(const char* path, song_info_t* playlist, int max_songs)
{
    if (path == NULL || playlist == NULL || max_songs <= 0) {
        // printf("Invalid parameters for music directory scan\n");
        return 0;
    }
    
    DIR* dir = opendir(path);
    if (dir == NULL) {
        // printf("Cannot open music directory: %s\n", path);
        return 0;
    }
    
    // printf("Scanning music directory: %s\n", path);
    
    struct dirent* entry;
    int song_count = 0;
    
    while ((entry = readdir(dir)) != NULL && song_count < max_songs) {
        // 检查是否为MP3文件
        if (is_mp3_file(entry->d_name)) {
            // 构建完整文件路径
            char full_path[256];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            
            // 设置歌曲信息
            strncpy(playlist[song_count].filename, full_path, sizeof(playlist[song_count].filename) - 1);
            
            // 设置显示名称（移除扩展名）
            strncpy(playlist[song_count].display_name, entry->d_name, sizeof(playlist[song_count].display_name) - 1);
            remove_extension(playlist[song_count].display_name);
            
            // 设置默认时长（实际应用中应该从文件读取）
            playlist[song_count].duration = 180000; // 3分钟
            
            // printf("Found MP3: %s -> %s\n", entry->d_name, playlist[song_count].display_name);
            
            song_count++;
        }
    }
    
    closedir(dir);
    
    total_songs_count = song_count;
    // printf("Total songs found: %d\n", song_count);
    
    return song_count;
}

// 获取歌单中的歌曲数量
int audio_ctl_get_song_count(void)
{
    return total_songs_count;
}

// 初始化音频系统
int audio_ctl_init(void)
{
    // printf("Initializing audio system...\n");
    
    // 尝试打开音频设备
    FILE *file  = open(AUDIO_DEVICE, "r");
    if (file == NULL) {
        // printf("Failed to open audio device: %s\n", AUDIO_DEVICE);
        // 返回0表示初始化失败但程序可以继续运行（UI仍然可用）
        return 0;
    }
    
    // printf("Audio system initialized successfully\n");
    return 1;
}

// 播放指定路径的音频文件
int audio_ctl_play(const char* path)
{
    // printf("Attempting to play: %s\n", path);
    
    // 提取并存储文件名
    const char* filename = extract_filename(path);
    strncpy(current_filename, filename, sizeof(current_filename) - 1);
    current_filename[sizeof(current_filename) - 1] = '\0';
    char command[64];
    snprintf(command, sizeof(command), "mplayer %s\n", filename);
    int ret = system(command);
    
    if (ret == 0) {
        // printf("mplayer fail set to play %s\n", filename);
        return 1;
    }
    return 1;
    
    // 移除扩展名用于显示
    remove_extension(current_filename);
    
    // printf("Current filename set to: %s\n", current_filename); 
    // 模拟播放成功
    current_state = AUDIO_STATE_PLAYING;
    current_position = 0;
    
    // printf("Playback started: %s\n", path);
    return 1;
}

// 获取当前播放的文件名
const char* audio_ctl_get_current_filename(void)
{
    return current_filename;
}

// 暂停播放
int audio_ctl_pause(void)
{
    if (audio_fd < 0 || current_state != AUDIO_STATE_PLAYING) {
        return 0;
    }
    
    // 在实际实现中，这里需要暂停音频输出
    current_state = AUDIO_STATE_PAUSED;
    // printf("Playback paused\n");
    return 1;
}

// 恢复播放
int audio_ctl_resume(void)
{
    if (audio_fd < 0 || current_state != AUDIO_STATE_PAUSED) {
        return 0;
    }
    
    // 在实际实现中，这里需要恢复音频输出
    current_state = AUDIO_STATE_PLAYING;
    // printf("Playback resumed\n");
    return 1;
}

// 停止播放
int audio_ctl_stop(void)
{
    if (audio_fd < 0) {
        return 0;
    }
    
    // 在实际实现中，这里需要停止音频输出并重置解码器
    current_state = AUDIO_STATE_STOPPED;
    current_position = 0;
    // printf("Playback stopped\n");
    return 1;
}

// 获取当前播放状态
audio_state_t audio_ctl_get_state(void)
{
    return current_state;
}

// 获取当前播放位置（毫秒）
int audio_ctl_get_position(void)
{
    // 在实际实现中，这里需要查询音频驱动的当前播放位置
    if (current_state == AUDIO_STATE_PLAYING) {
        // 模拟时间递增
        current_position += 1000; // 每秒增加1000毫秒
        if (current_position > total_duration) {
            current_position = total_duration;
            current_state = AUDIO_STATE_STOPPED;
        }
    }
    return current_position;
}

// 获取音频总时长（毫秒）
int audio_ctl_get_duration(void)
{
    return total_duration;
}

// 设置播放位置（毫秒）
int audio_ctl_seek(int position)
{
    if (position < 0 || position > total_duration) {
        return 0;
    }
    char command[64];
    snprintf(command, sizeof(command), "mplayer -ss %d", position);
    int ret = system(command);
    
    if (ret == 0) {
        // printf("mplayer play in %d\n", position);
        return 1;
    }
    return 1;
    current_position = position;
    // printf("Seek to: %d ms\n", position);
    return 1;
}

// 清理音频资源
void audio_ctl_cleanup(void)
{
    // printf("Audio resources cleaned up\n");
}

// 设置音量 (0-100)
int audio_ctl_set_volume(int volume)
{
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    
    current_volume = volume;
    // printf("Volume set to: %d%%\n", volume);
    
    char command[64];
    snprintf(command, sizeof(command), "amixer set -c 0 Master %d%% > /dev/null 2>&1", volume);
    int ret = system(command);
    
    if (ret == 0) {
        // printf("Volume set to %d%% using amixer\n", volume);
        return 1;
    }
    return 1;
}

// 获取当前音量 (0-100)
int audio_ctl_get_volume(void)
{
    // 返回缓存的音量值
    return current_volume;
}