#ifndef WEATHER_APP_H
#define WEATHER_APP_H

#include <lvgl/lvgl.h>
#include <math.h>
#include <time.h>
#include <netutils/cJSON.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "font_manager.h"
#include "../../utils/include/circle_screen.h"

extern const lv_image_dsc_t sunny;
extern const lv_image_dsc_t cloudy;
extern const lv_image_dsc_t rainy;
extern const lv_image_dsc_t snowy;
extern const lv_image_dsc_t foggy;
extern const lv_image_dsc_t sunny32;
extern const lv_image_dsc_t cloudy32;
extern const lv_image_dsc_t rainy32;
extern const lv_image_dsc_t snowy32;
extern const lv_image_dsc_t foggy32;
extern lv_font_t my_font_cn;

//天气数据结构
typedef struct {
    char temp[10];//温度
    char feelsLike[10];//体感温度
    char icon[10];//图标代码
    char text[20];//天气状况描述
    char windDir[20];//风向
    char windScale[10];//风力等级
    char humidity[10];//湿度
    char updateTime[30];//更新时间
} weather_data_t;

//24小时天气数据结构
typedef struct {
    char temp[10];//温度
    char icon[10];//图标代码
    char text[20];//天气状况描述
    char updateTime[30];//更新时间
} hourly_weather_data_t;

//7天天气预报数据结构
typedef struct {
    char date[20];        // 日期
    char tempMax[10];       //最高温度
    char tempMin[10];       //最低温度
    char icon[10];       // 天气图标
    char text[20];      //天气状况
    char windDir[20];    // 白天风向
    char windScale[10];  // 白天风力等级
} weekly_weather_data_t;


typedef struct {
    lv_obj_t* item_cont;       // 天气信息容器
    lv_obj_t* time_label;      // 时间标签
    lv_obj_t* temp_label;      // 温度标签
    lv_obj_t* text_label;      // 天气状况标签
    lv_obj_t* wind_label;      // 风向风力标签
    lv_obj_t* weather_icon;    // 天气图标
} weather_item_t;

typedef struct {
    lv_obj_t* page_cont;     // 界面容器
    lv_obj_t* location_label;  // 位置标签
    lv_obj_t* temp_label;      // 温度标签
    lv_obj_t* weather_label;   // 天气状况标签
    lv_obj_t* net_label;       // 联网状态标签  
    lv_obj_t* top_label;       // Weather标签
    lv_obj_t* humidity_label;   // 湿度标签
    lv_obj_t* wind_label;      // 风向风力标签
    lv_obj_t* spangroup;       // 体感温度文字组
    lv_span_t* text_span; // 文字span
    lv_span_t* temp_span;  // 温度span
    lv_obj_t* weather_icon;    // 天气图标
    weather_data_t weather_data; // 天气数据
} weather_ui_t;

typedef struct {
    lv_obj_t* page_cont;     // 界面容器
    lv_obj_t* location_label;  // 位置标签
    lv_obj_t* top_label;       // Weather标签
    lv_obj_t* scroll_cont;     // 滚动容器
    lv_obj_t* content_cont;    // 内容容器
    weather_item_t weather_item[24]; // 24小时天气项目数组
    hourly_weather_data_t hourly_data[24]; // 24小时天气数据
} hourly_weather_ui_t;

typedef struct {
    lv_obj_t* page_cont;       // 界面容器
    lv_obj_t* location_label;  // 位置标签
    lv_obj_t* top_label;       // Weather标签
    lv_obj_t* scroll_cont;     // 滚动容器
    lv_obj_t* content_cont;    // 内容容器
    weather_item_t weather_item[7]; //7天天气项目数组
    weekly_weather_data_t weekly_data[7]; //7天天气数据
} weekly_weather_ui_t;

typedef enum {
    WEATHER_PAGE = 0,
    WEATHER_PAGE_HOURLY,
    WEATHER_PAGE_WEEKLY
} weather_page_t;

void weather_app_start(void);
void weather_app_delete(void);
void weather_page(void);
void weekly_weather_page(void);
void hourly_weather_page(void);
void test_weather_update(void);

#endif /* WEATHER_APP_H */