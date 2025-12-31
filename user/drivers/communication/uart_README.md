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
In your initialization code (e.g., `app_main` or after `MX_USARTx_UART_Init`):

```c
#include "uart.h"
#include "usart.h" // Provides huart2

// Define buffers relative to your RAM budget
static uint8_t debug_rx_dma[64];
static uint8_t debug_rx_ring[256];
static uint8_t debug_tx_ring[512];

void app_main(void) {
    // Inject hardware handles AND memory buffers into logic channels
    // Ensure MX_USART2_UART_Init() and MX_DMA_Init() have been called!
    
    UART_Register(UART_DEBUG, &huart2,
                  debug_rx_dma, sizeof(debug_rx_dma),
                  debug_rx_ring, sizeof(debug_rx_ring),
                  debug_tx_ring, sizeof(debug_tx_ring));

    UART_SendString(UART_DEBUG, "System Booted.\r\n");
}
```

### 2.5 Register Callback (Optional, for RTOS)
```c
void MyRxCallback(UART_Channel ch) {
    // Signal your RTOS task/semaphore here
}

UART_SetRxCallback(UART_DEBUG, MyRxCallback);
```

### 3. Sending Data
```c
uint8_t data[] = {0x01, 0x02, 0x03};
// Returns true if buffered successfully, false if buffer full
if (!UART_Send(UART_DEBUG, data, 3)) {
    // Handle overflow
}
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
## Release Notes (2025-12)
### Enhancements
- **Dynamic Buffers**: Replaced static macros with runtime buffer configuration (API Change).
- **Thread Safety**: Added critical sections to `UART_ProcessDMA` and `UART_TxKick`.
- **Callback Support**: Added `UART_SetRxCallback` for RTOS notification.
- **Error Handling**: `UART_Send` returns `bool` (false = buffer full).
- **USB CDC**: Fixed circular dependency in `usbd_cdc_if.c`.
