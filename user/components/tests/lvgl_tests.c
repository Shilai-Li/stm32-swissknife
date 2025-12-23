/**
 * @file lvgl_tests.c
 * @brief LVGL Demo Test - Shows basic LVGL usage
 *
 * This test demonstrates:
 * 1. LVGL initialization
 * 2. Creating simple UI widgets
 * 3. Handling the main loop
 */

#include "lvgl_port.h"
#include <stdio.h>

/* ============================================================================
 *                          SIMPLE HELLO WORLD DEMO
 * ============================================================================ */

static void Demo_HelloWorld(void)
{
    /* Create a label on the active screen */
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello LVGL!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -40);

    /* Style the label */
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
}

/* ============================================================================
 *                          BUTTON DEMO
 * ============================================================================ */

static void btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        static uint32_t cnt = 0;
        cnt++;
        
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Clicked: %lu", cnt);
    }
}

static void Demo_Button(void)
{
    /* Create a button */
    lv_obj_t *btn = lv_button_create(lv_screen_active());
    lv_obj_set_size(btn, 120, 50);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 20);
    lv_obj_add_event_cb(btn, btn_event_handler, LV_EVENT_ALL, NULL);

    /* Add label to button */
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Click Me!");
    lv_obj_center(label);
}

/* ============================================================================
 *                          SLIDER DEMO
 * ============================================================================ */

static lv_obj_t *slider_label;

static void slider_event_handler(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider);
    
    lv_label_set_text_fmt(slider_label, "Value: %ld", value);
}

static void Demo_Slider(void)
{
    /* Create slider */
    lv_obj_t *slider = lv_slider_create(lv_screen_active());
    lv_obj_set_width(slider, 180);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 60);
    lv_obj_add_event_cb(slider, slider_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    /* Create label to show slider value */
    slider_label = lv_label_create(lv_screen_active());
    lv_label_set_text(slider_label, "Value: 0");
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

/* ============================================================================
 *                          SPINNER (Loading) DEMO
 * ============================================================================ */

static void Demo_Spinner(void)
{
    /* Create a spinning loading indicator */
    lv_obj_t *spinner = lv_spinner_create(lv_screen_active());
    lv_obj_set_size(spinner, 40, 40);
    lv_obj_align(spinner, LV_ALIGN_TOP_RIGHT, -20, 20);
}

/* ============================================================================
 *                          LED INDICATOR DEMO
 * ============================================================================ */

static void Demo_LED(void)
{
    /* Create LED indicators */
    lv_obj_t *led_red = lv_led_create(lv_screen_active());
    lv_obj_align(led_red, LV_ALIGN_TOP_LEFT, 20, 20);
    lv_led_set_color(led_red, lv_palette_main(LV_PALETTE_RED));
    lv_led_on(led_red);

    lv_obj_t *led_green = lv_led_create(lv_screen_active());
    lv_obj_align(led_green, LV_ALIGN_TOP_LEFT, 60, 20);
    lv_led_set_color(led_green, lv_palette_main(LV_PALETTE_GREEN));
    lv_led_off(led_green);
}

/* ============================================================================
 *                          MAIN TEST ENTRY
 * ============================================================================ */

/**
 * @brief LVGL Test Entry Point
 * 
 * Call this from main() after peripheral initialization.
 * 
 * Example main.c:
 * ```c
 * #include "lvgl_port.h"
 * extern void Test_LVGL_Entry(void);
 * 
 * int main(void) {
 *     HAL_Init();
 *     SystemClock_Config();
 *     MX_GPIO_Init();
 *     MX_SPI1_Init();
 *     
 *     Test_LVGL_Entry();  // Never returns
 * }
 * 
 * // In stm32f1xx_it.c:
 * void SysTick_Handler(void) {
 *     HAL_IncTick();
 *     LVGL_Port_Tick();
 * }
 * ```
 */
void Test_LVGL_Entry(void)
{
    /* 1. Initialize LVGL and display */
    LVGL_Port_Init();

    /* 2. Create demo UI */
    Demo_HelloWorld();
    Demo_Button();
    Demo_Slider();
    Demo_Spinner();
    Demo_LED();

    /* 3. Main loop - MUST call LVGL_Port_Task() repeatedly */
    while (1) {
        LVGL_Port_Task();
        
        /* Other application code can go here */
        /* Note: LVGL_Port_Task() is non-blocking */
        
        HAL_Delay(5);  /* Small delay to prevent busy-loop */
    }
}

/* ============================================================================
 *                          MINIMAL DEMO (单独测试用)
 * ============================================================================ */

/**
 * @brief Minimal LVGL test - just show "Hello"
 */
void Test_LVGL_Minimal(void)
{
    LVGL_Port_Init();

    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "LVGL OK!");
    lv_obj_center(label);

    while (1) {
        LVGL_Port_Task();
        HAL_Delay(5);
    }
}
