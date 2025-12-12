#include "sys.h"

//System clock configuration function
//Fvco=Fs*(plln/pllm);
//SYSCLK=Fvco/pllp=Fs*(plln/(pllm*pllp));
//Fusb=Fvco/pllq=Fs*(plln/(pllm*pllq));

//Fvco:VCO frequency
//SYSCLK:System clock frequency
//Fusb:USB,SDIO,RNG and other clock frequencies
//Fs:PLL input clock frequency, can be HSI,HSE etc.
//plln:Main PLL multiplication factor (PLL multiplication), range:64~432.
//pllm:Main PLL division PLL prescaler factor (division before PLL), range:2~63.
//pllp:System clock main PLL division factor (division after PLL), range:2,4,6,8.(only these 4 values!)
//pllq:USB/SDIO/random number generator etc. main PLL division factor (division after PLL), range:2~15.

//External crystal is 25M, recommended values: plln=360,pllm=25,pllp=2,pllq=8.
//Result: Fvco=25*(360/25)=360Mhz
//     SYSCLK=360/2=180Mhz
//     Fusb=360/8=45Mhz
//Return value: 0, success; 1, failure
void Stm32_Clock_Init(u32 plln,u32 pllm,u32 pllp,u32 pllq)
{
    HAL_StatusTypeDef ret = HAL_OK;
    RCC_OscInitTypeDef RCC_OscInitStructure; 
    RCC_ClkInitTypeDef RCC_ClkInitStructure;
    
    __HAL_RCC_PWR_CLK_ENABLE(); //Enable PWR clock
    
    //Configure voltage regulator output level to scale1 mode for use with overclocking, this feature is only available on STM32F42xx and STM32F43xx
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);//Set voltage regulator to scale 1
    
    RCC_OscInitStructure.OscillatorType=RCC_OSCILLATORTYPE_HSE;    //Clock source is HSE
    RCC_OscInitStructure.HSEState=RCC_HSE_ON;                      //Enable HSE
    RCC_OscInitStructure.PLL.PLLState=RCC_PLL_ON;//Enable PLL
    RCC_OscInitStructure.PLL.PLLSource=RCC_PLLSOURCE_HSE;//PLL clock source select HSE
    RCC_OscInitStructure.PLL.PLLM=pllm; //Main PLL division PLL prescaler factor (division before PLL), range:2~63.
    RCC_OscInitStructure.PLL.PLLN=plln; //Main PLL multiplication factor (PLL multiplication), range:64~432.  
    RCC_OscInitStructure.PLL.PLLP=pllp; //System clock main PLL division factor (division after PLL), range:2,4,6,8.(only these 4 values!)
    RCC_OscInitStructure.PLL.PLLQ=pllq; //USB/SDIO/random number generator etc. main PLL division factor (division after PLL), range:2~15.
    ret=HAL_RCC_OscConfig(&RCC_OscInitStructure);//Initialize
	
    if(ret!=HAL_OK) while(1);
    
    ret=HAL_PWREx_EnableOverDrive(); //Enable Over-Driver function
    if(ret!=HAL_OK) while(1);
    
    //Select PLL as system clock source and configure HCLK,PCLK1 and PCLK2
    RCC_ClkInitStructure.ClockType=(RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStructure.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK;//Set system clock source as PLL
    RCC_ClkInitStructure.AHBCLKDivider=RCC_SYSCLK_DIV1;//AHB division factor is 1
    RCC_ClkInitStructure.APB1CLKDivider=RCC_HCLK_DIV4; //APB1 division factor is 4
    RCC_ClkInitStructure.APB2CLKDivider=RCC_HCLK_DIV2; //APB2 division factor is 2
    ret=HAL_RCC_ClockConfig(&RCC_ClkInitStructure,FLASH_LATENCY_5);//Set FLASH delay time to 5WS, which is 6 CPU cycles.
		
    if(ret!=HAL_OK) while(1);
}

#ifdef  USE_FULL_ASSERT
//When using this function, the file name and line number will be displayed when an error occurs
//file: pointer to source file
//line: line number in the file
void assert_failed(uint8_t* file, uint32_t line)
{ 
	while (1)
	{
	}
}
#endif

//THUMB instructions do not support software interrupts
//The following method implements the execution of WFI instruction
__asm void WFI_SET(void)
{
	WFI;		  
}
//Disable all interrupts (but not fault and NMI interrupts)
__asm void INTX_DISABLE(void)
{
	CPSID   I
	BX      LR	  
}
//Enable all interrupts
__asm void INTX_ENABLE(void)
{
	CPSIE   I
	BX      LR  
}
//Set stack top address
//addr: stack top address
__asm void MSR_MSP(u32 addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}
