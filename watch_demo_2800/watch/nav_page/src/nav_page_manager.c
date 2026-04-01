#include "/home/ts/openvela/vela-opensource/apps/examples/watch/nav_page/include/nav_page_manager.h"
#include <stdlib.h>
#include <string.h>

#define MAX_PAGES 100
#define MAX_APPS 50
#define MAX_HISTORY 30

// 页面管理器内部数据
static struct {
    lv_obj_t* container;           // 页面容器
    nav_app_t* app_list;           // 应用链表
    nav_page_t* page_list;         // 页面链表
    page_history_node_t* history_head; // 历史记录头
    page_history_node_t* history_tail; // 历史记录尾
    nav_page_t* current_page;      // 当前页面
    uint32_t home_app_id;          // 主页应用ID
    uint32_t home_page_id;         // 主页页面ID
    page_animation_t default_anim; // 默认动画
    bool initialized;              // 初始化标志
    int page_count;                // 页面数量
    int app_count;                 // 应用数量
    int history_count;             // 历史记录数量
} nav_mgr = {0};

// ========== 内部工具函数 ==========

// 查找应用
static nav_app_t* find_app(uint32_t app_id) {
    nav_app_t* app = nav_mgr.app_list;
    while (app) {
        if (app->id == app_id) {
            return app;
        }
        app = app->next;
    }
    return NULL;
}

// 查找页面
static nav_page_t* find_page(uint32_t app_id, uint32_t page_id) {
    nav_page_t* page = nav_mgr.page_list;
    while (page) {
        if (page->app_id == app_id && page->page_id == page_id) {
            return page;
        }
        page = page->next;
    }
    return NULL;
}

// 查找应用的第一个页面
static nav_page_t* find_app_first_page(uint32_t app_id) {
    nav_page_t* page = nav_mgr.page_list;
    while (page) {
        if (page->app_id == app_id) {
            return page;
        }
        page = page->next;
    }
    return NULL;
}

// 创建页面动画
static void create_page_animation(lv_obj_t* old_page, lv_obj_t* new_page, 
                                 page_animation_t anim_type, bool is_back) {
    if (anim_type == PAGE_ANIM_NONE) return;
    
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_time(&a, 300);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    
    switch (anim_type) {
        case PAGE_ANIM_SLIDE_LEFT:
        case PAGE_ANIM_SLIDE_RIGHT:
            if (old_page) {
                int32_t start_x = 0;
                int32_t end_x = is_back ? lv_obj_get_width(old_page) : -lv_obj_get_width(old_page);
                if (anim_type == PAGE_ANIM_SLIDE_RIGHT) {
                    end_x = -end_x;
                }
                
                lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
                lv_anim_set_var(&a, old_page);
                lv_anim_set_values(&a, start_x, end_x);
                lv_anim_start(&a);
            }
            
            if (new_page) {
                int32_t start_x = is_back ? -lv_obj_get_width(new_page) : lv_obj_get_width(new_page);
                int32_t end_x = 0;
                if (anim_type == PAGE_ANIM_SLIDE_RIGHT) {
                    start_x = -start_x;
                }
                
                lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
                lv_anim_set_var(&a, new_page);
                lv_anim_set_values(&a, start_x, end_x);
                lv_anim_start(&a);
            }
            break;
            
        case PAGE_ANIM_FADE:
            if (old_page) {
                lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
                lv_anim_set_var(&a, old_page);
                lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
                lv_anim_start(&a);
            }
            
            if (new_page) {
                lv_obj_set_style_opa(new_page, LV_OPA_TRANSP, 0);
                lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
                lv_anim_set_var(&a, new_page);
                lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
                lv_anim_start(&a);
            }
            break;
            
        default:
            break;
    }
}

// 添加到历史记录
static bool add_to_history(uint32_t app_id, uint32_t page_id) {
    if (!nav_mgr.history_head && app_id == nav_mgr.home_app_id && 
        page_id == nav_mgr.home_page_id) {
        // 主页不添加到历史记录
        return true;
    }
    
    if (nav_mgr.history_count >= MAX_HISTORY) {
        // 移除最旧的记录
        page_history_node_t* old_head = nav_mgr.history_head;
        nav_mgr.history_head = old_head->next;
        if (nav_mgr.history_head) {
            nav_mgr.history_head->prev = NULL;
        }
        free(old_head);
        nav_mgr.history_count--;
    }
    
    page_history_node_t* new_node = malloc(sizeof(page_history_node_t));
    if (!new_node) return false;
    
    new_node->app_id = app_id;
    new_node->page_id = page_id;
    new_node->next = NULL;
    new_node->prev = nav_mgr.history_tail;
    
    if (nav_mgr.history_tail) {
        nav_mgr.history_tail->next = new_node;
    }
    nav_mgr.history_tail = new_node;
    
    if (!nav_mgr.history_head) {
        nav_mgr.history_head = new_node;
    }
    
    nav_mgr.history_count++;
    return true;
}

// 从历史记录移除
static void remove_from_history(void) {
    if (!nav_mgr.history_tail) return;
    
    page_history_node_t* tail = nav_mgr.history_tail;
    nav_mgr.history_tail = tail->prev;
    
    if (nav_mgr.history_tail) {
        nav_mgr.history_tail->next = NULL;
    } else {
        nav_mgr.history_head = NULL;
    }
    
    free(tail);
    nav_mgr.history_count--;
}

// 隐藏当前页面
static void hide_current_page(void) {
    if (!nav_mgr.current_page || !nav_mgr.current_page->page_obj) return;
    
    // 触发暂停回调
    if (nav_mgr.current_page->on_pause) {
        nav_mgr.current_page->on_pause(nav_mgr.current_page->page_obj);
    }
    
    // 隐藏页面
    lv_obj_add_flag(nav_mgr.current_page->page_obj, LV_OBJ_FLAG_HIDDEN);
    nav_mgr.current_page->state = PAGE_STATE_BACKGROUND;
}

// 显示目标页面
static bool show_target_page(nav_page_t* target_page) {
    if (!target_page) return false;
    
    // 加载页面（如果未加载）
    if (target_page->state == PAGE_STATE_UNLOADED) {
        if (target_page->on_create) {
            target_page->page_obj = target_page->on_create();
            if (target_page->page_obj) {
                lv_obj_set_parent(target_page->page_obj, nav_mgr.container);
                lv_obj_set_size(target_page->page_obj, LV_PCT(100), LV_PCT(100));
                lv_obj_add_flag(target_page->page_obj, LV_OBJ_FLAG_HIDDEN);
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
    
    // 显示页面
    if (target_page->page_obj) {
        lv_obj_clear_flag(target_page->page_obj, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(target_page->page_obj);
        
        target_page->state = PAGE_STATE_ACTIVE;
        
        // 更新应用加载状态
        if (target_page->app) {
            target_page->app->is_loaded = true;
        }
        
        // 触发恢复回调
        if (target_page->on_resume) {
            target_page->on_resume(target_page->page_obj);
        }
        
        return true;
    }
    
    return false;
}

// ========== 公开接口实现 ==========

void nav_page_manager_init(const nav_manager_config_t* config) {
    if (nav_mgr.initialized) return;
    
    memset(&nav_mgr, 0, sizeof(nav_mgr));
    
    nav_mgr.container = config->container;
    nav_mgr.home_app_id = config->home_app_id;
    nav_mgr.home_page_id = config->home_page_id;
    nav_mgr.default_anim = config->default_anim;
    nav_mgr.initialized = true;
    
    // 设置容器
    if (nav_mgr.container) {
        lv_obj_set_size(nav_mgr.container, LV_PCT(100), LV_PCT(100));
        lv_obj_clear_flag(nav_mgr.container, LV_OBJ_FLAG_SCROLLABLE);
    }
}

bool nav_register_app(nav_app_t* app) {
    if (!nav_mgr.initialized || !app || nav_mgr.app_count >= MAX_APPS) {
        return false;
    }
    
    // 检查是否已注册
    if (find_app(app->id)) {
        return false;
    }
    
    // 添加到应用链表头部
    app->next = nav_mgr.app_list;
    nav_mgr.app_list = app;
    app->is_loaded = false;
    app->first_page = NULL;
    
    nav_mgr.app_count++;
    return true;
}

bool nav_register_page(nav_page_t* page) {
    if (!nav_mgr.initialized || !page || nav_mgr.page_count >= MAX_PAGES) {
        return false;
    }
    
    // 检查是否已注册
    if (find_page(page->app_id, page->page_id)) {
        return false;
    }
    
    // 查找所属应用
    nav_app_t* app = find_app(page->app_id);
    if (!app) {
        // 应用未注册，自动创建默认应用
        static nav_app_t default_app = {0};
        default_app.id = page->app_id;
        default_app.package_name = "default.app";
        default_app.icon_url = "";
        default_app.get_name = NULL;
        
        if (!nav_register_app(&default_app)) {
            return false;
        }
        app = &default_app;
    }
    
    // 设置应用关联
    page->app = app;
    
    // 如果是应用的第一个页面，设置应用的首页面
    if (!app->first_page) {
        app->first_page = page;
    }
    
    // 添加到页面链表头部
    page->next = nav_mgr.page_list;
    nav_mgr.page_list = page;
    
    // 初始化页面状态
    page->state = PAGE_STATE_UNLOADED;
    page->page_obj = NULL;
    page->user_data = NULL;
    
    nav_mgr.page_count++;
    return true;
}

bool nav_switch_to_page(uint32_t app_id, uint32_t page_id, page_animation_t anim_type) {
    if (!nav_mgr.initialized) return false;
    
    nav_page_t* target_page = find_page(app_id, page_id);
    if (!target_page) return false;
    
    nav_page_t* current_page = nav_mgr.current_page;
    
    // 隐藏当前页面
    hide_current_page();
    
    // 显示目标页面
    if (!show_target_page(target_page)) {
        return false;
    }
    
    // 添加到历史记录
    if (current_page && (current_page->app_id != app_id || current_page->page_id != page_id)) {
        add_to_history(current_page->app_id, current_page->page_id);
    }
    
    nav_mgr.current_page = target_page;
    
    // 执行动画
    create_page_animation(current_page ? current_page->page_obj : NULL,
                         target_page->page_obj,
                         anim_type != PAGE_ANIM_NONE ? anim_type : nav_mgr.default_anim,
                         false);
    
    return true;
}

bool nav_switch_to_app(uint32_t app_id, page_animation_t anim_type) {
    // 查找应用的第一个页面
    nav_app_t* app = find_app(app_id);
    if (!app || !app->first_page) return false;
    
    return nav_switch_to_page(app_id, app->first_page->page_id, anim_type);
}

bool nav_go_back(page_animation_t anim_type) {
    if (!nav_mgr.initialized || !nav_mgr.history_tail) {
        // 没有历史记录，返回主页
        return nav_go_home(anim_type);
    }
    
    page_history_node_t* prev_node = nav_mgr.history_tail;
    remove_from_history();
    
    return nav_switch_to_page(prev_node->app_id, prev_node->page_id, anim_type);
}

bool nav_go_home(page_animation_t anim_type) {
    return nav_switch_to_page(nav_mgr.home_app_id, nav_mgr.home_page_id, anim_type);
}

bool nav_destroy_page(uint32_t app_id, uint32_t page_id) {
    nav_page_t* page = find_page(app_id, page_id);
    if (!page || !page->page_obj) return false;
    
    // 如果正在显示该页面，先切换页面
    if (nav_mgr.current_page == page) {
        if (nav_mgr.history_tail) {
            nav_go_back(PAGE_ANIM_NONE);
        } else {
            // 没有历史记录，隐藏当前页面
            hide_current_page();
            nav_mgr.current_page = NULL;
        }
    }
    
    // 调用销毁回调
    if (page->on_destroy) {
        page->on_destroy(page->page_obj);
    } else {
        lv_obj_del(page->page_obj);
    }
    
    page->page_obj = NULL;
    page->state = PAGE_STATE_UNLOADED;
    page->user_data = NULL;
    
    return true;
}

bool nav_destroy_app(uint32_t app_id) {
    nav_app_t* app = find_app(app_id);
    if (!app) return false;
    
    bool success = true;
    nav_page_t* page = nav_mgr.page_list;
    
    // 销毁该应用的所有页面
    while (page) {
        if (page->app_id == app_id) {
            if (!nav_destroy_page(app_id, page->page_id)) {
                success = false;
            }
        }
        page = page->next;
    }
    
    app->is_loaded = false;
    return success;
}

nav_page_t* nav_get_current_page(void) {
    return nav_mgr.current_page;
}

nav_app_t* nav_get_current_app(void) {
    return nav_mgr.current_page ? find_app(nav_mgr.current_page->app_id) : NULL;
}

page_state_t nav_get_page_state(uint32_t app_id, uint32_t page_id) {
    nav_page_t* page = find_page(app_id, page_id);
    return page ? page->state : PAGE_STATE_UNLOADED;
}

bool nav_is_app_loaded(uint32_t app_id) {
    nav_app_t* app = find_app(app_id);
    return app ? app->is_loaded : false;
}

bool nav_handle_back_pressed(void) {
    if (!nav_mgr.current_page || !nav_mgr.current_page->page_obj) {
        return false;
    }
    
    // 先让当前页面处理返回键
    if (nav_mgr.current_page->on_backpressed) {
        nav_mgr.current_page->on_backpressed(nav_mgr.current_page->page_obj);
        return true;
    }
    
    // 默认行为：返回上一页
    return nav_go_back(nav_mgr.default_anim);
}

bool nav_handle_key_event(uint32_t key) {
    if (!nav_mgr.current_page || !nav_mgr.current_page->page_obj) {
        return false;
    }
    
    if (nav_mgr.current_page->on_keyevent) {
        return nav_mgr.current_page->on_keyevent(nav_mgr.current_page->page_obj, key);
    }
    
    return false;
}

void nav_clear_history(void) {
    page_history_node_t* current = nav_mgr.history_head;
    while (current) {
        page_history_node_t* next = current->next;
        free(current);
        current = next;
    }
    
    nav_mgr.history_head = NULL;
    nav_mgr.history_tail = NULL;
    nav_mgr.history_count = 0;
}

int nav_get_history_count(void) {
    return nav_mgr.history_count;
}

void nav_set_default_animation(page_animation_t anim_type) {
    nav_mgr.default_anim = anim_type;
}

void nav_page_manager_deinit(void) {
    if (!nav_mgr.initialized) return;
    
    // 销毁所有页面
    nav_page_t* page = nav_mgr.page_list;
    while (page) {
        if (page->page_obj) {
            if (page->on_destroy) {
                page->on_destroy(page->page_obj);
            } else {
                lv_obj_del(page->page_obj);
            }
        }
        page = page->next;
    }
    
    // 清空历史记录
    nav_clear_history();
    
    // 重置管理器
    memset(&nav_mgr, 0, sizeof(nav_mgr));
}
