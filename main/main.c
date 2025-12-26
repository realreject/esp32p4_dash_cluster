#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_memory_utils.h"
#include "lvgl.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "bsp_board_extra.h"
#include "lv_demos.h"

#include "dash_cluster.h"
#include "high_beam.h"
#include "right_turn.h"
#include "left_turn.h"
#include "engine_start_stop.h"
#include "scale.h"
#include "needle_hub.h"
#include "needle_hub_glow.h"
#include "needle.h"

// Define colors manually
#define LV_COLOR_RED lv_color_make(0xFF, 0x00, 0x00)
#define LV_COLOR_GREEN lv_color_make(0x00, 0xFF, 0x00)
#define LV_COLOR_BLUE lv_color_make(0x00, 0x00, 0xFF)
#define LV_COLOR_WHITE lv_color_make(0xFF, 0xFF, 0xFF)
#define LV_COLOR_BLACK lv_color_make(0x00, 0x00, 0x00)

// GLOBAL COMPONENTS
lv_obj_t *main_scr;
lv_obj_t *current_img;
lv_obj_t *dash_cluster_scr;
lv_obj_t *tach_scale;
lv_obj_t *tach_hub;
lv_obj_t *tach_hub_glow;
lv_obj_t *tach_needle;
lv_obj_t *speedo_scale;
lv_obj_t *speedo_hub;
lv_obj_t *speedo_hub_glow;
lv_obj_t *speedo_needle;
lv_obj_t *high_beam_icon;
lv_obj_t *right_turn_icon;
lv_obj_t *left_turn_icon;
lv_obj_t *ign_btn;
lv_obj_t *ign_btn_led;

// GLOBAL IMAGES
LV_IMG_DECLARE(dash_cluster);
LV_IMG_DECLARE(scale);
LV_IMG_DECLARE(needle_hub);
LV_IMG_DECLARE(needle_hub_glow);
LV_IMG_DECLARE(needle);
LV_IMG_DECLARE(high_beam);
LV_IMG_DECLARE(right_turn);
LV_IMG_DECLARE(left_turn);
LV_IMG_DECLARE(engine_start_stop);

bool ignition_on = false;

static void bulb_check_timer_cb(lv_timer_t *timer)
{
    /* Turn everything back to black */
    lv_obj_set_style_image_recolor(left_turn_icon, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor(right_turn_icon, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor(high_beam_icon, LV_COLOR_BLACK, 0);
    // lv_obj_set_style_bg_color(ign_btn_led, LV_COLOR_BLACK, 0);

    lv_obj_set_style_image_recolor_opa(left_turn_icon, LV_OPA_COVER, 0);
    lv_obj_set_style_image_recolor_opa(right_turn_icon, LV_OPA_COVER, 0);
    lv_obj_set_style_image_recolor_opa(high_beam_icon, LV_OPA_COVER, 0);
    // lv_obj_set_style_bg_opa(ign_btn_led, LV_OPA_COVER, 0);

    lv_timer_del(timer); // one‑shot timer
}

static void ign_btn_event_cb(lv_event_t *e)
{

    if (ignition_on == false)
    {
        ignition_on = true;

        /* Turn signals → green */
        lv_obj_set_style_image_recolor(left_turn_icon, LV_COLOR_GREEN, 0);
        lv_obj_set_style_image_recolor(right_turn_icon, LV_COLOR_GREEN, 0);
        lv_obj_set_style_image_recolor_opa(left_turn_icon, LV_OPA_COVER, 0);
        lv_obj_set_style_image_recolor_opa(right_turn_icon, LV_OPA_COVER, 0);

        /* High beam → blue */
        lv_obj_set_style_image_recolor(high_beam_icon, LV_COLOR_BLUE, 0);
        lv_obj_set_style_image_recolor_opa(high_beam_icon, LV_OPA_COVER, 0);

        // ignition button on
        lv_obj_set_style_bg_color(ign_btn_led, LV_COLOR_GREEN, 0);
        lv_obj_set_style_bg_opa(ign_btn_led, LV_OPA_COVER, 0);

        // turn on gauge scale colors
        lv_obj_set_style_image_recolor_opa(tach_scale, LV_OPA_TRANSP, 0);
        lv_obj_set_style_image_recolor_opa(speedo_scale, LV_OPA_TRANSP, 0);
        lv_obj_set_style_image_recolor_opa(tach_hub_glow, LV_OPA_TRANSP, 0);
        lv_obj_set_style_image_recolor_opa(speedo_hub_glow, LV_OPA_TRANSP, 0);
        lv_obj_set_style_image_recolor_opa(tach_hub, LV_OPA_TRANSP, 0);
        lv_obj_set_style_image_recolor_opa(speedo_hub, LV_OPA_TRANSP, 0);
        lv_obj_set_style_image_recolor_opa(tach_needle, LV_OPA_TRANSP, 0);
        lv_obj_set_style_image_recolor_opa(speedo_needle, LV_OPA_TRANSP, 0);

        /* Create a one‑shot timer to restore colors after 2 seconds */
        lv_timer_create(bulb_check_timer_cb, 2000, NULL);
    }

    else
    {   // ignition button off
        ignition_on = false;

        lv_obj_set_style_bg_color(ign_btn_led, LV_COLOR_BLACK, 0);
        lv_obj_set_style_bg_opa(ign_btn_led, LV_OPA_COVER, 0);

        lv_obj_set_style_image_recolor(tach_scale, LV_COLOR_BLACK, 0);
        lv_obj_set_style_image_recolor_opa(tach_scale, LV_OPA_COVER, 0);

        lv_obj_set_style_image_recolor(tach_hub_glow, LV_COLOR_BLACK, 0);
        lv_obj_set_style_image_recolor_opa(tach_hub_glow, LV_OPA_COVER, 0);       

        lv_obj_set_style_image_recolor(tach_hub, LV_COLOR_BLACK, 0);
        lv_obj_set_style_image_recolor_opa(tach_hub, LV_OPA_COVER, 0);

        lv_obj_set_style_image_recolor(tach_needle, LV_COLOR_BLACK, 0);
        lv_obj_set_style_image_recolor_opa(tach_needle, LV_OPA_COVER, 0);

        lv_obj_set_style_image_recolor(speedo_scale, LV_COLOR_BLACK, 0);
        lv_obj_set_style_image_recolor_opa(speedo_scale, LV_OPA_COVER, 0);

        lv_obj_set_style_image_recolor(speedo_hub_glow, LV_COLOR_BLACK, 0);
        lv_obj_set_style_image_recolor_opa(speedo_hub_glow, LV_OPA_COVER, 0);

        lv_obj_set_style_image_recolor(speedo_hub, LV_COLOR_BLACK, 0);
        lv_obj_set_style_image_recolor_opa(speedo_hub, LV_OPA_COVER, 0);

        lv_obj_set_style_image_recolor(speedo_needle, LV_COLOR_BLACK, 0);
        lv_obj_set_style_image_recolor_opa(speedo_needle, LV_OPA_COVER, 0);
    }
}

void ignition_button(void)
{
    // create the image object (initially empty)
    current_img = lv_image_create(main_scr);
    lv_obj_center(current_img);
    lv_obj_add_flag(current_img, LV_OBJ_FLAG_HIDDEN);

    // create the button
    ign_btn = lv_btn_create(main_scr);
    lv_obj_set_size(ign_btn, 180, 180);
    // lv_obj_align(ign_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(ign_btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_image_recolor(ign_btn, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor_opa(ign_btn, LV_OPA_COVER, 0);
    lv_obj_align(ign_btn, LV_ALIGN_CENTER, -250, 450);
    lv_obj_set_style_transform_width(ign_btn, 0, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_transform_height(ign_btn, 0, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_transform_zoom(ign_btn, 256, LV_PART_MAIN | LV_STATE_PRESSED);

    ign_btn_led = lv_image_create(ign_btn);
    lv_obj_set_size(ign_btn_led, 180, 180);
    lv_obj_center(ign_btn_led);
    lv_obj_set_style_radius(ign_btn_led, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(ign_btn_led, LV_COLOR_BLACK, 0);
    lv_obj_set_style_bg_opa(ign_btn_led, LV_OPA_COVER, 0);

    lv_obj_t *img = lv_image_create(ign_btn_led);
    lv_img_set_src(img, &engine_start_stop);
    lv_img_set_zoom(img, 180);
    lv_obj_center(img);

    // Attach event callback
    lv_obj_add_event_cb(ign_btn, ign_btn_event_cb, LV_EVENT_CLICKED, NULL);
}

void create_cluster()
{
    // tachometer base images
    tach_scale = lv_image_create(main_scr);
    lv_image_set_src(tach_scale, &scale);
    lv_obj_align(tach_scale, LV_ALIGN_TOP_LEFT, 247, 98);
    lv_obj_set_style_image_recolor(tach_scale, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor_opa(tach_scale, LV_OPA_COVER, 0);

    tach_hub_glow = lv_image_create(main_scr);
    lv_image_set_src(tach_hub_glow, &needle_hub_glow);
    lv_obj_align(tach_hub_glow, LV_ALIGN_TOP_LEFT, (416), (263));
    lv_obj_set_style_image_recolor(tach_hub_glow, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor_opa(tach_hub_glow, LV_OPA_COVER, 0);

    tach_hub = lv_image_create(tach_hub_glow);
    lv_image_set_src(tach_hub, &needle_hub);
    lv_obj_align(tach_hub, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_image_recolor(tach_hub, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor_opa(tach_hub, LV_OPA_COVER, 0);

    tach_needle = lv_image_create(main_scr);
    lv_image_set_src(tach_needle, &needle);
    lv_obj_align(tach_needle, LV_ALIGN_TOP_LEFT,(800-375), 319);
    lv_obj_set_style_image_recolor(tach_needle, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor_opa(tach_needle, LV_OPA_COVER, 0);

    // speedometer base images
    speedo_scale = lv_image_create(main_scr);
    lv_image_set_src(speedo_scale, &scale);
    lv_obj_align(speedo_scale, LV_ALIGN_TOP_LEFT, 247, 679);
    lv_obj_set_style_image_recolor(speedo_scale, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor_opa(speedo_scale, LV_OPA_COVER, 0);

    speedo_hub_glow = lv_image_create(main_scr);
    lv_image_set_src(speedo_hub_glow, &needle_hub_glow);
    lv_obj_align(speedo_hub_glow, LV_ALIGN_TOP_LEFT, (416), (844));
    lv_obj_set_style_image_recolor(speedo_hub_glow, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor_opa(speedo_hub_glow, LV_OPA_COVER, 0);

    speedo_hub = lv_image_create(speedo_hub_glow);
    lv_image_set_src(speedo_hub, &needle_hub);
    lv_obj_align(speedo_hub, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_image_recolor(speedo_hub, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor_opa(speedo_hub, LV_OPA_COVER, 0);

    speedo_needle = lv_image_create(main_scr);
    lv_image_set_src(speedo_needle, &needle);
    lv_obj_align(speedo_needle, LV_ALIGN_TOP_LEFT,(800-375), 900);
    lv_obj_set_style_image_recolor(speedo_needle, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor_opa(speedo_needle, LV_OPA_COVER, 0);



    // Indicator images
    high_beam_icon = lv_image_create(main_scr);
    lv_image_set_src(high_beam_icon, &high_beam);
    lv_obj_align(high_beam_icon, LV_ALIGN_CENTER, 340, 0);
    lv_obj_set_style_image_recolor(high_beam_icon, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor_opa(high_beam_icon, LV_OPA_COVER, 0);

    left_turn_icon = lv_image_create(main_scr);
    lv_image_set_src(left_turn_icon, &left_turn);
    lv_obj_align(left_turn_icon, LV_ALIGN_CENTER, 340, -100);
    lv_obj_set_style_image_recolor(left_turn_icon, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor_opa(left_turn_icon, LV_OPA_COVER, 0);

    right_turn_icon = lv_image_create(main_scr);
    lv_image_set_src(right_turn_icon, &right_turn);
    lv_obj_align(right_turn_icon, LV_ALIGN_CENTER, 340, 100);
    lv_obj_set_style_image_recolor(right_turn_icon, LV_COLOR_BLACK, 0);
    lv_obj_set_style_image_recolor_opa(right_turn_icon, LV_OPA_COVER, 0);
}

// Function to fill the screen with a specific color and clear previous content
void create_background_fill(lv_color_t color)
{
    main_scr = lv_scr_act();                                       // Get the active screen
    lv_obj_clean(main_scr);                                        // Clear the screen (removes all objects)
    lv_obj_set_style_bg_color(main_scr, color, LV_PART_MAIN);      // Set the background color
    lv_obj_set_style_bg_opa(main_scr, LV_OPA_COVER, LV_PART_MAIN); // Ensure full opacity
}

void app_main(void)
{
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
        .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
        .flags = {
            .buff_dma = true,
            .buff_spiram = false,
            .sw_rotate = false,
        }};

    bsp_display_start_with_config(&cfg);
    bsp_display_backlight_on();
    create_background_fill(LV_COLOR_BLACK);
    create_cluster();

    ignition_button();

    for (int i = 0; i < 10; i++)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    lv_scr_load(main_scr);

    // Disable scrolling for the entire screen
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLL_CHAIN_VER);

    while (true)
    {
        // Let LVGL handle rendering and input
        vTaskDelay(pdMS_TO_TICKS(1));
        lv_task_handler(); // Call LVGL's task handler periodically
    }
}
