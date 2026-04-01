#include "weather_app.h"

const uint16_t WATCH_HEIGHT = 455;
const uint16_t WATCH_WIDTH = 455;
const uint16_t CONT_HEIGHT = 455;

static lv_obj_t* test_btn = NULL;
static lv_obj_t* watch_scr = NULL;
static weather_ui_t weather_ui;
static weekly_weather_ui_t weekly_weather_ui;
static hourly_weather_ui_t hourly_weather_ui;
static weather_page_t current_weather_page = WEATHER_PAGE;
static bool is_animating = false;//滑动动画标志
static lv_anim_t slide_anim;//滑动动画

//计算圆形屏幕内最大正方形边长
static int get_max_square_side(int circle_diameter)
{
    return (int)(circle_diameter / sqrt(2));
}

//创建表盘页面容器
lv_obj_t* create_page_container(void)
{
    //确保表盘已创建
    if (watch_scr == NULL) {
        watch_scr = lv_obj_create(get_watch_scr());
        lv_obj_set_style_radius(watch_scr, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(watch_scr, 0, 0);
        lv_obj_set_size(watch_scr, LV_CIRCLE_WATCH, LV_CIRCLE_WATCH);
        lv_obj_set_style_bg_color(watch_scr, lv_color_black(), 0);
        lv_obj_set_scrollbar_mode(watch_scr, LV_SCROLLBAR_MODE_OFF);
    }
    
    //创建全屏页面容器，覆盖整个圆形区域
    lv_obj_t* page_cont = lv_obj_create(watch_scr);
    lv_obj_set_size(page_cont, WATCH_HEIGHT ,WATCH_WIDTH);
    lv_obj_set_style_bg_color(page_cont, lv_color_black(), 0);
    lv_obj_center(page_cont);
    
    // 设置页面容器样式
    // lv_obj_set_style_bg_opa(page_cont, LV_OPA_TRANSP, 0);//背景透明
    lv_obj_set_style_border_width(page_cont, 0, 0);//边框宽度
    lv_obj_set_style_border_opa(page_cont, LV_OPA_TRANSP, 0);//边框透明
    lv_obj_set_style_outline_opa(page_cont, LV_OPA_TRANSP, 0);//轮廓透明
    lv_obj_set_style_radius(page_cont, LV_RADIUS_CIRCLE, 0);//背景样式
    
    // 关键：彻底禁用所有滚动和拖动
    lv_obj_set_scrollbar_mode(page_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(page_cont, LV_DIR_NONE);//禁用所有方向滚动
    lv_obj_clear_flag(page_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(page_cont, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(page_cont, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_clear_flag(page_cont, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_clear_flag(page_cont, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    return page_cont;
}

//动画完成回调
static void anim_completed_cb(lv_anim_t* anim)
{
    is_animating = false;
    
    //在动画完成后真正隐藏旧页面
    lv_obj_t* old_scr = anim->user_data;
    if(old_scr)
    {
        lv_obj_add_flag(old_scr, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_x(old_scr, 0);
    }
}

//页面滑动动画
static void slide_anim_cb(void* obj, int32_t value)
{
    lv_obj_set_x(obj, value);
}

//切换动画处理
static void execute_page_animation(lv_obj_t* old_page, lv_obj_t* new_page, lv_dir_t direction)
{
    if(is_animating || !new_page)
        return;

    is_animating = true;
    
    int square_side = get_max_square_side(WATCH_HEIGHT);
    int start_x_new, start_x_old;
    int target_x = 0;
    
    //根据方向设置初始位置
    if(direction == LV_DIR_LEFT)
    {
        start_x_new = square_side;
        start_x_old = 0;
    }
    else
    {
        start_x_new = -square_side;
        start_x_old = 0;
    }
    
    //设置新页面初始位置并显示
    lv_obj_set_x(new_page, start_x_new);
    lv_obj_clear_flag(new_page, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(new_page);//确保新页面在最前面
    
    //新页面动画
    lv_anim_init(&slide_anim);
    lv_anim_set_var(&slide_anim, new_page);
    lv_anim_set_exec_cb(&slide_anim, slide_anim_cb);
    lv_anim_set_values(&slide_anim, start_x_new, target_x);
    lv_anim_set_time(&slide_anim, 350);
    lv_anim_set_path_cb(&slide_anim, lv_anim_path_ease_out);
    lv_anim_start(&slide_anim);
    
    // 旧页面动画
    if(old_page && lv_obj_is_valid(old_page)) {
        int target_x_old = (direction == LV_DIR_LEFT) ? -square_side : square_side;
        
        lv_anim_t old_anim;
        lv_anim_init(&old_anim);
        lv_anim_set_var(&old_anim, old_page);
        lv_anim_set_exec_cb(&old_anim, slide_anim_cb);
        lv_anim_set_values(&old_anim, start_x_old, target_x_old);
        lv_anim_set_time(&old_anim, 350);
        lv_anim_set_path_cb(&old_anim, lv_anim_path_ease_out);
        lv_anim_set_completed_cb(&old_anim, anim_completed_cb);
        lv_anim_set_user_data(&old_anim, old_page);
        lv_anim_start(&old_anim);
    }
    else
    {
        is_animating = false;
    }
}

//切换到指定页面
static void switch_weather_page(weather_page_t target_page)
{
    if(is_animating)
        return;
    
    lv_obj_t* old_page = NULL;
    lv_obj_t* new_page = NULL;
    lv_dir_t direction = LV_DIR_LEFT;
    
    //获取当前页面
    switch(current_weather_page)
    {
        case WEATHER_PAGE: 
            old_page = weather_ui.page_cont;
            break;
        case WEATHER_PAGE_HOURLY:
            old_page = hourly_weather_ui.page_cont;
            break;
        case WEATHER_PAGE_WEEKLY: 
            old_page = weekly_weather_ui.page_cont;
            break;
    }
    
    //获取目标页面
    switch(target_page)
    {
        case WEATHER_PAGE: 
            new_page = weather_ui.page_cont;
            if(current_weather_page == WEATHER_PAGE_WEEKLY || current_weather_page == WEATHER_PAGE_HOURLY) {
                direction = LV_DIR_RIGHT;
            }
            break;
        case WEATHER_PAGE_HOURLY:
            new_page = hourly_weather_ui.page_cont;
            if(current_weather_page == WEATHER_PAGE_WEEKLY) {
                direction = LV_DIR_RIGHT;
            }
            break;
        case WEATHER_PAGE_WEEKLY:
            new_page = weekly_weather_ui.page_cont;
            break;
    }
    
    if(old_page == new_page)
        return;
    
    current_weather_page = target_page;
    execute_page_animation(old_page, new_page, direction);
}

// static void exit_weather_app(void)
// {
//     if(weather_ui.page_cont && lv_obj_is_valid(weather_ui.page_cont))
//     {
//         lv_obj_add_flag(weather_ui.page_cont, LV_OBJ_FLAG_HIDDEN);
//     }
//     if(hourly_weather_ui.page_cont && lv_obj_is_valid(hourly_weather_ui.page_cont))
//     {
//         lv_obj_add_flag(hourly_weather_ui.page_cont, LV_OBJ_FLAG_HIDDEN);
//     }
//     if(weekly_weather_ui.page_cont && lv_obj_is_valid(weekly_weather_ui.page_cont))
//     {
//         lv_obj_add_flag(weekly_weather_ui.page_cont, LV_OBJ_FLAG_HIDDEN);
//     }

//     main_page("digital_dial"); 
// }

//页面切换回调
static void weather_page_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_GESTURE && !is_animating)
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        
        switch(dir) {              
            case LV_DIR_LEFT:
                switch(current_weather_page) {
                    case WEATHER_PAGE:
                        switch_weather_page(WEATHER_PAGE_HOURLY);
                        break;
                    case WEATHER_PAGE_HOURLY:
                        switch_weather_page(WEATHER_PAGE_WEEKLY);
                        break;
                    case WEATHER_PAGE_WEEKLY:
                        break;
                }
                break;
            case LV_DIR_RIGHT:
                switch(current_weather_page)
                {
                    case WEATHER_PAGE_WEEKLY:
                        switch_weather_page(WEATHER_PAGE_HOURLY);
                        break;
                    case WEATHER_PAGE_HOURLY:
                        switch_weather_page(WEATHER_PAGE);
                        break;
                    case WEATHER_PAGE:
                        // exit_weather_app();
                        weather_app_delete();
                        break;//退出天气应用
                }
                break;                
            default:
                break;
        }
    }
}
static void get_date_string(int offset, char* date_str, int size)
{
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    
    timeinfo->tm_mday += offset;
    mktime(timeinfo);
    strftime(date_str, size, "%m/%d", timeinfo);
}

static void get_hour_string(int hour_offset, char* hour_str, int size)
{
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
 
    timeinfo->tm_hour += hour_offset;
    mktime(timeinfo);
    strftime(hour_str, size, "%H:00", timeinfo);
}

//JSON解析函数
static bool parse_weather_json(const char* json_str, weather_data_t* weather_data)
{
    cJSON* root = cJSON_Parse(json_str);
    if (!root) 
        return false;
    
    //状态码验证
    cJSON* code = cJSON_GetObjectItem(root, "code");
    if (!code || code->valuestring == NULL || strcmp(code->valuestring, "200") != 0)
    {
        cJSON_Delete(root);
        return false;
    }
    
    //解析更新时间
    cJSON* updateTime = cJSON_GetObjectItem(root, "updateTime");
    if (updateTime && updateTime->valuestring) {
        strncpy(weather_data->updateTime, updateTime->valuestring, 
                sizeof(weather_data->updateTime) - 1);
    }
    
    // 解析当前天气数据
    cJSON* now = cJSON_GetObjectItem(root, "now");
    if (now) 
    {
        cJSON* temp = cJSON_GetObjectItem(now, "temp");
        if (temp && temp->valuestring)
        {
            strncpy(weather_data->temp, temp->valuestring, sizeof(weather_data->temp) - 1);
        }
        
        cJSON* feelsLike = cJSON_GetObjectItem(now, "feelsLike");
        if (feelsLike && feelsLike->valuestring)
        {
            strncpy(weather_data->feelsLike, feelsLike->valuestring, sizeof(weather_data->feelsLike) - 1);
        }
        
        cJSON* icon = cJSON_GetObjectItem(now, "icon");
        if (icon && icon->valuestring)
        {
            strncpy(weather_data->icon, icon->valuestring, sizeof(weather_data->icon) - 1);
        }
        
        cJSON* text = cJSON_GetObjectItem(now, "text");
        if (text && text->valuestring)
        {
            strncpy(weather_data->text, text->valuestring, sizeof(weather_data->text) - 1);
        }
        
        cJSON* windDir = cJSON_GetObjectItem(now, "windDir");
        if (windDir && windDir->valuestring)
        {
            strncpy(weather_data->windDir, windDir->valuestring, sizeof(weather_data->windDir) - 1);
        }

        cJSON* windScale = cJSON_GetObjectItem(now, "windScale");
        if (windScale && windScale->valuestring)
        {
            strncpy(weather_data->windScale, windScale->valuestring, sizeof(weather_data->windScale) - 1);
        }
        
        cJSON* humidity = cJSON_GetObjectItem(now, "humidity");
        if (humidity && humidity->valuestring)
        {
            strncpy(weather_data->humidity, humidity->valuestring, sizeof(weather_data->humidity) - 1);
        }
    }
    
    cJSON_Delete(root);
    return true;
}

// 解析24小时天气JSON数据
static bool parse_hourly_weather_json(const char* json_str, hourly_weather_data_t* hourly_data)
{
    cJSON* root = cJSON_Parse(json_str);
    if (!root) 
        return false;
    
    // 状态码验证
    cJSON* code = cJSON_GetObjectItem(root, "code");
    if (!code || code->valuestring == NULL || strcmp(code->valuestring, "200") != 0)
    {
        cJSON_Delete(root);
        return false;
    }
    
    //解析hourly数组
    cJSON* hourly = cJSON_GetObjectItem(root, "hourly");
    if (cJSON_IsArray(hourly)) 
    {
        int index = 0;
        cJSON* item = NULL;
        
        cJSON_ArrayForEach(item, hourly) 
        {     
            //解析时间
            cJSON* fxTime = cJSON_GetObjectItem(item, "fxTime");
            if (fxTime && fxTime->valuestring)
            {
                strncpy(hourly_data[index].updateTime, fxTime->valuestring, sizeof(hourly_data[index].updateTime) - 1);
            }
            //解析温度
            cJSON* temp = cJSON_GetObjectItem(item, "temp");
            if (temp && temp->valuestring)
            {
                strncpy(hourly_data[index].temp, temp->valuestring, sizeof(hourly_data[index].temp) - 1);
            }
            //解析天气图标
            cJSON* icon = cJSON_GetObjectItem(item, "icon");
            if (icon && icon->valuestring)
            {
                strncpy(hourly_data[index].icon, icon->valuestring, sizeof(hourly_data[index].icon) - 1);
            }
            //解析天气状况
            cJSON* text = cJSON_GetObjectItem(item, "text");
            if (text && text->valuestring) {
                strncpy(hourly_data[index].text, text->valuestring, sizeof(hourly_data[index].text) - 1);
            }         
            index++;
        }
    }
    cJSON_Delete(root);
    return true;
}

//解析7天天气JSON数据
static bool parse_daily_weather_json(const char* json_str, weekly_weather_data_t* weekly_data)
{
    cJSON* root = cJSON_Parse(json_str);
    if (!root)
        return false;
    
    //状态码验证
    cJSON* code = cJSON_GetObjectItem(root, "code");
    if (!code || code->valuestring == NULL || strcmp(code->valuestring, "200") != 0)
    {
        cJSON_Delete(root);
        return false;
    }
    
    // 解析daily数组
    cJSON* daily = cJSON_GetObjectItem(root, "daily");
    if (cJSON_IsArray(daily)) 
    {
        int index = 0;
        cJSON* item = NULL;
        
        cJSON_ArrayForEach(item, daily) 
        {
            //解析日期
            cJSON* fxDate = cJSON_GetObjectItem(item, "fxDate");
            if (fxDate && fxDate->valuestring)
            {
                strncpy(weekly_data[index].date, fxDate->valuestring, sizeof(weekly_data[index].date) - 1);
            }
            //解析最高温度
            cJSON* tempMax = cJSON_GetObjectItem(item, "tempMax");
            if (tempMax && tempMax->valuestring)
            {
                strncpy(weekly_data[index].tempMax, tempMax->valuestring, sizeof(weekly_data[index].tempMax) - 1);
            }
            //解析最低温度
            cJSON* tempMin = cJSON_GetObjectItem(item, "tempMin");
            if (tempMin && tempMin->valuestring)
            {
                strncpy(weekly_data[index].tempMin, tempMin->valuestring, sizeof(weekly_data[index].tempMin) - 1);
            }
            //解析天气图标
            cJSON* iconDay = cJSON_GetObjectItem(item, "iconDay");
            if (iconDay && iconDay->valuestring)
            {
                strncpy(weekly_data[index].icon, iconDay->valuestring, sizeof(weekly_data[index].icon) - 1);
            }
            //解析天气状况
            cJSON* textDay = cJSON_GetObjectItem(item, "textDay");
            if (textDay && textDay->valuestring)
            {
                strncpy(weekly_data[index].text, textDay->valuestring, sizeof(weekly_data[index].text) - 1);
            }
            //解析风向
            cJSON* windDirDay = cJSON_GetObjectItem(item, "windDirDay");
            if (windDirDay && windDirDay->valuestring)
            {
                strncpy(weekly_data[index].windDir, windDirDay->valuestring, sizeof(weekly_data[index].windDir) - 1);
            }
            //解析风力
            cJSON* windScaleDay = cJSON_GetObjectItem(item, "windScaleDay");
            if (windScaleDay && windScaleDay->valuestring)
            {
                strncpy(weekly_data[index].windScale, windScaleDay->valuestring, sizeof(weekly_data[index].windScale) - 1);
            }
            index++;
        }
    }
    cJSON_Delete(root);
    return true;
}

static const lv_img_dsc_t* get_weather_icon(const char* icon_code)
{
    if (!icon_code) 
        return &sunny;
    
    int code = atoi(icon_code);

    switch (code) {
        case 100:
        case 150:
            return &sunny;
        case 101:
        case 104:
            return &cloudy;
        case 300:
            return &rainy;
        case 400:
            return &snowy;
        case 500:
            return &foggy;
        default:
            return &sunny;
    }
}

static const lv_img_dsc_t* get_weather_icon_small(const char* icon_code)
{
    if (!icon_code) 
        return &sunny32;
    
    int code = atoi(icon_code);

    switch (code) {
        case 100:
        case 150:
            return &sunny32;
        case 101:
        case 104:
            return &cloudy32;
        case 300:
            return &rainy32;
        case 400:
            return &snowy32;
        case 500:
            return &foggy32;
        default:
            return &sunny32;
    }
}

//更新天气页面
void update_weather_page(const weather_data_t* data)
{  
    //更新温度
    char temp_str[64];
    snprintf(temp_str, sizeof(temp_str), "%s度", data->temp);
    lv_label_set_text(weather_ui.temp_label, temp_str);   
    //更新天气状况
    lv_label_set_text(weather_ui.weather_label, data->text);
    //更新天气图标
    lv_img_set_src(weather_ui.weather_icon, get_weather_icon(data->icon));
    //更新湿度
    snprintf(temp_str, sizeof(temp_str), "湿度: %s%%", data->humidity);
    lv_label_set_text(weather_ui.humidity_label, temp_str);
    //更新风向风力
    snprintf(temp_str, sizeof(temp_str), "%s %s级", data->windDir, data->windScale);
    lv_label_set_text(weather_ui.wind_label, temp_str);

    if (weather_ui.text_span && weather_ui.temp_span)
    {
        lv_span_set_text(weather_ui.text_span, "体感温度:");
        snprintf(temp_str, sizeof(temp_str), "%s度", data->feelsLike);
        lv_span_set_text(weather_ui.temp_span, temp_str);
        lv_spangroup_refr_mode(weather_ui.spangroup);
    }
    //更新时间
    char update_str[50];
    snprintf(update_str, sizeof(update_str), "更新于 %s", data->updateTime);
    lv_label_set_text(weather_ui.net_label, update_str);
}

//更新24小时天气页面
void update_hourly_weather_page(const hourly_weather_data_t* data)
{
    char temp_str[20];
    for(int i = 0; i < 24; i++)
    {
        if (strlen(data[i].updateTime) >= 16)
        {
            snprintf(temp_str, sizeof(temp_str), "%.5s", data[i].updateTime + 11);
        }
        else
        {
            strcpy(temp_str, "--:--");
        }
        lv_label_set_text(hourly_weather_ui.weather_item[i].time_label, temp_str);
        snprintf(temp_str, sizeof(temp_str), "%s度", data[i].temp);
        lv_label_set_text(hourly_weather_ui.weather_item[i].temp_label, temp_str);
        lv_img_set_src(hourly_weather_ui.weather_item[i].weather_icon, get_weather_icon_small(data[i].icon));
    }  
}

// 更新7天天气页面
void update_weekly_weather_page(const weekly_weather_data_t* data)
{
    char temp_str[32];
    for(int i = 0; i < 7; i++)
    {
        if (strlen(data[i].date) >= 10)
        {
            //显示月/日格式
            snprintf(temp_str, sizeof(temp_str), "%.2s/%.2s", data[i].date + 5, data[i].date + 8);
            lv_label_set_text(weekly_weather_ui.weather_item[i].time_label, temp_str);
        }
        else
        {
            lv_label_set_text(weekly_weather_ui.weather_item[i].time_label, "--/--");
        }
        
        snprintf(temp_str, sizeof(temp_str), "%s度 / %s度", data[i].tempMax, data[i].tempMin);
        lv_label_set_text(weekly_weather_ui.weather_item[i].temp_label, temp_str);
        lv_img_set_src(weekly_weather_ui.weather_item[i].weather_icon, get_weather_icon_small(data[i].icon));
        lv_label_set_text(weekly_weather_ui.weather_item[i].text_label, data[i].text);
        snprintf(temp_str, sizeof(temp_str), "%s%s级", data[i].windDir, data[i].windScale);
        lv_label_set_text(weekly_weather_ui.weather_item[i].wind_label, temp_str);
    }
}

bool update_weather_from_json(const char* json_str1,const char* json_str2,const char* json_str3)
{
    weather_data_t weather_data;
    hourly_weather_data_t hourly_data[24];
    weekly_weather_data_t weekly_data[7]; 

    memset(&weather_data, 0, sizeof(weather_data));
    memset(hourly_data, 0, sizeof(hourly_data));
    memset(weekly_data, 0, sizeof(weekly_data));

    if (!parse_weather_json(json_str1, &weather_data)) 
    {
        return false;
    }

    if (!parse_hourly_weather_json(json_str2, hourly_data)) 
    {
        return false;
    }

    if (!parse_daily_weather_json(json_str3, weekly_data)) {
        return false;
    }

    memcpy(&weather_ui.weather_data, &weather_data, sizeof(weather_data_t));
    memcpy(hourly_weather_ui.hourly_data, hourly_data, sizeof(hourly_weather_data_t) * 24);
    memcpy(weekly_weather_ui.weekly_data, weekly_data, sizeof(weekly_weather_data_t) * 7);
    update_weather_page(&weather_ui.weather_data);
    update_hourly_weather_page(hourly_weather_ui.hourly_data);
    update_weekly_weather_page(weekly_weather_ui.weekly_data);
    return true;
}

void test_weather_update(void)
{
    const char* test_json1 = "{\"code\":\"200\",\"updateTime\":\"2025-11-10T15:50+08:00\",\"fxLink\":\"https://www.qweather.com/weather/beijing-101010100.html\",\"now\":{\"obsTime\":\"2025-11-07T16:32+08:00\",\"temp\":\"9\",\"feelsLike\":\"8\",\"icon\":\"300\",\"text\":\"小雨\",\"wind360\":\"0\",\"windDir\":\"北风\",\"windScale\":\"2\",\"windSpeed\":\"7\",\"humidity\":\"92\",\"precip\":\"0.0\",\"pressure\":\"1023\",\"vis\":\"3\",\"cloud\":\"100\",\"dew\":\"6\"},\"refer\":{\"sources\":[\"QWeather\"],\"license\":[\"QWeather Developers License\"]}}";
    const char* test_json2 = "{\"code\":\"200\",\"updateTime\":\"2025-11-10T15:50+08:00\",\"fxLink\":\"https://www.qweather.com/weather/beijing-101010100.html\",\"hourly\":[{\"fxTime\":\"2025-11-10T16:00+08:00\",\"temp\":\"13\",\"icon\":\"100\",\"text\":\"晴\",\"wind360\":\"198\",\"windDir\":\"南风\",\"windScale\":\"1-3\",\"windSpeed\":\"13\",\"humidity\":\"32\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1015\",\"cloud\":\"100\",\"dew\":\"-2\"},{\"fxTime\":\"2025-11-10T17:00+08:00\",\"temp\":\"13\",\"icon\":\"100\",\"text\":\"晴\",\"wind360\":\"201\",\"windDir\":\"南风\",\"windScale\":\"1-3\",\"windSpeed\":\"13\",\"humidity\":\"38\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1015\",\"cloud\":\"100\",\"dew\":\"-1\"},{\"fxTime\":\"2025-11-10T18:00+08:00\",\"temp\":\"12\",\"icon\":\"150\",\"text\":\"晴\",\"wind360\":\"207\",\"windDir\":\"西南风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"44\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1015\",\"cloud\":\"96\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-10T19:00+08:00\",\"temp\":\"10\",\"icon\":\"150\",\"text\":\"晴\",\"wind360\":\"218\",\"windDir\":\"西南风\",\"windScale\":\"1-3\",\"windSpeed\":\"9\",\"humidity\":\"43\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1015\",\"cloud\":\"83\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-10T20:00+08:00\",\"temp\":\"9\",\"icon\":\"150\",\"text\":\"晴\",\"wind360\":\"236\",\"windDir\":\"西南风\",\"windScale\":\"1-3\",\"windSpeed\":\"7\",\"humidity\":\"48\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1015\",\"cloud\":\"75\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-10T21:00+08:00\",\"temp\":\"7\",\"icon\":\"150\",\"text\":\"晴\",\"wind360\":\"265\",\"windDir\":\"西风\",\"windScale\":\"1-3\",\"windSpeed\":\"7\",\"humidity\":\"51\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1016\",\"cloud\":\"54\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-10T22:00+08:00\",\"temp\":\"6\",\"icon\":\"150\",\"text\":\"晴\",\"wind360\":\"321\",\"windDir\":\"西北风\",\"windScale\":\"1-3\",\"windSpeed\":\"7\",\"humidity\":\"55\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1016\",\"cloud\":\"32\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-10T23:00+08:00\",\"temp\":\"4\",\"icon\":\"150\",\"text\":\"晴\",\"wind360\":\"357\",\"windDir\":\"北风\",\"windScale\":\"1-3\",\"windSpeed\":\"9\",\"humidity\":\"57\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1017\",\"cloud\":\"10\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-11T00:00+08:00\",\"temp\":\"3\",\"icon\":\"150\",\"text\":\"晴\",\"wind360\":\"13\",\"windDir\":\"北风\",\"windScale\":\"1-3\",\"windSpeed\":\"9\",\"humidity\":\"57\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1017\",\"cloud\":\"10\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-11T01:00+08:00\",\"temp\":\"3\",\"icon\":\"150\",\"text\":\"晴\",\"wind360\":\"23\",\"windDir\":\"东北风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"59\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1018\",\"cloud\":\"9\",\"dew\":\"-1\"},{\"fxTime\":\"2025-11-11T02:00+08:00\",\"temp\":\"3\",\"icon\":\"150\",\"text\":\"晴\",\"wind360\":\"28\",\"windDir\":\"东北风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"61\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1018\",\"cloud\":\"8\",\"dew\":\"-1\"},{\"fxTime\":\"2025-11-11T03:00+08:00\",\"temp\":\"2\",\"icon\":\"150\",\"text\":\"晴\",\"wind360\":\"26\",\"windDir\":\"东北风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"65\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1018\",\"cloud\":\"6\",\"dew\":\"-1\"},{\"fxTime\":\"2025-11-11T04:00+08:00\",\"temp\":\"2\",\"icon\":\"150\",\"text\":\"晴\",\"wind360\":\"19\",\"windDir\":\"北风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"70\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1017\",\"cloud\":\"3\",\"dew\":\"-1\"},{\"fxTime\":\"2025-11-11T05:00+08:00\",\"temp\":\"2\",\"icon\":\"150\",\"text\":\"晴\",\"wind360\":\"13\",\"windDir\":\"北风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"74\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1017\",\"cloud\":\"0\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-11T06:00+08:00\",\"temp\":\"2\",\"icon\":\"150\",\"text\":\"晴\",\"wind360\":\"7\",\"windDir\":\"北风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"79\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1016\",\"cloud\":\"0\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-11T07:00+08:00\",\"temp\":\"2\",\"icon\":\"100\",\"text\":\"晴\",\"wind360\":\"1\",\"windDir\":\"北风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"86\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1016\",\"cloud\":\"0\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-11T08:00+08:00\",\"temp\":\"3\",\"icon\":\"100\",\"text\":\"晴\",\"wind360\":\"360\",\"windDir\":\"北风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"72\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1016\",\"cloud\":\"0\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-11T09:00+08:00\",\"temp\":\"7\",\"icon\":\"100\",\"text\":\"晴\",\"wind360\":\"4\",\"windDir\":\"北风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"58\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1016\",\"cloud\":\"0\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-11T10:00+08:00\",\"temp\":\"7\",\"icon\":\"100\",\"text\":\"晴\",\"wind360\":\"14\",\"windDir\":\"北风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"50\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1017\",\"cloud\":\"0\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-11T11:00+08:00\",\"temp\":\"11\",\"icon\":\"100\",\"text\":\"晴\",\"wind360\":\"23\",\"windDir\":\"东北风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"44\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1017\",\"cloud\":\"0\",\"dew\":\"0\"},{\"fxTime\":\"2025-11-11T12:00+08:00\",\"temp\":\"12\",\"icon\":\"100\",\"text\":\"晴\",\"wind360\":\"46\",\"windDir\":\"东北风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"39\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1017\",\"cloud\":\"0\",\"dew\":\"-1\"},{\"fxTime\":\"2025-11-11T13:00+08:00\",\"temp\":\"13\",\"icon\":\"100\",\"text\":\"晴\",\"wind360\":\"111\",\"windDir\":\"东风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"34\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1017\",\"cloud\":\"0\",\"dew\":\"-1\"},{\"fxTime\":\"2025-11-11T14:00+08:00\",\"temp\":\"13\",\"icon\":\"100\",\"text\":\"晴\",\"wind360\":\"136\",\"windDir\":\"东南风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"33\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1017\",\"cloud\":\"0\",\"dew\":\"-1\"},{\"fxTime\":\"2025-11-11T15:00+08:00\",\"temp\":\"13\",\"icon\":\"100\",\"text\":\"晴\",\"wind360\":\"140\",\"windDir\":\"东南风\",\"windScale\":\"1-3\",\"windSpeed\":\"11\",\"humidity\":\"31\",\"pop\":\"0\",\"precip\":\"0.0\",\"pressure\":\"1018\",\"cloud\":\"0\",\"dew\":\"-1\"}],\"refer\":{\"sources\":[\"QWeather\"],\"license\":[\"QWeather Developers License\"]}}";
     const char* test_json3 = "{\"code\": \"200\", \"updateTime\": \"2025-11-10T17:51+08:00\", \"fxLink\": \"https://www.qweather.com/weather/beijing-101010100.html\", \"daily\": [{\"fxDate\": \"2025-11-10\", \"sunrise\": \"06:54\", \"sunset\": \"17:04\", \"moonrise\": \"21:13\", \"moonset\": \"12:06\", \"moonPhase\": \"亏凸月\", \"moonPhaseIcon\": \"805\", \"tempMax\": \"15\", \"tempMin\": \"1\", \"iconDay\": \"100\", \"textDay\": \"晴\", \"iconNight\": \"150\", \"textNight\": \"晴\", \"wind360Day\": \"225\", \"windDirDay\": \"西南风\", \"windScaleDay\": \"1-3\", \"windSpeedDay\": \"3\", \"wind360Night\": \"0\", \"windDirNight\": \"北风\", \"windScaleNight\": \"1-3\", \"windSpeedNight\": \"3\", \"humidity\": \"44\", \"precip\": \"0.0\", \"pressure\": \"1016\", \"vis\": \"25\", \"cloud\": \"0\", \"uvIndex\": \"2\"}, {\"fxDate\": \"2025-11-11\", \"sunrise\": \"06:55\", \"sunset\": \"17:03\", \"moonrise\": \"22:28\", \"moonset\": \"12:48\", \"moonPhase\": \"亏凸月\", \"moonPhaseIcon\": \"805\", \"tempMax\": \"13\", \"tempMin\": \"1\", \"iconDay\": \"100\", \"textDay\": \"晴\", \"iconNight\": \"151\", \"textNight\": \"多云\", \"wind360Day\": \"90\", \"windDirDay\": \"东风\", \"windScaleDay\": \"1-3\", \"windSpeedDay\": \"3\", \"wind360Night\": \"0\", \"windDirNight\": \"北风\", \"windScaleNight\": \"1-3\", \"windSpeedNight\": \"3\", \"humidity\": \"54\", \"precip\": \"0.0\", \"pressure\": \"1016\", \"vis\": \"25\", \"cloud\": \"0\", \"uvIndex\": \"3\"}, {\"fxDate\": \"2025-11-12\", \"sunrise\": \"06:56\", \"sunset\": \"17:02\", \"moonrise\": \"23:41\", \"moonset\": \"13:20\", \"moonPhase\": \"下弦月\", \"moonPhaseIcon\": \"806\", \"tempMax\": \"14\", \"tempMin\": \"2\", \"iconDay\": \"100\", \"textDay\": \"晴\", \"iconNight\": \"150\", \"textNight\": \"晴\", \"wind360Day\": \"270\", \"windDirDay\": \"西风\", \"windScaleDay\": \"1-3\", \"windSpeedDay\": \"3\", \"wind360Night\": \"0\", \"windDirNight\": \"北风\", \"windScaleNight\": \"1-3\", \"windSpeedNight\": \"3\", \"humidity\": \"32\", \"precip\": \"0.0\", \"pressure\": \"1020\", \"vis\": \"25\", \"cloud\": \"0\", \"uvIndex\": \"3\"}, {\"fxDate\": \"2025-11-13\", \"sunrise\": \"06:57\", \"sunset\": \"17:02\", \"moonrise\": \"\", \"moonset\": \"13:47\", \"moonPhase\": \"残月\", \"moonPhaseIcon\": \"807\", \"tempMax\": \"11\", \"tempMin\": \"1\", \"iconDay\": \"100\", \"textDay\": \"晴\", \"iconNight\": \"150\", \"textNight\": \"晴\", \"wind360Day\": \"225\", \"windDirDay\": \"西南风\", \"windScaleDay\": \"1-3\", \"windSpeedDay\": \"3\", \"wind360Night\": \"270\", \"windDirNight\": \"西风\", \"windScaleNight\": \"1-3\", \"windSpeedNight\": \"3\", \"humidity\": \"44\", \"precip\": \"0.0\", \"pressure\": \"1017\", \"vis\": \"25\", \"cloud\": \"0\", \"uvIndex\": \"3\"}, {\"fxDate\": \"2025-11-14\", \"sunrise\": \"06:59\", \"sunset\": \"17:01\", \"moonrise\": \"00:49\", \"moonset\": \"14:10\", \"moonPhase\": \"残月\", \"moonPhaseIcon\": \"807\", \"tempMax\": \"11\", \"tempMin\": \"2\", \"iconDay\": \"100\", \"textDay\": \"晴\", \"iconNight\": \"150\", \"textNight\": \"晴\", \"wind360Day\": \"180\", \"windDirDay\": \"南风\", \"windScaleDay\": \"1-3\", \"windSpeedDay\": \"3\", \"wind360Night\": \"0\", \"windDirNight\": \"北风\", \"windScaleNight\": \"1-3\", \"windSpeedNight\": \"3\", \"humidity\": \"34\", \"precip\": \"0.0\", \"pressure\": \"1013\", \"vis\": \"25\", \"cloud\": \"0\", \"uvIndex\": \"3\"}, {\"fxDate\": \"2025-11-15\", \"sunrise\": \"07:00\", \"sunset\": \"17:00\", \"moonrise\": \"01:55\", \"moonset\": \"14:31\", \"moonPhase\": \"残月\", \"moonPhaseIcon\": \"807\", \"tempMax\": \"13\", \"tempMin\": \"4\", \"iconDay\": \"100\", \"textDay\": \"晴\", \"iconNight\": \"151\", \"textNight\": \"多云\", \"wind360Day\": \"225\", \"windDirDay\": \"西南风\", \"windScaleDay\": \"1-3\", \"windSpeedDay\": \"3\", \"wind360Night\": \"315\", \"windDirNight\": \"西北风\", \"windScaleNight\": \"1-3\", \"windSpeedNight\": \"16\", \"humidity\": \"16\", \"precip\": \"0.0\", \"pressure\": \"1022\", \"vis\": \"25\", \"cloud\": \"0\", \"uvIndex\": \"3\"}, {\"fxDate\": \"2025-11-16\", \"sunrise\": \"07:01\", \"sunset\": \"16:59\", \"moonrise\": \"02:57\", \"moonset\": \"14:51\", \"moonPhase\": \"残月\", \"moonPhaseIcon\": \"807\", \"tempMax\": \"7\", \"tempMin\": \"0\", \"iconDay\": \"100\", \"textDay\": \"晴\", \"iconNight\": \"150\", \"textNight\": \"晴\", \"wind360Day\": \"315\", \"windDirDay\": \"西北风\", \"windScaleDay\": \"1-3\", \"windSpeedDay\": \"16\", \"wind360Night\": \"315\", \"windDirNight\": \"西北风\", \"windScaleNight\": \"1-3\", \"windSpeedNight\": \"3\", \"humidity\": \"17\", \"precip\": \"0.0\", \"pressure\": \"1027\", \"vis\": \"25\", \"cloud\": \"0\", \"uvIndex\": \"3\"}], \"refer\": {\"sources\": [\"QWeather\"], \"license\": [\"QWeather Developers License\"]}}";
    update_weather_from_json(test_json1,test_json2,test_json3);
}
static void add_weather_content(void)
{
    char date_str[10];
    int square_side = get_max_square_side(WATCH_HEIGHT);

    //添加15天天气
    for(int i = 0; i < 7; i++)
    {
        weekly_weather_ui.weather_item[i].item_cont = lv_obj_create(weekly_weather_ui.content_cont);
        lv_obj_set_size(weekly_weather_ui.weather_item[i].item_cont, square_side, square_side / 3);
        lv_obj_align(weekly_weather_ui.weather_item[i].item_cont, LV_ALIGN_TOP_MID, 0, i * (square_side / 3));
        
        //项目样式
        lv_obj_set_style_bg_color(weekly_weather_ui.weather_item[i].item_cont, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(weekly_weather_ui.weather_item[i].item_cont, LV_OPA_COVER, 0);
        
        //设置底部边框
        lv_obj_set_style_border_color(weekly_weather_ui.weather_item[i].item_cont, lv_color_hex(0x555555), 0);
        lv_obj_set_style_border_width(weekly_weather_ui.weather_item[i].item_cont, 2, 0);
        lv_obj_set_style_border_side(weekly_weather_ui.weather_item[i].item_cont, LV_BORDER_SIDE_BOTTOM, 0);
        
        //日期标签
        get_date_string(i, date_str, sizeof(date_str));
        weekly_weather_ui.weather_item[i].time_label = lv_label_create(weekly_weather_ui.weather_item[i].item_cont);
        lv_label_set_text(weekly_weather_ui.weather_item[i].time_label, date_str);
        lv_obj_set_style_text_font(weekly_weather_ui.weather_item[i].time_label, font_manager_get_font(FONT_SMALL), 0);
        lv_obj_set_style_text_color(weekly_weather_ui.weather_item[i].time_label, lv_color_white(), 0);
        lv_obj_align(weekly_weather_ui.weather_item[i].time_label, LV_ALIGN_LEFT_MID, 10, 0);
        
        //温度信息
        weekly_weather_ui.weather_item[i].temp_label = lv_label_create(weekly_weather_ui.weather_item[i].item_cont);
        lv_label_set_text_fmt(weekly_weather_ui.weather_item[i].temp_label, "%d度 / %d度", 25 + i, 18 + i);
        lv_obj_set_style_text_font(weekly_weather_ui.weather_item[i].temp_label, font_manager_get_font(FONT_SMALL), 0);
        lv_obj_set_style_text_color(weekly_weather_ui.weather_item[i].temp_label, lv_color_hex(0xCCCCCC), 0);
        lv_obj_align(weekly_weather_ui.weather_item[i].temp_label, LV_ALIGN_RIGHT_MID, -10, -15);

        weekly_weather_ui.weather_item[i].weather_icon = lv_img_create(weekly_weather_ui.weather_item[i].item_cont);
        lv_img_set_src(weekly_weather_ui.weather_item[i].weather_icon, &rainy32);
        lv_obj_align(weekly_weather_ui.weather_item[i].weather_icon, LV_ALIGN_LEFT_MID, 80, 0);

        //天气状况文字
        weekly_weather_ui.weather_item[i].text_label = lv_label_create(weekly_weather_ui.weather_item[i].item_cont);
        lv_label_set_text(weekly_weather_ui.weather_item[i].text_label, "雨");
        lv_obj_set_style_text_font(weekly_weather_ui.weather_item[i].text_label, font_manager_get_font(FONT_SMALL), 0);
        lv_obj_set_style_text_color(weekly_weather_ui.weather_item[i].text_label, lv_color_hex(0xCCCCCC), 0);
        lv_obj_align(weekly_weather_ui.weather_item[i].text_label, LV_ALIGN_CENTER, 10, 0);

        //风力风向信息
        weekly_weather_ui.weather_item[i].wind_label = lv_label_create(weekly_weather_ui.weather_item[i].item_cont);
        lv_label_set_text(weekly_weather_ui.weather_item[i].wind_label, "北风 3级");
        lv_obj_set_style_text_font(weekly_weather_ui.weather_item[i].wind_label, font_manager_get_font(FONT_SMALL), 0);
        lv_obj_set_style_text_color(weekly_weather_ui.weather_item[i].wind_label, lv_color_hex(0xAAAAAA), 0);
        lv_obj_align(weekly_weather_ui.weather_item[i].wind_label, LV_ALIGN_RIGHT_MID, -10, 15);
        
    }
}

static void add_hourly_weather_content(void)
{
    char hour_str[10];
    int square_side = get_max_square_side(WATCH_HEIGHT);

    //添加24小时天气
    for(int i = 0; i < 24; i++)
    {
        hourly_weather_ui.weather_item[i].item_cont = lv_obj_create(hourly_weather_ui.content_cont);
        lv_obj_set_size(hourly_weather_ui.weather_item[i].item_cont, square_side, square_side / 4);
        lv_obj_align(hourly_weather_ui.weather_item[i].item_cont, LV_ALIGN_TOP_MID, 0, i * (square_side / 4));
        
        //项目样式
        lv_obj_set_style_bg_color(hourly_weather_ui.weather_item[i].item_cont, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(hourly_weather_ui.weather_item[i].item_cont, LV_OPA_COVER, 0);
        //设置底部边框
        lv_obj_set_style_border_color(hourly_weather_ui.weather_item[i].item_cont, lv_color_hex(0x555555), 0);
        lv_obj_set_style_border_width(hourly_weather_ui.weather_item[i].item_cont, 2, 0);
        lv_obj_set_style_border_side(hourly_weather_ui.weather_item[i].item_cont, LV_BORDER_SIDE_BOTTOM, 0);
        
        //时间标签
        get_hour_string(i, hour_str, sizeof(hour_str));
        hourly_weather_ui.weather_item[i].time_label = lv_label_create(hourly_weather_ui.weather_item[i].item_cont);
        lv_label_set_text(hourly_weather_ui.weather_item[i].time_label, hour_str);
        lv_obj_set_style_text_font(hourly_weather_ui.weather_item[i].time_label, font_manager_get_font(FONT_SMALL), 0);
        lv_obj_set_style_text_color(hourly_weather_ui.weather_item[i].time_label, lv_color_white(), 0);
        lv_obj_align(hourly_weather_ui.weather_item[i].time_label, LV_ALIGN_LEFT_MID, 10, 0);
        
        //温度信息
        hourly_weather_ui.weather_item[i].temp_label = lv_label_create(hourly_weather_ui.weather_item[i].item_cont);
        lv_label_set_text_fmt(hourly_weather_ui.weather_item[i].temp_label, "%d度", 20 + (i % 5));
        lv_obj_set_style_text_font(hourly_weather_ui.weather_item[i].temp_label, font_manager_get_font(FONT_SMALL), 0);
        lv_obj_set_style_text_color(hourly_weather_ui.weather_item[i].temp_label, lv_color_hex(0xCCCCCC), 0);
        lv_obj_align(hourly_weather_ui.weather_item[i].temp_label, LV_ALIGN_RIGHT_MID, -10, 0);

        //天气图标
        hourly_weather_ui.weather_item[i].weather_icon = lv_img_create(hourly_weather_ui.weather_item[i].item_cont);
        lv_img_set_src(hourly_weather_ui.weather_item[i].weather_icon, &rainy32);
        lv_obj_align(hourly_weather_ui.weather_item[i].weather_icon, LV_ALIGN_CENTER, -20, 0);

        //天气状况标签
        hourly_weather_ui.weather_item[i].text_label = lv_label_create(hourly_weather_ui.weather_item[i].item_cont);
        lv_label_set_text(hourly_weather_ui.weather_item[i].text_label, "雨");
        lv_obj_set_style_text_font(hourly_weather_ui.weather_item[i].text_label, font_manager_get_font(FONT_SMALL), 0);
        lv_obj_set_style_text_color(hourly_weather_ui.weather_item[i].text_label, lv_color_hex(0xAAAAAA), 0);
        lv_obj_align(hourly_weather_ui.weather_item[i].text_label, LV_ALIGN_CENTER, 50, 0);
    }
}

// 创建7天滚动屏幕
static void create_weekly_scrollable_content(void)
{
    int square_side = get_max_square_side(WATCH_HEIGHT);
    int center_offset = square_side / 6;

    //顶部标签
    weekly_weather_ui.top_label = lv_label_create(weekly_weather_ui.page_cont);
    lv_label_set_text(weekly_weather_ui.top_label, "7日天气");
    lv_obj_set_style_text_font( weekly_weather_ui.top_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_color( weekly_weather_ui.top_label, lv_color_white(), 0);
    lv_obj_align(weekly_weather_ui.top_label, LV_ALIGN_TOP_MID, 0, (0.5 * center_offset));
    //位置标签
    weekly_weather_ui.location_label = lv_label_create(weekly_weather_ui.page_cont);
    lv_label_set_text(weekly_weather_ui.location_label, "北京市");
    lv_obj_set_style_text_font(weekly_weather_ui.location_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_color(weekly_weather_ui.location_label, lv_color_white(), 0);
    lv_obj_align(weekly_weather_ui.location_label, LV_ALIGN_BOTTOM_MID, 0, -(0.5 *center_offset));
    //创建滚动容器
    weekly_weather_ui.scroll_cont = lv_obj_create(weekly_weather_ui.page_cont);
    lv_obj_set_size(weekly_weather_ui.scroll_cont, square_side, square_side);
    lv_obj_align(weekly_weather_ui.scroll_cont, LV_ALIGN_CENTER, 0, 0);
    
    //设置滚动属性 - 关键修改：完全禁用水平滚动
    lv_obj_set_scroll_dir(weekly_weather_ui.scroll_cont, LV_DIR_VER); // 只允许垂直滚动
    lv_obj_set_scrollbar_mode(weekly_weather_ui.scroll_cont, LV_SCROLLBAR_MODE_OFF); // 隐藏滚动条
    //完全禁用水平滚动能力
    lv_obj_clear_flag(weekly_weather_ui.scroll_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(weekly_weather_ui.scroll_cont, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(weekly_weather_ui.scroll_cont, LV_OBJ_FLAG_SCROLL_MOMENTUM);

    //设置样式为完全透明
    lv_obj_set_style_bg_opa(weekly_weather_ui.scroll_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(weekly_weather_ui.scroll_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_outline_opa(weekly_weather_ui.scroll_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_opa(weekly_weather_ui.scroll_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(weekly_weather_ui.scroll_cont, 0, 0);

    weekly_weather_ui.content_cont = lv_obj_create(weekly_weather_ui.scroll_cont);
    lv_obj_set_size(weekly_weather_ui.content_cont, square_side, square_side);
    lv_obj_align(weekly_weather_ui.content_cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(weekly_weather_ui.content_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(weekly_weather_ui.content_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_scroll_dir(weekly_weather_ui.content_cont, LV_DIR_VER); //只允许垂直滚动
    lv_obj_set_scrollbar_mode(weekly_weather_ui.content_cont, LV_SCROLLBAR_MODE_OFF); //隐藏滚动条
    
}

//创建24小时滚动容器
static void create_hourly_scrollable_content(void)
{
    int square_side = get_max_square_side(WATCH_HEIGHT);
    int center_offset = square_side / 6;

    //顶部标签
    hourly_weather_ui.top_label = lv_label_create(hourly_weather_ui.page_cont);
    lv_label_set_text(hourly_weather_ui.top_label, "24时天气");
    lv_obj_set_style_text_font(hourly_weather_ui.top_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_color(hourly_weather_ui.top_label, lv_color_white(), 0);
    lv_obj_align(hourly_weather_ui.top_label, LV_ALIGN_TOP_MID, 0, (0.5 * center_offset));
    //位置标签
    hourly_weather_ui.location_label = lv_label_create(hourly_weather_ui.page_cont);
    lv_label_set_text(hourly_weather_ui.location_label, "北京市");
    lv_obj_set_style_text_font(hourly_weather_ui.location_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_color(hourly_weather_ui.location_label, lv_color_white(), 0);
    lv_obj_align(hourly_weather_ui.location_label, LV_ALIGN_BOTTOM_MID, 0, -(0.5 *center_offset));
    
    //创建滚动容器
    hourly_weather_ui.scroll_cont = lv_obj_create(hourly_weather_ui.page_cont);
    lv_obj_set_size(hourly_weather_ui.scroll_cont, square_side, square_side);
    lv_obj_align(hourly_weather_ui.scroll_cont, LV_ALIGN_CENTER, 0, 0);
    
    //设置滚动属性
    lv_obj_set_scroll_dir(hourly_weather_ui.scroll_cont, LV_DIR_VER);//只允许垂直滚动
    lv_obj_set_scrollbar_mode(hourly_weather_ui.scroll_cont, LV_SCROLLBAR_MODE_OFF);//隐藏滚动条
    lv_obj_clear_flag(hourly_weather_ui.scroll_cont, LV_OBJ_FLAG_SCROLLABLE);//清除可滚动标志
    lv_obj_add_flag(hourly_weather_ui.scroll_cont, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC);//添加可滚动和弹性滚动标志
    lv_obj_clear_flag(hourly_weather_ui.scroll_cont, LV_OBJ_FLAG_SCROLL_MOMENTUM);//清除动量滚动

    //设置样式为完全透明
    lv_obj_set_style_bg_opa(hourly_weather_ui.scroll_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(hourly_weather_ui.scroll_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_outline_opa(hourly_weather_ui.scroll_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_opa(hourly_weather_ui.scroll_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(hourly_weather_ui.scroll_cont, 0, 0);//内边距为0

    //创建内容容器
    hourly_weather_ui.content_cont = lv_obj_create(hourly_weather_ui.scroll_cont);
    lv_obj_set_size(hourly_weather_ui.content_cont, square_side, square_side);
    lv_obj_align(hourly_weather_ui.content_cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(hourly_weather_ui.content_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(hourly_weather_ui.content_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_scroll_dir(hourly_weather_ui.content_cont, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(hourly_weather_ui.content_cont, LV_SCROLLBAR_MODE_OFF);
}

static void test_btn_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_CLICKED)
    {
        test_weather_update(); 
    }
}
//创建天气页面内容
static void create_weather_content(void)
{
    int square_side = get_max_square_side(WATCH_HEIGHT);
    int center_offset = square_side / 6;

    //顶部标签
    weather_ui.top_label = lv_label_create(weather_ui.page_cont);
    lv_label_set_text(weather_ui.top_label, "天气");
    lv_obj_set_style_text_font( weather_ui.top_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_color( weather_ui.top_label, lv_color_white(), 0);
    lv_obj_align(weather_ui.top_label, LV_ALIGN_TOP_MID, 0, 0.5 * center_offset);

    //位置标签
    weather_ui.location_label = lv_label_create(weather_ui.page_cont);
    lv_label_set_text(weather_ui.location_label, "北京市");
    lv_obj_set_style_text_font(weather_ui.location_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_color(weather_ui.location_label, lv_color_white(), 0);
    lv_obj_align(weather_ui.location_label, LV_ALIGN_BOTTOM_MID, 0, -(0.5 *center_offset));

    //联网标签
    weather_ui.net_label = lv_label_create(weather_ui.page_cont);
    lv_label_set_text(weather_ui.net_label, "天气未更新 请检查手机网络设置");
    lv_obj_set_style_text_font(weather_ui.net_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_color(weather_ui.net_label, lv_color_white(), 0);
    lv_obj_align(weather_ui.net_label, LV_ALIGN_TOP_MID, 0, center_offset * 1.5);

    //温度标签
    weather_ui.temp_label = lv_label_create(weather_ui.page_cont);
    lv_label_set_text(weather_ui.temp_label, "25度");
    lv_obj_set_style_text_font(weather_ui.temp_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_color(weather_ui.temp_label, lv_color_white(), 0);
    lv_obj_align(weather_ui.temp_label, LV_ALIGN_CENTER, (1.5 * center_offset), -(0.5 * center_offset));

    //湿度标签
    weather_ui.humidity_label = lv_label_create(weather_ui.page_cont);
    lv_label_set_text(weather_ui.humidity_label, "湿度: 65%");
    lv_obj_set_style_text_font(weather_ui.humidity_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_color(weather_ui.humidity_label, lv_color_hex(0xCCCCCC), 0);
    lv_obj_align(weather_ui.humidity_label, LV_ALIGN_CENTER, (1.5 * center_offset), center_offset);

    //风向风力标签
    weather_ui.wind_label = lv_label_create(weather_ui.page_cont);
    lv_label_set_text(weather_ui.wind_label, "北风 3级");
    lv_obj_set_style_text_font(weather_ui.wind_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_color(weather_ui.wind_label, lv_color_hex(0xCCCCCC), 0);
    lv_obj_align(weather_ui.wind_label, LV_ALIGN_CENTER, (1.5 * center_offset), -center_offset);

    //天气状况标签
    weather_ui.weather_label = lv_label_create(weather_ui.page_cont);
    lv_label_set_text(weather_ui.weather_label, "晴");
    lv_obj_set_style_text_font(weather_ui.weather_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_color(weather_ui.weather_label, lv_color_white(), 0);
    lv_obj_align(weather_ui.weather_label, LV_ALIGN_CENTER, (1.5 * center_offset), (0.5 * center_offset));

    //体感标签部分
    weather_ui.spangroup = lv_spangroup_create(weather_ui.page_cont);
    lv_obj_set_style_text_font(weather_ui.spangroup, font_manager_get_font(FONT_SMALL), 0);
    //文字部分
    weather_ui.text_span = lv_spangroup_new_span(weather_ui.spangroup);
    lv_span_set_text(weather_ui.text_span, "体感温度:");
    lv_style_set_text_color(&weather_ui.text_span->style, lv_color_white());
    //温度部分
    weather_ui.temp_span = lv_spangroup_new_span(weather_ui.spangroup);
    lv_span_set_text(weather_ui.temp_span, "20度");
    lv_style_set_text_color(&weather_ui.temp_span->style, lv_color_hex(0x888888));
    lv_obj_align(weather_ui.spangroup, LV_ALIGN_BOTTOM_MID, 0, -(center_offset * 1.5));

    //添加测试按钮
    test_btn = lv_btn_create(weather_ui.page_cont);
    lv_obj_set_size(test_btn, 40, 30);
    lv_obj_align_to(test_btn, weather_ui.spangroup, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_set_style_bg_color(test_btn, lv_color_hex(0x444444), 0);
    lv_obj_set_style_radius(test_btn, 5, 0);

    lv_obj_t* btn_label = lv_label_create(test_btn);
    lv_label_set_text(btn_label, "刷新");
    lv_obj_set_style_text_font(btn_label, font_manager_get_font(FONT_SMALL), 0);
    lv_obj_set_style_text_color(btn_label, lv_color_white(), 0);
    lv_obj_center(btn_label);
    lv_obj_add_event_cb(test_btn, test_btn_event_cb, LV_EVENT_CLICKED, NULL);

    //天气图标
    weather_ui.weather_icon = lv_img_create(weather_ui.page_cont);
    lv_img_set_src(weather_ui.weather_icon, &sunny);
    lv_obj_align(weather_ui.weather_icon, LV_ALIGN_CENTER, -(1.5 * center_offset), 0);

    lv_obj_remove_flag(weather_ui.page_cont, LV_OBJ_FLAG_GESTURE_BUBBLE);//阻止将手势事件从对象传递给其父对象
    lv_obj_add_event_cb(weather_ui.page_cont, weather_page_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(weather_ui.page_cont, LV_OBJ_FLAG_HIDDEN);
    
}

void weekly_weather_page(void)
{
    //创建天气界面
    weekly_weather_ui.page_cont = create_page_container();
    //页面初始位置为0
    lv_obj_set_x(weekly_weather_ui.page_cont, 0);

    create_weekly_scrollable_content();
    add_weather_content();

    lv_obj_remove_flag(weekly_weather_ui.page_cont, LV_OBJ_FLAG_GESTURE_BUBBLE);//阻止将手势事件从对象传递给其父对象
    lv_obj_add_event_cb(weekly_weather_ui.page_cont, weather_page_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_flag(weekly_weather_ui.page_cont, LV_OBJ_FLAG_HIDDEN);
}
void hourly_weather_page(void)
{
    //创建天气界面
    hourly_weather_ui.page_cont = create_page_container();
    //页面初始位置为0
    lv_obj_set_x(hourly_weather_ui.page_cont, 0);

    //创建滚动容器和内容
    create_hourly_scrollable_content();
    add_hourly_weather_content();

    lv_obj_remove_flag(hourly_weather_ui.page_cont, LV_OBJ_FLAG_GESTURE_BUBBLE);//阻止将手势事件从对象传递给其父对象
    lv_obj_add_event_cb(hourly_weather_ui.page_cont, weather_page_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_flag(hourly_weather_ui.page_cont, LV_OBJ_FLAG_HIDDEN);
}

void weather_page(void)
{
    //创建天气界面
    weather_ui.page_cont = create_page_container();
    //页面初始位置为0
    lv_obj_set_x(weather_ui.page_cont, 0);
    create_weather_content();
    //设置当前屏幕
    current_weather_page = WEATHER_PAGE;
}

void weather_app_start(void)
{
    LV_LOG_USER("==================weather_app_start==================");
    weather_page();
    hourly_weather_page();
    weekly_weather_page();
}

void weather_app_delete(void)
{
    lv_obj_add_flag(weekly_weather_ui.page_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(hourly_weather_ui.page_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(weather_ui.page_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(watch_scr, LV_OBJ_FLAG_HIDDEN);

    lv_obj_delete(weekly_weather_ui.page_cont);
    lv_obj_delete(hourly_weather_ui.page_cont);
    lv_obj_delete(weather_ui.page_cont);
    lv_obj_delete(watch_scr);

    weekly_weather_ui.page_cont = NULL;
    hourly_weather_ui.page_cont = NULL;
    weather_ui.page_cont = NULL;
    watch_scr = NULL;
}