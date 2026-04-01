#include "../include/quick_air.h"
#include "../../utils/include/circle_screen.h"
#include "../include/air_image.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

lv_obj_t * quick_air_page(void);

static lv_obj_t * create_empty_page(lv_obj_t * root);
static Air_PageInfo_t* createAndInitAirPageInfo(void);
static void air_game_timer_unstart_cb(lv_timer_t *timer);
static void air_game_timer_started_cb(lv_timer_t *timer);
static void tostart_anim_cb(void *, int32_t v);
static void tostart_anim_ready_cb(lv_anim_t *a);
static void startbak_anim_ready_cb(lv_anim_t *a);
static void start_img_click(lv_event_t *e);
static void air_page_gesture_event_cb(lv_event_t * e);
static void air_game_again_event_cb(lv_event_t * e);
static void air_game_quit_event_cb(lv_event_t * e);

static void game_to_boss(void);
static void game_to_next(void);
static void game_to_fail(void);
static void user_shoot(void);
static void game_component_reset(void);
static void ene_move_shoot(Enemy_t * plane);
static void boss_move_shoot(Enemy_t * plane);
static void game_air_over(void);

static lv_image_dsc_t const *get_health_img(int health);
static lv_image_dsc_t const *get_ene_explo_img(int num);
static lv_image_dsc_t const *get_boss_explo_img(int num);
static lv_image_dsc_t const *get_game_sc_img(int num);

static void calcued_bullets(void);
static void game_sc_fresh(int sc);

Air_PageInfo_t *air_infos = NULL;

lv_obj_t * quick_air_page(void){
    air_infos=createAndInitAirPageInfo();

    if (air_infos->root_page == NULL)
    {
        air_infos->root_page = create_empty_page(get_watch_scr());
        lv_obj_remove_flag(air_infos->root_page,LV_OBJ_FLAG_GESTURE_BUBBLE);
        lv_obj_add_event_cb(air_infos->root_page,air_page_gesture_event_cb,LV_EVENT_PRESSING,NULL);
        lv_obj_add_event_cb(air_infos->root_page,air_page_gesture_event_cb,LV_EVENT_RELEASED,NULL);

        lv_obj_t * bg_img = lv_image_create(air_infos->root_page);
        lv_image_set_src(bg_img,AIR_STR_TO_SRC(air_bg));
        lv_obj_center(bg_img);

        air_infos->op_page = create_empty_page(air_infos->root_page);
        lv_obj_set_style_bg_opa(air_infos->op_page,0,0);
        lv_obj_add_event_cb(air_infos->op_page,air_page_gesture_event_cb,LV_EVENT_PRESSING,NULL);
        lv_obj_add_event_cb(air_infos->op_page,air_page_gesture_event_cb,LV_EVENT_RELEASED,NULL);

        air_infos->user->img = lv_image_create(air_infos->op_page);
        lv_image_set_src(air_infos->user->img,AIR_STR_TO_SRC(air_big_plane));
        lv_obj_set_pos(air_infos->user->img,90,80);

        lv_obj_set_style_transform_pivot_x(air_infos->user->img,136,0);
        lv_obj_set_style_transform_pivot_y(air_infos->user->img,142,0);

        air_infos->title_img = lv_image_create(air_infos->root_page);
        lv_image_set_src(air_infos->title_img,AIR_STR_TO_SRC(air_sar));
        lv_obj_set_pos(air_infos->title_img,51,20);

        air_infos->start_img = lv_image_create(air_infos->root_page);
        lv_image_set_src(air_infos->start_img,AIR_STR_TO_SRC(air_start));
        lv_obj_set_pos(air_infos->start_img,106,340);

        lv_obj_add_flag(air_infos->start_img,LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(air_infos->start_img,start_img_click,LV_EVENT_CLICKED,NULL);

        for (int i = 0; i < 4; i++) {
            air_infos->sc_imgs[i] = lv_image_create(air_infos->root_page);
            lv_image_set_src(air_infos->sc_imgs[i],AIR_STR_TO_SRC(air_number_0));
            lv_obj_set_pos(air_infos->sc_imgs[i],249 - i*22,-165);
        }

        air_infos->health_img = lv_image_create(air_infos->root_page);
        lv_image_set_src(air_infos->health_img,AIR_STR_TO_SRC(air_health_5));
        lv_obj_set_pos(air_infos->health_img,133,-135);

        air_infos->game_timer = lv_timer_create(air_game_timer_unstart_cb,60,NULL);
    }else{

    }

    return air_infos->root_page;
}

static void calcued_bullets(void){
    for (int j = 0; j < MAX_BULLET; j++)
    {
        if (air_infos->bullets[j]->img != NULL)
        {
            air_infos->bullets[j]->y += air_infos->bullets[j]->speed;
            if (air_infos->bullets[j]->speed > 0)
            {
                if (air_infos->bullets[j]->y >= 455)
                {
                    air_infos->bullets[j]->alive = false;
                    lv_obj_delete(air_infos->bullets[j]->img);
                    air_infos->bullets[j]->img = NULL;
                }else{
                    int arm_pos_y =air_infos->bullets[j]->y + 48;
                    if (arm_pos_y >= air_infos->user->y && arm_pos_y <= air_infos->user->y + 88 && 
                        air_infos->bullets[j]->x >= air_infos->user->x +10 && air_infos->bullets[j]->x <= air_infos->user->x + 40)
                    {
                        air_infos->bullets[j]->alive = false;
                        lv_obj_delete(air_infos->bullets[j]->img);
                        air_infos->bullets[j]->img = NULL;

                        air_infos->user->health--;
                        lv_image_set_src(air_infos->health_img,get_health_img(air_infos->user->health));
                        if (air_infos->user->health <= 0)
                        {
                            // 游戏结束
                            game_to_fail();
                        }
                        
                    }else{
                        lv_obj_set_pos(air_infos->bullets[j]->img,air_infos->bullets[j]->x,air_infos->bullets[j]->y);
                    } 
                }
                
            }else{
                if (air_infos->bullets[j]->y <= 0)
                {
                    air_infos->bullets[j]->alive = false;
                    lv_obj_delete(air_infos->bullets[j]->img);
                    air_infos->bullets[j]->img = NULL;
                }else{
                    int arm_pos_y =air_infos->bullets[j]->y;
                    int arm_pos_x =air_infos->bullets[j]->x;
                    bool desed = false;
                    for (int i = 0; i < 3; i++){
                        if (air_infos->enemys[i]->img == NULL || !air_infos->enemys[i]->alive)
                        {
                            continue;
                        }
                        
                        if (arm_pos_y >= air_infos->enemys[i]->y  && arm_pos_y <= air_infos->enemys[i]->y + air_infos->enemys[i]->radius && 
                        arm_pos_x >= air_infos->enemys[i]->x && arm_pos_x + 36 <= air_infos->enemys[i]->x + air_infos->enemys[i]->radius )
                        {
                            air_infos->bullets[j]->alive = false;
                            lv_obj_delete(air_infos->bullets[j]->img);
                            air_infos->bullets[j]->img = NULL;
                            desed=true;

                            air_infos->enemys[i]->health--;
                            if (air_infos->enemys[i]->health <= 0)
                            {
                                // 敌机摧毁
                                air_infos->score += GAME_ENE_SCORE;
                                if (air_infos->score>9999)
                                {
                                    air_infos->score = 9999;
                                }
                                // 提升level
                                air_infos->boss_ready_num--;
                                if (air_infos->boss_ready_num<=0 && strcmp(air_infos->game_state,"start")==0)
                                {
                                    game_to_boss();
                                }
                                

                                game_sc_fresh(air_infos->score);

                                air_infos->enemys[i]->alive = false;
                                lv_obj_delete(air_infos->enemys[i]->img);
                                air_infos->enemys[i]->img = NULL;

                                air_infos->enemys[i]->gun_num=0;
                                air_infos->enemys[i]->explo_img = lv_image_create(air_infos->op_page);
                                lv_image_set_src(air_infos->enemys[i]->explo_img,get_ene_explo_img(air_infos->enemys[i]->gun_num));
                                lv_obj_set_pos(air_infos->enemys[i]->explo_img,air_infos->enemys[i]->x+19,air_infos->enemys[i]->y+19);
                            }
                            break;
                        }
                    }

                    // 判断boss
                    if (!desed && strcmp(air_infos->game_state,"boss")==0){
                        if (air_infos->boss->img != NULL && air_infos->boss->alive)
                        {
                            if (arm_pos_y >= air_infos->boss->y  && arm_pos_y <= air_infos->boss->y + 172 && 
                            arm_pos_x >= air_infos->boss->x && arm_pos_x + 36 <= air_infos->boss->x + air_infos->boss->radius )
                            {
                                air_infos->bullets[j]->alive = false;
                                lv_obj_delete(air_infos->bullets[j]->img);
                                air_infos->bullets[j]->img = NULL;
                                desed=true;

                                air_infos->boss->health--;
                                if (air_infos->boss->health <= 0)
                                {
                                    // 敌机摧毁
                                    air_infos->score += GAME_BOSS_SCORE;
                                    if (air_infos->score>9999)
                                    {
                                        air_infos->score = 9999;
                                    }
                                    game_sc_fresh(air_infos->score);

                                    air_infos->boss->alive = false;
                                    air_infos->boss->gun_num=0;
                                    air_infos->boss->explo_img = lv_image_create(air_infos->op_page);
                                    lv_image_set_src(air_infos->boss->explo_img,get_boss_explo_img(air_infos->boss->gun_num));
                                    lv_obj_set_pos(air_infos->boss->explo_img,air_infos->boss->x+44,air_infos->boss->y-14);
                                }
                            }
                        }
                    }

                    if (!desed)
                    {
                        lv_obj_set_pos(air_infos->bullets[j]->img,air_infos->bullets[j]->x,air_infos->bullets[j]->y);
                    } 
                }
            }
        }
    }
}

static void air_game_timer_started_cb(lv_timer_t *timer){
    // 出兵
    if (strcmp(air_infos->game_state,"start")==0)
    {
        bool ene_calued = false;
        // 所以敌机行为
        for (int i = 0; i < 3; i++){
            // 创建敌机
            if (!ene_calued && air_infos->enemys[i]->img == NULL && air_infos->enemys[i]->explo_img == NULL)
            {
                ene_calued = true;
                air_infos->ene_create_step--;
                if (air_infos->ene_create_step<=0)
                {
                    LV_LOG_USER("ene create");
                    air_infos->ene_create_step = 40;
                    air_infos->enemys[i]->img = lv_image_create(air_infos->op_page);
                    lv_image_set_src(air_infos->enemys[i]->img,AIR_STR_TO_SRC(air_enemy));

                    srand(time(NULL));

                    air_infos->enemys[i]->x = rand() % 360 +40;
                    air_infos->enemys[i]->y = -90;
                    air_infos->enemys[i]->health = air_infos->level;
                    air_infos->enemys[i]->radius = 88;
                    air_infos->enemys[i]->speed = rand() % ENE_SPEED_INCRE + ENE_SPEED_BASE;
                    air_infos->enemys[i]->arm_x = air_infos->user->x;
                    air_infos->enemys[i]->arm_y = rand() % 100 +20;
                    air_infos->enemys[i]->alive = true;
                    air_infos->enemys[i]->gun_num = 10;

                    lv_obj_set_pos(air_infos->enemys[i]->img,air_infos->enemys[i]->x,air_infos->enemys[i]->y);
                }
            }
            // 毁灭动效
            else if (!air_infos->enemys[i]->alive && air_infos->enemys[i]->explo_img != NULL)
            {
                air_infos->enemys[i]->gun_num++;
                if (air_infos->enemys[i]->gun_num>6)
                {
                    lv_obj_delete(air_infos->enemys[i]->explo_img);
                    air_infos->enemys[i]->explo_img = NULL;
                }else{
                    lv_image_set_src(air_infos->enemys[i]->explo_img,get_ene_explo_img(air_infos->enemys[i]->gun_num));
                }
                
            }
            // 移动加发射
            else if (air_infos->enemys[i]->img != NULL)
            {
                ene_move_shoot(air_infos->enemys[i]); 
            }
        }

        user_shoot();

        // 子弹运动
        calcued_bullets();
    }
    // 打boss
    else if (strcmp(air_infos->game_state,"toboss")==0)
    {
        air_infos->flo_down--;
        if (air_infos->flo_down<=0)
        {
            strcpy(air_infos->game_state,"boss");
            lv_obj_delete(air_infos->flo_img);
            air_infos->flo_img=NULL;
            air_infos->flo_cen_img=NULL;

            if (air_infos->boss->img!=NULL)
            {
                lv_obj_delete(air_infos->boss->img);
            }

            air_infos->boss->img = lv_image_create(air_infos->op_page);
            lv_image_set_src(air_infos->boss->img,AIR_STR_TO_SRC(air_boss));

            srand(time(NULL));

            air_infos->boss->x = rand() % 160;
            air_infos->boss->y = -90;
            air_infos->boss->health = air_infos->level * 10;
            air_infos->boss->radius = 288;
            air_infos->boss->speed = rand() % ENE_SPEED_INCRE + ENE_SPEED_BASE;
            air_infos->boss->arm_x = air_infos->user->x - 100;
            air_infos->boss->arm_y = rand() % 120;
            air_infos->boss->alive = true;
            air_infos->boss->gun_num = BOSS_GUN_STEP;

            lv_obj_set_pos(air_infos->boss->img,air_infos->boss->x,air_infos->boss->y);

        }
        
    }
    // 下一难度
    else if (strcmp(air_infos->game_state,"next")==0)
    {
        air_infos->flo_down--;
        if (air_infos->flo_down<=0)
        {
            strcpy(air_infos->game_state,"start");
            lv_obj_delete(air_infos->flo_img);
            air_infos->flo_img=NULL;
            air_infos->flo_cen_img=NULL;

            air_infos->level++;
            air_infos->boss_ready_num = GAME_BOSS_READY;
        }
    }
    // 打boss
    else if (strcmp(air_infos->game_state,"boss")==0)
    {
        bool ene_calued = false;
        // 所以敌机行为
        for (int i = 0; i < 3; i++){
            // 创建敌机
            if (!ene_calued && air_infos->enemys[i]->img == NULL && air_infos->enemys[i]->explo_img == NULL)
            {
                continue;
            }
            // 毁灭动效
            else if (!air_infos->enemys[i]->alive && air_infos->enemys[i]->explo_img != NULL)
            {
                air_infos->enemys[i]->gun_num++;
                if (air_infos->enemys[i]->gun_num>6)
                {
                    lv_obj_delete(air_infos->enemys[i]->explo_img);
                    air_infos->enemys[i]->explo_img = NULL;
                }else{
                    lv_image_set_src(air_infos->enemys[i]->explo_img,get_ene_explo_img(air_infos->enemys[i]->gun_num));
                }
                
            }
            // 移动加发射
            else if (air_infos->enemys[i]->img != NULL)
            {
                ene_move_shoot(air_infos->enemys[i]);
            }
        }
        if (air_infos->boss!=NULL && air_infos->boss->alive)
        {
            boss_move_shoot(air_infos->boss);
        }else if (air_infos->boss->explo_img!=NULL)
        {
            air_infos->boss->gun_num++;
            if (air_infos->boss->gun_num>6)
            {
                lv_obj_delete(air_infos->boss->img);
                air_infos->boss->img = NULL;
                lv_obj_delete(air_infos->boss->explo_img);
                air_infos->boss->explo_img = NULL;

                game_to_next();
            }else{
                lv_image_set_src(air_infos->boss->explo_img,get_boss_explo_img(air_infos->boss->gun_num));
            }
        }
        
        user_shoot();
        calcued_bullets();
    }
    // 结束
    else if (strcmp(air_infos->game_state,"fail")==0)
    {
        air_infos->flo_down--;
        if (air_infos->flo_down<=0)
        {
            strcpy(air_infos->game_state,"over");
            lv_obj_delete(air_infos->flo_img);
            air_infos->flo_img=NULL;
            air_infos->flo_cen_img=NULL;

            game_air_over();
        }
    }
    // 结束
    else if (strcmp(air_infos->game_state,"over")==0)
    {
        /* code */
    }
}

static void game_air_over(){
    strcpy(air_infos->game_state,"over");
    if (air_infos->over_flo!=NULL)
    {
        lv_obj_delete(air_infos->over_flo);
    }
    air_infos->over_flo = create_empty_page(air_infos->root_page);
    lv_obj_set_style_bg_color(air_infos->over_flo,lv_color_hex(0x000000),0);
    lv_obj_set_style_bg_opa(air_infos->over_flo,160,0);

    lv_obj_t *again_img = lv_image_create(air_infos->over_flo);
    lv_image_set_src(again_img,AIR_STR_TO_SRC(air_again));
    lv_obj_align(again_img,LV_ALIGN_CENTER,0,-80);
    lv_obj_add_flag(again_img,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(again_img,air_game_again_event_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t *quit_img = lv_image_create(air_infos->over_flo);
    lv_image_set_src(quit_img,AIR_STR_TO_SRC(air_quit));
    lv_obj_align(quit_img,LV_ALIGN_CENTER,0,80);
    lv_obj_add_flag(quit_img,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(quit_img,air_game_quit_event_cb,LV_EVENT_CLICKED,NULL);
}

static void boss_move_shoot(Enemy_t * plane){
    int abs_x = abs(plane->x - plane->arm_x);
    int abs_y = abs(plane->y - plane->arm_y);
    if (abs_x == 0 && abs_y == 0)
    {
        // 重定位
        plane->arm_x = air_infos->user->x - 100;
        plane->arm_y = rand() % 120 - 20;
    }else{
        if (abs_x <= plane->speed)
        {
            plane->x = plane->arm_x;
        }else{
            if (plane->x > plane->arm_x)
            {
                plane->x -= plane->speed;
            }else if (plane->x < plane->arm_x)
            {
                plane->x += plane->speed;
            }
        }
        if (abs_y <= plane->speed)
        {
            plane->y = plane->arm_y;
        }else{
            if (plane->y > plane->arm_y)
            {
                plane->y -= plane->speed;
            }else if (plane->y < plane->arm_y)
            {
                plane->y += plane->speed;
            }
        }
        lv_obj_set_pos(plane->img,plane->x,plane->y);
    }
    // 发射
    plane->gun_num--;
    if (plane->gun_num <= 0)
    {
        int boss_gun_num = 8;
        for (int j = 0; j < MAX_BULLET; j++)
        {
            if (air_infos->bullets[j]->img == NULL)
            {
                air_infos->bullets[j]->img = lv_image_create(air_infos->op_page);
                lv_image_set_src(air_infos->bullets[j]->img,AIR_STR_TO_SRC(air_gun_1));
                air_infos->bullets[j]->x = plane->x + 24 *j;
                air_infos->bullets[j]->y = plane->y + 160;
                air_infos->bullets[j]->speed = GUN_SPEED;
                air_infos->bullets[j]->alive = true;
                lv_obj_set_pos(air_infos->bullets[j]->img,air_infos->bullets[j]->x,air_infos->bullets[j]->y);

                boss_gun_num--;
                if (boss_gun_num<=0)
                {
                    break;
                }
            }
        }
        plane->gun_num = BOSS_GUN_STEP;
    } 
}

static void ene_move_shoot(Enemy_t * plane){
    int abs_x = abs(plane->x - plane->arm_x);
    int abs_y = abs(plane->y - plane->arm_y);
    if (abs_x == 0 && abs_y == 0)
    {
        // 重定位
        plane->arm_x = air_infos->user->x;
        plane->arm_y = rand() % 100 +20;
    }else{
        if (abs_x <= plane->speed)
        {
            plane->x = plane->arm_x;
        }else{
            if (plane->x > plane->arm_x)
            {
                plane->x -= plane->speed;
            }else if (plane->x < plane->arm_x)
            {
                plane->x += plane->speed;
            }
        }
        if (abs_y <= plane->speed)
        {
            plane->y = plane->arm_y;
        }else{
            if (plane->y > plane->arm_y)
            {
                plane->y -= plane->speed;
            }else if (plane->y < plane->arm_y)
            {
                plane->y += plane->speed;
            }
        }
        lv_obj_set_pos(plane->img,plane->x,plane->y);
    }
    // 发射
    plane->gun_num--;
    if (plane->gun_num <= 0)
    {
        for (int j = 0; j < MAX_BULLET; j++)
        {
            if (air_infos->bullets[j]->img == NULL)
            {
                air_infos->bullets[j]->img = lv_image_create(air_infos->op_page);
                lv_image_set_src(air_infos->bullets[j]->img,AIR_STR_TO_SRC(air_gun_1));
                air_infos->bullets[j]->x = plane->x + 24;
                air_infos->bullets[j]->y = plane->y + 80;
                air_infos->bullets[j]->speed = GUN_SPEED;
                air_infos->bullets[j]->alive = true;
                lv_obj_set_pos(air_infos->bullets[j]->img,air_infos->bullets[j]->x,air_infos->bullets[j]->y);
                break;
            }
        }
        plane->gun_num = ENE_GUN_STEP;
    } 
}

static void user_shoot(){
     // 用户发射
    air_infos->user->gun_num--;
    if (air_infos->user->gun_num <= 0)
    {
        for (int j = 0; j < MAX_BULLET; j++)
        {
            if (air_infos->bullets[j]->img == NULL)
            {
                    
                air_infos->bullets[j]->img = lv_image_create(air_infos->op_page);
                lv_image_set_src(air_infos->bullets[j]->img,AIR_STR_TO_SRC(air_gun));
                air_infos->bullets[j]->x = air_infos->user->x + 24;
                air_infos->bullets[j]->y = air_infos->user->y - 40;
                air_infos->bullets[j]->speed = GUN_SPEED * -1;
                air_infos->bullets[j]->alive = true;
                lv_obj_set_pos(air_infos->bullets[j]->img,air_infos->bullets[j]->x,air_infos->bullets[j]->y);
                break;
            }
        }
        air_infos->user->gun_num = USER_GUN_STEP;
    } 
}

static void game_to_fail(void){
    strcpy(air_infos->game_state,"fail");
    air_infos->flo_down = GAME_FLO_DOWN_NUM;
    if (air_infos->flo_img!=NULL)
    {
        lv_obj_delete(air_infos->flo_img);
    }
    air_infos->flo_img = create_empty_page(air_infos->root_page);
    lv_obj_set_style_bg_color(air_infos->flo_img,lv_color_hex(0x000000),0);
    lv_obj_set_style_bg_opa(air_infos->flo_img,160,0);

    air_infos->flo_cen_img = lv_image_create(air_infos->flo_img);
    lv_image_set_src(air_infos->flo_cen_img,AIR_STR_TO_SRC(air_fail));
    lv_obj_center(air_infos->flo_cen_img);
}

static void game_to_next(void){
    strcpy(air_infos->game_state,"next");
    air_infos->flo_down = GAME_FLO_DOWN_NUM;
    if (air_infos->flo_img!=NULL)
    {
        lv_obj_delete(air_infos->flo_img);
    }
    air_infos->flo_img = create_empty_page(air_infos->root_page);
    lv_obj_set_style_bg_color(air_infos->flo_img,lv_color_hex(0x000000),0);
    lv_obj_set_style_bg_opa(air_infos->flo_img,160,0);

    air_infos->flo_cen_img = lv_image_create(air_infos->flo_img);
    lv_image_set_src(air_infos->flo_cen_img,AIR_STR_TO_SRC(air_next));
    lv_obj_center(air_infos->flo_cen_img);
}

static void game_to_boss(void){
    strcpy(air_infos->game_state,"toboss");
    air_infos->flo_down = GAME_FLO_DOWN_NUM;
    if (air_infos->flo_img!=NULL)
    {
        lv_obj_delete(air_infos->flo_img);
    }
    air_infos->flo_img = create_empty_page(air_infos->root_page);
    lv_obj_set_style_bg_color(air_infos->flo_img,lv_color_hex(0x000000),0);
    lv_obj_set_style_bg_opa(air_infos->flo_img,160,0);

    air_infos->flo_cen_img = lv_image_create(air_infos->flo_img);
    lv_image_set_src(air_infos->flo_cen_img,AIR_STR_TO_SRC(air_Incoming));
    lv_obj_center(air_infos->flo_cen_img);
    
}

static void game_sc_fresh(int sc){
    int temp = sc;
    int index = 0;
    while (temp>0)
    {
        int cur_sc = temp % 10;
        temp = temp / 10;
        lv_image_set_src(air_infos->sc_imgs[index++] , get_game_sc_img(cur_sc));
    }
}

static void startbak_anim_ready_cb(lv_anim_t *a){
    lv_anim_delete(air_infos->user->img,tostart_anim_cb);
    lv_timer_set_cb(air_infos->game_timer,air_game_timer_unstart_cb);
    lv_obj_set_pos(air_infos->user->img,90,80);
    strcpy(air_infos->page_state,"index");
}

static void tostart_anim_ready_cb(lv_anim_t *a){
    lv_anim_delete(air_infos->user->img,tostart_anim_cb);

    lv_obj_delete(air_infos->user->img);
    air_infos->user->img = lv_image_create(air_infos->op_page);
    lv_image_set_src(air_infos->user->img,AIR_STR_TO_SRC(air_user));
    lv_obj_set_pos(air_infos->user->img,185,286);

    for (int i = 0; i < 4; i++){
        lv_image_set_src(air_infos->sc_imgs[i],AIR_STR_TO_SRC(air_number_0));
        lv_obj_move_foreground(air_infos->sc_imgs[i]);
    }
    lv_obj_move_foreground(air_infos->health_img);

    air_infos->score=0;
    air_infos->user->x=185;
    air_infos->user->y=286;
    air_infos->user->radius=88;
    air_infos->user->alive=true;
    air_infos->user->health=5;

    air_infos->level = 1;
    air_infos->boss_ready_num = GAME_BOSS_READY;

    lv_image_set_src(air_infos->health_img,AIR_STR_TO_SRC(air_health_5));

    strcpy(air_infos->page_state,"game");
    strcpy(air_infos->game_state,"start");
    lv_timer_set_cb(air_infos->game_timer,air_game_timer_started_cb);
}

static void tostart_anim_cb(void *, int32_t v){
    lv_obj_set_style_transform_scale(air_infos->user->img,v,0);
    lv_obj_set_y(air_infos->user->img,106+((255-v)/2));
    lv_obj_set_y(air_infos->title_img,20-(255-v));
    lv_obj_set_y(air_infos->start_img,340+(255-v));
    for (int i = 0; i < 4; i++){
        lv_obj_set_y(air_infos->sc_imgs[i],(255-v)-165);
    }
    lv_obj_set_y(air_infos->health_img,(255-v)-135);
}

static void start_img_click(lv_event_t *e){
    LV_LOG_USER("start_img_click click");
    strcpy(air_infos->game_state,"tostart");
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, air_infos->user->img);
    lv_anim_set_values(&a, 255, 80);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_exec_cb(&a, tostart_anim_cb);
    lv_anim_set_ready_cb(&a,tostart_anim_ready_cb);
    lv_anim_set_path_cb(&a,lv_anim_path_ease_in);
    lv_anim_start(&a);
}

static void air_game_timer_unstart_cb(lv_timer_t *timer){
    if (strcmp(air_infos->game_state,"unstart")==0)
    {
        if (air_infos->start_up)
        {
            air_infos->start_y--;
            if (air_infos->start_y<=335)
            {
                air_infos->start_up = false;
            }
            
        }else{
            air_infos->start_y++;
            if (air_infos->start_y>=345)
            {
                air_infos->start_up = true;
            }
        }
        lv_obj_set_y(air_infos->start_img,air_infos->start_y);
        
    }else if (strcmp(air_infos->game_state,"tostart")==0)
    {
        /*code*/
    }
    
}

static void air_game_again_event_cb(lv_event_t * e){
    LV_LOG_USER("air_game_again_event_cb click");
    game_component_reset();

    lv_obj_set_pos(air_infos->user->img,185,286);

    for (int i = 0; i < 4; i++){
        lv_image_set_src(air_infos->sc_imgs[i],AIR_STR_TO_SRC(air_number_0));
        lv_obj_move_foreground(air_infos->sc_imgs[i]);
    }
    lv_obj_move_foreground(air_infos->health_img);

    air_infos->score=0;
    air_infos->user->x=185;
    air_infos->user->y=286;
    air_infos->user->radius=88;
    air_infos->user->alive=true;
    air_infos->user->health=5;

    air_infos->level = 1;
    air_infos->boss_ready_num = GAME_BOSS_READY;

    lv_image_set_src(air_infos->health_img,AIR_STR_TO_SRC(air_health_5));

    strcpy(air_infos->page_state,"game");
    strcpy(air_infos->game_state,"start");
    // lv_timer_set_cb(air_infos->game_timer,air_game_timer_started_cb);
}

static void game_component_reset(void){
    if (air_infos->flo_img!=NULL)
    {
        lv_obj_delete(air_infos->flo_img);
        air_infos->flo_img=NULL;
        air_infos->flo_cen_img=NULL;
    }

    if (air_infos->over_flo!=NULL)
    {
        lv_obj_delete(air_infos->over_flo);
        air_infos->over_flo=NULL;
    }

    for(int i=0;i<3;i++){
        if (air_infos->enemys[i]->img != NULL)
        {
            lv_obj_delete(air_infos->enemys[i]->img);
            air_infos->enemys[i]->img = NULL;
        }
        if (air_infos->enemys[i]->explo_img != NULL)
        {
            lv_obj_delete(air_infos->enemys[i]->explo_img);
            air_infos->enemys[i]->explo_img = NULL;
        }
    }

    for (int j = 0; j < MAX_BULLET; j++)
    {
        if (air_infos->bullets[j]->img != NULL)
        {
            lv_obj_delete(air_infos->bullets[j]->img);
            air_infos->bullets[j]->img=NULL;
        }
    }

    if (air_infos->boss->img != NULL)
    {
        lv_obj_delete(air_infos->boss->img);
        air_infos->boss->img = NULL;
    }
    if (air_infos->boss->explo_img != NULL)
    {
        lv_obj_delete(air_infos->boss->explo_img);
        air_infos->boss->explo_img = NULL;
    }
}

static void air_game_quit_event_cb(lv_event_t * e){
    LV_LOG_USER("air_game_quit_event_cb click");
    lv_timer_delete(air_infos->game_timer);
    lv_obj_delete(air_infos->root_page);
    free(air_infos);
}

// 右滑回调函数
static void air_page_gesture_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t *indev = lv_event_get_indev(e);

    if (!indev) return;

    lv_point_t point;
    lv_indev_get_vect(indev, &point);

    switch (code) {
        case LV_EVENT_PRESSING:
            if (!air_infos->is_right_swipe)
            {
                if (point.x>=40)
                {
                    air_infos->is_right_swipe = true;
                }else if (strcmp(air_infos->game_state,"start")==0 || strcmp(air_infos->game_state,"boss")==0)
                {
                    air_infos->user->x+=point.x;
                    air_infos->user->y+=point.y;
                    lv_obj_set_pos(air_infos->user->img,air_infos->user->x,air_infos->user->y);
                }
            }
            break;

        case LV_EVENT_RELEASED:
            if (air_infos->is_right_swipe)
            {
                LV_LOG_USER("cont_right_swipe");
                air_infos->is_right_swipe = false;

                if (strcmp(air_infos->page_state,"game")==0){
                    game_component_reset();

                    lv_obj_delete(air_infos->user->img);
                    air_infos->user->img = lv_image_create(air_infos->op_page);
                    lv_image_set_src(air_infos->user->img,AIR_STR_TO_SRC(air_big_plane));
                    lv_obj_set_pos(air_infos->user->img,90,80);

                    lv_obj_set_style_transform_pivot_x(air_infos->user->img,136,0);
                    lv_obj_set_style_transform_pivot_y(air_infos->user->img,142,0); 
                    lv_obj_set_style_transform_scale(air_infos->user->img,80,0);

                    lv_anim_t a;
                    lv_anim_init(&a);
                    lv_anim_set_var(&a, air_infos->user->img);
                    lv_anim_set_values(&a, 80, 255);
                    lv_anim_set_time(&a, 1000);
                    lv_anim_set_exec_cb(&a, tostart_anim_cb);
                    lv_anim_set_ready_cb(&a,startbak_anim_ready_cb);
                    lv_anim_set_path_cb(&a,lv_anim_path_ease_in_out);
                    strcpy(air_infos->game_state,"unstart");
                    lv_anim_start(&a);
                    
                }else if (strcmp(air_infos->page_state,"index")==0){
                    lv_timer_delete(air_infos->game_timer);
                    lv_obj_delete(air_infos->root_page);
                    free(air_infos);
                }
            }
            break;

        default:
            break;
    }
}

static lv_obj_t * create_empty_page(lv_obj_t * root){
    lv_obj_t * page = lv_obj_create(root);
    lv_obj_set_size(page,455,455);
    lv_obj_set_style_radius(page,lv_pct(50),0);
    lv_obj_set_style_bg_color(page,lv_color_hex(0x000000),0);

    lv_obj_set_style_pad_all(page,0,0);
    lv_obj_set_style_outline_width(page,0,0);
    lv_obj_set_style_border_width(page,0,0);

    lv_obj_set_style_clip_corner(page,true,0);
    lv_obj_set_scrollbar_mode(page,LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(page,LV_DIR_NONE);

    lv_obj_center(page);

    return page;
}

static lv_image_dsc_t const *get_game_sc_img(int num){
    switch (num)
    {
    case 0:
        return AIR_STR_TO_SRC(air_number_0);
    case 1:
        return AIR_STR_TO_SRC(air_number_1);
    case 2:
        return AIR_STR_TO_SRC(air_number_2);
    case 3:
        return AIR_STR_TO_SRC(air_number_3);
    case 4:
        return AIR_STR_TO_SRC(air_number_4);
    case 5:
        return AIR_STR_TO_SRC(air_number_5);
    case 6:
        return AIR_STR_TO_SRC(air_number_6);
    case 7:
        return AIR_STR_TO_SRC(air_number_7);
    case 8:
        return AIR_STR_TO_SRC(air_number_8);
    case 9:
        return AIR_STR_TO_SRC(air_number_9);
    
    default:
        return AIR_STR_TO_SRC(air_number_0);
    }
}

static lv_image_dsc_t const *get_ene_explo_img(int num){
    switch (num)
    {
    case 0:
        return AIR_STR_TO_SRC(air_explode_small_0);
    case 1:
        return AIR_STR_TO_SRC(air_explode_small_1);
    case 2:
        return AIR_STR_TO_SRC(air_explode_small_2);
    case 3:
        return AIR_STR_TO_SRC(air_explode_small_3);
    case 4:
        return AIR_STR_TO_SRC(air_explode_small_4);
    case 5:
        return AIR_STR_TO_SRC(air_explode_small_5);
    case 6:
        return AIR_STR_TO_SRC(air_explode_small_6);
    
    default:
        return AIR_STR_TO_SRC(air_explode_small_6);
    }
}

static lv_image_dsc_t const *get_boss_explo_img(int num){
    switch (num)
    {
    case 0:
        return AIR_STR_TO_SRC(air_explode_big_0);
    case 1:
        return AIR_STR_TO_SRC(air_explode_big_1);
    case 2:
        return AIR_STR_TO_SRC(air_explode_big_2);
    case 3:
        return AIR_STR_TO_SRC(air_explode_big_3);
    case 4:
        return AIR_STR_TO_SRC(air_explode_big_4);
    case 5:
        return AIR_STR_TO_SRC(air_explode_big_5);
    case 6:
        return AIR_STR_TO_SRC(air_explode_big_6);
    
    default:
        return AIR_STR_TO_SRC(air_explode_big_6);
    }
}

static lv_image_dsc_t const *get_health_img(int health){
    switch (health)
    {
    case 0:
        return AIR_STR_TO_SRC(air_health_0);
    case 1:
        return AIR_STR_TO_SRC(air_health_1);
    case 2:
        return AIR_STR_TO_SRC(air_health_2);
    case 3:
        return AIR_STR_TO_SRC(air_health_3);
    case 4:
        return AIR_STR_TO_SRC(air_health_4);
    case 5:
        return AIR_STR_TO_SRC(air_health_5);
    
    default:
        return AIR_STR_TO_SRC(air_health_0);
    }
}

static Air_PageInfo_t* createAndInitAirPageInfo() {
    Air_PageInfo_t *pageInfo = (Air_PageInfo_t *)malloc(sizeof(Air_PageInfo_t));
    if (pageInfo == NULL) {
        perror("malloc for Air_PageInfo_t");
        return NULL;
    }

    // 初始化基本成员
    pageInfo->root_page = NULL;
    pageInfo->op_page = NULL;
    pageInfo->title_img = NULL;
    pageInfo->start_img = NULL;
    pageInfo->game_timer = NULL;
    for (int i = 0; i < 4; i++) {
        pageInfo->sc_imgs[i] = NULL;
    }
    pageInfo->health_img=NULL;
    pageInfo->score = 0;
    pageInfo->flo_img = NULL;
    pageInfo->flo_down = 0;
    pageInfo->flo_cen_img = NULL;
    pageInfo->over_flo = NULL;
    strcpy(pageInfo->page_state, "index");
    strcpy(pageInfo->game_state, "unstart");
    pageInfo->start_up = false;
    pageInfo->start_y = 340;
    pageInfo->started = false;

    // 初始化Plane_t
    pageInfo->user = (Plane_t *)malloc(sizeof(Plane_t));
    if (pageInfo->user == NULL) {
        perror("malloc for Plane_t");
        free(pageInfo);
        return NULL;
    }
    pageInfo->user->img = NULL;
    pageInfo->user->x = 0;
    pageInfo->user->y = 0;
    pageInfo->user->radius = 0;
    pageInfo->user->alive = true;
    pageInfo->user->health = 0;
    pageInfo->user->gun_num = 0;

    // 初始化Bullet_t数组
    for (int i = 0; i < MAX_BULLET; i++) {
        pageInfo->bullets[i] = (Bullet_t *)malloc(sizeof(Bullet_t));
        if (pageInfo->bullets[i] == NULL) {
            perror("malloc for Bullet_t");
            // 释放之前分配的内存
            for (int j = 0; j < i; j++) {
                free(pageInfo->bullets[j]);
            }
            free(pageInfo->user);
            free(pageInfo);
            return NULL;
        }
        pageInfo->bullets[i]->img = NULL;
        pageInfo->bullets[i]->x = 0;
        pageInfo->bullets[i]->y = 0;
        pageInfo->bullets[i]->speed = 0;
        pageInfo->bullets[i]->alive = false;
    }

    // 初始化Enemy_t数组
    for (int i = 0; i < 3; i++) {
        pageInfo->enemys[i] = (Enemy_t *)malloc(sizeof(Enemy_t));
        if (pageInfo->enemys[i] == NULL) {
            perror("malloc for Enemy_t");
            // 释放之前分配的内存
            for (int j = 0; j < i; j++) {
                free(pageInfo->enemys[j]);
            }
            for (int j = 0; j < MAX_BULLET; j++) {
                free(pageInfo->bullets[j]);
            }
            free(pageInfo->user);
            free(pageInfo);
            return NULL;
        }
        pageInfo->enemys[i]->img = NULL;
        pageInfo->enemys[i]->explo_img = NULL;
        pageInfo->enemys[i]->x = 0;
        pageInfo->enemys[i]->y = 0;
        pageInfo->enemys[i]->health = 100;
        pageInfo->enemys[i]->radius = 0;
        pageInfo->enemys[i]->speed = 0;
        pageInfo->enemys[i]->arm_x = 0;
        pageInfo->enemys[i]->arm_y = 0;
        pageInfo->enemys[i]->alive = true;
        pageInfo->enemys[i]->gun_num = 10;
    }

    // 初始化boss
    pageInfo->boss = (Enemy_t *)malloc(sizeof(Enemy_t));
    if (pageInfo->boss == NULL) {
        perror("malloc for boss");
        // 释放之前分配的内存
        for (int i = 0; i < 3; i++) {
            free(pageInfo->enemys[i]);
        }
        for (int i = 0; i < MAX_BULLET; i++) {
            free(pageInfo->bullets[i]);
        }
        free(pageInfo->user);
        free(pageInfo);
        return NULL;
    }
    pageInfo->boss->img = NULL;
    pageInfo->boss->explo_img = NULL;
    pageInfo->boss->x = 0;
    pageInfo->boss->y = 0;
    pageInfo->boss->health = 500;
    pageInfo->boss->radius = 0;
    pageInfo->boss->speed = 0;
    pageInfo->boss->arm_x = 0;
    pageInfo->boss->arm_y = 0;
    pageInfo->boss->alive = true;
    pageInfo->boss->gun_num = 10;

    pageInfo->is_right_swipe = false;
    pageInfo->ene_create_step = 10;

    pageInfo->level = 1;
    pageInfo->boss_ready_num = GAME_BOSS_READY;

    return pageInfo;
}
