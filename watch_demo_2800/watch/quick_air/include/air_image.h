#ifndef AIR_IMAGE_H
#define AIR_IMAGE_H

#include <lvgl/lvgl.h>

#define AIR_DECLARE_IMAGE(image_name) \
    LV_IMAGE_DECLARE(image_name); \
    lv_image_dsc_t const *get_img_##image_name(void){ \
        return &image_name; \
    }

#define AIR_STR_TO_SRC(func_name) (lv_image_dsc_t const *) get_img_##func_name()

AIR_DECLARE_IMAGE(air_again);
AIR_DECLARE_IMAGE(air_bg);
AIR_DECLARE_IMAGE(air_big_plane);
AIR_DECLARE_IMAGE(air_boss);
AIR_DECLARE_IMAGE(air_clear);
AIR_DECLARE_IMAGE(air_enemy);
AIR_DECLARE_IMAGE(air_explode_big_0);
AIR_DECLARE_IMAGE(air_explode_big_1);
AIR_DECLARE_IMAGE(air_explode_big_2);
AIR_DECLARE_IMAGE(air_explode_big_3);
AIR_DECLARE_IMAGE(air_explode_big_4);
AIR_DECLARE_IMAGE(air_explode_big_5);
AIR_DECLARE_IMAGE(air_explode_big_6);
AIR_DECLARE_IMAGE(air_explode_small_0);
AIR_DECLARE_IMAGE(air_explode_small_1);
AIR_DECLARE_IMAGE(air_explode_small_2);
AIR_DECLARE_IMAGE(air_explode_small_3);
AIR_DECLARE_IMAGE(air_explode_small_4);
AIR_DECLARE_IMAGE(air_explode_small_5);
AIR_DECLARE_IMAGE(air_explode_small_6);
AIR_DECLARE_IMAGE(air_fail);
AIR_DECLARE_IMAGE(air_gun);
AIR_DECLARE_IMAGE(air_gun_1);
AIR_DECLARE_IMAGE(air_gun_2);
AIR_DECLARE_IMAGE(air_health_0);
AIR_DECLARE_IMAGE(air_health_1);
AIR_DECLARE_IMAGE(air_health_2);
AIR_DECLARE_IMAGE(air_health_3);
AIR_DECLARE_IMAGE(air_health_4);
AIR_DECLARE_IMAGE(air_health_5);
AIR_DECLARE_IMAGE(air_Incoming);
AIR_DECLARE_IMAGE(air_next);
AIR_DECLARE_IMAGE(air_number_0);
AIR_DECLARE_IMAGE(air_number_1);
AIR_DECLARE_IMAGE(air_number_2);
AIR_DECLARE_IMAGE(air_number_3);
AIR_DECLARE_IMAGE(air_number_4);
AIR_DECLARE_IMAGE(air_number_5);
AIR_DECLARE_IMAGE(air_number_6);
AIR_DECLARE_IMAGE(air_number_7);
AIR_DECLARE_IMAGE(air_number_8);
AIR_DECLARE_IMAGE(air_number_9);
AIR_DECLARE_IMAGE(air_quit);
AIR_DECLARE_IMAGE(air_sar);
AIR_DECLARE_IMAGE(air_start);
AIR_DECLARE_IMAGE(air_user);



#endif