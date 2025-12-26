# Potentiometer (ADC Knob) Driver

Debounced potentiometer/knob reader with moving average filter and multiple output formats.

## Features
- **Moving Average Filter**: 8-sample window for stable readings
- **Deadzone Support**: Eliminate jitter at endpoints
- **Flexible Output**: Raw ADC, percentage, ratio, or custom mapping
- **Inverse Mode**: Reverse rotation direction in software
- **Error Tracking**: ADC failure detection

## Hardware Requirements
- **ADC Channel**: Any STM32 ADC channel
- **Potentiometer**: 10kΩ typical (1kΩ-100kΩ works)
- **Wiring**: 
  - One end → GND
  - Wiper → ADC pin
  - Other end → VCC (3.3V)

## Usage

```c
#include "potentiometer.h"

ADC_HandleTypeDef hadc1;
Pot_Handle_t knob;

void app_init(void) {
    Pot_Config_t config = {
        .hadc = &hadc1,
        .deadzone_low = 50,
        .deadzone_high = 4045,
        .inverse = false
    };
    
    Pot_Init(&knob, &config);
}

void read_knob(void) {
    uint16_t raw = Pot_Update(&knob);
    
    uint8_t percent = Pot_GetPercent(&knob);
    printf("Knob: %d%%\r\n", percent);
    
    float ratio = Pot_GetRatio(&knob);
    
    int32_t speed = Pot_Map(&knob, -100, 100);
    printf("Motor speed: %ld\r\n", speed);
}
```

## API Reference

```c
void Pot_Init(Pot_Handle_t *handle, const Pot_Config_t *config);
void Pot_SetErrorCallback(Pot_Handle_t *handle, void (*cb)(Pot_Handle_t*));

uint16_t Pot_Update(Pot_Handle_t *handle);        // Returns 0-4095
uint8_t Pot_GetPercent(Pot_Handle_t *handle);     // Returns 0-100
float Pot_GetRatio(Pot_Handle_t *handle);         // Returns 0.0-1.0
int32_t Pot_Map(Pot_Handle_t *handle, int32_t min, int32_t max);
```

## Configuration

### Deadzones
Prevent jitter when at min/max position:
```c
config.deadzone_low = 50;   // 0-50 → clamp to 0
config.deadzone_high = 4045; // 4045-4095 → clamp to 4095
```

### Inverse Rotation
```c
config.inverse = true;  // Flip CW/CCW direction
```

## Troubleshooting

### Jittery Readings
- Increase filter window (edit `filter_buffer[8]` to larger size)
- Adjust deadzones
- Check ADC reference voltage stability

### No Response
- Verify ADC channel configuration in CubeMX
- Check potentiometer wiring (continuity test)

## Test Program
See `user/drivers/tests/potentiometer_tests.c`
