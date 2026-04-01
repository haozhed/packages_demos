#include "../include/quicks_main.h"
#include "../include/quicks_utils.h"

#include "../include/quicks_rps.h"
#include "../../utils/include/circle_screen.h"

lv_obj_t * root;

lv_obj_t * create_main_page(void);

lv_obj_t * create_main_page(void){
    root = lv_obj_create(get_watch_scr());
    lv_obj_set_size(root, LV_CIRCLE_WATCH, LV_CIRCLE_WATCH);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_scroll_dir(root,LV_DIR_NONE);
    lv_obj_set_style_radius(root, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(root, 0, 0);

    // TODO
    create_rps_page(root);

    return root;
}