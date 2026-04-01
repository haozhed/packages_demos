#ifndef QUICKS_RPS_H
#define QUICKS_RPS_H

#include <lvgl/lvgl.h>

lv_obj_t * create_rps_page(lv_obj_t * parent);
void on_rps_destroy(lv_obj_t* page);              // 页面销毁
void on_rps_resume(lv_obj_t* page);               // 页面恢复
void on_rps_pause(lv_obj_t* page);                // 页面暂停

#endif