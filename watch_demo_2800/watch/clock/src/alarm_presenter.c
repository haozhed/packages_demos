#include <stdio.h>
#include "../include/alarm_presenter.h"
#include "../include/alarm_model.h"
#include "../include/alarm_view.h"

static void update_time_cb(lv_timer_t* timer) {
    alarm_presenter_update_time();
}

void alarm_presenter_init() {
    AlarmModel *model = get_alarm_model();
    alarm_model_init();
    LV_LOG_USER("1");
    alarm_view_init();
    alarm_view_restore_alarms_display();
    LV_LOG_USER("2");
    // 创建定时器
    lv_timer_create(update_time_cb, 60000, NULL);

    alarm_model_get_current_time();//newnew code
    alarm_presenter_update_time();//new code
    
    // 显示主屏幕
    alarm_view_show_main_screen();
    
    // while (1) {
    //     lv_timer_handler();
    //     usleep(1000);
    // }
    LV_LOG_USER("结束alarm_presenter_init\n");
}

void alarm_presenter_update_time() {
    alarm_model_get_current_time();
    alarm_view_update_time(get_alarm_model()->current_time);
}

void alarm_presenter_add_alarm(const char * hour, const char* minute) {
    LV_LOG_USER("进入alarm_presenter_add_alarm\n");
    if (!hour || !minute) {
        LV_LOG_USER("错误:hour或minute为NULL\n");
        return;
    }
    char time_str[16];
    snprintf(time_str, sizeof(time_str), "%.2s:%.2s", hour, minute);
    LV_LOG_USER("hour:%.2s,minute:%.2s\n", hour ? hour : "NULL", minute ? minute : "NULL");
    //snprintf(time_str, sizeof(time_str), "%s:%s", hour, minute);
    LV_LOG_USER("model添加时间:%s\n", time_str);
    bool success = alarm_model_add_alarm(time_str);
    LV_LOG_USER("model添加结果:%d\n", success);
    if (success == true) {
        LV_LOG_USER("model添加成功\n");
        AlarmModel* model = get_alarm_model(); 
        alarm_view_create_alarm(time_str, model->alarm_count -1);
        alarm_view_update_alarm_list();
        alarm_view_show_main_screen();
    }
}

void alarm_presenter_remove_alarm(int index) {
    if (alarm_model_remove_alarm(index)) {
        alarm_view_remove_alarm(index); 
        alarm_view_update_alarm_list(); 
    }
}

void alarm_presenter_remove_selected_alarms() {
    alarm_model_remove_selected_alarms(); 
    alarm_view_remove_selected_alarms(); 
    alarm_view_update_alarm_list(); 
    alarm_presenter_set_long_pressed(false);
}

void alarm_presenter_select_alarm(int index, bool selected) {
    alarm_model_select_alarm(index, selected);
    alarm_view_update_alarm_list();
}

void alarm_presenter_set_long_pressed(bool long_pressed) {
    get_alarm_model()->long_pressed = long_pressed;
    alarm_view_update_alarm_list();
}