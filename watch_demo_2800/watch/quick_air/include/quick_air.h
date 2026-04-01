#ifndef QUICK_AIR_H
#define QUICK_AIR_H

#include <lvgl/lvgl.h>

lv_obj_t * quick_air_page(void);

#define GUN_SPEED 8
#define ENE_SPEED_BASE 3
#define ENE_SPEED_INCRE 3
#define MAX_BULLET 30 

#define ENE_GUN_STEP 20
#define USER_GUN_STEP 20

#define BOSS_GUN_STEP 40

#define GAME_BOSS_READY 20
#define GAME_BOSS_SCORE 100
#define GAME_ENE_SCORE 5

#define GAME_FLO_DOWN_NUM 30


// 飞机（玩家）
typedef struct {
    lv_obj_t *img;
    int x;        
    int y;        
    int radius;   
    bool alive;       
    int health;
    int gun_num;
} Plane_t;

// 子弹
typedef struct {
    lv_obj_t *img;
    int x;
    int y;
    int speed;             
    bool alive;
} Bullet_t;

// 敌机：简化行为，固定轨迹
typedef struct {
    lv_obj_t *img;
    lv_obj_t *explo_img;
    int x;
    int y;
    int health;
    int radius;
    int speed;
    int arm_x;
    int arm_y;
    bool alive;
    int gun_num;
} Enemy_t;

typedef struct{
    lv_obj_t *root_page;
    lv_obj_t *op_page;

    lv_obj_t *title_img;
    lv_obj_t *start_img;

    lv_timer_t *game_timer;
    
    lv_obj_t *sc_imgs[4];
    lv_obj_t *health_img;
    int score;

    lv_obj_t *flo_img;   
    lv_obj_t *flo_cen_img;  
    int flo_down;

    lv_obj_t *over_flo; 

    char page_state[10];
    char game_state[10];

    bool start_up;
    int start_y;

    bool started;

    Plane_t *user;
    Bullet_t *bullets[MAX_BULLET]; // TODO img delete
    Enemy_t *enemys[3]; // TODO  img delete
    Enemy_t *boss;  // TODO img delete

    bool is_right_swipe;
    int ene_create_step;

    // new
    int level;
    int boss_ready_num;

} Air_PageInfo_t;

#endif