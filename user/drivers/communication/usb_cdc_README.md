# USB CDC Driver Module

This module wraps the STM32 USB CDC Middleware into a simple, Arduino-like Serial interface.

## ⚠️ CRITICAL WARNINGS (READ FIRST!)

If your USB is not working (enumerates failed, or can send but not receive), it is 99% due to one of these two reasons:

### 1. You MUST Increase Stack Size
The USB Stack consumes a lot of memory during initialization and enumeration.
*   **Bare Metal**: Increase `Stack_Size` in `startup_xxx.s` or Linker Settings (min **0x800**).
*   **FreeRTOS**: If `MX_USB_DEVICE_Init()` is called inside a Task (e.g., `defaultTask`), you **MUST** increase that task's stack size to at least **512 words (2048 bytes)**. The default 128 words is NOT enough and will cause Silent HardFaults or Descriptor Request Failures.

### 2. You MUST Manually Hook `usbd_cdc_if.c`
CubeMX generates `USB_DEVICE/App/usbd_cdc_if.c`. By default, it throws away received data.
**You must manually edit `CDC_Receive_FS` in that file** to call `USB_CDC_RxCallback`.
See the **Integration Steps** section below for exact code.

---

## Prerequisites

1.  **STM32CubeMX Configuration**:
    *   **Connectivity -> USB**: Enable "Device (FS)".
    *   **Connectivity -> USB_OTG_FS**: Disable "VBUS Sensing" (PA9) if your board doesn't wire it to 5V (Common cause of "No Enumeration").
    *   **Middleware -> USB_DEVICE**: Select "Communication Device Class (VCP)".
    *   **Configuration -> NVIC**: Enable USB global interrupt.

## Integration Steps

**⚠️ You MUST perform these modifications manually. The driver will NOT receive data otherwise.**

Since CubeMX generates the USB Middleware, the `CDC_Receive_FS` callback is isolated inside `USB_DEVICE/App/usbd_cdc_if.c`. Typically, this generated function receives data but **throws it away** (does nothing with it).

### 1. Modify `USB_DEVICE/App/usbd_cdc_if.c`

Open the generated file `usbd_cdc_if.c`. Detailed steps:

**Step A: Include Header**
Go to `/* USER CODE BEGIN Includes */` section:
```c
/* USER CODE BEGIN Includes */
#include "communication/usb_cdc.h" // Adjust path if using different include structure
/* USER CODE END Includes */
```

**Step B: Capture Received Data (The Hook)**
Find the function **`CDC_Receive_FS`**. It is the ONLY place where incoming data is available. You must inject our callback here:

```c
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
  
  // ============================================================
  // >>> CRITICAL: Pass data to our RingBuffer <<<
  // Without this line, you can SEND data but will never RECEIVE anything!
  // ============================================================
  USB_CDC_RxCallback(Buf, *Len); 

  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
  /* USER CODE END 6 */
}
```

### 2. Initialization
In your `main.c`:
*   Ensure `MX_USB_DEVICE_Init()` is called (CubeMX does this).
*   Call `USB_CDC_Init()` to clear buffers.
*   **Wait for Enumeration**: USB enumeration takes 1-2 seconds. `USB_CDC_Send` will fail (silent drop) if called before the PC recognizes the device.


---

## Disabling USB in CubeMX
If you decide to disable USB support in CubeMX:
1.  **Regenerate Code**: CubeMX might delete or unlink `usbd_cdc_if.c`.
2.  **Update CMakeLists**: You MUST remove `usb_cdc` from your `ENABLED_MODULES` list.
    *   Since `usb_cdc.c` includes `usbd_cdc_if.h`, compiling it without the USB stack enabled will cause immediate build errors.


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
