/**
 * @file lvgl_port.h
 * @brief LVGL Port for STM32 HAL
 * @version 1.0
 *
 * ============================================================================
 *                          CUBEMX CONFIGURATION GUIDE
 * ============================================================================
 *
 * 1. SYSTEM TICK (Required):
 *    SYS -> Timebase Source: SysTick
 *    - LVGL_Port_Tick() must be called every 1ms (in SysTick_Handler or TIM)
 *
 * 2. DISPLAY INTERFACE - Choose based on your screen:
 *
 *    A) SPI Display (ST7789, ILI9341, ST7735):
 *       - Enable SPI1 (or SPI2)
 *       - Mode: Transmit Only Master
 *       - Data Size: 8 Bits
 *       - Prescaler: SPI_BAUDRATEPRESCALER_2 (最快速度)
 *       - Configure GPIO:
 *         - CS:  PA4 (Output Push-Pull)
 *         - DC:  PA3 (Output Push-Pull)
 *         - RST: PA2 (Output Push-Pull)
 *         - BLK: PA1 (Output Push-Pull, optional backlight)
 *
 *    B) I2C Display (SSD1306):
 *       - Enable I2C1
 *       - Speed Mode: Fast Mode (400kHz)
 *       - SSD1306 I2C Address: 0x3C (7-bit) or 0x78 (8-bit)
 *
 *    C) Parallel 8080 Display (ILI9341 parallel):
 *       - Configure FSMC/FMC or GPIO bit-banging
 *
 * 3. OPTIONAL - Touch Panel:
 *    - XPT2046 (SPI touch): Enable SPI2
 *    - GT911/FT6336 (I2C touch): Enable I2C2
 *
 * ============================================================================
 *                          INTEGRATION STEPS
 * ============================================================================
 *
 * 1. In main.c, include and call:
 *    ```c
 *    #include "lvgl_port.h"
 *
 *    int main(void) {
 *        HAL_Init();
 *        SystemClock_Config();
 *        MX_GPIO_Init();
 *        MX_SPI1_Init();  // or MX_I2C1_Init()
 *
 *        LVGL_Port_Init();  // Initialize LVGL and display
 *
 *        while (1) {
 *            LVGL_Port_Task();  // Call in main loop (handles rendering)
 *            // Your other code...
 *        }
 *    }
 *    ```
 *
 * 2. In stm32f1xx_it.c (or your timer ISR):
 *    ```c
 *    #include "lvgl_port.h"
 *
 *    void SysTick_Handler(void) {
 *        HAL_IncTick();
 *        LVGL_Port_Tick();  // Call every 1ms
 *    }
 *    ```
 *
 * 3. Create your UI after LVGL_Port_Init():
 *    ```c
 *    lv_obj_t *label = lv_label_create(lv_screen_active());
 *    lv_label_set_text(label, "Hello LVGL!");
 *    lv_obj_center(label);
 *    ```
 *
 * ============================================================================
 */

#ifndef LVGL_PORT_H
#define LVGL_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "lvgl.h"

/* ============================================================================
 *                          DISPLAY CONFIGURATION
 * ============================================================================ */

/**
 * Select your display type (uncomment ONE)
 */
#define LVGL_DISPLAY_ST7789      1   /* 240x240 or 240x320 SPI Color LCD */
// #define LVGL_DISPLAY_ILI9341     1   /* 320x240 SPI Color LCD */
// #define LVGL_DISPLAY_ST7735      1   /* 128x160 SPI Color LCD */
// #define LVGL_DISPLAY_SSD1306     1   /* 128x64 I2C Monochrome OLED */

/**
 * Display resolution
 */
#define LVGL_HOR_RES     240
#define LVGL_VER_RES     240

/**
 * Display buffer size (in pixels)
 * Larger = smoother, but uses more RAM
 * Minimum: 10 lines (LVGL_HOR_RES * 10)
 */
#define LVGL_DISP_BUF_SIZE    (LVGL_HOR_RES * 20)

/* ============================================================================
 *                          HARDWARE HANDLES
 * ============================================================================ */

/* Define which SPI/I2C to use */
extern SPI_HandleTypeDef hspi1;
// extern I2C_HandleTypeDef hi2c1;

/* ============================================================================
 *                          API FUNCTIONS
 * ============================================================================ */

/**
 * @brief Initialize LVGL library and display driver
 * @note  Call this once at startup after HAL/peripheral init
 */
void LVGL_Port_Init(void);

/**
 * @brief LVGL tick handler - call every 1ms
 * @note  Call from SysTick_Handler() or a hardware timer ISR
 */
void LVGL_Port_Tick(void);

/**
 * @brief LVGL main task - call in main loop
 * @note  Handles rendering and internal timers
 *        Call this as often as possible (non-blocking)
 */
void LVGL_Port_Task(void);

/**
 * @brief Set display backlight level
 * @param level 0-100 (0=off, 100=full brightness)
 */
void LVGL_Port_SetBacklight(uint8_t level);

/**
 * @brief Get current tick count (for LVGL internal use)
 */
uint32_t LVGL_Port_GetTick(void);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_PORT_H */
