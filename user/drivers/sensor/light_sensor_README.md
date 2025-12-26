# Light Sensor (LDR/Photocell) Driver

ADC-based light sensor driver with moving average filter and hysteresis.

## Features
- **16-Sample Moving Average**: Smooth ambient light tracking
- **Hysteresis Logic**: Stable dark/light state detection
- **Inverse Mode**: Support both pull-up and pull-down circuits
- **Percentage Output**: 0-100% light intensity
- **Error Recovery**: Returns last valid value on ADC failure

## Hardware Requirements
- **ADC Channel**: Any STM32 ADC
- **Circuit Options**:
  1. **Pull-Down** (High voltage = Bright):
     - LDR between VCC and ADC pin
     - 10kΩ resistor between ADC pin and GND
  2. **Pull-Up** (Low voltage = Bright):
     - 10kΩ resistor between VCC and ADC pin
     - LDR between ADC pin and GND
     - Set `inverse_logic = true`

## Usage

```c
#include "light_sensor.h"

LightSensor_Handle_t ldr;
ADC_HandleTypeDef hadc1;

void app_init(void) {
    LightSensor_Config_t config = {
        .hadc = &hadc1,
        .dark_threshold = 500,    // Below = Dark
        .light_threshold = 800,   // Above = Light
        .inverse_logic = false    // false for pull-down
    };
    
    LightSensor_Init(&ldr, &config);
}

void read_light(void) {
    uint16_t raw = LightSensor_Update(&ldr);
    
    if (LightSensor_IsDark(&ldr)) {
        printf("It's dark - turn on lights\r\n");
    }
    
    uint8_t brightness = LightSensor_GetIntensityPercentage(&ldr);
    printf("Brightness: %d%%\r\n", brightness);
}
```

## API Reference

```c
void LightSensor_Init(LightSensor_Handle_t *handle, 
                      const LightSensor_Config_t *config);

void LightSensor_SetErrorCallback(LightSensor_Handle_t *handle,
                                   void (*cb)(LightSensor_Handle_t*));

uint16_t LightSensor_Update(LightSensor_Handle_t *handle);
bool LightSensor_IsDark(LightSensor_Handle_t *handle);
uint8_t LightSensor_GetIntensityPercentage(LightSensor_Handle_t *handle);
```

## Hysteresis Configuration

Prevent oscillation when light level is at boundary:
```c
config.dark_threshold = 500;   // Transition to Dark below 500
config.light_threshold = 800;  // Transition to Light above 800
// Between 500-800: maintain previous state
```

### Example Behavior (Pull-Down Circuit)
```
ADC Value   | Previous State | New State
----------------------------------------------
400         | Any            | Dark
650         | Dark           | Dark (hysteresis)
650         | Light          | Light (hysteresis)
900         | Any            | Light
```

## Calibration

1. **Find Dark Value**: Cover sensor completely, read ADC
2. **Find Light Value**: Expose to ambient/bright light, read ADC
3. **Set Thresholds**: 
   ```c
   dark_threshold = dark_value + margin
   light_threshold = light_value - margin
   ```

## Troubleshooting

### Unstable Dark/Light Switching
- Increase hysteresis gap (larger difference between thresholds)
- Increase filter window size (edit `filter_buffer[16]`)

### Wrong Polarity
- Toggle `inverse_logic` setting
- Or swap LDR and fixed resistor in circuit

### Slow Response
- Reduce filter window to 8 samples
- Use faster ADC sampling rate

## Test Program
See `user/drivers/tests/light_sensor_tests.c`
