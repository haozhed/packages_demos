#include "../include/quicks_balloon.h"
#include "../../main_page/include/main_page.h"
#include "../include/quicks_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../utils/include/circle_screen.h"


lv_image_dsc_t const *get_img_balloon_balloon(void);
lv_image_dsc_t const *get_img_balloon_bg(void);
lv_image_dsc_t const *get_img_balloon_bottom(void);
lv_image_dsc_t const *get_img_balloon_continu(void);
lv_image_dsc_t const *get_img_balloon_down_1(void);
lv_image_dsc_t const *get_img_balloon_down_2(void);
lv_image_dsc_t const *get_img_balloon_down_3(void);
lv_image_dsc_t const *get_img_balloon_exit(void);
lv_image_dsc_t const *get_img_balloon_flo(void);
lv_image_dsc_t const *get_img_balloon_help(void);
lv_image_dsc_t const *get_img_balloon_help_0(void);
lv_image_dsc_t const *get_img_balloon_help_1(void);
lv_image_dsc_t const *get_img_balloon_help_2(void);
lv_image_dsc_t const *get_img_balloon_help_3(void);
lv_image_dsc_t const *get_img_balloon_help_4(void);
lv_image_dsc_t const *get_img_balloon_help_5(void);
lv_image_dsc_t const *get_img_balloon_help_6(void);
lv_image_dsc_t const *get_img_balloon_help_7(void);
lv_image_dsc_t const *get_img_balloon_index(void);
lv_image_dsc_t const *get_img_balloon_logo(void);
lv_image_dsc_t const *get_img_balloon_max(void);
lv_image_dsc_t const *get_img_balloon_ov_tx(void);
lv_image_dsc_t const *get_img_balloon_ov1(void);
lv_image_dsc_t const *get_img_balloon_sc_0(void);
lv_image_dsc_t const *get_img_balloon_sc_1(void);
lv_image_dsc_t const *get_img_balloon_sc_2(void);
lv_image_dsc_t const *get_img_balloon_sc_3(void);
lv_image_dsc_t const *get_img_balloon_sc_4(void);
lv_image_dsc_t const *get_img_balloon_sc_5(void);
lv_image_dsc_t const *get_img_balloon_sc_6(void);
lv_image_dsc_t const *get_img_balloon_sc_7(void);
lv_image_dsc_t const *get_img_balloon_sc_8(void);
lv_image_dsc_t const *get_img_balloon_sc_9(void);
lv_image_dsc_t const *get_img_balloon_sc_mi(void);
lv_image_dsc_t const *get_img_balloon_top(void);


void balloon_page(void);

lv_obj_t * create_balloon_page(lv_obj_t * parent);
void on_balloon_destroy(lv_obj_t* page);              // 页面销毁
void on_balloon_resume(lv_obj_t* page);               // 页面恢复
void on_balloon_pause(lv_obj_t* page);                // 页面暂停

static void help_show(void);
static void help_close(void);
static void game_start(void);
static void game_over(void);
static void restart(void);
static void balloon_exit(void);
static void balloon_pause(void);
static void conti(void);
static void help_btn_click(lv_event_t * event);
static void start_btn_click(lv_event_t * event);
static void game_page_click(lv_event_t * event);
static void help_page_scroll(lv_event_t * event);
static void root_page_gesture(lv_event_t * event);

static void help_anim_timer(lv_timer_t *timer);
static void game_run_timer(lv_timer_t *timer);
static void game_down_timer(lv_timer_t *timer);
static lv_image_dsc_t const * help_img_calcu(int num);
static lv_image_dsc_t const * game_down_img_calcu(int num);
static lv_image_dsc_t const * sc_img_calcu(int num);
static void reflesh_game_sc(int num);
static void init_game_sc(void);
static void init_over_sc(void);
static void init_balloon(void);

Balloon_PageInfo_t *ball_infos = NULL;
Balloon_Data_t *ball_data = NULL;
Balloon_Compenent_t *ball_compenent = NULL;

void balloon_page(void){
    Balloon_PageInfo_t *infos = (Balloon_PageInfo_t*)malloc(sizeof(Balloon_PageInfo_t));
    Balloon_Data_t *datas = (Balloon_Data_t*)malloc(sizeof(Balloon_Data_t));
    Balloon_Data_t *compenents = (Balloon_Compenent_t*)malloc(sizeof(Balloon_Compenent_t));
    ball_compenent = compenents;
    ball_infos = infos;
    ball_data = datas;
    strcpy(ball_infos->foreground_str,"index");

    ball_data->help_timer = NULL;
    ball_data->game_timer = NULL;
    ball_data->help_anim_num=0;
    ball_data->game_balloon_y=140;
    ball_data->game_meter=0;
    ball_data->game_score=0;
    ball_data->game_delay_count=0;
    ball_data->down_num=3;
    ball_data->overd=false;

    ball_compenent->game_ls_len=0;
    // ball_compenent->over_ls_len=0;
    ball_compenent->game_down_flo=NULL;
    ball_compenent->game_down_img=NULL;
    ball_compenent->over_img_mi=NULL;

    ball_infos->balloon_help_page = NULL;
    ball_infos->balloon_game_page = NULL;
    ball_infos->balloon_pause_page = NULL;
    ball_infos->balloon_over_page = NULL;
    ball_infos->balloon_root = create_empty_page(get_watch_scr());
    lv_obj_add_event_cb(ball_infos->balloon_root,root_page_gesture,LV_EVENT_GESTURE,NULL);
    lv_obj_remove_flag(ball_infos->balloon_root,LV_OBJ_FLAG_GESTURE_BUBBLE);
    create_balloon_page(ball_infos->balloon_root);
}

lv_obj_t * create_balloon_page(lv_obj_t * parent){
    ball_infos->balloon_index_page = create_empty_page(ball_infos->balloon_root);
    lv_obj_set_style_bg_color(ball_infos->balloon_index_page,lv_color_hex(0x209fe4),0);
    lv_obj_t * index_img = lv_image_create(ball_infos->balloon_index_page);
    lv_image_set_src(index_img,get_img_balloon_index());
    lv_obj_align(index_img,LV_ALIGN_CENTER,0,20);
    
    lv_obj_t * help_btn = lv_obj_create(ball_infos->balloon_index_page);
    lv_obj_set_size(help_btn,80,80);
    lv_obj_align(help_btn,LV_ALIGN_TOP_MID,0,0);
    lv_obj_set_style_opa(help_btn,0,0);
    lv_obj_add_flag(help_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(help_btn,help_btn_click,LV_EVENT_CLICKED,NULL);

    lv_obj_t * start_btn = lv_obj_create(ball_infos->balloon_index_page);
    lv_obj_set_size(start_btn,240,80);
    lv_obj_align(start_btn,LV_ALIGN_BOTTOM_MID,0,0);
    lv_obj_set_style_opa(start_btn,0,0);
    lv_obj_add_flag(start_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(start_btn,start_btn_click,LV_EVENT_CLICKED,NULL);

    return ball_infos->balloon_index_page;
}

static void help_show(void){
    if (strcmp(ball_infos->foreground_str,"index") == 0)
    {
        if (ball_infos->balloon_help_page == NULL)
        {
            ball_infos->balloon_help_page = create_empty_page(ball_infos->balloon_root);
            lv_obj_set_style_bg_color(ball_infos->balloon_help_page,lv_color_hex(0x000000),0);
            lv_obj_set_style_bg_opa(ball_infos->balloon_help_page,180,0);

            lv_obj_t * help_img_cont = lv_obj_create(ball_infos->balloon_help_page);
            lv_obj_set_size(help_img_cont,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
            lv_obj_set_style_pad_top(help_img_cont,20,0);
            lv_obj_set_style_pad_bottom(help_img_cont,20,0);
            lv_obj_set_pos(help_img_cont,0,0);
            lv_obj_set_style_bg_opa(help_img_cont,0,0);

            lv_obj_add_event_cb(help_img_cont,help_page_scroll,LV_EVENT_PRESSING,help_img_cont);

            lv_obj_t *help_img = lv_image_create(help_img_cont);
            lv_image_set_src(help_img,get_img_balloon_help());

            lv_obj_t *anim_img = lv_image_create(help_img_cont);
            ball_compenent->help_anim_img = anim_img;
            lv_obj_set_pos(ball_compenent->help_anim_img,160,80);
            lv_image_set_src(ball_compenent->help_anim_img,get_img_balloon_help_0());

            lv_obj_t *help_clo_btn = lv_obj_create(help_img_cont);
            lv_obj_set_style_opa(help_clo_btn,0,0);
            lv_obj_set_size(help_clo_btn,60,60);
            lv_obj_align(help_clo_btn,LV_ALIGN_BOTTOM_MID,0,-10);
            lv_obj_add_flag(help_clo_btn,LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(help_clo_btn,help_close,LV_EVENT_CLICKED,NULL);

        }else{
            lv_obj_move_foreground(ball_infos->balloon_help_page);
        }
        strcpy(ball_infos->foreground_str,"help");
        if (ball_data->help_timer == NULL)
        {
            ball_data->help_timer = lv_timer_create(help_anim_timer,200,NULL);
        }
        
    }
}

static void game_start(void){
    if (strcmp(ball_infos->foreground_str,"index") == 0)
    {
        if (ball_infos->balloon_game_page == NULL)
        {
            ball_infos->balloon_game_page = create_empty_page(ball_infos->balloon_root);
            lv_obj_set_style_bg_color(ball_infos->balloon_game_page,lv_color_hex(0x209fe4),0);

            lv_obj_add_flag(ball_infos->balloon_game_page,LV_OBJ_FLAG_CHECKABLE);
            // lv_obj_add_event_cb(ball_infos->balloon_game_page,game_page_click,LV_EVENT_CLICKED,NULL);

            lv_obj_t * game_img = lv_image_create(ball_infos->balloon_game_page);
            lv_image_set_src(game_img,get_img_balloon_bg());
            lv_obj_align(game_img,LV_ALIGN_CENTER,0,20);

            lv_obj_t * top_img1 = lv_image_create(ball_infos->balloon_game_page);
            lv_image_set_src(top_img1,get_img_balloon_top());
            lv_obj_align(top_img1,LV_ALIGN_TOP_MID,-24,0);
            lv_obj_t * top_img2 = lv_image_create(ball_infos->balloon_game_page);
            lv_image_set_src(top_img2,get_img_balloon_top());
            lv_obj_align(top_img2,LV_ALIGN_TOP_MID,24,0);

            lv_obj_t * bottom_img1 = lv_image_create(ball_infos->balloon_game_page);
            lv_image_set_src(bottom_img1,get_img_balloon_bottom());
            lv_obj_align(bottom_img1,LV_ALIGN_BOTTOM_MID,-48,20);
            lv_obj_t * bottom_img2 = lv_image_create(ball_infos->balloon_game_page);
            lv_image_set_src(bottom_img2,get_img_balloon_bottom());
            lv_obj_align(bottom_img2,LV_ALIGN_BOTTOM_MID,0,20);
            lv_obj_t * bottom_img3 = lv_image_create(ball_infos->balloon_game_page);
            lv_image_set_src(bottom_img3,get_img_balloon_bottom());
            lv_obj_align(bottom_img3,LV_ALIGN_BOTTOM_MID,48,20);

            lv_obj_t * ball_img = lv_image_create(ball_infos->balloon_game_page);
            ball_compenent->game_balloon_img = ball_img;
            lv_image_set_src(ball_compenent->game_balloon_img,get_img_balloon_balloon());
            lv_obj_set_pos(ball_compenent->game_balloon_img,170,140);

            lv_obj_t *mi_img = lv_image_create(ball_infos->balloon_game_page);
            lv_image_set_src(mi_img,get_img_balloon_sc_mi());
            lv_obj_set_pos(mi_img,400,200);

            lv_obj_t *game_sc1 = lv_image_create(ball_infos->balloon_game_page);
            lv_obj_set_pos(game_sc1,290,200);
            lv_obj_t *game_sc2 = lv_image_create(ball_infos->balloon_game_page);
            lv_obj_set_pos(game_sc2,325,200);
            lv_obj_t *game_sc3 = lv_image_create(ball_infos->balloon_game_page);
            lv_obj_set_pos(game_sc3,360,200);
            lv_image_set_src(game_sc3,get_img_balloon_sc_0());

            ball_compenent->game_img_ls[0] = game_sc3;
            ball_compenent->game_img_ls[1] = game_sc2;
            ball_compenent->game_img_ls[2] = game_sc1;

        }else{
            lv_obj_move_foreground(ball_infos->balloon_game_page);
            init_game_sc();
            init_balloon();
        }
        strcpy(ball_infos->foreground_str,"game");

        if (ball_data->game_timer==NULL)
        {
            ball_data->overd=false;
            ball_data->down_num=3;
            if (ball_compenent->game_down_flo==NULL)
            {
                ball_compenent->game_down_flo = create_empty_page(ball_infos->balloon_game_page);
                lv_obj_set_style_bg_color(ball_compenent->game_down_flo,lv_color_hex(0x000000),0);
                lv_obj_set_style_bg_opa(ball_compenent->game_down_flo,180,0);
                lv_obj_set_size(ball_compenent->game_down_flo,455,455);
                lv_obj_center(ball_compenent->game_down_flo);

                ball_compenent->game_down_img = lv_image_create(ball_compenent->game_down_flo);
                lv_image_set_src(ball_compenent->game_down_img,game_down_img_calcu(ball_data->down_num));
                lv_obj_center(ball_compenent->game_down_img);
            }
            
            ball_data->game_timer = lv_timer_create(game_down_timer,1000,NULL);
        }
        
    }
}

static void game_over(void){
    if (strcmp(ball_infos->foreground_str,"game") == 0)
    {
        if (ball_infos->balloon_over_page == NULL)
        {
            ball_infos->balloon_over_page = create_empty_page(ball_infos->balloon_root);
            lv_obj_set_style_bg_color(ball_infos->balloon_over_page,lv_color_hex(0x209fe4),0);

            lv_obj_t * over_img = lv_image_create(ball_infos->balloon_over_page);
            lv_image_set_src(over_img,get_img_balloon_ov1());
            lv_obj_align(over_img,LV_ALIGN_CENTER,0,-20);

            lv_obj_t * restart_btn = lv_obj_create(ball_infos->balloon_over_page);
            lv_obj_set_size(restart_btn,180,80);
            lv_obj_align(restart_btn,LV_ALIGN_BOTTOM_MID,-100,-55);
            lv_obj_set_style_opa(restart_btn,0,0);
            lv_obj_add_flag(restart_btn,LV_OBJ_FLAG_CHECKABLE);
            lv_obj_add_event_cb(restart_btn,restart,LV_EVENT_CLICKED,NULL);

            lv_obj_t * exit_btn = lv_obj_create(ball_infos->balloon_over_page);
            lv_obj_set_size(exit_btn,180,80);
            lv_obj_align(exit_btn,LV_ALIGN_BOTTOM_MID,100,-55);
            lv_obj_set_style_opa(exit_btn,0,0);
            lv_obj_add_flag(exit_btn,LV_OBJ_FLAG_CHECKABLE);
            lv_obj_add_event_cb(exit_btn,balloon_exit,LV_EVENT_CLICKED,NULL);

            lv_obj_t *over_mi = lv_image_create(ball_infos->balloon_over_page);
            lv_image_set_src(over_mi,get_img_balloon_sc_mi());
            ball_compenent->over_img_mi=over_mi;

            char str[20]; // 足够大以存储一般整数对应的字符串
            sprintf(str, "%d", ball_data->game_score);
            int len = strlen(str);
            int end_po = 226 + ((len * 35)+40) / 2;
            int cur_po = end_po - 40;

            lv_obj_set_pos(ball_compenent->over_img_mi,cur_po,130);

            lv_obj_t *over_sc1 = lv_image_create(ball_infos->balloon_over_page);
            lv_obj_t *over_sc2 = lv_image_create(ball_infos->balloon_over_page);
            lv_obj_t *over_sc3 = lv_image_create(ball_infos->balloon_over_page);
            

            ball_compenent->over_img_ls[0] = over_sc3;
            ball_compenent->over_img_ls[1] = over_sc2;
            ball_compenent->over_img_ls[2] = over_sc1;

            int temp = ball_data->game_score;
            int end = 0;
            int index = 0;
            while (temp>0)
            {
                cur_po -= 35;
                end = temp % 10;
                temp = (temp - end)/10;
                lv_image_set_src(ball_compenent->over_img_ls[index],sc_img_calcu(end));
                lv_obj_set_pos(ball_compenent->over_img_ls[index],cur_po,130);
                index++;
            }

        }else{
            lv_obj_move_foreground(ball_infos->balloon_over_page);
            init_over_sc();
        }
        strcpy(ball_infos->foreground_str,"over");

        
    }
}

static void init_balloon(void){
    lv_obj_delete(ball_compenent->game_balloon_img);
    lv_obj_t * ball_img = lv_image_create(ball_infos->balloon_game_page);
    ball_compenent->game_balloon_img = ball_img;
    lv_image_set_src(ball_compenent->game_balloon_img,get_img_balloon_balloon());
    lv_obj_set_pos(ball_compenent->game_balloon_img,170,140);
    ball_data->game_balloon_y=140;
}

static void init_over_sc(void){
    lv_obj_delete(ball_compenent->over_img_ls[0]);
    lv_obj_delete(ball_compenent->over_img_ls[1]);
    lv_obj_delete(ball_compenent->over_img_ls[2]);

    char str[20]; // 足够大以存储一般整数对应的字符串
    sprintf(str, "%d", ball_data->game_score);
    int len = strlen(str);
    int end_po = 226 + ((len * 35)+40) / 2;
    int cur_po = end_po - 40;

    lv_obj_set_pos(ball_compenent->over_img_mi,cur_po,130);

    lv_obj_t *over_sc1 = lv_image_create(ball_infos->balloon_over_page);
    lv_obj_t *over_sc2 = lv_image_create(ball_infos->balloon_over_page);
    lv_obj_t *over_sc3 = lv_image_create(ball_infos->balloon_over_page);
            

    ball_compenent->over_img_ls[0] = over_sc3;
    ball_compenent->over_img_ls[1] = over_sc2;
    ball_compenent->over_img_ls[2] = over_sc1;

    int temp = ball_data->game_score;
    int end = 0;
    int index = 0;
    while (temp>0)
    {
        cur_po -= 35;
        end = temp % 10;
        temp = (temp - end)/10;
        lv_image_set_src(ball_compenent->over_img_ls[index],sc_img_calcu(end));
        lv_obj_set_pos(ball_compenent->over_img_ls[index],cur_po,130);
        index++;
    }
}

static void reflesh_game_sc(int num){
    int temp = num;
    int end = 0;
    int index = 0;
    while (temp>0)
    {
        end = temp % 10;
        temp = (temp - end)/10;
        lv_image_set_src(ball_compenent->game_img_ls[index++],sc_img_calcu(end));
    }
}

static void restart(void){
    LV_LOG_USER("restart clicked");
    ball_data->game_balloon_y=140;
    ball_data->game_meter=0;
    ball_data->game_score=0;
    lv_image_set_src(ball_compenent->game_balloon_img,get_img_balloon_balloon());
    lv_obj_set_pos(ball_compenent->game_balloon_img,170,140);
    lv_obj_move_foreground(ball_infos->balloon_game_page);

    init_game_sc();

    strcpy(ball_infos->foreground_str,"game");
    if (ball_data->game_timer==NULL)
    {
        ball_data->overd=false;
        ball_data->down_num=3;
        if (ball_compenent->game_down_flo==NULL)
        {
            ball_compenent->game_down_flo = create_empty_page(ball_infos->balloon_game_page);
            lv_obj_set_style_bg_color(ball_compenent->game_down_flo,lv_color_hex(0x000000),0);
            lv_obj_set_style_bg_opa(ball_compenent->game_down_flo,180,0);
            lv_obj_set_size(ball_compenent->game_down_flo,455,455);
            lv_obj_center(ball_compenent->game_down_flo);

            ball_compenent->game_down_img = lv_image_create(ball_compenent->game_down_flo);
            lv_image_set_src(ball_compenent->game_down_img,game_down_img_calcu(ball_data->down_num));
            lv_obj_center(ball_compenent->game_down_img);
        }
            
        ball_data->game_timer = lv_timer_create(game_down_timer,1000,NULL);
    }
}

static void init_game_sc(void){
    lv_obj_delete(ball_compenent->game_img_ls[0]);
    lv_obj_delete(ball_compenent->game_img_ls[1]);
    lv_obj_delete(ball_compenent->game_img_ls[2]);
    lv_obj_t *game_sc1 = lv_image_create(ball_infos->balloon_game_page);
    lv_obj_set_pos(game_sc1,290,200);
    lv_obj_t *game_sc2 = lv_image_create(ball_infos->balloon_game_page);
    lv_obj_set_pos(game_sc2,325,200);
    lv_obj_t *game_sc3 = lv_image_create(ball_infos->balloon_game_page);
    lv_obj_set_pos(game_sc3,360,200);
    lv_image_set_src(game_sc3,get_img_balloon_sc_0());

    ball_compenent->game_img_ls[0] = game_sc3;
    ball_compenent->game_img_ls[1] = game_sc2;
    ball_compenent->game_img_ls[2] = game_sc1;
}

static void balloon_exit(void){
    LV_LOG_USER("balloon_exit clicked");
    lv_obj_delete(ball_infos->balloon_root);
    free(ball_infos);
    free(ball_data);
    free(ball_compenent);
}

static void game_run_timer(lv_timer_t *timer){
    // 72 274
    if (!ball_data->overd)
    {
        int cur_calu = ball_data->game_balloon_y - 4;
        lv_obj_set_y(ball_compenent->game_balloon_img,cur_calu);
        ball_data->game_balloon_y = cur_calu;
        ball_data->game_meter += 4;
        if (ball_data->game_meter>=40)
        {
            ball_data->game_meter -= 40;
            ball_data->game_score++;
            if (ball_data->game_score>999)
            {
                ball_data->game_score=999;
            }
            reflesh_game_sc(ball_data->game_score);
        }
        
        LV_LOG_USER("game_run_timer sc:%d",ball_data->game_score);
        if (cur_calu <= 72 || cur_calu >= 274)
        {
            ball_data->overd=true;
            ball_data->game_delay_count=50;
            lv_obj_remove_event_cb(ball_infos->balloon_game_page,game_page_click);
            lv_image_set_src(ball_compenent->game_balloon_img,get_img_balloon_ov_tx());
            lv_obj_set_x(ball_compenent->game_balloon_img,146);
        }
        
    }else{
        if (ball_data->game_delay_count<=0)
        {
            /* code */
            if (ball_data->game_timer!=NULL)
            {
                lv_timer_delete(ball_data->game_timer);
                ball_data->game_timer=NULL;
            }
            if (ball_compenent->game_down_flo!=NULL)
            {
                lv_obj_delete(ball_compenent->game_down_flo);
                ball_compenent->game_down_flo=NULL;
                ball_compenent->game_down_img=NULL;
            }
            game_over();
            
        }else{
            ball_data->game_delay_count--;
        }
    }
}

static void game_down_timer(lv_timer_t *timer){
    LV_LOG_USER("game_down_timer run");
    ball_data->down_num--;
    lv_image_set_src(ball_compenent->game_down_img,game_down_img_calcu(ball_data->down_num));
    if (ball_data->down_num<=0)
    {
        ball_data->down_num=0;
        lv_obj_add_event_cb(ball_infos->balloon_game_page,game_page_click,LV_EVENT_CLICKED,NULL);
        lv_timer_set_cb(ball_data->game_timer,game_run_timer);
        lv_timer_set_period(ball_data->game_timer,30);
        lv_obj_delete(ball_compenent->game_down_flo);
        ball_compenent->game_down_flo=NULL;
        ball_compenent->game_down_img=NULL;
    }
}

static void game_page_click(lv_event_t * event){
    LV_LOG_USER("game_page_click click");
    int cur_calu = ball_data->game_balloon_y + 120;
    lv_obj_set_y(ball_compenent->game_balloon_img,cur_calu);
    ball_data->game_balloon_y = cur_calu;
}

static void help_anim_timer(lv_timer_t *timer){
    ball_data->help_anim_num = (ball_data->help_anim_num + 1) % 8;
    lv_image_set_src(ball_compenent->help_anim_img,help_img_calcu(ball_data->help_anim_num));
}

static void help_close(void){
    LV_LOG_USER("help_close click");
    lv_obj_move_foreground(ball_infos->balloon_index_page);
    strcpy(ball_infos->foreground_str,"index");
    lv_timer_delete(ball_data->help_timer);
    ball_data->help_timer=NULL;
}

static void help_btn_click(lv_event_t * event){
    LV_LOG_USER("help_btn_click");
    help_show();
}

static void start_btn_click(lv_event_t * event){
    LV_LOG_USER("start_btn_click");
    game_start();
}

static lv_image_dsc_t const * game_down_img_calcu(int num){
    switch (num)
    {
    case 1:
        return get_img_balloon_down_1();
    case 2:
        return get_img_balloon_down_2();
    case 3:
        return get_img_balloon_down_3();
    
    default:
        break;
    }
    return get_img_balloon_down_1();
}

static lv_image_dsc_t const * sc_img_calcu(int num){
    switch (num)
    {
    case 0:
        return get_img_balloon_sc_0();
    case 1:
        return get_img_balloon_sc_1();
    case 2:
        return get_img_balloon_sc_2();
    case 3:
        return get_img_balloon_sc_3();
    case 4:
        return get_img_balloon_sc_4();
    case 5:
        return get_img_balloon_sc_5();
    case 6:
        return get_img_balloon_sc_6();
    case 7:
        return get_img_balloon_sc_7();
    case 8:
        return get_img_balloon_sc_8();
    case 9:
        return get_img_balloon_sc_9();
    
    default:
        return get_img_balloon_sc_mi();
    }
}

static lv_image_dsc_t const * help_img_calcu(int num){
    switch (num)
    {
    case 0:
        return get_img_balloon_help_0();
    case 1:
        return get_img_balloon_help_1();
    case 2:
        return get_img_balloon_help_2();
    case 3:
        return get_img_balloon_help_3();
    case 4:
        return get_img_balloon_help_4();
    case 5:
        return get_img_balloon_help_5();
    case 6:
        return get_img_balloon_help_6();
    case 7:
        return get_img_balloon_help_7();
    
    default:
        return get_img_balloon_help_0();
    }
    return get_img_balloon_help_0();
}

static void help_page_scroll(lv_event_t * event){
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t * cont = lv_event_get_user_data(event);

    lv_indev_t *indev = lv_event_get_indev(event);
    if (!indev) return;

    lv_point_t point;
    lv_indev_get_vect(indev, &point);

    int cur_y = lv_obj_get_y(cont);
    int calcu_y = cur_y + point.y * 2;
    if (calcu_y>0)
    {
        calcu_y = 0;
    }else if (calcu_y < -260)
    {
        calcu_y = -260;
    }
    lv_obj_set_pos(cont,0,calcu_y);
    // LV_LOG_USER("calcu_y :%d",calcu_y);
}

static void root_page_gesture(lv_event_t * event){
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_RIGHT)
    {
        LV_LOG_USER("root_page_gesture right swipe");
        if (strcmp(ball_infos->foreground_str,"index") == 0){
            LV_LOG_USER("root_page_gesture right swipe index");
            lv_obj_delete(ball_infos->balloon_root);
            free(ball_infos);
            free(ball_data);
            free(ball_compenent);
        }
        else if (strcmp(ball_infos->foreground_str,"help") == 0)
        {
            LV_LOG_USER("root_page_gesture right swipe help");
            lv_obj_move_foreground(ball_infos->balloon_index_page);
            strcpy(ball_infos->foreground_str,"index");
            lv_timer_delete(ball_data->help_timer);
            ball_data->help_timer=NULL;
        }
        else if (strcmp(ball_infos->foreground_str,"game") == 0)
        {
            LV_LOG_USER("root_page_gesture right swipe game");
            if (ball_data->game_timer!=NULL)
            {
                lv_timer_delete(ball_data->game_timer);
                ball_data->game_timer=NULL;
            }
            if (ball_compenent->game_down_flo!=NULL)
            {
                lv_obj_delete(ball_compenent->game_down_flo);
                ball_compenent->game_down_flo=NULL;
                ball_compenent->game_down_img=NULL;
            }
            ball_data->game_balloon_y=140;
            ball_data->game_meter=0;
            ball_data->game_score=0;
            lv_image_set_src(ball_compenent->game_balloon_img,get_img_balloon_balloon());
            lv_obj_set_pos(ball_compenent->game_balloon_img,170,140);

            lv_obj_move_foreground(ball_infos->balloon_index_page);
            strcpy(ball_infos->foreground_str,"index");
        }
        else if (strcmp(ball_infos->foreground_str,"pause") == 0)
        {
            LV_LOG_USER("root_page_gesture right swipe over");
            lv_obj_move_foreground(ball_infos->balloon_game_page);
            strcpy(ball_infos->foreground_str,"game");
        }
        else if (strcmp(ball_infos->foreground_str,"over") == 0)
        {
            LV_LOG_USER("root_page_gesture right swipe over");

            ball_data->game_balloon_y=140;
            ball_data->game_meter=0;
            ball_data->game_score=0;
            lv_image_set_src(ball_compenent->game_balloon_img,get_img_balloon_balloon());
            lv_obj_set_pos(ball_compenent->game_balloon_img,170,140);

            lv_obj_move_foreground(ball_infos->balloon_index_page);
            strcpy(ball_infos->foreground_str,"index");
        }
    }
}