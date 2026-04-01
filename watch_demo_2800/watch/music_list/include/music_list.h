#ifndef MUSIC_LIST_H
#define MUSIC_LIST_H

#include <lvgl/lvgl.h>
#include <stdbool.h>

// 音乐数据结构
typedef struct {
    const char *title;
    const char *artist;
    int play_status; // 1表示播放，0表示暂停
} music_t;

// 初始化音乐列表
void music_list_create(void);

// 刷新音乐列表显示
void music_list_refresh(void);

#endif // MUSIC_LIST_H