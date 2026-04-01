#include "../include/quicks_utils.h"

const int PAGE_SIZE = 455;

lv_obj_t * create_empty_page(lv_obj_t * root){
    lv_obj_t * page = lv_obj_create(root);
    lv_obj_set_size(page,PAGE_SIZE,PAGE_SIZE);
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