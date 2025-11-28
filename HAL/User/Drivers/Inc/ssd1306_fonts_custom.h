#ifndef __SSD1306_FONTS_CUSTOM_H__
#define __SSD1306_FONTS_CUSTOM_H__

#include "ssd1306_fonts.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* character;
    const uint8_t width;
    const uint8_t height;
    const uint8_t* data;
} SSD1306_ChineseChar_t;

typedef struct {
    const uint8_t width;
    const uint8_t height;
    const SSD1306_ChineseChar_t* chars;
    const uint16_t char_count;
} SSD1306_ChineseFont_t;

extern const uint8_t chinese_char_24x24_e4bda0[];
extern const uint8_t chinese_char_24x24_e5a5bd[];
extern const uint8_t chinese_char_24x24_e4b896[];
extern const uint8_t chinese_char_24x24_e7958c[];

extern const SSD1306_ChineseFont_t SSD1306_ChineseFont_24x24;

#ifdef __cplusplus
}
#endif

#endif //__SSD1306_FONTS_CUSTOM_H__