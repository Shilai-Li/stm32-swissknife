/**
 * @file lvgl_port.c
 * @brief LVGL Port Implementation for STM32 HAL
 *
 * This file provides the hardware abstraction layer for LVGL:
 * - Display flush callback
 * - Tick source
 * - Optional input device handling
 */

#include "lvgl_port.h"
#include <string.h>

/* ============================================================================
 *                          PRIVATE VARIABLES
 * ============================================================================ */

static lv_display_t *disp;
static uint32_t lvgl_tick_count = 0;

/* Display buffer - LVGL will render to this buffer before flushing */
static lv_color_t disp_buf1[LVGL_DISP_BUF_SIZE];

/* Optional: Double buffering for smoother rendering (uses 2x RAM) */
// static lv_color_t disp_buf2[LVGL_DISP_BUF_SIZE];

/* ============================================================================
 *                          DISPLAY DRIVER HELPERS
 * ============================================================================ */

/* GPIO Pin definitions - adjust to match your wiring */
/* These should match CubeMX configuration */
#ifndef LCD_CS_GPIO_Port
#define LCD_CS_GPIO_Port    GPIOA
#define LCD_CS_Pin          GPIO_PIN_4
#endif

#ifndef LCD_DC_GPIO_Port
#define LCD_DC_GPIO_Port    GPIOA
#define LCD_DC_Pin          GPIO_PIN_3
#endif

#ifndef LCD_RST_GPIO_Port
#define LCD_RST_GPIO_Port   GPIOA
#define LCD_RST_Pin         GPIO_PIN_2
#endif

#ifndef LCD_BLK_GPIO_Port
#define LCD_BLK_GPIO_Port   GPIOA
#define LCD_BLK_Pin         GPIO_PIN_1
#endif

/* Low-level GPIO control macros */
#define LCD_CS_LOW()    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)
#define LCD_CS_HIGH()   HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)
#define LCD_DC_LOW()    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET)
#define LCD_DC_HIGH()   HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET)
#define LCD_RST_LOW()   HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET)
#define LCD_RST_HIGH()  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET)
#define LCD_BLK_ON()    HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_SET)
#define LCD_BLK_OFF()   HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_RESET)

/* ============================================================================
 *                          ST7789 DRIVER (SPI)
 * ============================================================================ */

#ifdef LVGL_DISPLAY_ST7789

/* ST7789 Commands */
#define ST7789_NOP       0x00
#define ST7789_SWRESET   0x01
#define ST7789_SLPOUT    0x11
#define ST7789_NORON     0x13
#define ST7789_INVON     0x21
#define ST7789_DISPON    0x29
#define ST7789_CASET     0x2A
#define ST7789_RASET     0x2B
#define ST7789_RAMWR     0x2C
#define ST7789_COLMOD    0x3A
#define ST7789_MADCTL    0x36

/**
 * @brief Send command to ST7789
 */
static void ST7789_WriteCmd(uint8_t cmd)
{
    LCD_DC_LOW();
    LCD_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

/**
 * @brief Send data to ST7789
 */
static void ST7789_WriteData(uint8_t *data, uint16_t len)
{
    LCD_DC_HIGH();
    LCD_CS_LOW();
    HAL_SPI_Transmit(&hspi1, data, len, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

/**
 * @brief Send single byte data
 */
static void ST7789_WriteData8(uint8_t data)
{
    ST7789_WriteData(&data, 1);
}

/**
 * @brief Set drawing window
 */
static void ST7789_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t data[4];

    ST7789_WriteCmd(ST7789_CASET);
    data[0] = (x0 >> 8) & 0xFF;
    data[1] = x0 & 0xFF;
    data[2] = (x1 >> 8) & 0xFF;
    data[3] = x1 & 0xFF;
    ST7789_WriteData(data, 4);

    ST7789_WriteCmd(ST7789_RASET);
    data[0] = (y0 >> 8) & 0xFF;
    data[1] = y0 & 0xFF;
    data[2] = (y1 >> 8) & 0xFF;
    data[3] = y1 & 0xFF;
    ST7789_WriteData(data, 4);

    ST7789_WriteCmd(ST7789_RAMWR);
}

/**
 * @brief Initialize ST7789 display
 */
static void ST7789_Init(void)
{
    /* Hardware reset */
    LCD_RST_LOW();
    HAL_Delay(50);
    LCD_RST_HIGH();
    HAL_Delay(150);

    /* Software reset */
    ST7789_WriteCmd(ST7789_SWRESET);
    HAL_Delay(150);

    /* Exit sleep mode */
    ST7789_WriteCmd(ST7789_SLPOUT);
    HAL_Delay(120);

    /* Color mode: 16-bit RGB565 */
    ST7789_WriteCmd(ST7789_COLMOD);
    ST7789_WriteData8(0x55);  /* 16-bit color */
    HAL_Delay(10);

    /* Memory access control (orientation) */
    ST7789_WriteCmd(ST7789_MADCTL);
    ST7789_WriteData8(0x00);  /* Default orientation */

    /* Inversion on (most ST7789 modules need this) */
    ST7789_WriteCmd(ST7789_INVON);
    HAL_Delay(10);

    /* Normal display mode */
    ST7789_WriteCmd(ST7789_NORON);
    HAL_Delay(10);

    /* Display on */
    ST7789_WriteCmd(ST7789_DISPON);
    HAL_Delay(10);

    /* Turn on backlight */
    LCD_BLK_ON();
}

/**
 * @brief LVGL flush callback for ST7789
 */
static void ST7789_Flush(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);

    ST7789_SetWindow(area->x1, area->y1, area->x2, area->y2);

    /* Send pixel data */
    LCD_DC_HIGH();
    LCD_CS_LOW();
    
    /* RGB565 = 2 bytes per pixel */
    HAL_SPI_Transmit(&hspi1, px_map, size * 2, HAL_MAX_DELAY);
    
    LCD_CS_HIGH();

    /* IMPORTANT: Tell LVGL flushing is done */
    lv_display_flush_ready(display);
}

#endif /* LVGL_DISPLAY_ST7789 */

/* ============================================================================
 *                          SSD1306 DRIVER (I2C) - Monochrome
 * ============================================================================ */

#ifdef LVGL_DISPLAY_SSD1306

#define SSD1306_I2C_ADDR    0x78  /* 0x3C << 1 */

/* TODO: Implement SSD1306 driver
 * For SSD1306, you typically need a full frame buffer in RAM
 * because it doesn't support partial updates well.
 */

static void SSD1306_Init(void)
{
    /* Initialize SSD1306 via I2C */
    /* ... */
}

static void SSD1306_Flush(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    /* For monochrome: convert color data to 1-bit and send to display */
    /* ... */
    lv_display_flush_ready(display);
}

#endif /* LVGL_DISPLAY_SSD1306 */

/* ============================================================================
 *                          PUBLIC API IMPLEMENTATION
 * ============================================================================ */

/**
 * @brief Initialize LVGL and display
 */
void LVGL_Port_Init(void)
{
    /* 1. Initialize LVGL library */
    lv_init();

    /* 2. Create display */
    disp = lv_display_create(LVGL_HOR_RES, LVGL_VER_RES);

    /* 3. Set display draw buffers */
    lv_display_set_buffers(disp, disp_buf1, NULL, sizeof(disp_buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* 4. Initialize hardware display and set flush callback */
#ifdef LVGL_DISPLAY_ST7789
    ST7789_Init();
    lv_display_set_flush_cb(disp, ST7789_Flush);
#endif

#ifdef LVGL_DISPLAY_SSD1306
    SSD1306_Init();
    lv_display_set_flush_cb(disp, SSD1306_Flush);
    /* For monochrome displays */
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_I1);
#endif

    /* 5. Optional: Set default theme */
#if LV_USE_THEME_DEFAULT
    lv_theme_t *th = lv_theme_default_init(disp,
                                           lv_palette_main(LV_PALETTE_BLUE),
                                           lv_palette_main(LV_PALETTE_RED),
                                           LV_THEME_DEFAULT_DARK,
                                           LV_FONT_DEFAULT);
    lv_display_set_theme(disp, th);
#endif
}

/**
 * @brief LVGL tick handler - must be called every 1ms
 */
void LVGL_Port_Tick(void)
{
    lvgl_tick_count++;
    lv_tick_inc(1);  /* Tell LVGL that 1ms has passed */
}

/**
 * @brief LVGL main task - call in main loop
 */
void LVGL_Port_Task(void)
{
    lv_timer_handler();  /* Handle LVGL timers, rendering, etc. */
}

/**
 * @brief Set backlight brightness
 */
void LVGL_Port_SetBacklight(uint8_t level)
{
    if (level > 0) {
        LCD_BLK_ON();
    } else {
        LCD_BLK_OFF();
    }
    /* TODO: For PWM dimming, use TIM PWM output */
}

/**
 * @brief Get tick count (for debugging)
 */
uint32_t LVGL_Port_GetTick(void)
{
    return lvgl_tick_count;
}

/* ============================================================================
 *                          INPUT DEVICE PORT (Optional)
 * ============================================================================ */

/* 
 * To add touch support:
 * 1. Create an input device: lv_indev_t *indev = lv_indev_create();
 * 2. Set type: lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
 * 3. Set read callback: lv_indev_set_read_cb(indev, touch_read_cb);
 *
 * Example touch read callback:
 * 
 * static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
 *     if (XPT2046_IsTouched()) {
 *         data->point.x = XPT2046_GetX();
 *         data->point.y = XPT2046_GetY();
 *         data->state = LV_INDEV_STATE_PRESSED;
 *     } else {
 *         data->state = LV_INDEV_STATE_RELEASED;
 *     }
 * }
 */
