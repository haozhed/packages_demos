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
#include "../include/music_list.h"
#include <stdio.h>
#include <string.h>
#include "../../utils/include/circle_screen.h"
#include "../../utils/include/font_manager.h"

static lv_obj_t *music_list_obj = NULL;
static lv_obj_t *music_list_container = NULL;
static lv_obj_t *total_songs_label = NULL;
static lv_obj_t* left_arrow_icon = NULL;

// 添加滑动相关变量
static lv_point_t start_point = {0, 0};
static lv_point_t end_point = {0, 0};
static bool is_dragging = false;
static const int SWIPE_THRESHOLD = 20; // 滑动阈值

// 音乐数据
static music_t music_list[] = {
    {"ruoshuisanqian", "Morning Rhythm", 1},
    {"yanwuxie", "Urban Landscape", 0},
    {"yuanyangxi", "artist3", 0},
    {"unknownmusic", "artist4", 0},

};

static int music_count = sizeof(music_list) / sizeof(music_list[0]);

LV_IMAGE_DECLARE(playing);
LV_IMAGE_DECLARE(play);

typedef struct {
    lv_obj_t *icon;
    lv_obj_t *title_label;
    lv_obj_t *artist_label;
    int index;
} item_data_t;

static item_data_t item_data_array[100];

// 退出音乐列表界面
static void exit_music_list(void) {
    if (music_list_container) {
        lv_obj_add_flag(music_list_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_del(music_list_container);
        music_list_container = NULL;
         //music_list_obj = NULL;
         //total_songs_label = NULL;
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
                // printf("Right swipe detected, exiting music list\n");
                exit_music_list();
            }
        }
        is_dragging = false;
    }
    else if (code == LV_EVENT_PRESS_LOST) {
        is_dragging = false;
    }
}

static void gesture_event_handler_music_list_cb(lv_event_t *e)
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
            LV_LOG_USER("右滑退出music_list手势事件\n");
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_delete(obj);
            obj = NULL;
            break;
        }

    default:
        break;
    }
}

// 刷新所有列表项的显示状态
static void refresh_all_items(void) {
    if (!music_list_obj || !music_list) return;

    // 更新总数标签
    if (total_songs_label) {
        lv_label_set_text_fmt(total_songs_label, "共%d首歌", music_count);
    }

    // 更新每个列表项的显示
    for (int i = 0; i < music_count; i++) {
        lv_obj_t *item = lv_obj_get_child(music_list_obj, i);
        if (item) {
            item_data_t *data = (item_data_t *)lv_obj_get_user_data(item);
            if (data) {
                // 更新图标
                if (music_list[i].play_status == 1) {
                    lv_img_set_src(data->icon, &playing);
                } else {
                    lv_img_set_src(data->icon, &play);
                    //lv_img_set_src(data->icon, "/data/play.png");
                }

                // 更新文字颜色
                if (music_list[i].play_status == 1) {
                    lv_obj_set_style_text_color(data->title_label, lv_color_hex(0x00FF00), 0);
                    lv_obj_set_style_text_color(data->artist_label, lv_color_hex(0x556B2F), 0);
                } else {
                    lv_obj_set_style_text_color(data->title_label, lv_color_hex(0xFFFFFF), 0);
                    lv_obj_set_style_text_color(data->artist_label, lv_color_hex(0xAAAAAA), 0);
                }
            }
        }
    }
}

// 刷新音乐列表显示
void music_list_refresh(void) {
    refresh_all_items();
}

// 图标点击事件处理函数
static void play_icon_click_handler(lv_event_t *e) {
    lv_obj_t *icon = lv_event_get_target(e);
    lv_obj_t *item = lv_obj_get_parent(lv_obj_get_parent(icon));
    item_data_t *data = (item_data_t *)lv_obj_get_user_data(item);
    
    if (!data || !music_list) return;

    int index = data->index;

    // 如果点击的不是当前播放的歌曲，或者当前歌曲没有在播放，则播放该歌曲
    if (music_list[index].play_status == 0) {
        // 停止其他正在播放的歌曲
        for (int i = 0; i < music_count; i++) {
            if (i != index) {
                music_list[i].play_status = 0;
            }
        }

        // 播放当前歌曲
        music_list[index].play_status = 1;

        // 这里可以添加播放逻辑
        printf("Play music: %s - %s\n", 
               music_list[index].title, 
               music_list[index].artist);

        // 刷新UI显示
        music_list_refresh();
    }
}

// 创建音乐列表项
static lv_obj_t *create_music_item(lv_obj_t *parent, int index) {
    if (!music_list || index >= music_count) return NULL;

    music_t *music = &music_list[index];

    // 使用lv_list_add_btn创建列表项按钮
    lv_obj_t *item = lv_list_add_btn(parent, NULL, NULL);
    lv_obj_set_size(item, lv_pct(100), 110);
    lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(item, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_left(item, 20, 0);
    lv_obj_set_style_pad_right(item, 20, 0);
    lv_obj_set_style_pad_top(item, 0, 0);
    lv_obj_set_style_pad_bottom(item, 0, 0);

    // 创建内容容器来管理文本和分割线的位置
    lv_obj_t *content = lv_obj_create(item);
    lv_obj_set_size(content, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(content, 0, 0);
    lv_obj_align(content, LV_ALIGN_CENTER, 0, 0);

    // 歌曲名
    lv_obj_t *title = lv_label_create(content);
    lv_label_set_text(title, music->title);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 20);

    // 歌手名
    lv_obj_t *artist = lv_label_create(content);
    lv_label_set_text_fmt(artist, "-%s", music->artist);
    lv_obj_set_style_text_color(artist, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_text_font(artist, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_align(artist, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align_to(artist, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 18);

    // 播放状态图标 - 根据播放状态显示不同图标
    lv_obj_t *play_icon = lv_img_create(content);
    lv_img_set_src(play_icon, music->play_status == 1 ? &playing : &play);
    //lv_img_set_src(play_icon, "/data/play.png");
    lv_obj_set_size(play_icon, 80, 80);
    lv_obj_align(play_icon, LV_ALIGN_RIGHT_MID, -0, -3);

    // 为图标添加点击事件
    lv_obj_add_flag(play_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(play_icon, play_icon_click_handler, LV_EVENT_CLICKED, NULL);

    // 使用静态数组而不是动态内存分配
    if (index < 100) {
        item_data_t *item_data = &item_data_array[index];
        item_data->icon = play_icon;
        item_data->title_label = title;
        item_data->artist_label = artist;
        item_data->index = index;
        lv_obj_set_user_data(item, item_data);
    }

    // 添加分隔线（如果不是最后一项）
    if (index < music_count - 1) {
        lv_obj_t *separator = lv_obj_create(content);
        lv_obj_set_size(separator, lv_pct(100), 1);
        lv_obj_set_style_bg_color(separator, lv_color_hex(0x333333), 0);
        lv_obj_set_style_bg_opa(separator, LV_OPA_COVER, 0);
        lv_obj_set_style_border_opa(separator, LV_OPA_TRANSP, 0);
        lv_obj_align(separator, LV_ALIGN_BOTTOM_MID, 0, 0);
    }

    return item;
}

// 初始化音乐列表
void music_list_create(void) {
    // 限制最大数量，防止数组越界
    if (music_count > 100) {
        music_count = 100;
    }

    // 创建UI
    //music_list_container = create_watch_scr();
    music_list_container = lv_obj_create(get_watch_scr());
    //lv_obj_set_style_bg_color(music_list_container, lv_color_hex(0x1a1a2e), 0);
    
    lv_obj_set_size(music_list_container, LV_CIRCLE_WATCH, LV_CIRCLE_WATCH);
    lv_obj_set_style_radius(music_list_container, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(music_list_container, lv_color_black(), 0);
    lv_obj_set_style_border_width(music_list_container, 0, 0);

    left_arrow_icon = lv_label_create(music_list_container);
    lv_label_set_text(left_arrow_icon, LV_SYMBOL_LEFT);
    //lv_obj_set_style_text_font(left_arrow_icon, font_manager_get_font(FONT_SMALL), 0);
    
    // 设置样式
    lv_obj_set_style_text_color(left_arrow_icon, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_text_opa(left_arrow_icon, LV_OPA_100, 0);
    lv_obj_align(left_arrow_icon, LV_ALIGN_LEFT_MID, 25, 0);
    // 启用拖拽功能以支持手势识别
    lv_obj_add_flag(music_list_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(music_list_container, LV_OBJ_FLAG_GESTURE_BUBBLE);

    // 为容器添加手势事件监听
    //lv_obj_add_event_cb(music_list_container, gesture_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(music_list_container, gesture_event_handler_music_list_cb, LV_EVENT_GESTURE, music_list_container);
    lv_obj_remove_flag(music_list_container, LV_OBJ_FLAG_GESTURE_BUBBLE);

    // 创建歌曲总数标签
    total_songs_label = lv_label_create(music_list_container);
    lv_label_set_text_fmt(total_songs_label, "共%d首歌", music_count);
    lv_obj_set_style_text_color(total_songs_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(total_songs_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_align(total_songs_label, LV_ALIGN_TOP_MID, 0, 30);

    // 创建音乐列表
    music_list_obj = lv_list_create(music_list_container);
    lv_obj_set_size(music_list_obj, lv_pct(90), lv_pct(80));
    lv_obj_align(music_list_obj, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(music_list_obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(music_list_obj, LV_OPA_TRANSP, 0);

    lv_obj_remove_style(music_list_obj, NULL, LV_PART_SCROLLBAR);

    // 添加音乐列表项
    if (music_list) {
        for (int i = 0; i < music_count; i++) {
            create_music_item(music_list_obj, i);
        }
    }

    // 初始刷新显示状态
    refresh_all_items();
}

// 获取音乐列表容器（用于外部管理）
lv_obj_t *music_list_get_container(void) {
    return music_list_container;
}