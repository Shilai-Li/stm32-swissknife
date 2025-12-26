# USB CDC Driver Module

This module wraps the STM32 USB CDC Middleware into a simple, Arduino-like Serial interface.

## Prerequisites

1.  **STM32CubeMX Configuration**:
    *   **Connectivity -> USB**: Enable "Device (FS)".
    *   **Middleware -> USB_DEVICE**: Select "Communication Device Class (VCP)".
    *   **Configuration -> NVIC**: Enable USB global interrupt.
    *   **Heap/Stack**: Increase Heap Size (e.g., 0x400) and Stack Size (e.g., 0x400) in "Project Manager -> Linker Settings" slightly, as USB stack uses some memory.

## Integration Steps (Critical!)

Since CubeMX re-generates the `USB_DEVICE` folder, you must manually add hooks to link our driver with the ST USB Stack.

### 1. Modify `USB_DEVICE/App/usbd_cdc_if.c`

Open the generated file `usbd_cdc_if.c` and make two changes:

**Step A: Include Header**
Go to `/* USER CODE BEGIN Includes */` section:
```c
/* USER CODE BEGIN Includes */
#include "usb_cdc.h"
/* USER CODE END Includes */
```

**Step B: Capture Received Data**
Find the function `CDC_Receive_FS`. Add the callback hook inside:

```c
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
  
  // >>> INSERT THIS LINE <<<
  USB_CDC_RxCallback(Buf, *Len); 
  // >>> END INSERT <<<

  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
  /* USER CODE END 6 */
}
```

### 2. Initialization
In your `main.c` (or `user_main.c`):
*   Ensure `MX_USB_DEVICE_Init()` is called (CubeMX does this).
*   Call `USB_CDC_Init()` to reset buffers.
*   **Wait for Enumeration**: USB takes 1-2 seconds to enumerate on the PC. Don't send data immediately after boot.

## Usage API

```c
#include "usb_cdc.h"

// 1. Check & Read
if (USB_CDC_Available()) {
    uint8_t c;
    USB_CDC_Read(&c);
}

// 2. Send (Blocking with small timeout)
USB_CDC_SendString("Hello USB!\r\n");
USB_CDC_Printf("Value: %d\r\n", 123);

// 3. Receive with Timeout
uint8_t buf;
if (USB_CDC_Receive(&buf, 1000)) { // Wait 1s
    // Got it
}

// 4. Flush
USB_CDC_Flush();
```

## Running Tests

An interactive test suite is provided in `usb_cdc_tests.c`.

### Commands
Send characters via Serial Terminal (e.g., Putty, Teraterm) to trigger:
*   `s`: **Burst Send** test.
*   `f`: **Flush** RX buffer.
*   `e`: **Echo** (Default behavior).

### Known Issues
*   **Windows Driver**: Windows 10/11 usually auto-installs. Windows 7 may need ST VCP Driver.
*   **Blocking**: `USB_CDC_Send` is blocking (with 50ms timeout). Don't use it in high-priority interrupts.
