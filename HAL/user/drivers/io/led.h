#ifndef __LED_DRIVER_H__
#define __LED_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

// LED driver interface configuration
typedef enum
{
  LED_1 = 0,
  LED_2,

  // Number of LEDs
  LED_n
} Led_TypeDef;

void LED_Init(void);
void LED_On(Led_TypeDef Led);
void LED_Off(Led_TypeDef Led);
void LED_Toggle(Led_TypeDef Led);

#ifdef __cplusplus
}
#endif

#endif /* __LED_DRIVER_H__ */