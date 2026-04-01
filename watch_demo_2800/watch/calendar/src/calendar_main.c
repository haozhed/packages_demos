#include <nuttx/config.h>
#include <unistd.h>
#include <sys/boardctl.h>

#include "../include/calendar_main.h"
#include "../../nav_page/include/nav_page_manager.h"
#include "../../utils/include/circle_screen.h"
#include "../../utils/include/font_manager.h"
#ifdef CONFIG_LV_USE_NUTTX_LIBUV
#include <uv.h>
#endif

// 手势相关变量
static int gesture_start_x = 0;
static int gesture_start_y = 0;
static bool gesture_in_progress = false;
static const int gesture_threshold = 50; // 滑动阈值（像素）

//获取当前日期
static lv_calendar_date_t get_now_day(void)
{
    time_t current_time;
    struct tm *time_info;
    
    time(&current_time);
    time_info = localtime(&current_time);

    lv_calendar_date_t today;
    today.year = time_info->tm_year + 1900;
    today.month = time_info->tm_mon + 1;
    today.day = time_info->tm_mday;

    return today;
}

/**
 * Get the day of the week
 * @param year a year
 * @param month a  month [1..12]
 * @param day a day [1..32]
 * @return [0..6] which means [Sun..Sat] or [Mon..Sun] depending on LV_CALENDAR_WEEK_STARTS_MONDAY
 */
static uint8_t get_day_of_week(uint32_t year, uint32_t month, uint32_t day)
{
    uint32_t a = month < 3 ? 1 : 0;
    uint32_t b = year - a;

#if LV_CALENDAR_WEEK_STARTS_MONDAY
    uint32_t day_of_week = (day + (31 * (month - 2 + 12 * a) / 12) + b + (b / 4) - (b / 100) + (b / 400) - 1) % 7;
#else
    uint32_t day_of_week = (day + (31 * (month - 2 + 12 * a) / 12) + b + (b / 4) - (b / 100) + (b / 400)) % 7;
#endif

    return day_of_week;
}

// 获取当月天数
static uint32_t get_month_days(uint32_t year, uint32_t month) {
    // 每月的天数（非闰年）
    const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    if(month < 1 || month > 12) {
        return 0;
    }
    
    // 二月特殊处理（闰年）
    if(month == 2) {
        // 闰年判断：能被4整除但不能被100整除，或者能被400整除
        bool is_leap_year = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return is_leap_year ? 29 : 28;
    }
    
    return days_in_month[month - 1];
}

// 周末高亮显示
static void set_highlight_weekends(lv_obj_t * obj, lv_calendar_date_t *showed)
{
    static lv_calendar_date_t highlighted_dates[12];
    static uint32_t weekend = 0;
    static uint32_t count = 0;

    for (int day = 1; day <= get_month_days(showed->year, showed->month); day++)
    {
        weekend = get_day_of_week(showed->year, showed->month, day);
        if (weekend == 0 || weekend == 6) {
            highlighted_dates[count].year = showed->year;
            highlighted_dates[count].month = showed->month;
            highlighted_dates[count].day = day;
            count += 1;
            lv_calendar_set_highlighted_dates(obj, highlighted_dates, count+1);
        }
    }
}

// static void calendar_event_handler(lv_event_t * e) {
//     lv_obj_t *obj = lv_event_get_target(e);
//     lv_event_code_t code = lv_event_get_code(e);
    
//     // 当用户点击前后箭头切换月份时触发
//     if(code == LV_EVENT_CLICKED) {
//         // 获取当前显示的年月
//         lv_calendar_date_t *showed = lv_calendar_get_showed_date(obj);
        
//         set_highlight_weekends(obj, showed);
//     }
// }

//设置当前日期
static void set_now_day(lv_obj_t * obj)
{
    lv_calendar_date_t today = get_now_day();

    lv_calendar_set_today_date(obj, today.year, today.month, today.day);
    lv_calendar_set_showed_date(obj, today.year, today.month);
    lv_calendar_date_t *showed = lv_calendar_get_showed_date(obj);
    set_highlight_weekends(obj, showed);
}

// 右滑回调函数
static void cont_right_scroll_event_cb(lv_event_t * e)
{
    // LV_LOG_USER("cont_right_scroll_event_cb start");
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_PRESSED && code != LV_EVENT_RELEASED) return;
    lv_indev_t *indev = lv_event_get_indev(e);
    if (!indev) return;
    lv_obj_t * cont = lv_event_get_user_data(e);

    lv_point_t point;
    lv_indev_get_point(indev, &point);

    switch (code) {
        case LV_EVENT_PRESSED:
            gesture_start_x = point.x;
            gesture_start_y = point.y;
            gesture_in_progress = true;
            break;

        case LV_EVENT_RELEASED:
            if (gesture_in_progress) {
                int delta_x = point.x - gesture_start_x;
                int delta_y = point.y - gesture_start_y;

                // 检查是否是右滑手势（水平滑动距离大于阈值，且垂直滑动较小）
                if (delta_x > gesture_threshold && abs(delta_y) < gesture_threshold) {
                    LV_LOG_USER("右滑事件触发");
                    // 右滑隐藏
                    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_delete(cont);
                    cont = NULL;
                }
                else if (delta_x < (0 - gesture_threshold) && abs(delta_y) < gesture_threshold) {
                    LV_LOG_USER("左滑事件触发");
                }

                gesture_in_progress = false;
            }
            break;

        default:
            break;
    }
}

static void calendar_gesture_event_cb(lv_event_t *e)
{
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
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_delete(obj);
            obj = NULL;
            break;
        }

    default:
        break;
    }
}

// 创建日历
static void create_calendar(void)
{
    LV_LOG_USER("create_calendar start");
    // 创建日历容器
    lv_obj_t * cont = lv_obj_create(get_watch_scr());
    lv_obj_set_size(cont, LV_CIRCLE_WATCH, LV_CIRCLE_WATCH);
    lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(cont, lv_color_black(), 0);
    lv_obj_set_style_border_width(cont, 0, 0);

    // 创建标题
    lv_obj_t * title = lv_label_create(cont);
    lv_label_set_text(title, "日历");
    lv_obj_set_style_text_font(title, font_manager_get_font(FONT_SMALL), LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(0xC0C0C0), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // 创建日历控件
    lv_obj_t * calendar = lv_calendar_create(cont);
    lv_obj_set_size(calendar, 330, 290);
    lv_obj_align_to(calendar, cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(calendar, lv_color_black(), 0);
    // 移除边框
    lv_obj_set_style_border_width(calendar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(calendar, 30, 0);
    lv_obj_set_style_text_color(calendar, lv_color_hex(0xf0f0f0), 0);

    static const char *day_names[] =
    {
        "日", "一", "二", "三", "四", "五", "六"
    };

    lv_obj_set_style_text_font(calendar, font_manager_get_font(FONT_SMALL), LV_PART_MAIN);
    lv_calendar_set_day_names(calendar, day_names);

    // 创建日历头部
    lv_obj_t * header = lv_calendar_header_arrow_create(calendar);
    // 为头部设置包含箭头符号的字体
    lv_obj_set_style_text_font(header, font_manager_get_font(FONT_SMALL), 0);

    // 设置当前日期
    set_now_day(calendar);
    // 添加事件监听
    // lv_obj_add_event_cb(header, calendar_event_handler, LV_EVENT_CLICKED, NULL);
    // lv_obj_send_event(header, LV_EVENT_CLICKED, NULL);

    //启用拖拽功能以支持手势识别
    // lv_obj_add_flag(calendar, LV_OBJ_FLAG_EVENT_BUBBLE);
    // lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_scroll_snap_x(cont, LV_SCROLL_SNAP_NONE);

    // lv_obj_set_scroll_dir(cont, LV_DIR_HOR);

    // 添加手势事件处理器
    // lv_obj_add_event_cb(cont, cont_right_scroll_event_cb, LV_EVENT_ALL, cont);

    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    // 添加手势事件处理
    lv_obj_add_event_cb(cont, calendar_gesture_event_cb, LV_EVENT_GESTURE, cont);
    lv_obj_remove_flag(cont, LV_OBJ_FLAG_GESTURE_BUBBLE);

    LV_LOG_USER("create_calendar end");
}

int calendar_main(void)
{
    LV_LOG_USER("calendar_main start");

    // 创建日历
    create_calendar();

    return 0;
}