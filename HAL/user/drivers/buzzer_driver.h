#ifndef __BUZZER_DRIVER_H__
#define __BUZZER_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

// Buzzer driver interface configuration
typedef enum
{
  BUZZER_1 = 0,
  BUZZER_2 = 1,

  // Number of Buzzers
  BUZZER_n
} Buzzer_TypeDef;

void BUZZER_Init(void);
void BUZZER_On(Buzzer_TypeDef Buzzer);
void BUZZER_Off(Buzzer_TypeDef Buzzer);
void BUZZER_Toggle(Buzzer_TypeDef Buzzer);

#ifdef __cplusplus
}
#endif

#endif /* __BUZZER_DRIVER_H__ */
