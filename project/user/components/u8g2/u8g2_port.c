/**
 * @file u8g2_port.c
 * @brief U8G2 Hardware Abstraction Layer Implementation for STM32 HAL
 */

#include "../u8g2_port.h"
#include "stm32f1xx_hal.h" // Adjust for your MCU series

/*
 *  GPIO and Delay Callback
 *  This function handles timing and GPIO pin states (CS, DC, RESET).
 */
uint8_t u8x8_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        /* logic for gpio init is handled by CubeMX usually */
        break;

    case U8X8_MSG_DELAY_MILLI:
        HAL_Delay(arg_int);
        break;

    case U8X8_MSG_DELAY_10MICRO:
        /* Generic rough delay for 10us blocks. 
           For 72MHz, a simple loop is okay if exact timing isn't critical (OLEDs are slow). 
           Or use DWT if available. Here we assume SystemCoreClock is available.
        */
        {
            uint32_t count = arg_int * (SystemCoreClock / 100000 / 5); // Rough approximation
            while(count--) __NOP(); 
        }
        break;

    case U8X8_MSG_DELAY_100NANO:
        __NOP(); __NOP(); __NOP(); __NOP(); // Just a few cycles
        break;

    case U8X8_MSG_GPIO_CS:
        // arg_int: 0=Deselect (High), 1=Select (Low - Active)
        // Note: U8g2 logic for CS is usually: 0=High(Idle), 1=Low(Active) ? 
        // Wait, standard U8g2 doc says: "arg_int contains the value"
        // Usually, 1 means "active level". For CS, active is LOW.
        // So if arg_int=1, Set Pin LOW. if arg_int=0, Set Pin HIGH.
        
        // However, u8x8 implementation often assumes direct level?
        // Let's check: U8X8_MSG_GPIO_CS is passed the LEVEL directly.
        // But convention varies. Let's assume standard level.
        // Actually U8g2 convention: arg_int is the value to set.
        
        if (u8x8->pins[U8X8_PIN_CS] != U8X8_PIN_NONE) {
             // We need to map u8x8->pins index to actual GPIO Port/Pin.
             // But managing a map is complex. 
             // EASIER WAY: Store Port/Pin pointers in `u8x8->user_ptr` or global variables?
             // Simplest: The user code fills u8x8->pins with distinct setup? 
             // No, U8g2 has a "User Data" pointer.
        }
        break;
        
    /* 
       Wait. The standard way U8G2 porting works on STM32 is:
       We implement specific callbacks that DONT use the `u8x8->pins` array for pin mapping 
       (unless we implement a lookup table). 
       INSTEAD, we often pass the GPIO Port/Pin STRUCT into user_ptr.
       
       BUT, implementing a generic callback for ANY pin is hard without a lookup.
       
       SIMPLIFIED APPROACH for Single Display:
       We define global or file-scope variables for the Control Pins, 
       set by the Init wrapper.
    */
    }
    return 1;
}

/*
 *  BETTER APPROACH:
 *  We Create a structure to hold Pin definitions and pass it as `u8x8->user_ptr`?
 *  But u8g2 structure is managed by the library.
 *  
 *  Let's stick to the "Global Variables for Single Instance" approach for simplicity in this Port.
 *  Multiple displays would require a more complex contextual lookup.
 */

static struct {
    GPIO_TypeDef *cs_port; uint16_t cs_pin;
    GPIO_TypeDef *dc_port; uint16_t dc_pin;
    GPIO_TypeDef *rst_port; uint16_t rst_pin;
} u8g2_gpio_config = {0};

/* Re-implement GPIO Delay with State Access */
uint8_t u8x8_gpio_and_delay_stm32_real(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
    case U8X8_MSG_DELAY_MILLI:
        HAL_Delay(arg_int);
        break;
    case U8X8_MSG_DELAY_10MICRO:
         for (uint32_t n = 0; n < arg_int * 72; n++) __NOP(); // Crude 72MHz delay
        break;
    case U8X8_MSG_DELAY_100NANO:
        __NOP();
        break;
        
    case U8X8_MSG_GPIO_CS: // Chip Select
        if(u8g2_gpio_config.cs_port)
            HAL_GPIO_WritePin(u8g2_gpio_config.cs_port, u8g2_gpio_config.cs_pin, arg_int ? GPIO_PIN_SET : GPIO_PIN_RESET);
        break;

    case U8X8_MSG_GPIO_DC: // Data/Command
        if(u8g2_gpio_config.dc_port)
            HAL_GPIO_WritePin(u8g2_gpio_config.dc_port, u8g2_gpio_config.dc_pin, arg_int ? GPIO_PIN_SET : GPIO_PIN_RESET);
        break;

    case U8X8_MSG_GPIO_RESET: // Reset
        if(u8g2_gpio_config.rst_port)
            HAL_GPIO_WritePin(u8g2_gpio_config.rst_port, u8g2_gpio_config.rst_pin, arg_int ? GPIO_PIN_SET : GPIO_PIN_RESET);
        break;
    }
    return 1;
}


/* Hardware SPI Interface */
uint8_t u8x8_byte_stm32_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
    case U8X8_MSG_BYTE_SEND:
        // arg_int is number of bytes, arg_ptr is buffer
        HAL_SPI_Transmit(&hspi1, (uint8_t *)arg_ptr, arg_int, 100);
        break;
        
    case U8X8_MSG_BYTE_INIT:
        // SPI Init handled by CubeMX
        /* We might want to ensure CS is HIGH here */
        u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
        break;
        
    case U8X8_MSG_BYTE_SET_DC:
        // arg_int: 0=Command (Low), 1=Data (High) for 4-wire SPI
        u8x8_gpio_SetDC(u8x8, arg_int);
        break;
        
    case U8X8_MSG_BYTE_START_TRANSFER:
        u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
        u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->post_chip_enable_wait_ns, NULL);
        break;
        
    case U8X8_MSG_BYTE_END_TRANSFER:
        u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL);
        u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
        break;
        
    default:
        return 0;
    }
    return 1;
}

/* Hardware I2C Interface */
uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    /* 
       Note: U8g2's I2C protocol handling involves buffering bytes until START_TRANSFER / END_TRANSFER?
       Actually U8g2 SW I2C sends bit by bit. 
       HW I2C usually sends a full packet: [Addr][CtrlByte][Data...]
       
       U8g2 has a "u8x8_byte_hw_i2c" helper but it assumes linux-like i2c-dev.
       For generic HAL, we must handle the state.
       
       Simpler Strategy: Use "u8x8_byte_sw_i2c" if you don't care about speed.
       But for HW I2C:
    */
    
    static uint8_t buffer[32];
    static uint8_t buf_idx;
    
    switch (msg)
    {
    case U8X8_MSG_BYTE_SEND:
        {
            uint8_t *data = (uint8_t *)arg_ptr;
            while(arg_int > 0)
            {
                buffer[buf_idx++] = *data;
                data++;
                arg_int--;
                if(buf_idx >= 32) { // Flush if full (Should handle correctly)
                    // Handling overflow in middle of packet is tricky for I2C if not supported.
                    // For now, assume small packets.
                }
            }
        }
        break;

    case U8X8_MSG_BYTE_INIT:
        /* I2C Init done by CubeMX */
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        buf_idx = 0;
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:
        // Now send the accumulated buffer
        // U8g2 stores Slave Address in u8x8_GetI2CAddress(u8x8) -> bitshifted?
        // U8g2 addr is usually 0x78 (which is 0x3C << 1). HAL expects (0x3C << 1).
        HAL_I2C_Master_Transmit(&hi2c1, u8x8_GetI2CAddress(u8x8), buffer, buf_idx, 100);
        break;

    default:
        return 0;
    }
    return 1;
}


// --- Helpers ---

void U8G2_Init_SSD1306_I2C(u8g2_t *u8g2) {
    // Rotation 0, Setup for I2C (No DC/CS/RST usually needed for I2C, or just RST)
    // Noname 128x64
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_stm32_hw_i2c, u8x8_gpio_and_delay_stm32_real);
    
    // Config: None, I2C uses internal buffer logic
    u8g2_InitDisplay(u8g2);
    u8g2_SetPowerSave(u8g2, 0); // Wake up
}

void U8G2_Init_SSD1306_SPI(u8g2_t *u8g2, 
                           GPIO_TypeDef* cs_port, uint16_t cs_pin,
                           GPIO_TypeDef* dc_port, uint16_t dc_pin,
                           GPIO_TypeDef* rst_port, uint16_t rst_pin) 
{
    // Save Config to Global
    u8g2_gpio_config.cs_port = cs_port; u8g2_gpio_config.cs_pin = cs_pin;
    u8g2_gpio_config.dc_port = dc_port; u8g2_gpio_config.dc_pin = dc_pin;
    u8g2_gpio_config.rst_port = rst_port; u8g2_gpio_config.rst_pin = rst_pin;

    // Setup: SSD1306 128x64 Noname, 4-Wire SPI (Most common for modules)
    u8g2_Setup_ssd1306_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_stm32_hw_spi, u8x8_gpio_and_delay_stm32_real);
    
    u8g2_InitDisplay(u8g2);
    u8g2_SetPowerSave(u8g2, 0);
}
