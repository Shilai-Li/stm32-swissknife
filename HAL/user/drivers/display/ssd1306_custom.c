#include "../ssd1306_custom.h"
#include <string.h>

void ssd1306_DrawBitmap_Custom(uint8_t x, uint8_t y, const uint8_t *font, uint8_t w, uint8_t h, SSD1306_COLOR color) {
    uint8_t page, col, bit;

    uint8_t pages = h / 8; // Calculate pages (16px->2, 24px->3)
    if (h % 8 != 0) pages++; // Handle non-8x height

    for (page = 0; page < pages; page++) {
        for (col = 0; col < w; col++) {
            uint8_t byte = font[page * w + col]; // Get byte from font data

            for (bit = 0; bit < 8; bit++) {
                if ((page * 8 + bit) >= h) break; // Avoid exceeding char height

                if (byte & (1 << bit)) {
                    ssd1306_DrawPixel(x + col, y + page * 8 + bit, color); // Draw pixel if bit set
                }
            }
        }
    }
}

static uint8_t get_utf8_len(char c) {
    if ((c & 0x80) == 0) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1; // Fallback
}

void ssd1306_DrawString_Custom(uint8_t x, uint8_t y, const char *str, const SSD1306_ChineseFont_t *font, SSD1306_COLOR color) {
    const char *p = str;
    while (*p) {
        uint8_t len = get_utf8_len(*p);
        uint8_t found = 0;
        
        for (uint16_t i = 0; i < font->char_count; i++) {
            if (strncmp(p, font->chars[i].character, len) == 0 && font->chars[i].character[len] == '\0') {
                ssd1306_DrawBitmap_Custom(x, y, font->chars[i].data, font->chars[i].width, font->chars[i].height, color);
                x += font->chars[i].width;
                found = 1;
                break;
            }
        }
        
        // If not found, we could skip or handle ASCII if we had a standard font fallback
        // For now, we just skip the bytes of this character
        p += len;
    }
}

void ssd1306_TestChineseDisplay(void)
{
    ssd1306_Init();

    ssd1306_Fill(Black);

    SSD1306_Font_t font = Font_7x10;

    ssd1306_SetCursor(0, 0);

    ssd1306_WriteString("24x24 Test:", font, White);

    ssd1306_SetCursor(0, 12);

    ssd1306_DrawString_Custom(0, 24, "你好，世界！", &SSD1306_ChineseFont_24x24, White);

    ssd1306_UpdateScreen();
}