/**
 * @file ds18b20.c
 * @brief DS18B20 Driver Implementation
 */

#include "ds18b20.h"
#include "delay.h" // Must provide Delay_us()

/* 1-Wire Timing Constants */
// Adjust these if cable is very long or capacitance is high
#define OW_RESET_PULSE      480
#define OW_RESET_WAIT       70
#define OW_PRESENCE_WAIT    410

// Bit banging timing
#define OW_WRITE_0_LOW      60
#define OW_WRITE_0_HIGH     10
#define OW_WRITE_1_LOW      6
#define OW_WRITE_1_HIGH     64

#define OW_READ_LOW         6
#define OW_READ_SAMPLE      9
#define OW_READ_RECOVER     55

/* GPIO Helper Functions */

// Helper: Set Pin as Output
static void Mode_Output(DS18B20_Handle_t *h) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = h->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Open Drain usually better but PP works if strictly timing controlled
    // Actually, OneWire requires Open Drain usually with Pullup.
    // If using internal pullup or external, Open Drain is safest. 
    // BUT, standard bit-bang often switches direction.
    // Let's use Open Drain (OD) if external pullup exists.
    // If not, Push Pull + switching to Input is common logic.
    // To be safe and generic w/o external resistor assumptions, we switch mode.
    
    // Switch to Output Push Pull
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(h->port, &GPIO_InitStruct);
}

// Helper: Set Pin as Input
static void Mode_Input(DS18B20_Handle_t *h) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = h->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Floating if external pullup, or PullUp
    GPIO_InitStruct.Pull = GPIO_PULLUP;     // Enable internal pullup just in case
    HAL_GPIO_Init(h->port, &GPIO_InitStruct);
}

static void Pin_Low(DS18B20_Handle_t *h) {
    HAL_GPIO_WritePin(h->port, h->pin, GPIO_PIN_RESET);
}

static void Pin_High(DS18B20_Handle_t *h) {
    HAL_GPIO_WritePin(h->port, h->pin, GPIO_PIN_SET);
}

static uint8_t Pin_Read(DS18B20_Handle_t *h) {
    return (uint8_t)HAL_GPIO_ReadPin(h->port, h->pin);
}

/* 1-Wire Primitives */

static bool OW_Reset(DS18B20_Handle_t *h) {
    Mode_Output(h);
    Pin_Low(h);
    Delay_us(OW_RESET_PULSE);
    
    Mode_Input(h); // Release line (Pull-up pulls high)
    Delay_us(OW_RESET_WAIT);
    
    uint8_t presence = Pin_Read(h); // Should be Low if slave present
    Delay_us(OW_PRESENCE_WAIT);
    
    return (presence == 0);
}

static void OW_WriteBit(DS18B20_Handle_t *h, uint8_t bit) {
    Mode_Output(h);
    if (bit) {
        // Write 1
        Pin_Low(h);
        Delay_us(OW_WRITE_1_LOW);
        Mode_Input(h); // Release
        Delay_us(OW_WRITE_1_HIGH);
    } else {
        // Write 0
        Pin_Low(h);
        Delay_us(OW_WRITE_0_LOW);
        Mode_Input(h); // Release
        Delay_us(OW_WRITE_0_HIGH);
    }
}

static uint8_t OW_ReadBit(DS18B20_Handle_t *h) {
    uint8_t bit = 0;
    Mode_Output(h);
    Pin_Low(h);
    Delay_us(OW_READ_LOW);
    
    Mode_Input(h);
    Delay_us(OW_READ_SAMPLE);
    bit = Pin_Read(h);
    Delay_us(OW_READ_RECOVER);
    
    return bit;
}

static void OW_WriteByte(DS18B20_Handle_t *h, uint8_t byte) {
    for (int i=0; i<8; i++) {
        OW_WriteBit(h, byte & 0x01);
        byte >>= 1;
    }
}

static uint8_t OW_ReadByte(DS18B20_Handle_t *h) {
    uint8_t byte = 0;
    for (int i=0; i<8; i++) {
        if (OW_ReadBit(h)) {
            byte |= (1 << i);
        }
    }
    return byte;
}

/* Public API */

void DS18B20_Init(DS18B20_Handle_t *h, GPIO_TypeDef *port, uint16_t pin) {
    h->port = port;
    h->pin = pin;
    h->last_temp = 0.0f;
    h->error = false;
    
    // Ensure high idle
    Mode_Input(h); 
}

void DS18B20_StartConversion(DS18B20_Handle_t *h) {
    if (!OW_Reset(h)) {
        h->error = true;
        return;
    }
    h->error = false;
    OW_WriteByte(h, 0xCC); // Skip ROM
    OW_WriteByte(h, 0x44); // Convert T
}

float DS18B20_ReadTemp(DS18B20_Handle_t *h) {
    if (!OW_Reset(h)) {
        h->error = true;
        return -999.0f;
    }
    
    OW_WriteByte(h, 0xCC); // Skip ROM
    OW_WriteByte(h, 0xBE); // Read Scratchpad
    
    uint8_t temp_l = OW_ReadByte(h);
    uint8_t temp_h = OW_ReadByte(h);
    
    // Process 16-bit generic Temp
    int16_t temp_raw = (temp_h << 8) | temp_l;
    
    // Resolution default 12-bit: 0.0625 step
    float temp = (float)temp_raw * 0.0625f;
    h->last_temp = temp;
    
    return temp;
}

float DS18B20_ReadTempBlocked(DS18B20_Handle_t *h) {
    DS18B20_StartConversion(h);
    if (h->error) return -999.0f;
    
    // Max conv time 750ms
    Delay_ms(750);
    
    return DS18B20_ReadTemp(h);
}
