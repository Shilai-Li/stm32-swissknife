/**
 * @file sfud_port.c
 * @brief SFUD Port Layer Implementation for STM32 HAL
 * @author stm32-swissknife Project
 * @date 2025
 * 
 * This file implements the platform-specific functions required by SFUD.
 * 
 * Hardware Connection (Example for W25Q64 on SPI1):
 * ┌─────────────┬──────────────┬─────────────┐
 * │ STM32       │ W25Qxx       │ Description │
 * ├─────────────┼──────────────┼─────────────┤
 * │ PA5 (SCK)   │ CLK (Pin 6)  │ SPI Clock   │
 * │ PA6 (MISO)  │ DO  (Pin 2)  │ Data Out    │
 * │ PA7 (MOSI)  │ DI  (Pin 5)  │ Data In     │
 * │ PA4 (GPIO)  │ CS  (Pin 1)  │ Chip Select │
 * │ 3.3V        │ VCC (Pin 8)  │ Power       │
 * │ GND         │ GND (Pin 4)  │ Ground      │
 * └─────────────┴──────────────┴─────────────┘
 */

#include "sfud_port.h"
#include "csrc/sfud.h"
#include "main.h"
#include <stdarg.h>
#include <stdio.h>

/*******************************************************************************
 * CONFIGURATION
 ******************************************************************************/

// SPI handle (defined in main.c by CubeMX)
extern SPI_HandleTypeDef hspi1;

// CS Pin Configuration (modify to match your hardware)
#define SFUD_CS_PORT    GPIOA
#define SFUD_CS_PIN     GPIO_PIN_4

// SPI Timeout (milliseconds)
#define SFUD_SPI_TIMEOUT 1000

/*******************************************************************************
 * HELPER MACROS
 ******************************************************************************/

#define SFUD_CS_LOW()   HAL_GPIO_WritePin(SFUD_CS_PORT, SFUD_CS_PIN, GPIO_PIN_RESET)
#define SFUD_CS_HIGH()  HAL_GPIO_WritePin(SFUD_CS_PORT, SFUD_CS_PIN, GPIO_PIN_SET)

/*******************************************************************************
 * PLATFORM FUNCTIONS (Required by SFUD)
 ******************************************************************************/

/**
 * @brief SPI write and read function
 * 
 * This function is called by SFUD to perform SPI transactions.
 * It must handle both write-only, read-only, and write-then-read operations.
 * 
 * @param spi SFUD SPI device
 * @param write_buf Data to write (NULL if read-only)
 * @param write_size Number of bytes to write
 * @param read_buf Buffer for read data (NULL if write-only)
 * @param read_size Number of bytes to read
 * @return sfud_err SFUD_SUCCESS on success
 */
static sfud_err spi_write_read(const sfud_spi *spi, 
                                const uint8_t *write_buf, size_t write_size,
                                uint8_t *read_buf, size_t read_size)
{
    HAL_StatusTypeDef status;
    SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)spi->user_data;
    
    // Pull CS low to start transaction
    SFUD_CS_LOW();
    
    // Write phase
    if (write_size > 0 && write_buf != NULL) {
        status = HAL_SPI_Transmit(hspi, (uint8_t *)write_buf, write_size, SFUD_SPI_TIMEOUT);
        if (status != HAL_OK) {
            SFUD_CS_HIGH();
            return SFUD_ERR_TIMEOUT;
        }
    }
    
    // Read phase
    if (read_size > 0 && read_buf != NULL) {
        status = HAL_SPI_Receive(hspi, read_buf, read_size, SFUD_SPI_TIMEOUT);
        if (status != HAL_OK) {
            SFUD_CS_HIGH();
            return SFUD_ERR_TIMEOUT;
        }
    }
    
    // Pull CS high to end transaction
    SFUD_CS_HIGH();
    
    return SFUD_SUCCESS;
}

/**
 * @brief Delay function (milliseconds)
 * 
 * @param ms Number of milliseconds to delay
 */
static void spi_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief SPI bus lock (for multi-threaded environments)
 * 
 * Not needed for single-threaded bare-metal applications.
 * Implement if using RTOS.
 */
static void spi_lock(const sfud_spi *spi)
{
    // TODO: Implement mutex lock if using RTOS
    // Example: osMutexWait(spi_mutex, osWaitForever);
}

/**
 * @brief SPI bus unlock
 * 
 * Not needed for single-threaded bare-metal applications.
 * Implement if using RTOS.
 */
static void spi_unlock(const sfud_spi *spi)
{
    // TODO: Implement mutex unlock if using RTOS
    // Example: osMutexRelease(spi_mutex);
}

/**
 * @brief SFUD port initialization
 * 
 * This function is called by SFUD to initialize the SPI port.
 * It must configure the function pointers for SPI operations.
 * 
 * @param flash SFUD Flash device
 * @return sfud_err SFUD_SUCCESS on success
 */
sfud_err sfud_spi_port_init(sfud_flash *flash)
{
    if (flash == NULL) {
        return SFUD_ERR_NOT_FOUND;
    }
    
    // Bind SPI operations
    flash->spi.wr = spi_write_read;
    flash->spi.lock = spi_lock;
    flash->spi.unlock = spi_unlock;
    flash->spi.user_data = &hspi1;  // Attach HAL SPI handle
    
    // Set retry configuration
    flash->retry.times = 10000;     // Retry times for busy wait
    flash->retry.delay = spi_delay_ms;
    
    return SFUD_SUCCESS;
}

/*******************************************************************************
 * DEBUG LOGGING (Optional)
 ******************************************************************************/

#ifdef SFUD_DEBUG_MODE

/**
 * @brief Debug log output
 * 
 * This function is called by SFUD for debug messages.
 * Redirect to your UART or other logging mechanism.
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...)
{
    va_list args;
    
    // Print file and line
    printf("[SFUD] (%s:%ld) ", file, line);
    
    // Print formatted message
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\r\n");
}

/**
 * @brief Info log output
 */
void sfud_log_info(const char *format, ...)
{
    va_list args;
    
    printf("[SFUD] ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\r\n");
}

#endif /* SFUD_DEBUG_MODE */

/*******************************************************************************
 * USER API (Convenience Functions)
 ******************************************************************************/

/**
 * @brief Initialize SFUD with default configuration
 * 
 * This is a simple wrapper that calls sfud_init() and returns
 * the first Flash device.
 * 
 * @return sfud_err SFUD_SUCCESS on success
 */
sfud_err SFUD_Port_Init(void)
{
    sfud_err result;
    
    // Initialize SFUD library
    result = sfud_init();
    
    if (result == SFUD_SUCCESS) {
        // Get the first Flash device
        const sfud_flash *flash = sfud_get_device_table();
        if (flash) {
            printf("[SFUD] Flash initialized: %s\r\n", flash->name);
            printf("[SFUD] Manufacturer ID: 0x%02X\r\n", flash->chip.mf_id);
            printf("[SFUD] Type ID: 0x%02X\r\n", flash->chip.type_id);
            printf("[SFUD] Capacity ID: 0x%02X\r\n", flash->chip.capacity_id);
            printf("[SFUD] Capacity: %ld KB\r\n", flash->chip.capacity / 1024);
        }
    }
    
    return result;
}

/**
 * @brief Get default Flash device
 * 
 * @return const sfud_flash* Pointer to first Flash device
 */
const sfud_flash *SFUD_Port_GetDefaultFlash(void)
{
    return sfud_get_device_table();
}
