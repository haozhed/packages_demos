#ifndef MUSICDEMO_H
#define MUSICDEMO_H

#include "lvgl/lvgl.h"
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <lvgl.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 创建圆形音乐播放器UI
 */

/**
 * @brief 更新播放进度（模拟函数）
 */
void update_playback_progress(void);

void music_player_init(void);

void music_app(void);

#ifdef __cplusplus
}
#endif

#endif