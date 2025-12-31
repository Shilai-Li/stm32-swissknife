# HC-SR04 Ultrasonic Distance Sensor Driver

Robust driver for HC-SR04 ultrasonic ranging module with timeout protection and statistics.

## Features
- **Non-blocking Timer-Based**: No busy-wait CPU stalling
- **Timeout Protection**: Configurable timeout prevents hang
- **Wrap-Around Safe**: Handles 16/32-bit timer overflow
- **Error Statistics**: Track timeouts and invalid readings
- **Thread-Safe Trigger**: IRQ protection during pulse generation

## Hardware Requirements

### GPIO Configuration (CubeMX)
- **Trig Pin**: Output Push-Pull, Speed: Medium/High
- **Echo Pin**: Input, No Pull

### Timer Configuration
- **Prescaler**: 1MHz (1μs per tick)
  - Example: 72MHz → PSC = 71
- **Period (ARR)**: 0xFFFF (max)
- **Mode**: Continuous (no auto-reload interrupt needed)
- **Must call**: `HAL_TIM_Base_Start(&htimX)` once at boot

## Usage

```c
#include "hc_sr04.h"

HCSR04_HandleTypeDef ultrasonic;

void app_init(void) {
    HAL_TIM_Base_Start(&htim4);
    
    HCSR04_Init(&ultrasonic, &htim4,
                GPIOA, GPIO_PIN_0,  // Trig
                GPIOA, GPIO_PIN_1); // Echo
    
    // Default timeout is 30ms (~5m range)
    ultrasonic.TimeoutUs = 25000;  // Adjust if needed
}

void measure_distance(void) {
    float distance_cm = HCSR04_Read(&ultrasonic);
    
    if (distance_cm > 0) {
        printf("Distance: %.1f cm\r\n", distance_cm);
    } else {
        printf("Out of range or error\r\n");
    }
}
```

## API Reference

### Initialization
```c
void HCSR04_Init(HCSR04_HandleTypeDef *hsensor, TIM_HandleTypeDef *htim,
                 GPIO_TypeDef *trig_port, uint16_t trig_pin,
                 GPIO_TypeDef *echo_port, uint16_t echo_pin);
```

### Distance Measurement
```c
float HCSR04_Read(HCSR04_HandleTypeDef *hsensor);
```
Returns: Distance in cm (2-400cm), or `-1.0f` on error/timeout

### Error Handling
```c
void HCSR04_SetErrorCallback(HCSR04_HandleTypeDef *hsensor,
                             void (*callback)(HCSR04_HandleTypeDef*));

// Statistics:
// hsensor->error_cnt
// hsensor->timeout_cnt
// hsensor->success_cnt
```

## Specifications
- **Range**: 2cm - 400cm
- **Accuracy**: ±3mm (ideal conditions)
- **Beam Angle**: ~15 degrees
- **Min Measurement Interval**: 60ms (to avoid echo overlap)

## Troubleshooting

### Always Returns -1.0f
1. **Timeout**: Check Echo pin wiring
2. **Timer not running**: Verify `HAL_TIM_Base_Start()` called
3. **Wrong prescaler**: Must be 1MHz (1μs/tick)
4. **No target**: Sensor needs reflective surface in range

### Erratic Readings
1. **Soft surfaces**: Carpet/fabric absorbs ultrasound
2. **Angled surfaces**: Reflect sound away from sensor
3. **EMI**: Keep away from motors/relays during measurement
4. **Temperature**: Speed of sound varies with temp (not compensated)

### Short Range Only
1. Check VCC is 5V (HC-SR04 requires 5V, not 3.3V)
2. Echo pin may need level shifter if MCU is 3.3V logic

## Advanced Configuration

### Adjust Timeout (Range)
```c
hsensor.TimeoutUs = 20000;  // 20ms = ~3.4m max range
```

### Speed of Sound Compensation
Current formula assumes 340m/s at 20°C. For precision:
```c
// Modify in driver or post-process:
// distance_corrected = distance_measured * (331.3 + 0.606 * temp_C) / 340.0;
```

## Test Program
See `user/drivers/tests/hc_sr04_tests.c` for continuous ranging example.
