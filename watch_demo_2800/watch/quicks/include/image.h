#ifndef IMAGE_H
#define IMAGE_H

#include <lvgl/lvgl.h>

typedef struct {
    const char* key;
    const char* img_src;
} resource_img_t;

LV_IMAGE_DECLARE(bg);
LV_IMAGE_DECLARE(dy0);
LV_IMAGE_DECLARE(dy1);
LV_IMAGE_DECLARE(dy2);
LV_IMAGE_DECLARE(dy3);
LV_IMAGE_DECLARE(dy4);
LV_IMAGE_DECLARE(dy5);
LV_IMAGE_DECLARE(reload);
LV_IMAGE_DECLARE(start);
LV_IMAGE_DECLARE(title);
LV_IMAGE_DECLARE(tag0);
LV_IMAGE_DECLARE(tag1);
LV_IMAGE_DECLARE(tag2);


lv_image_dsc_t const *get_img_bg(void);
lv_image_dsc_t const *get_img_dy0(void);
lv_image_dsc_t const *get_img_dy1(void);
lv_image_dsc_t const *get_img_dy2(void);
lv_image_dsc_t const *get_img_dy3(void);
lv_image_dsc_t const *get_img_dy4(void);
lv_image_dsc_t const *get_img_dy5(void);
lv_image_dsc_t const *get_img_reload(void);
lv_image_dsc_t const *get_img_start(void);
lv_image_dsc_t const *get_img_title(void);
lv_image_dsc_t const *get_img_tag1(void);
lv_image_dsc_t const *get_img_tag2(void);
lv_image_dsc_t const *get_img_tag3(void);


#endif