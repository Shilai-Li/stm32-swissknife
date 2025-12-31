# DS18B20 OneWire Temperature Sensor Driver

High-precision digital temperature sensor driver with OneWire protocol implementation.

## Features
- **OneWire Protocol**: Software-based 1-Wire bit-banging
- **Thread-Safe Timing**: Protected critical GPIO sequences
- **0.0625°C Resolution**: 12-bit temperature readings
- **Error Tracking**: Comprehensive statistics and callbacks
- **Multi-Instance**: Support multiple DS18B20 sensors

## Hardware Requirements

### GPIO Configuration (CubeMX)
- **Pin Mode**: Output Push-Pull 
- **Speed**: High
- **Pull**: Internal Pull-Up enabled (or 4.7k external)
- **Note**: Pin dynamically switches between Input/Output

### Timer Configuration
- **Prescaler**: 1MHz (1μs per tick)
  - Example: 72MHz → PSC = 71
- **Period**: 0xFFFF
- **Must call**: `HAL_TIM_Base_Start(&htimX)` before init

## Usage

```c
#include "ds18b20.h"

DS18B20_Handle_t temp_sensor;

void app_init(void) {
    HAL_TIM_Base_Start(&htim2);
    DS18B20_Init(&temp_sensor, GPIOB, GPIO_PIN_1, &htim2);
}

void read_temperature(void) {
    // Non-blocking: start conversion manually
    DS18B20_StartConversion(&temp_sensor);
    HAL_Delay(750);  // Wait for 12-bit conversion
    float temp = DS18B20_ReadTemp(&temp_sensor);
    
    if (temp != -999.0f) {
        printf("%.2f°C\r\n", temp);
    }
}

void read_temperature_blocking(void) {
    // Blocking wrapper (starts + waits + reads)
    float temp = DS18B20_ReadTempBlocked(&temp_sensor);
    printf("%.2f°C\r\n", temp);
}
```

## API Reference

### Initialization
```c
void DS18B20_Init(DS18B20_Handle_t *handle, GPIO_TypeDef *port, 
                  uint16_t pin, TIM_HandleTypeDef *htim);
```

### Temperature Reading
```c
void DS18B20_StartConversion(DS18B20_Handle_t *handle);
float DS18B20_ReadTemp(DS18B20_Handle_t *handle);
float DS18B20_ReadTempBlocked(DS18B20_Handle_t *handle);  // All-in-one
```
Returns: Temperature in °C, or `-999.0f` on error

### Error Handling
```c
void DS18B20_SetErrorCallback(DS18B20_Handle_t *handle, 
                              void (*callback)(DS18B20_Handle_t*));

// Statistics available in handle:
// handle->error_cnt
// handle->success_cnt
// handle->last_temp
```

## Timing Specifications
- **Conversion Time**: 
  - 12-bit: 750ms
  - 11-bit: 375ms (configurable via register write)
- **Min Read Interval**: 1 second recommended

## Troubleshooting

### Returns -999.0f (No Device Present)
1. Check 4.7k pull-up resistor to VCC
2. Verify GPIO pin connection
3. Ensure sensor is powered (3.0-5.5V)
4. Check timer is running at 1MHz

### Incorrect Temperature Readings
1. Verify timer prescaler: must be 1μs per tick
2. Long cable (>3m) may need stronger pull-up (e.g., 2.2k)
3. Disable interrupts during read if EMI is high

### Intermittent Failures
1. Add 100nF capacitor near sensor VCC/GND
2. Use shielded cable for long runs
3. Check power supply stability

## Multiple Sensors
To read multiple DS18B20 on one bus, you need ROM commands:
```c
// Current driver uses Skip ROM (0xCC)
// For multi-drop, implement:
// - Search ROM (0xF0) to enumerate devices
// - Match ROM (0x55) to address specific sensor
```
Current implementation supports one sensor per GPIO pin (simpler single-drop mode).

## Test Program
See `user/drivers/tests/ds18b20_tests.c`
