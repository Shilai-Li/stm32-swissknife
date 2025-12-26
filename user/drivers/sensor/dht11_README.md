# DHT11 Temperature & Humidity Sensor Driver

Industrial-grade driver for DHT11 sensor with robust error handling and statistics tracking.

## Features
- **Instance-based Design**: Support multiple DHT11 sensors
- **Thread-Safe**: Critical sections protect timing-sensitive operations
- **Error Statistics**: Track timeouts, checksum errors, success count
- **Callback Support**: Get notified on errors via callback
- **Hardware Timer**: Uses configurable TIM for precise microsecond delays

## Hardware Requirements

### GPIO Configuration (CubeMX)
- **Pin Mode**: Output Push-Pull (or Open Drain with external pull-up)
- **Speed**: Low/Medium
- **Pull**: No Pull (4.7k-10k external pull-up to VCC required)

### Timer Configuration
- **Prescaler**: Set for 1MHz (1μs per tick)
  - Example: 72MHz clock → PSC = 71
- **Period (ARR)**: 0xFFFF (max)
- **Note**: Call `HAL_TIM_Base_Start(&htimX)` before DHT11_Init

## Usage

```c
#include "dht11.h"
#include "tim.h"

DHT11_Handle_t dht11;

void app_main(void) {
    // Init Timer for μs delays
    HAL_TIM_Base_Start(&htim2);
    
    // Init DHT11 on PA1
    DHT11_Init(&dht11, GPIOA, GPIO_PIN_1, &htim2);
    
    // Optional: Register error callback
    DHT11_SetErrorCallback(&dht11, my_error_handler);
}

void sensor_task(void) {
    DHT11_Status status = DHT11_Read(&dht11);
    
    if (status == DHT11_OK) {
        printf("Temp: %d.%d°C, Humidity: %d.%d%%\r\n",
               dht11.temp_int, dht11.temp_dec,
               dht11.humidity_int, dht11.humidity_dec);
    } else if (status == DHT11_ERROR_CHECKSUM) {
        printf("Checksum error\r\n");
    } else if (status == DHT11_ERROR_TIMEOUT) {
        printf("Timeout - check wiring\r\n");
    }
}
```

## API Reference

### Initialization
```c
void DHT11_Init(DHT11_Handle_t *dev, GPIO_TypeDef *port, 
                uint16_t pin, TIM_HandleTypeDef *htim);
```

### Read Data
```c
DHT11_Status DHT11_Read(DHT11_Handle_t *dev);
```
Returns: `DHT11_OK`, `DHT11_ERROR_CHECKSUM`, `DHT11_ERROR_TIMEOUT`, `DHT11_ERROR_GPIO`

After successful read, access data via handle:
- `dev->temp_int`, `dev->temp_dec`
- `dev->humidity_int`, `dev->humidity_dec`

### Error Callback
```c
void DHT11_SetErrorCallback(DHT11_Handle_t *dev, 
                            void (*callback)(DHT11_Handle_t*));
```

### Statistics
```c
uint32_t DHT11_GetErrorCount(DHT11_Handle_t *dev);
// Also available:
// dev->timeout_cnt
// dev->checksum_error_cnt
// dev->successful_read_cnt
```

## Timing Requirements
- **Read Interval**: Min 2 seconds between reads
- **Startup Time**: ~1 second after power-on

## Troubleshooting

### Always Returns DHT11_ERROR_TIMEOUT
1. Check pull-up resistor (4.7k-10k to VCC)
2. Verify timer is running and configured for 1MHz
3. Check GPIO pin connection
4. Ensure sensor has stable power (3.3V or 5V)

### Checksum Errors
1. RF interference - add 100nF capacitor near sensor VCC
2. Cable too long (max ~20cm recommended)
3. Timing issue - verify timer PSC setting

### Inconsistent Readings
1. Respect 2-second minimum read interval
2. Check power supply stability

## Test Program
See `user/drivers/tests/dht11_tests.c` for complete example.
