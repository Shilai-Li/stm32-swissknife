# UART Driver Module

This module provides a DMA-based, Ring-Buffered UART driver with logic channel abstraction and zero hardcoded hardware dependencies.

## Features
- **Zero Copy (ish)**: Uses DMA for both Transmission and Reception to minimize CPU checks.
- **Ring Buffers**: Software ring buffers for RX and TX to handle bursts of data smoothly.
- **Dependency Injection**: Decoupled from `main.h` and hardcoded `huart` handles. You register hardware handles to logic channels at runtime.
- **Cross-Series Compatible**: Auto-detects STM32F1/F4/etc HAL libraries.

## CubeMX Configuration Requirements

To use this driver, you **MUST** configure the UART peripheral in STM32CubeMX (or manually) as follows:

1.  **Mode**: Asynchronous.
2.  **DMA Settings** (Critical!):
    *   **RX**: 
        *   Mode: **Circular** (Required for continuous reception)
        *   Data Width: Byte
    *   **TX**:
        *   Mode: **Normal**
        *   Data Width: Byte
3.  **NVIC Settings**:
    *   Enable global interrupt for the UART (e.g., `USART2 global interrupt`).
4.  **Parameter Settings**:
    *   Configure Baud Rate, Word Length, etc. as needed here.

**Note:** If you do not configure DMA, `UART_Register` will succeed, but no data will be received or transmitted because the internal DMA handles will be NULL.

## Usage Guide

### 1. Define Logic Channels
In your application header or `main.c`:
```c
// Define unique IDs for your serial ports
#define UART_DEBUG 0
#define UART_SENSOR 1
```

### 2. Register Handles (Dependency Injection)
In your initialization code (e.g., `user_main` or after `MX_USARTx_UART_Init`):

```c
#include "uart.h"
#include "usart.h" // Provides huart2, huart3

void user_main(void) {
    // Inject hardware handles into logic channels
    // Ensure MX_USART2_UART_Init() and MX_DMA_Init() have been called!
    UART_Register(UART_DEBUG, &huart2);
    // UART_Register(UART_SENSOR, &huart3);

    UART_SendString(UART_DEBUG, "System Booted.\r\n");
}
```

### 3. Sending Data
```c
uint8_t data[] = {0x01, 0x02, 0x03};
UART_Send(UART_DEBUG, data, 3);
UART_SendString(UART_DEBUG, "Hello World\n");
```

### 4. Receiving Data
```c
// Polling Mode (Check in loop)
uint8_t byte;
if (UART_Read(UART_DEBUG, &byte)) {
    // Process byte...
}

// Check available count
if (UART_Available(UART_DEBUG) > 10) {
    // ...
}
```

### 5. Main Loop Polling
You **MUST** call `UART_Poll()` periodically (e.g., inside your main `while(1)` or a FreeRTOS task) to process DMA pointers and handle errors.

```c
while (1) {
    UART_Poll(); 
    // ... other tasks
}
```

## Troubleshooting

### No Data Received?
1.  **Check DMA**: Did you set RX DMA to **Circular** in CubeMX?
2.  **Check Interrupts**: Is the UART global interrupt enabled in NVIC?
3.  **Check Registration**: Did you call `UART_Register` with a valid, initialized `&huart`?
4.  **RAM Overflow**: If your chip has small RAM (like F103C8T6's 20KB), check `UART_RX_BUF_SIZE` in `uart.h`. Default might be too large (e.g., 2048), try 128.

### Garbage Data?
1.  Check Baud Rate mismatch.
2.  Check CPU Clock configuration in CubeMX.


## Running Tests
An interactive test suite is provided in `user/drivers/communication/uart_tests.c`. It runs a CLI-like loop to verify all driver features.

### Commands
Send single characters to the UART to trigger tests:
*   `s` (**Send Bursts**): Rapidly calls `UART_SendString` 3 times. Tests TX DMA queueing and High-Throughput stability.
*   `f` (**Flush**): Clears the RX RingBuffer and syncs DMA pointers. Tests `UART_Flush`.
*   `b` (**Busy Check**): Sends a long string and immediately queries `UART_IsTxBusy`. Verifies non-blocking TX status.
*   `e` (**Error Stats**): Prints hardware/DMA error counters (Overrun, Noise, Frame, DMA Errors).
*   `r` (**Blocking RX**): Enters a blocking receive mode with a 5s timeout. Tests `UART_Receive` and timeout logic.
*   **Echo**: Any other character will be echoed back immediately.

### Expected Output (Example)
```
===================================
      UART Driver Test Suite       
===================================
Cmds: [s]SendBursts [f]Flush [b]BusyCheck [e]Errors [r]BlockRx
[Tick] System Alive: 1000 ms
[Tick] System Alive: 2000 ms
```
