#include <string.h>
#include <time.h>
#include <stdio.h>
#include "../include/alarm_model.h"
#include "../include/alarm_view.h"
#include "../include/alarm_presenter.h"


// static AlarmModel alarm_model;
static AlarmModel alarm_model = {0};

void alarm_model_init() {
    LV_LOG_USER("model 初始化");


    AlarmModel* model = get_alarm_model();
    // 不再每次都清零，只在第一次初始化时清零
    static bool initialized = false;
    if (!initialized) {
        memset(model, 0, sizeof(AlarmModel));
        initialized = true;
        LV_LOG_USER("首次初始化闹钟模型\n");
    }
    
    LV_LOG_USER("当前闹钟数量: %d\n", model->alarm_count);
    // alarm_model.alarm_count = 0;
    // alarm_model.long_pressed = false;
    // alarm_model.del_alarm_count = 0;
    // memset(alarm_model.current_time, 0, sizeof(alarm_model.current_time));
    LV_LOG_USER("model 初始化结束");
}

void alarm_model_get_current_time() {
    time_t rawtime = time(NULL);
    struct tm *timeinfo = localtime(&rawtime);
    strftime(alarm_model.current_time, sizeof(alarm_model.current_time), "%H:%M", timeinfo);
}

bool alarm_model_add_alarm(const char* time_str) {
    LV_LOG_USER("进入alarm_model_add_alarm\n");
    LV_LOG_USER("time_str: [%s] (长度: %zu)\n", time_str, time_str ? strlen(time_str) : 0);
    if (alarm_model.alarm_count >= MAX_ALARM_COUNT) return false;
    
    AlarmData* alarm = &alarm_model.alarms[alarm_model.alarm_count];
    
    strncpy(alarm->time, time_str, sizeof(alarm->time));
    LV_LOG_USER("添加闹钟：%s\n", alarm->time);
    strcpy(alarm->mode, "每天");
    LV_LOG_USER("添加模式：%s\n", alarm->mode);
    //alarm->enabled = true;
    alarm->selected = false;
    //printf("添加状态：%s\n", alarm->enabled ? "启用" : "禁用");
    LV_LOG_USER("selected: %d\n",alarm->selected);
    alarm_model.alarm_count++;
    LV_LOG_USER("闹钟数量：%d\n", alarm_model.alarm_count);
    return true;
}

bool alarm_model_remove_alarm(int index) {
    if (index < 0 || index >= MAX_ALARM_COUNT) return false;
    
    // 将后面的闹钟前移
    for (int i = index; i < MAX_ALARM_COUNT - 1; i++) {
        alarm_model.alarms[i] = alarm_model.alarms[i + 1];
    }
    
    alarm_model.alarm_count--;
    return true;
}

void alarm_model_select_alarm(int index, bool selected) {
    if (index < 0 || index >= MAX_ALARM_COUNT) return;
    
    if (selected && !alarm_model.alarms[index].selected) {
        alarm_model.del_alarm_count++;
    } else if (!selected && alarm_model.alarms[index].selected) {
        alarm_model.del_alarm_count--;
    }
    
    alarm_model.alarms[index].selected = selected;
}

void alarm_model_remove_selected_alarms() {

    for (int i = MAX_ALARM_COUNT - 1; i >= 0; i--) {
        if (alarm_model.alarms[i].selected) {
            alarm_model_remove_alarm(i);
        }
    }
    alarm_model.del_alarm_count = 0;
}

AlarmModel* get_alarm_model() {
    return &alarm_model;
}