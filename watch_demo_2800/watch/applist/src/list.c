#include "../include/list.h"

#include "../../calendar/include/calendar_main.h"
#include "../../stopwatch/include/stopwatch.h"
#include "../../clock/include/alarm_main.h"
#include "../../nav_page/include/nav_page_manager.h"
#include "../../weather/include/weather_app.h"
#include "../../utils/include/circle_screen.h"
#include "../../quicks/include/quicks_main.h"
#include "../../quick_balloon/include/quicks_balloon.h"
#include "../../music_demo/include/musicdemo.h"

/****************************************************************************
 * 全局变量
 ****************************************************************************/
static lv_obj_t *launcher_screen = NULL;
static lv_obj_t *circle_container = NULL;  // 添加对圆形容器的引用

static int gesture_start_x = 0;
static int gesture_start_y = 0;
static bool gesture_in_progress = false;
static const int gesture_threshold = 50; // 滑动阈值

static const struct {
    const char *sym;
    uint32_t flag;
    const char *name;
} app_list[TOTAL_APP] = {
    {LV_SYMBOL_DIRECTORY,       FILE_APP,           "游戏3"},
    {LV_SYMBOL_ENVELOPE,        CALENDAR_APP,       "日历"},
    {LV_SYMBOL_BARS,            STOPWATCH_APP,      "秒表"},
    {LV_SYMBOL_BATTERY_3,       BATTERY_APP,        "电池"},
    {LV_SYMBOL_SETTINGS,        SETTINGS_APP,       "设置"},
    {LV_SYMBOL_BLUETOOTH,       BLUETOOTH_APP,      "蓝牙"},
    {LV_SYMBOL_WIFI,            WIFI_APP,           "无线"},
    {LV_SYMBOL_VIDEO,           CAMERA_APP,         "相机"},
    {LV_SYMBOL_EYE_OPEN,        MAP_APP,            "地图"},
    {LV_SYMBOL_AUDIO,           MUSIC_APP,          "音乐"},
    {LV_SYMBOL_BELL,            CLOCK_APP,          "闹钟"},
    {LV_SYMBOL_SHUFFLE,         GAMES_ONE_APP,      "游戏1"},
    {LV_SYMBOL_DRIVE,           GAMES_TWO_APP,      "游戏2"},
    {LV_SYMBOL_TINT,            WEATHER_APP,        "天气"},
    {LV_SYMBOL_LOOP,            SPORT_APP,          "运动"},
    {LV_SYMBOL_POWER,           HEARTRATE_APP,      "心率"},
    {LV_SYMBOL_WARNING,         TIMER_APP,          "计时器"},
};

/****************************************************************************
 * 手势事件处理
 ****************************************************************************/

// 隐藏页面（直接隐藏）
void applist_hide_page(void)
{
    if (circle_container) {
        // 直接隐藏对象
        lv_obj_add_flag(circle_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_delete(circle_container);
        circle_container = NULL;
        LV_LOG_ERROR("=== Page hide by left swipe ===");
    }
}

// 显示页面（直接显示）
static void show_page(void)
{
    if (circle_container) {
        // 直接显示对象
        lv_obj_clear_flag(circle_container, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_ERROR("=== Page shown by left swipe ===");
    }
}

//手势事件处理函数
static void gesture_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_indev_t *indev = lv_event_get_indev(e);
    
    if (!indev) return;
    
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
                    hide_page();
                }
                
                gesture_in_progress = false;
            }
            break;
        default:
            break;
    }
}

// 优化事件注册
static void add_gesture_events(lv_obj_t *obj) {}

static void app_long_click_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
}

static void app_click_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    uint32_t index = lv_event_get_user_data(e);
    LV_LOG_USER("app_click_event_cb index %d \n", index);
    switch (index)
    {
        case FILE_APP:
            //quick_air_page();
            break;
        case TIMER_APP:
            LV_LOG_USER(" go to timer APP \n");
            timer_app_start();
            break;
        case CLOCK_APP:
            LV_LOG_USER("go to clock APP \n");
            clock_main_start();
            break;
        case SETTINGS_APP:
            settings_app_start();
            break;
        case STOPWATCH_APP:
            stopwatch_start();
            break;
        case WEATHER_APP:
            weather_app_start();
            break;
        case MUSIC_APP:
            music_app();
            break;
        case CALENDAR_APP:
            calendar_main();
            break;
        case GAMES_ONE_APP:
            //create_main_page();
            break;
        case GAMES_TWO_APP:
            //balloon_page();
            break;
        case AI_APP:
            break;
        case HOME_APP:
            break;
        default:
            LV_LOG_USER("The icon you clicked is undefined\n");
            // lv_obj_t *mbox1 = lv_msgbox_create(lv_screen_active());
            // lv_msgbox_add_title(mbox1, "Error");
            // lv_msgbox_add_text(mbox1, "Application didn't exit！!!!!");
            // lv_msgbox_add_close_button(mbox1);
            // lv_obj_center(mbox1);
            break;
    }
}

static lv_obj_t *create_app_item(lv_obj_t *parent, int idx, int x, int y) {
    // 应用容器
    lv_obj_t *app = lv_obj_create(parent);
    lv_obj_set_size(app, 80, 80);
    lv_obj_set_pos(app, x, y);
    lv_obj_set_style_radius(app, 20, 0);
    
    // 设置背景颜色
    lv_color_t colors[] = {
        lv_color_hex(0x3498db), // 蓝色
        lv_color_hex(0xe74c3c), // 红色
        lv_color_hex(0x2ecc71), // 绿色
        lv_color_hex(0xf39c12), // 橙色
        lv_color_hex(0x9b59b6), // 紫色
        lv_color_hex(0x1abc9c), // 青色
        lv_color_hex(0x34495e), // 深蓝
        lv_color_hex(0xe67e22), // 胡萝卜色
        lv_color_hex(0x16a085), // 深青色
        lv_color_hex(0x8e44ad), // 深紫色
        lv_color_hex(0xf1c40f), // 向日葵黄
        lv_color_hex(0xe84393), // 粉红色
        lv_color_hex(0x00cec9), // 蓝绿色
        lv_color_hex(0xd63031), // 深红色
        lv_color_hex(0xfd79a8), // 浅粉色
        lv_color_hex(0x636e72), // 灰色
        lv_color_hex(0x6c5ce7), // 紫罗兰色
    };
    
    lv_obj_set_style_bg_color(app, colors[idx % 10], 0);
    lv_obj_set_style_bg_color(app, lv_color_lighten(colors[idx % 10], 50), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(app, 0, 0);
    lv_obj_set_style_shadow_width(app, 10, 0);
    lv_obj_set_style_shadow_opa(app, LV_OPA_40, 0);
    lv_obj_set_style_shadow_color(app, lv_color_hex(0x000000), 0);
    lv_obj_set_style_outline_width(app, 0, 0);
    
    // 存储应用索引
    // lv_obj_set_user_data(app, (void*)(intptr_t)idx);

    // 添加事件
    lv_obj_add_event_cb(app, app_click_event_cb, LV_EVENT_CLICKED, app_list[idx].flag);
    lv_obj_add_event_cb(app, app_long_click_event_cb, LV_EVENT_LONG_PRESSED, NULL);

    // 图标
    lv_obj_t *icon = lv_label_create(app);
    lv_label_set_text(icon, app_list[idx].sym);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xffffff), 0);
    //lv_obj_set_style_text_font(icon, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, 0); 

    // 应用名称 - 在应用图标的右侧
    lv_obj_t *name_label = lv_label_create(parent);
    lv_label_set_text(name_label, app_list[idx].name);
    lv_obj_set_style_text_color(name_label, lv_color_hex(0x3498db), 0);
    lv_obj_set_style_text_font(name_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_align(name_label, LV_TEXT_ALIGN_LEFT, 0);
    
    // 计算名称标签的位置（应用图标的右侧）
    int name_x = x + APP_SIZE + 8;
    int name_y = y + (APP_SIZE - 20) / 2;
    
    lv_obj_set_pos(name_label, name_x, name_y);

    return app;
}

lv_obj_t *applist_create_list(lv_obj_t *scr) {
    LV_LOG_ERROR("=== Creating launcher list ===");

    // 创建圆形表盘容器
    circle_container = lv_obj_create(scr);
    lv_obj_set_size(circle_container, LAUNCHER_SIZE, LAUNCHER_SIZE);
    lv_obj_set_style_bg_color(circle_container, lv_color_hex(0x000000), 0);
    lv_obj_clear_flag(circle_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(circle_container, RADIUS, 0);
    lv_obj_set_style_border_width(circle_container, 0, 0);
    lv_obj_set_style_clip_corner(circle_container, true, 0);
    lv_obj_set_pos(circle_container, 0, 0);
    lv_obj_set_style_pad_all(circle_container, 0, 0);

    // 计算布局参数
    int container_width = APP_SIZE * 3 + APP_GAP;
    int start_x = (LAUNCHER_SIZE - container_width) / 2 - 5;
    int start_y = 40;
    
    // 计算内容总高度
    int content_height = ROWS * (APP_SIZE + APP_GAP) + start_y;
    
    LV_LOG_ERROR("Settings Layout: rows=%d, width=%d, height=%d, start_x=%d, start_y=%d", 
                ROWS, container_width, content_height, start_x, start_y);

    // 创建滚动容器
     lv_obj_t *scroll_container = lv_obj_create(circle_container);
    lv_obj_set_size(scroll_container, LAUNCHER_SIZE, LAUNCHER_SIZE);
    lv_obj_set_pos(scroll_container, 0, 0);

    // 关键滚动设置
    lv_obj_add_flag(scroll_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_scroll_dir(scroll_container, LV_DIR_VER);
    lv_obj_set_scroll_snap_x(scroll_container, LV_SCROLL_SNAP_NONE);

    lv_obj_set_scrollbar_mode(scroll_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_opa(scroll_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_opa(scroll_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(scroll_container, 0, 0);
    // lv_obj_set_style_radius(scroll_container, RADIUS, 0);
    lv_obj_set_style_bg_color(scroll_container, lv_color_hex(0x001000), 0);
    // add_gesture_events(scroll_container);

    // 创建内容容器
    lv_obj_t *content_container = lv_obj_create(scroll_container);
    lv_obj_set_size(content_container, LAUNCHER_SIZE, content_height);
    lv_obj_set_pos(content_container, 0, 0);
    lv_obj_set_style_bg_opa(content_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(content_container, LV_OPA_TRANSP, 0);
    lv_obj_set_size(content_container, LAUNCHER_SIZE, content_height);
    lv_obj_set_scrollbar_mode(content_container, LV_SCROLLBAR_MODE_OFF);
    //lv_obj_set_style_radius(content_container, RADIUS, 0);
    lv_obj_set_style_bg_color(content_container, lv_color_hex(0xe67e22), 0);

    // 创建双列应用布局
    LV_LOG_ERROR("Creating %d apps in 2-column layout", TOTAL_APP);
    
    for (int i = 0; i < TOTAL_APP; i++) {
        int row = i / 2;
        int col = i % 2;

        int x = start_x + col * (APP_SIZE + 60);
        int y = start_y + row * (APP_SIZE + APP_GAP);

        LV_LOG_ERROR("App %d: row=%d, col=%d, pos=(%d, %d)", i, row, col, x, y);
        create_app_item(content_container, i, x, y);
    }

    // 7. 验证滚动设置
  lv_coord_t cont_height = lv_obj_get_height(content_container);
  lv_coord_t scroll_height = lv_obj_get_height(scroll_container);
  LV_LOG_ERROR("cont_height: %ld, scroll_height: %ld ", (long)cont_height, (long)scroll_height);
    
    // 8. 确保有足够的滚动空间
    if (cont_height <= scroll_height) {
        // lv_obj_set_height(content_container, scroll_height + LAUNCHER_SIZE);
        // LV_LOG_ERROR("Adjusted content height to %ld", (long)(scroll_height + 200));
    }

    return launcher_screen;
}