#ifndef NAV_PAGE_MANAGER_H
#define NAV_PAGE_MANAGER_H

#include "lvgl.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    LAUNCHER_APP = 0,
    HOME_APP,
    NOTIFICATION_APP,
    CONTROL_APP,
    CLOCK_APP,
    TIMER_APP,
    STOPWATCH_APP,
    WEATHER_APP,
    MUSIC_APP,
    CALENDAR_APP,
    SETTINGS_APP,
    GAMES_ONE_APP,
    GAMES_TWO_APP,
    FILE_APP,
    BATTERY_APP,
    MAP_APP,
    BLUETOOTH_APP,
    WIFI_APP,
    CAMERA_APP,
    SPORT_APP,
    HEARTRATE_APP,
    AI_APP,
    OTHER_APP,
};

// 页面状态
typedef enum {
    PAGE_STATE_UNLOADED = 0,  // 未加载
    PAGE_STATE_STANDBY,       // 待机状态
    PAGE_STATE_ACTIVE,        // 激活状态
    PAGE_STATE_BACKGROUND     // 后台状态
} page_state_t;

// 页面切换动画类型
typedef enum {
    PAGE_ANIM_NONE = 0,       // 无动画
    PAGE_ANIM_SLIDE_LEFT,     // 向左滑动
    PAGE_ANIM_SLIDE_RIGHT,    // 向右滑动
    PAGE_ANIM_FADE,           // 淡入淡出
    PAGE_ANIM_ZOOM,           // 缩放
    PAGE_ANIM_OVERLAY         // 覆盖（用于弹窗）
} page_animation_t;

// 页面照片信息
typedef struct {
    const void* data;         // 图片数据
    uint32_t size;           // 数据大小
    uint16_t width;          // 宽度
    uint16_t height;         // 高度
} page_photo_t;

// 应用信息结构体
typedef struct nav_app_t {
    const char* icon_url;     // 应用图标路径
    const char* package_name; // 应用包名
    uint32_t id;             // 应用ID
    const char* (*get_name)(void); // 获取应用名称函数
    
    // 内部使用
    bool is_loaded;          // 应用是否已加载
    struct nav_page_t* first_page; // 应用的首个页面
    struct nav_app_t* next;  // 下一个应用
} nav_app_t;

// 页面配置结构体
typedef struct nav_page_t {
    uint32_t app_id;         // 应用ID
    uint32_t page_id;        // 页面ID
    page_state_t state;      // 页面状态
    const page_photo_t* _photo_; // 页面照片/图标
    const char* uri;         // 页面URI标识
    
    // 生命周期回调
    lv_obj_t* (*on_create)(void);                    // 页面创建
    void (*on_destroy)(lv_obj_t* page);              // 页面销毁
    void (*on_resume)(lv_obj_t* page);               // 页面恢复
    void (*on_pause)(lv_obj_t* page);                // 页面暂停
    void (*on_backpressed)(lv_obj_t* page);          // 返回键处理
    bool (*on_keyevent)(lv_obj_t* page, uint32_t key); // 按键事件
    
    // 内部使用
    lv_obj_t* page_obj;      // 页面对象
    void* user_data;         // 用户数据
    struct nav_page_t* next; // 下一个页面（同一应用）
    nav_app_t* app;          // 所属应用
} nav_page_t;

// 页面历史记录
typedef struct page_history_node {
    uint32_t app_id;
    uint32_t page_id;
    struct page_history_node* prev;
    struct page_history_node* next;
} page_history_node_t;

// 页面管理器配置
typedef struct {
    uint32_t home_app_id;    // 主页应用ID
    uint32_t home_page_id;   // 主页页面ID
    lv_obj_t* container;     // 页面容器
    page_animation_t default_anim; // 默认动画
} nav_manager_config_t;

// ========== 页面管理器初始化 ==========
void nav_page_manager_init(const nav_manager_config_t* config);

// ========== 应用注册管理 ==========
bool nav_register_app(nav_app_t* app);
bool nav_unregister_app(uint32_t app_id);
nav_app_t* nav_get_app(uint32_t app_id);

// ========== 页面注册管理 ==========
bool nav_register_page(nav_page_t* page);
bool nav_unregister_page(uint32_t app_id, uint32_t page_id);
nav_page_t* nav_get_page(uint32_t app_id, uint32_t page_id);

// ========== 页面跳转功能 ==========
bool nav_switch_to_page(uint32_t app_id, uint32_t page_id, page_animation_t anim_type);
bool nav_switch_to_app(uint32_t app_id, page_animation_t anim_type);
bool nav_go_back(page_animation_t anim_type);
bool nav_go_home(page_animation_t anim_type);

// ========== 页面销毁功能 ==========
bool nav_destroy_page(uint32_t app_id, uint32_t page_id);
bool nav_destroy_app(uint32_t app_id);

// ========== 状态查询功能 ==========
nav_page_t* nav_get_current_page(void);
nav_app_t* nav_get_current_app(void);
page_state_t nav_get_page_state(uint32_t app_id, uint32_t page_id);
bool nav_is_app_loaded(uint32_t app_id);

// ========== 历史记录管理 ==========
void nav_clear_history(void);
int nav_get_history_count(void);

// ========== 事件处理 ==========
bool nav_handle_back_pressed(void);
bool nav_handle_key_event(uint32_t key);

// ========== 工具函数 ==========
void nav_set_default_animation(page_animation_t anim_type);
void nav_page_manager_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // NAV_PAGE_MANAGER_H
