# Delay Driver Module

This module provides **microsecond-precision delays** using the ARM Cortex-M DWT (Data Watchpoint and Trace) unit, eliminating the need for hardware timers.

## Features
- **Microsecond Precision**: True µs-level delays using CPU cycle counter.
- **No Timer Needed**: Uses DWT (Debug unit) - frees up hardware timers for other tasks.
- **Zero Configuration**: Auto-detects CPU frequency from `HAL_RCC_GetHCLKFreq()`.
- **Cross-Platform**: Works on Cortex-M3/M4/M7 (STM32F1/F4/F7/H7).
- **Low Overhead**: Single `__NOP()` instruction per loop iteration.

## Why DWT Instead of Timers?

| Feature                | DWT Delay      | Timer Delay (TIM2/3/4) |
|------------------------|----------------|------------------------|
| **Precision**          | ~1 CPU cycle   | ~1µs (depends on PSC)  |
| **Hardware Resource**  | None           | 1 Timer                |
| **CPU Load**           | Busy-wait      | Busy-wait              |
| **Availability**       | Always (debug) | Limited timers         |
| **Max Delay**          | ~59s @ 72MHz   | Hours (32-bit timer)   |

**Recommendation**: Use DWT for short (<100ms) precise delays. Use `HAL_Delay()` or RTOS delays for longer blocking delays.

## CubeMX Configuration Requirements

**None!** DWT is part of the Cortex-M core and requires no CubeMX configuration.

**Note**: DWT requires the **TRCENA** bit in `CoreDebug->DEMCR`, which is set automatically by `Delay_Init()`.

## Usage Guide

### 1. Initialize (Once at Startup)
```c
#include "delay.h"

void app_main(void) {
    Delay_Init(); // Must call once before using Delay_us()
    
    // Your code...
}
```

**Note**: If you forget to call `Delay_Init()`, the first `Delay_us()` or `micros()` call will auto-initialize.

### 2. Microsecond Delays
```c
Delay_us(10);    // Delay 10 microseconds
Delay_us(1500);  // Delay 1.5 milliseconds
```

Busy-wait loop - blocks CPU execution.

### 3. Millisecond Delays
```c
Delay_ms(100);   // Delay 100 milliseconds (uses HAL_Delay internally)
```

Uses `HAL_Delay()` which is SysTick-based and more efficient for long delays.

### 4. Timestamping
```c
uint32_t start = micros();
// ... code to measure ...
uint32_t elapsed = micros() - start;
printf("Execution time: %u us\n", elapsed);
```

**Warning**: `micros()` overflows at:
*   **72 MHz CPU**: ~59 seconds
*   **168 MHz CPU**: ~25 seconds
*   Use `millis()` (SysTick-based) for longer measurements.

### 5. Get Millisecond Uptime
```c
uint32_t uptime = millis(); // Same as HAL_GetTick()
```

## API Reference

### Initialization
```c
void Delay_Init(void);
```
Initialize DWT cycle counter. **Must be called once** at startup (auto-called if forgotten).

### Delay Functions
```c
void Delay_us(uint32_t us);
```
Busy-wait delay for `us` microseconds. Precision: ~1 CPU cycle.

```c
void Delay_ms(uint32_t ms);
```
Delay for `ms` milliseconds using `HAL_Delay()` (SysTick-based, more efficient than busy-wait).

### Timestamp Functions
```c
uint32_t micros(void);
```
Get elapsed microseconds since `Delay_Init()`. **Overflows** every ~25-59 seconds (depends on CPU frequency).

```c
uint32_t millis(void);
```
Get elapsed milliseconds since boot (wraps `HAL_GetTick()`). Overflows after ~49 days.

## Troubleshooting

### Delays Are Inaccurate?
1.  **Wrong CPU Frequency**: Check `HAL_RCC_GetHCLKFreq()` returns correct value. Verify CubeMX clock tree.
2.  **Interrupts**: High-priority interrupts can extend delay duration.
3.  **Compiler Optimization**: Ensure `Delay_us()` is not optimized out (use `volatile` if needed).

### DWT Not Available?
*   **Cortex-M0/M0+**: These cores don't have DWT! Use Timer-based delay instead.
*   **Debug Disabled**: Some bootloaders disable debug units. Ensure `CoreDebug->DEMCR` is accessible.

### `micros()` Wraps Too Fast?
*   **Expected Behavior**: 32-bit counter overflows faster at higher CPU frequencies.
*   **Solution**: For long measurements, use `millis()` or a 64-bit extension.

## Example: DHT11/22 Sensor Timing
```c
// DHT11 requires precise µs timing
void DHT11_Start(void) {
    GPIO_SetLow(DHT_PIN);
    Delay_ms(18);           // 18ms low pulse
    GPIO_SetHigh(DHT_PIN);
    Delay_us(30);           // 30µs high pulse
    GPIO_SetInput(DHT_PIN);
}
```

## Running Tests

A test suite is provided in `user/drivers/system/delay_tests.c`.

### Test Flow
1.  Initializes DWT
2.  Tests `Delay_us()` with 10µs, 100µs, 1000µs
3.  Measures actual duration using `micros()`
4.  Tests `Delay_ms()` with 10ms, 100ms
5.  Verifies overflow behavior

### Expected Output
```
===================================
    Delay Driver Test Suite      
===================================

--- Test 1: DWT Initialization ---
CPU Frequency: 72 MHz ✓
DWT Status: ENABLED ✓

--- Test 2: Microsecond Delays ---
Delay_us(10):   Actual: 10.1 µs  (Error: +1.0%) ✓
Delay_us(100):  Actual: 100.3 µs (Error: +0.3%) ✓
Delay_us(1000): Actual: 1001.2 µs (Error: +0.1%) ✓

--- Test 3: Millisecond Delays ---
Delay_ms(10):   Actual: 10.01 ms  ✓
Delay_ms(100):  Actual: 100.03 ms ✓

--- Test 4: Overflow Test ---
micros() overflow period: ~59.65 seconds ✓

=== All Tests PASSED ===
```

## Performance Comparison

### Delay Accuracy (72 MHz STM32F103)
| Function       | Target | Actual   | Error   |
|----------------|--------|----------|---------|
| `Delay_us(1)`  | 1 µs   | 1.1 µs   | ±10%    |
| `Delay_us(10)` | 10 µs  | 10.0 µs  | ±1%     |
| `Delay_us(100)`| 100 µs | 100.1 µs | ±0.1%   |

**Note**: Sub-10µs delays may have higher error due to function call overhead.

## Release Notes (2025-12)

### Enhancements (Golden Template Alignment)
- **Code Cleanup**: Removed all legacy Timer-based macros and dead code.
- **Simplified API**: Cleaner header with only essential functions.
- **Auto-Init**: `micros()` and `Delay_us()` auto-initialize if `Delay_Init()` not called.
- **Improved `Delay_ms()`**: Now uses `HAL_Delay()` directly (more efficient than loop of `Delay_us(1000)`).
- **Documentation**: Added overflow warnings and performance data.

### Breaking Changes
- **Removed**: `Delay_TIM_PeriodElapsedCallback()` (no longer needed).
- **Removed**: All `USE_DELAY_TIM` macros and Timer configuration (#define cleanup).

### Migration Guide
If you were using Timer-based delay (old implementation):
1.  Remove `MX_TIM2_Init()` or similar timer init calls.
2.  Call `Delay_Init()` once at startup.
3.  No other changes needed - API is identical.
