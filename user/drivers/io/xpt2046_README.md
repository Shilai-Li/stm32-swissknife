# XPT2046 Touch Controller Driver

Totally decoupled, interface-based driver for XPT2046 Resistive Touch Controller.

## Features
*   **Zero Dependency**: Use ANY communication interface (HAL SPI, Soft SPI, Custom) via function pointers.
*   **Rotation Support**: Built-in coordinate mapping for 0/90/180/270 degrees.
*   **Median Filter**: Software filtering to reduce jitter.

## Usage

### 1. Define Wrappers (Application Layer)
You need to provide a function that matches the `XPT_TransmitReceive_Func` signature.

```c
// Example Wrapper for HAL SPI
uint8_t HAL_SPI_Wrapper(void *handle, uint8_t *tx, uint8_t *rx, uint16_t size, uint32_t timeout) {
    return (uint8_t)HAL_SPI_TransmitReceive((SPI_HandleTypeDef*)handle, tx, rx, size, timeout);
}

// Example Wrapper for Soft SPI
uint8_t Soft_SPI_Wrapper(void *handle, uint8_t *tx, uint8_t *rx, uint16_t size, uint32_t timeout) {
    return Soft_SPI_TransmitReceive((Soft_SPI_HandleTypeDef*)handle, tx, rx, size, timeout);
}
```

### 2. Initialization

```c
XPT2046_HandleTypeDef touch;
extern SPI_HandleTypeDef hspi1;

// Init with Hardware SPI
XPT2046_Init(&touch, &hspi1, HAL_SPI_Wrapper, 
             GPIOA, GPIO_PIN_4, // CS
             GPIOA, GPIO_PIN_1); // IRQ

// OR Init with Software SPI
XPT2046_Init(&touch, &soft_spi, Soft_SPI_Wrapper, 
             GPIOA, GPIO_PIN_4, 
             GPIOA, GPIO_PIN_1);
```

### 3. Loop
```c
if (XPT2046_IsTouched(&touch)) {
    uint16_t x, y;
    if (XPT2046_GetCoordinates(&touch, &x, &y)) {
        printf("Touch: %d, %d\r\n", x, y);
    }
}
```
