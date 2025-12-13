#ifndef PROJECT_SSD1306_CUSTOM_H
#define PROJECT_SSD1306_CUSTOM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "afiskon-stm32-ssd1306/ssd1306.h"
#include "ssd1306_fonts_custom.h"

void ssd1306_DrawBitmap_Custom(uint8_t x, uint8_t y, const uint8_t *font, uint8_t w, uint8_t h, SSD1306_COLOR color);
void ssd1306_DrawString_Custom(uint8_t x, uint8_t y, const char *str, const SSD1306_ChineseFont_t *font, SSD1306_COLOR color);
void ssd1306_TestChineseDisplay(void);

#ifdef __cplusplus
}
#endif

#endif //PROJECT_SSD1306_CUSTOM_H