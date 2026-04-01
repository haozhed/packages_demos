#include <dirent.h>
#include "../include/musicdemo.h"
#include <string.h>
#include <stdlib.h>
#include <nuttx/config.h>
#include <unistd.h>
#include <sys/boardctl.h>
#include <lvgl/lvgl.h>
#include <lvgl/demos/lv_demos.h>
#include <lv_conf.h>
#include "../include/audio_ctl.h"
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "../../music_list/include/music_list.h"
#include "../../utils/include/font_manager.h"
#include "../../utils/include/circle_screen.h"
#ifdef CONFIG_LV_USE_NUTTX_LIBUV
#include <uv.h>
#endif
#define LV_USE_UNICODE 1        // 启用Unicode支持
#define LV_USE_SYMBOLS 1        // 启用基本符号（如果存在此选项）
// #define LV_SYMBOL_VOLUME_MUTE   "\xef\x9a\x9b" // 检查这些定义是否存在
// #define LV_SYMBOL_VOLUME_LOW    "\xef\x9a\x9c"
// #define LV_SYMBOL_VOLUME_HIGH   "\xef\x9a\x9e"

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
static void lv_nuttx_uv_loop(uv_loop_t *loop, lv_nuttx_result_t *result)
{
  lv_nuttx_uv_t uv_info;
  void *data;

  uv_loop_init(loop);

  lv_memset(&uv_info, 0, sizeof(uv_info));
  uv_info.loop = loop;
  uv_info.disp = result->disp;
  uv_info.indev = result->indev;
#ifdef CONFIG_UINPUT_TOUCH
  uv_info.uindev = result->utouch_indev;
#endif

  data = lv_nuttx_uv_init(&uv_info);
  uv_run(loop, UV_RUN_DEFAULT);
  lv_nuttx_uv_deinit(&data);
}
#endif

#define MAX_SONGS 50  // 增加最大歌曲数量
#define PLAYLIST_PAGE 2  // 歌单页面
// 更新页面枚举
typedef enum {
    PAGE_MUSIC_PLAYER,    // 音乐播放器页面
    PAGE_APP_LAUNCHER,    // 应用页面
    PAGE_PLAYLIST         // 歌单页面
} app_page_t;

// 播放状态枚举（仅用于UI状态）
typedef enum {
    PLAY_STATUS_STOP,
    PLAY_STATUS_PAUSE, 
    PLAY_STATUS_PLAY
} play_status_t;

// 专辑信息结构体
typedef struct {
    char filename[128];  // 存储完整文件名（带路径）
    uint32_t total_time;
} album_info_t;

// 全局状态结构体
typedef struct {
    play_status_t play_status;
    uint32_t current_time;  // 当前播放时间（毫秒）
    album_info_t current_album;
} app_state_t;

// 全局变量声明
static app_state_t C;
static lv_obj_t *root = NULL;
static lv_obj_t* progress_arc = NULL;
static lv_obj_t* play_btn = NULL;
static lv_obj_t* time_label = NULL;
static lv_obj_t* play_img = NULL;  // 播放按钮内的图像对象
static lv_obj_t* status_label = NULL;
static lv_obj_t* volume_slider = NULL;
static lv_obj_t* volume_label = NULL;
static lv_obj_t* volume_icon = NULL;
static int total_songs = 3;
static int current_song_index = 0;

// 添加滑动相关变量
static lv_point_t start_point = {0, 0};
static lv_point_t end_point = {0, 0};
static bool is_dragging = false;
static const int SWIPE_THRESHOLD = 10; // 滑动阈值

// 全局歌单变量
static song_info_t playlist[MAX_SONGS]={
    {"res/music/ruoshuisanqian.mp3", "Morning Rhythm", 180000},
    {"res/music/yanwuxie.mp3", "Urban Landscape", 180000},
    {"res/music/yuanyangxi.mp3", "Digital Dreams", 180000},
};;

// 更新歌曲显示
static void update_song_display(void)
{
    // 找到歌曲名称标签并更新
    lv_obj_t* parent = lv_obj_get_parent(play_btn);
    lv_obj_t* song_label = NULL;
    
    // 遍历子对象找到歌曲名称标签
    uint32_t child_cnt = lv_obj_get_child_cnt(parent);
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t* child = lv_obj_get_child(parent, i);
        if (lv_obj_check_type(child, &lv_label_class)) {
            // 假设第一个找到的标签是歌曲名称标签
            // 在实际应用中，你可能需要更好的方法来识别特定的标签
            song_label = child;
            break;
        }
    }
    
    if (song_label) {
        const char* current_song = C.current_album.filename;
        lv_label_set_text(song_label, current_song);
        // printf("Song display updated to: %s\n", current_song);
    }
}

// 切换到下一首歌曲
static void play_next_song(void)
{
    current_song_index = (current_song_index + 1) % total_songs;
    strcpy(C.current_album.filename, extract_filename(playlist[current_song_index].filename));
    C.current_album.total_time = playlist[current_song_index].duration;
    
    // 开始播放新歌曲
    audio_ctl_play(C.current_album.filename);
    update_song_display();
    
    // printf("Now playing: %s\n", playlist[current_song_index].display_name);
}

// 切换到上一首歌曲
static void play_previous_song(void)
{
    current_song_index = (current_song_index - 1 + total_songs) % total_songs;
    strcpy(C.current_album.filename, extract_filename(playlist[current_song_index].filename));
    C.current_album.total_time = playlist[current_song_index].duration;
    
    audio_ctl_play(C.current_album.filename);
    update_song_display();
    
    // printf("Now playing: %s\n", playlist[current_song_index].display_name);
}

// }
// 设置播放状态（仅更新UI状态）
static void app_set_play_status(play_status_t status)
{
    C.play_status = status;
    // printf("UI Play status changed to: %d\n", status);
}

// 初始化应用状态（更新）
static void init_app_state(void)
{
    C.play_status = PLAY_STATUS_STOP;
    C.current_time = 0;
    // 使用实际音乐文件
    strcpy(C.current_album.filename, "ruoshuisanqian"); // 实际音乐文件路径
    C.current_album.total_time = 180000;
}

// 播放状态事件处理
static void app_play_status_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        switch (C.play_status) {
            case PLAY_STATUS_STOP:
            case PLAY_STATUS_PAUSE:
                // 切换到播放状态
                app_set_play_status(PLAY_STATUS_PLAY);
                lv_image_set_src(play_img, LV_SYMBOL_PAUSE);
                  // 开始播放时更新歌曲名称显示
                update_song_display();
                // 开始播放音乐
                if (audio_ctl_get_state() == AUDIO_STATE_PAUSED) {
                    audio_ctl_resume();
                } else {
                    audio_ctl_play(C.current_album.filename);
                }
                break;
                
            case PLAY_STATUS_PLAY:
                // 切换到暂停状态
                app_set_play_status(PLAY_STATUS_PAUSE);
                lv_image_set_src(play_img, LV_SYMBOL_PLAY);
                
                // 暂停播放
                audio_ctl_pause();
                break;
        }
    }
}

static void previous_song_handler(lv_event_t* e)
{
    play_previous_song();
}

static void next_song_handler(lv_event_t* e)
{
    play_next_song();
}

static void music_list_handler(lv_event_t* e)
{
    font_manager_init(); 
    music_list_create();
    
}


// 创建播放控制按钮（使用普通按钮+图像）
static void create_play_button(lv_obj_t* parent)
{
    // 创建普通按钮
    play_btn = lv_button_create(parent);
    lv_obj_set_size(play_btn, 80, 80);
    lv_obj_set_style_radius(play_btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(play_btn, lv_color_hex(0x2E8B57), 0);
    lv_obj_set_style_bg_color(play_btn, lv_color_hex(0x3CB371), LV_STATE_PRESSED);
    lv_obj_align(play_btn, LV_ALIGN_CENTER, 0, 50);
    lv_obj_add_event_cb(play_btn, app_play_status_event_handler, LV_EVENT_CLICKED, NULL);
    
    // 在按钮内创建图像对象
    play_img = lv_image_create(play_btn);
    lv_image_set_src(play_img, LV_SYMBOL_PLAY);  // 使用LVGL内置符号
    lv_obj_align(play_img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_image_recolor(play_img, lv_color_white(), 0);
    lv_obj_set_style_image_recolor_opa(play_img, LV_OPA_COVER, 0);

     // 上一首按钮
    lv_obj_t* prev_btn = lv_btn_create(parent);
    lv_obj_set_size(prev_btn, 50, 50);
    lv_obj_set_style_radius(prev_btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_align(prev_btn, LV_ALIGN_CENTER, -100, 50);
    lv_obj_t* prev_label = lv_label_create(prev_btn);
    lv_label_set_text(prev_label, LV_SYMBOL_PREV);
    lv_obj_center(prev_label);
    lv_obj_add_event_cb(prev_btn, previous_song_handler, LV_EVENT_CLICKED, NULL);
    
    // 下一首按钮
    lv_obj_t* next_btn = lv_btn_create(parent);
    lv_obj_set_size(next_btn, 50, 50);
    lv_obj_set_style_radius(next_btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_align(next_btn, LV_ALIGN_CENTER, 100, 50);
    lv_obj_t* next_label = lv_label_create(next_btn);
    lv_label_set_text(next_label, LV_SYMBOL_NEXT);
    lv_obj_center(next_label);
    lv_obj_add_event_cb(next_btn, next_song_handler, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* list_btn = lv_btn_create(parent);
    lv_obj_set_size(list_btn, 50, 50);
    lv_obj_set_style_bg_opa(list_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(list_btn, 0, 0);
    lv_obj_set_style_shadow_width( list_btn, 0, 0);
    lv_obj_align(list_btn, LV_ALIGN_RIGHT_MID, -10, 10);
    lv_obj_t* list_label = lv_label_create(list_btn);
    lv_obj_set_size(list_label, 50, 50);
    lv_obj_set_style_border_width(list_label, 0, 0);
    lv_obj_set_style_bg_opa(list_label, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_label_set_text(list_label, LV_SYMBOL_LIST);
    lv_obj_align(list_label, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_center(list_label);
    lv_obj_add_event_cb(list_btn, music_list_handler, LV_EVENT_CLICKED, NULL);
    
}

static void create_watch_background(lv_obj_t* parent)
{
    // 创建图片对象
    lv_obj_t * background_img = lv_image_create(parent);
    // 尝试加载背景图片，如果失败则使用纯色背景
    lv_obj_set_size(background_img, 455, 455);
    lv_obj_align(background_img, LV_ALIGN_CENTER, 0, 0);
    
    // 设置背景图片源 - 根据实际资源路径调整
    const char* bg_path = "/home/ts/Openvela/vela-opensource/apps/examples/watch/musicDemo/res/UX/Background Copy.png";
    lv_image_set_src(background_img, bg_path);

    // 如果图片加载失败，设置默认背景色
    lv_obj_set_style_bg_color(background_img, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_bg_opa(background_img, LV_OPA_COVER, 0);
    
    // 将背景置于最底层
    lv_obj_move_background(background_img);
    
    // printf("Watch background created\n");
}

//进度条点击事件处理
static void progress_arc_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        lv_obj_t * arc = lv_event_get_target(e);

        // 获取点击位置
        lv_point_t p;
        lv_indev_get_point(lv_indev_get_act(), &p);

        // 获取arc的绝对坐标
        lv_area_t coords;
        lv_obj_get_coords(arc, &coords);

        // 计算圆心
        lv_coord_t center_x = coords.x1 + lv_obj_get_width(arc) / 2;
        lv_coord_t center_y = coords.y1 + lv_obj_get_height(arc) / 2;

        // 计算相对坐标
        lv_coord_t rel_x = p.x - center_x;
        lv_coord_t rel_y = p.y - center_y;

        // 使用标准数学计算角度
        double angle_rad = atan2(rel_y, rel_x);
        double angle_deg = angle_rad * 180.0 / 3.14;

        // 转换为 0-360 度
        if(angle_deg < 0) angle_deg += 360.0;

        // LVGL arc 默认从右侧(0°)开始，顺时针到360°
        // 但我们想要从顶部开始，所以需要调整
        // 顶部对应270°，所以减去270°
        angle_deg = angle_deg - 270.0;
        if(angle_deg < 0) angle_deg += 360.0;

        // 将角度转换为 0-100 的值
        int32_t value = (int32_t)((angle_deg * 100.0) / 360.0);

        // 确保值在范围内
        if(value < 0) value = 0;
        if(value > 100) value = 100;

        // 设置新值
        lv_arc_set_value(progress_arc, value);
        
        // 跳转到对应位置
        int total_duration = audio_ctl_get_duration();
        if (total_duration > 0) {
            int seek_position = (total_duration * value) / 100;
            if (audio_ctl_seek(seek_position)) {
                // printf("Seek to: %d%% (%d ms)\n", value, seek_position);
            } else {
                // printf("Seek failed\n");
            }
        }
        // 计算总秒数
        static char buffer[32];
        int totalSeconds = value * 0.01 * 180000 / 1000;      
        // 计算分钟和秒数
        long minutes = totalSeconds / 60;
        long seconds = totalSeconds % 60;


        sprintf(buffer, "%02ld:%02ld/03:00", minutes, seconds);
        lv_label_set_text(time_label, buffer);
    }
}

// 创建环形进度条
static void create_progress_arc(lv_obj_t* parent)
{
    progress_arc = lv_arc_create(parent);
    lv_obj_set_size(progress_arc, 410, 410);
    lv_arc_set_range(progress_arc, 0, 100);
    lv_arc_set_value(progress_arc, 0);
    lv_arc_set_bg_angles(progress_arc, 0, 360);
    lv_arc_set_rotation(progress_arc, 270);  // 从顶部开始
    
    // 移除旋钮
    lv_obj_remove_style(progress_arc, NULL, LV_PART_KNOB);
    
    // 设置背景弧样式
    lv_obj_set_style_arc_width(progress_arc, 15, LV_PART_MAIN);
    lv_obj_set_style_arc_color(progress_arc, lv_color_hex(0x404040), LV_PART_MAIN);
    
    // 设置进度弧样式
    lv_obj_set_style_arc_width(progress_arc, 15, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(progress_arc, lv_color_hex(0x2E8B57), LV_PART_INDICATOR);
    
    lv_obj_align(progress_arc, LV_ALIGN_CENTER, 0, 0);
    // 添加进度条点击事件
    lv_obj_add_event_cb(progress_arc, progress_arc_event_handler, LV_EVENT_CLICKED, NULL);
}

// 音量滑块事件处理
static void volume_slider_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * slider = lv_event_get_target(e);
    // printf("step in volume_slider_event_handler\n");
    if(code == LV_EVENT_VALUE_CHANGED) {
        // 获取滑块的当前值，假设范围是0-100
        int32_t volume_value = lv_slider_get_value(slider);
        volume_icon = lv_event_get_user_data(e); // 获取用户关联的音量图标

        // printf("step in lv_event_get_user_data %d\n",volume_value);

        // 根据音量值切换图标
        if(volume_value == 0) {
            lv_label_set_text(volume_icon, LV_SYMBOL_MUTE);
        } else if(volume_value > 0 && volume_value < 100) {
            lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MID);
            // printf("step in lv_img_set_src\n");
        } else {
            lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MAX);
        }

        // 实时更新音量
        audio_ctl_set_volume(volume_value);
        // printf("step in audio_ctl_set_volume\n");
        char vol_text[8];
        snprintf(vol_text, sizeof(vol_text), "%d%%", volume_value);
        lv_label_set_text(volume_label, vol_text);
        
        // printf("Volume changed: %d%%\n", volume_value);
    }
}
static uint32_t previous_volume = 50; // 记录静音前的音量

static void volume_icon_event_cb(lv_event_t* e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * icon = lv_event_get_target(e);
    lv_obj_t * slider = lv_event_get_user_data(e); // 获取关联的滑块

    if(code == LV_EVENT_CLICKED) {
        int32_t current_vol = lv_slider_get_value(slider);
        
        if(current_vol > 0) {
            // 如果当前不是静音，则静音
            previous_volume = current_vol;
            lv_slider_set_value(slider, 0, LV_ANIM_ON);
            lv_label_set_text(volume_icon, LV_SYMBOL_MUTE);
        } else {
            // 如果当前是静音，则恢复之前的音量
            lv_slider_set_value(slider, previous_volume, LV_ANIM_ON);// 根据音量值切换图标
            if(previous_volume == 0) {
                lv_label_set_text(volume_icon, LV_SYMBOL_MUTE);
            } else if(previous_volume > 0 && previous_volume < 100) {
                lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MID);
                // printf("step in lv_img_set_src\n");
            } else {
                lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MAX);
            }

            // 图标会根据滑块的值在滑块的事件回调中自动更新
        }
    }

}
// 创建音量调节控件
static void create_volume_control(lv_obj_t* parent)
{

    // 创建音量图标
    volume_icon = lv_label_create(parent);
    lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MID);
    lv_obj_set_style_text_color(volume_icon, lv_color_hex(0xCCCCCC), 0);
    //lv_obj_set_style_text_font(volume_icon, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_align(volume_icon, LV_ALIGN_CENTER, -100, 120);
    
    // 创建音量滑块
    volume_slider = lv_slider_create(parent);
    lv_obj_set_size(volume_slider, 180, 10);
    lv_slider_set_range(volume_slider, 0, 100);
    lv_slider_set_value(volume_slider, audio_ctl_get_volume(), LV_ANIM_OFF);
    lv_obj_align(volume_slider, LV_ALIGN_CENTER, 20, 120);
    
    // 设置滑块样式
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x404040), LV_PART_MAIN);
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x2E8B57), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x2E8B57), LV_PART_KNOB);
    lv_obj_set_style_pad_all(volume_slider, 0, LV_PART_KNOB);
    lv_obj_set_style_radius(volume_slider, 5, LV_PART_KNOB);
    
    // 创建音量标签
    volume_label = lv_label_create(parent);
    char vol_text[8];
    snprintf(vol_text, sizeof(vol_text), "%d%%", audio_ctl_get_volume());
    lv_label_set_text(volume_label, vol_text);
    lv_obj_set_style_text_color(volume_label, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_text_font(volume_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_align(volume_label, LV_ALIGN_CENTER, 100, 120);
    
    lv_obj_add_flag(volume_icon,LV_OBJ_FLAG_CLICKABLE);
    // 添加滑块事件回调
    lv_obj_add_event_cb(volume_slider, volume_slider_event_handler, LV_EVENT_VALUE_CHANGED, volume_icon);
    lv_obj_add_event_cb(volume_icon, volume_icon_event_cb, LV_EVENT_CLICKED, volume_slider);
    // printf("Volume control created with value: %d\n", audio_ctl_get_volume());
}

// 创建信息标签
static void create_info_labels(lv_obj_t* parent)
{
    
    // 获取当前播放的文件名（不包含路径和扩展名）
    const char* current_song = audio_ctl_get_current_filename();
    // 创建歌曲名称标签 - 显示真实文件名
    lv_obj_t* song_label = lv_label_create(parent);
    lv_label_set_text(song_label, C.current_album.filename);
    lv_obj_set_style_text_color(song_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(song_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_align(song_label, LV_ALIGN_CENTER, 0, -100);
    
    
    // 创建时间标签
    time_label = lv_label_create(parent);
    lv_label_set_text(time_label, "00:00/03:00");
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_text_font(time_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -60);
    
}

// 退出音乐列表界面
static void exit_music_player(void) {
    if(root){
        // printf("step in exit_music_player\n");
        lv_obj_add_flag(exit_music_player, LV_OBJ_FLAG_HIDDEN);
        lv_obj_del(root);
        root = NULL;
        progress_arc = NULL;
        play_btn = NULL;
        time_label = NULL;
        play_img = NULL;  // 播放按钮内的图像对象
        status_label = NULL;
        volume_slider = NULL;
        volume_label = NULL;
        volume_icon = NULL;
         //watch_start();
    }
    
}

// 手势事件处理函数
static void gesture_event_handler(lv_event_t *e) {

    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED) {
        // 记录按压起始点
        lv_indev_t *indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &start_point);
            is_dragging = true;
        }
    }
    else if (code == LV_EVENT_RELEASED) {
        if (!is_dragging) return;

        // 记录释放点
        lv_indev_t *indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &end_point);

            // 计算滑动距离
            int delta_x = end_point.x - start_point.x;
            int delta_y = end_point.y - start_point.y;

            // 判断是否为右滑手势（X轴移动距离大于阈值，且X轴移动距离大于Y轴移动距离）
            if (delta_x > SWIPE_THRESHOLD && abs(delta_x) > abs(delta_y)) {
                // printf("Right swipe detected, exiting music player\n");
                exit_music_player();
            }
        }
        is_dragging = false;
    }
    else if (code == LV_EVENT_PRESS_LOST) {
        is_dragging = false;
    }
}

static void gesture_event_handler_music_demo_cb(lv_event_t *e)
{
    LV_LOG_USER("检测手势时间music_demo\n");
    lv_obj_t * obj = lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    switch (code)
    {
    case LV_EVENT_GESTURE:
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        if (dir == LV_DIR_BOTTOM)
        {
            LV_LOG_USER(" LOG_DIR_BOTTOM---But do nothing\n");
        }
        if (dir == LV_DIR_TOP)
        {
            LV_LOG_USER(" LV_DIR_TOP---But do nothing\n");
        }
        if (dir == LV_DIR_LEFT)
        {
            LV_LOG_USER(" LV_DIR_LEFT---But do nothing\n");
        }
        if (dir == LV_DIR_RIGHT)
        {
            LV_LOG_USER("右滑退出music_demo手势事件\n");
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_delete(obj);
            obj = NULL;
            break;
        }

    default:
        break;
    }
}
// 创建UI界面
static void create_music_player_ui(void)
{
    // 创建根容器
    // root = create_watch_scr();
    // lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), 0);
    // lv_obj_set_size(root, 455, 455);
    LV_LOG_USER("创建ui界面");
    // 创建根容器
    root = lv_obj_create(get_watch_scr());
    // lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), 0);
    // lv_obj_set_size(root, 455, 455);

    lv_obj_set_size(root, LV_CIRCLE_WATCH, LV_CIRCLE_WATCH);
    lv_obj_set_style_radius(root, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_border_width(root, 0, 0);

    //new code
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(root, LV_OBJ_FLAG_EVENT_BUBBLE); // 允许事件冒泡
    //new_code

    // 启用拖拽功能以支持手势识别
    // lv_obj_add_flag(root, LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_add_flag(root, LV_OBJ_FLAG_GESTURE_BUBBLE);
    // 为容器添加手势事件监听
     //lv_obj_add_event_cb(root, gesture_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(root, gesture_event_handler_music_demo_cb, LV_EVENT_GESTURE, root);
    lv_obj_remove_flag(root, LV_OBJ_FLAG_GESTURE_BUBBLE);

    // 创建各个UI组件
    create_watch_background(root);
    create_progress_arc(root);
    create_play_button(root);
    create_info_labels(root);
    create_volume_control(root);  // 新增音量控件
    LV_LOG_USER("创建ui界面结束");
}

// 更新进度显示（模拟）
static void app_update_progress(void)
{
    // 获取音频播放状态
    audio_state_t audio_state = audio_ctl_get_state();
    
    // 同步UI状态与音频状态
    if (audio_state == AUDIO_STATE_PLAYING && C.play_status != PLAY_STATUS_PLAY) {
        app_set_play_status(PLAY_STATUS_PLAY);
        lv_img_set_src(play_img, LV_SYMBOL_PAUSE);
        lv_label_set_text(status_label, "Playing");
    } else if (audio_state == AUDIO_STATE_PAUSED && C.play_status != PLAY_STATUS_PAUSE) {
        app_set_play_status(PLAY_STATUS_PAUSE);
        lv_img_set_src(play_img, LV_SYMBOL_PLAY);
        lv_label_set_text(status_label, "Paused");
    } else if (audio_state == AUDIO_STATE_STOPPED && C.play_status != PLAY_STATUS_STOP) {
        app_set_play_status(PLAY_STATUS_STOP);
        lv_img_set_src(play_img, LV_SYMBOL_PLAY);
        lv_label_set_text(status_label, "Stopped");
    }
  
    
    // 更新进度显示
    if (audio_state == AUDIO_STATE_PLAYING || audio_state == AUDIO_STATE_PAUSED) {
        // 获取当前播放位置和总时长
        C.current_time = audio_ctl_get_position();
        int total_time = audio_ctl_get_duration();
        
        // 更新进度条
        if (total_time > 0) {
            int32_t progress = (C.current_time * 100) / total_time;
            if (progress_arc != NULL) {
                lv_arc_set_value(progress_arc, progress);
            }
          }
    if (C.play_status == PLAY_STATUS_PLAY) {
        // 模拟时间增加
        C.current_time += 1000;  // 每次增加1秒
        
        if (C.current_time > C.current_album.total_time) {
            C.current_time = C.current_album.total_time;
            app_set_play_status(PLAY_STATUS_STOP);
            lv_img_set_src(play_img, LV_SYMBOL_PLAY);
        }
        
        // 更新进度条
        int32_t progress = (C.current_time * 100) / C.current_album.total_time;
        if (progress_arc != NULL) {
            lv_arc_set_value(progress_arc, progress);
        }
        
        // 更新时间显示
        if (time_label != NULL) {
            char time_text[32];
            snprintf(time_text, sizeof(time_text), "%02d:%02d/%02d:%02d",
                     (int)(C.current_time/60000), (int)((C.current_time/1000)%60),
                     (int)(C.current_album.total_time/60000), 
                     (int)((C.current_album.total_time/1000)%60));
            lv_label_set_text(time_label, time_text);
        }
    }
  }
}

// 初始化函数
void music_player_init(void)
{
  // 初始化音频系统
  if (!audio_ctl_init()) {
    //   printf("Audio initialization failed - UI only mode\n");
      lv_label_set_text(status_label, "Audio init failed");
  }

  init_app_state();
  create_music_player_ui();
//   printf("Music Player UI initialized\n");
}

// 主循环更新函数（在应用主循环中调用）
void music_player_update(void)
{
    app_update_progress();
}

// 清理函数
void music_player_cleanup(void)
{
    audio_ctl_stop();
    audio_ctl_cleanup();
    // printf("Music Player cleaned up\n");
}

int main(int argc, FAR char *argv[])
{
  lv_nuttx_dsc_t info;
  lv_nuttx_result_t result;

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
  uv_loop_t ui_loop;
  lv_memzero(&ui_loop, sizeof(ui_loop));
#endif

  if (lv_is_initialized())
    {
      LV_LOG_ERROR("LVGL already initialized! aborting.");
      return -1;
    }

#ifdef NEED_BOARDINIT
  /* Perform board-specific driver initialization */

  boardctl(BOARDIOC_INIT, 0);

#endif

  lv_init();

  lv_nuttx_dsc_init(&info);

#ifdef CONFIG_LV_USE_NUTTX_LCD
  info.fb_path = "/dev/lcd0";
#endif

#ifdef CONFIG_INPUT_TOUCHSCREEN
  info.input_path = CONFIG_EXAMPLES_LVGLDEMO_INPUT_DEVPATH;
#endif

  lv_nuttx_init(&info, &result);

  if (result.disp == NULL)
    {
      LV_LOG_ERROR("lv_demos initialization failure!");
      return 1;
    }

  if (!lv_demos_create(&argv[1], argc - 1))
    {
        lv_demos_show_help();
        
        create_music_player_ui();
    
      
        goto demo_end;
    }

      /* we can add custom demos here */

      


#ifdef CONFIG_LV_USE_NUTTX_LIBUV
  lv_nuttx_uv_loop(&ui_loop, &result);
#else
  while (1)
    {
      uint32_t idle;
      idle = lv_timer_handler();

      /* Minimum sleep of 1ms */

      idle = idle ? idle : 1;
      usleep(idle * 1000);
    }
#endif

demo_end:
  lv_nuttx_deinit(&result);
  lv_deinit();

  return 0;
}

void music_app(void)
{
    create_music_player_ui();
}
