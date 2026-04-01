#ifndef __LIST_H
#define __LIST_H

#include "lvgl.h"
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#include "string.h"
#include "../../utils/include/font_manager.h"

// 尺寸配置
#define LAUNCHER_SIZE 455     // 圆形表盘尺寸
#define RADIUS        (LAUNCHER_SIZE / 2)
#define APP_SIZE      80      // 应用项大小
#define APP_GAP       20      // 应用间距
#define TOTAL_APP     17      // 应用总数
#define ROWS          ((TOTAL_APP + 1) / 2)  // 行数

// 函数声明
lv_obj_t *applist_create_list(lv_obj_t *scr);
lv_obj_t *applist_create_grid(void);
void applist_hide_page(void);
void applist_destroy(lv_obj_t *launcher);
void applist_show(void);
void applist_hide(void);
void cleanup_resources(void);

#endif 