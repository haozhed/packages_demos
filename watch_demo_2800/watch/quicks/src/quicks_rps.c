#include "../include/quicks_rps.h"
#include "../include/quicks_utils.h"

#include "../include/image.h"

#include <stdlib.h>
#include <time.h>

lv_obj_t * create_rps_page(lv_obj_t * parent);
void on_rps_destroy(lv_obj_t* page);              // 页面销毁
void on_rps_resume(lv_obj_t* page);               // 页面恢复
void on_rps_pause(lv_obj_t* page);                // 页面暂停

lv_obj_t *cen_img;
lv_obj_t *btn_img;
lv_timer_t * game_timer;

int anim_num=0;
int tag_num = 0;
bool btn_show=true;
bool actived = false;

lv_image_dsc_t const *get_img_tag0(void);

static void rps_timer(lv_timer_t * timer);
static void btn_click(lv_event_t *event);
static void handle_swipe_right(lv_event_t *event);
static lv_image_dsc_t const *get_anim_dy(int num);
static lv_image_dsc_t const *get_anim_tag(int num);

// 右滑返回上一层
static void on_rps_gesture_event_cb(lv_event_t *e)
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
            lv_timer_delete(game_timer);
            lv_obj_delete(obj);
            obj = NULL;
            break;
        }

    default:
        break;
    }
}

lv_obj_t * create_rps_page(lv_obj_t * parent){

    lv_obj_t *page = create_empty_page(parent);
    lv_obj_set_style_clip_corner(page,true,0);
    lv_obj_set_scrollbar_mode(page,LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(page,LV_DIR_NONE);
    lv_obj_set_style_bg_color(page,lv_color_hex(0xffffff),0);

    lv_obj_remove_flag(page,LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_event_cb(page,handle_swipe_right,LV_EVENT_GESTURE,NULL);

    lv_obj_t * bg_img = lv_image_create(page);
    lv_image_set_src(bg_img,get_img_bg());
    lv_obj_center(bg_img);
    lv_obj_set_scroll_dir(bg_img,LV_DIR_NONE);

    cen_img = lv_image_create(page);
    lv_image_set_src(cen_img,get_img_dy0());
    lv_obj_center(cen_img);

    lv_obj_t * title_img = lv_image_create(page);
    lv_image_set_src(title_img,get_img_title());
    lv_obj_align(title_img,LV_ALIGN_TOP_MID,0,40);

    btn_img = lv_image_create(page);
    lv_image_set_src(btn_img,get_img_start());
    lv_obj_align(btn_img,LV_ALIGN_BOTTOM_MID,0,40);
    lv_obj_add_flag(btn_img,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn_img,btn_click,LV_EVENT_CLICKED,NULL);

    game_timer = lv_timer_create(rps_timer,200,NULL);

    // 添加手势事件处理
    lv_obj_add_event_cb(page, on_rps_gesture_event_cb, LV_EVENT_GESTURE, parent);
    lv_obj_remove_flag(page, LV_OBJ_FLAG_GESTURE_BUBBLE);

    return page;
}

void on_rps_pause(lv_obj_t* page){
    if (!actived)
    {
        lv_timer_pause(game_timer);
    }else{
        if (tag_num>0)
        {
            lv_timer_pause(game_timer);
        }else{
            lv_timer_pause(game_timer);
        }
        
    }
}



void on_rps_resume(lv_obj_t* page){
    if (!actived)
    {
        lv_timer_resume(game_timer);
    }else{
        if (tag_num>0)
        {
            lv_timer_resume(game_timer);
        }else{
            lv_timer_resume(game_timer);
            lv_obj_remove_flag(btn_img,LV_OBJ_FLAG_HIDDEN);
        }
        
    }
}

void on_rps_destroy(lv_obj_t* page){
    lv_timer_delete(game_timer);
}

static void rps_timer(lv_timer_t * timer){
    lv_image_dsc_t const * cen =NULL;
    if (actived)
    {
        anim_num = (anim_num+1)%3;
        cen = get_anim_tag(anim_num);
        tag_num--;
        if (tag_num<=0)
        {
            lv_timer_pause(game_timer);
            lv_obj_remove_flag(btn_img,LV_OBJ_FLAG_HIDDEN);
        }
    }else{
        anim_num = (anim_num+1)%10;
        cen = get_anim_dy(anim_num);
    }
    lv_image_set_src(cen_img,cen);
}

static void btn_click(lv_event_t *event){
    if (!actived){
        actived = true;
        lv_image_set_src(btn_img,get_img_reload());
        lv_obj_add_flag(btn_img,LV_OBJ_FLAG_HIDDEN);
        srand(time(NULL));
        
        int random_number = rand();
        anim_num=random_number % 3;
        tag_num = 12;
    }else{
        srand(time(NULL));
        int random_number = rand();
        anim_num=random_number % 3;
        tag_num = 12;
        lv_timer_resume(game_timer);
        lv_obj_add_flag(btn_img,LV_OBJ_FLAG_HIDDEN);
    }
}

static void handle_swipe_right(lv_event_t *event){
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_RIGHT)
    {
        if (actived)
        {
            anim_num = 0;
            tag_num=0;
            actived = false;
            lv_image_set_src(btn_img,get_img_start());
            lv_obj_remove_flag(btn_img,LV_OBJ_FLAG_HIDDEN);
            lv_timer_resume(game_timer);
        }else{
            // TODO
        }
        
    }
    
}

static lv_image_dsc_t const *get_anim_dy(int num){
    switch (num)
    {
    case 0:
        return get_img_dy0();
    case 1:
        return get_img_dy1();
    case 2:
        return get_img_dy2();
    case 3:
        return get_img_dy3();
    case 4:
        return get_img_dy4();
    case 5:
        return get_img_dy5();
    case 6:
        return get_img_dy4();
    case 7:
        return get_img_dy3();
    case 8:
        return get_img_dy2();
    case 9:
        return get_img_dy1();
    
    default:
        return NULL;
    }
    
}

static lv_image_dsc_t const *get_anim_tag(int num){
    switch (num)
    {
    case 0:
        return get_img_tag0();
    case 1:
        return get_img_tag1();
    case 2:
        return get_img_tag2();
    
    default:
        return NULL;
    }
}