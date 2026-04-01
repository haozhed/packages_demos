#ifndef AUDIO_CTL_H
#define AUDIO_CTL_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// 音频播放状态
typedef enum {
    AUDIO_STATE_STOPPED,
    AUDIO_STATE_PLAYING,
    AUDIO_STATE_PAUSED
} audio_state_t;

// 歌曲信息结构体
typedef struct {
    char filename[128];      // 完整文件路径
    char display_name[64];   // 显示名称（无扩展名）
    uint32_t  duration;       // 歌曲时长（毫秒）
} song_info_t;

// 初始化音频系统
int audio_ctl_init(void);

// 播放指定路径的音频文件
int audio_ctl_play(const char* path);

// 暂停播放
int audio_ctl_pause(void);

// 恢复播放
int audio_ctl_resume(void);

// 停止播放
int audio_ctl_stop(void);

// 获取当前播放状态
audio_state_t audio_ctl_get_state(void);

// 获取当前播放位置（毫秒）
int audio_ctl_get_position(void);

// 获取音频总时长（毫秒）
int audio_ctl_get_duration(void);

// 设置播放位置（毫秒）
int audio_ctl_seek(int position);


// 获取当前播放的文件名
const char* audio_ctl_get_current_filename(void);

// 扫描音乐目录并生成歌单（新增）
int audio_ctl_scan_music_directory(const char* path, song_info_t* playlist, int max_songs);

// 获取歌单中的歌曲数量（新增）
int audio_ctl_get_song_count(void);

// 清理音频资源
void audio_ctl_cleanup(void);

int audio_ctl_get_volume(void);

int audio_ctl_set_volume(int volume);

const char* extract_filename(const char* path);

void remove_extension(char* filename);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_CTL_H