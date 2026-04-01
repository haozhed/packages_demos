#ifndef QUICKS_BALLOON_H
#define QUICKS_BALLOON_H

#include <lvgl/lvgl.h>

void balloon_page(void);

lv_obj_t * create_balloon_page(lv_obj_t * parent);
void on_balloon_destroy(lv_obj_t* page);              // 页面销毁
void on_balloon_resume(lv_obj_t* page);               // 页面恢复
void on_balloon_pause(lv_obj_t* page);                // 页面暂停


typedef struct {
    lv_obj_t * balloon_root;
    lv_obj_t * balloon_index_page;
    lv_obj_t * balloon_help_page;
    lv_obj_t * balloon_game_page;
    lv_obj_t * balloon_pause_page;
    lv_obj_t * balloon_over_page;
    char foreground_str[20];
} Balloon_PageInfo_t;

typedef struct {
    lv_timer_t * help_timer;
    int help_anim_num;
    lv_timer_t * game_timer;
    int game_balloon_y;
    int game_meter;
    int game_score;
    int game_delay_count;
    int down_num;

    bool overd;
    
} Balloon_Data_t;

typedef struct{
    lv_obj_t *help_anim_img;
    lv_obj_t *game_balloon_img;

    lv_obj_t *game_down_flo;
    lv_obj_t *game_down_img;

    lv_obj_t *game_img_ls[3];
    int game_ls_len;

    lv_obj_t *over_img_ls[3];
    lv_obj_t *over_img_mi;
    // int over_ls_len;

} Balloon_Compenent_t;

#endif