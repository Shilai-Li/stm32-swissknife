#include "delay.h"
#include "sys.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//If using ucos, include the following header files.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos use	  	  
#endif

static u32 fac_us=0;							//us delay multiplier

#if SYSTEM_SUPPORT_OS		
    static u16 fac_ms=0;				        //ms delay multiplier, in os, represents ms per OS tick
#endif

#if SYSTEM_SUPPORT_OS							//If SYSTEM_SUPPORT_OS is defined, it means OS support is needed (likely UCOS).
//When delay_us/delay_ms need OS support, the following three OS support functions are needed
//These 3 defines:
//delay_osrunning: indicates whether OS is currently running, 0=not running; 1=running
//delay_ostickspersec: indicates OS clock tick rate, number of ticks per second
//delay_osintnesting: indicates OS interrupt nesting level, because interrupt service routines should not call delay_ms
//Then these 3 functions:
//delay_osschedlock: disables OS scheduling to prevent interruption during us delay
//delay_osschedunlock: re-enables OS scheduling after us delay
//delay_ostimedly: calls OS delay function, this function will cause task scheduling

//Suitable for both UCOSII and UCOSIII support, for other OS please refer to the porting guide
//Support UCOSII
#ifdef 	OS_CRITICAL_METHOD					//If OS_CRITICAL_METHOD is defined, it means UCOSII support is needed				
#define delay_osrunning		OSRunning			//OS running flag, 0=not running; 1=running
#define delay_ostickspersec	OS_TICKS_PER_SEC	//OS clock ticks, number of ticks per second
#define delay_osintnesting 	OSIntNesting		//Interrupt nesting level, indicates interrupt service status
#endif

//Support UCOSIII
#ifdef 	CPU_CFG_CRITICAL_METHOD				//If CPU_CFG_CRITICAL_METHOD is defined, it means UCOSIII support is needed	
#define delay_osrunning		OSRunning			//OS running flag, 0=not running; 1=running
#define delay_ostickspersec	OSCfg_TickRate_Hz	//OS clock ticks, number of ticks per second
#define delay_osintnesting 	OSIntNestingCtr		//Interrupt nesting level, indicates interrupt service status
#endif


//Disable OS scheduling during us delay (to prevent interruption of us delay)
void delay_osschedlock(void)
{
#ifdef CPU_CFG_CRITICAL_METHOD   			//Use UCOSIII
	OS_ERR err; 
	OSSchedLock(&err);						//UCOSIII way, disable scheduling, prevent interruption of us delay
#else										//Otherwise UCOSII
	OSSchedLock();							//UCOSII way, disable scheduling, prevent interruption of us delay
#endif
}

//Re-enable OS scheduling after us delay
void delay_osschedunlock(void)	
{
#ifdef CPU_CFG_CRITICAL_METHOD  			//Use UCOSIII
	OS_ERR err; 
	OSSchedUnlock(&err);					//UCOSIII way, restore scheduling
#else										//Otherwise UCOSII
	OSSchedUnlock();						//UCOSII way, restore scheduling
#endif
}

//Call OS's built-in delay function to delay
//ticks: number of delay ticks
void delay_ostimedly(u32 ticks)
{
#ifdef CPU_CFG_CRITICAL_METHOD
	OS_ERR err; 
	OSTimeDly(ticks,OS_OPT_TIME_PERIODIC,&err); //UCOSIII delay function, periodic mode
#else
	OSTimeDly(ticks);						    //UCOSII delay
#endif 
}
 
//systick interrupt service function, used with OS
void SysTick_Handler(void)
{	
    HAL_IncTick();
	if(delay_osrunning==1)					//OS started, execute normal OS scheduling
	{
		OSIntEnter();						//Enter interrupt
		OSTimeTick();       				//Call ucos clock service routine               
		OSIntExit();       	 			//Trigger task scheduling switch
	}
}
#endif
			   
//��ʼ���ӳٺ���
//Initialize delay function
//When using ucos, this function will initialize ucos clock tick
//SYSTICK's clock is fixed to AHB clock
//SYSCLK: system clock frequency
void delay_init(u8 SYSCLK)
{
#if SYSTEM_SUPPORT_OS 							//If OS support is needed.
	u32 reload;
#endif
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);//SysTick frequency is HCLK
	fac_us=SYSCLK;										//Whether OS is used or not, fac_us needs to be used
#if SYSTEM_SUPPORT_OS 							//If OS support is needed.
	reload=SYSCLK;									    //Count per second in K units	   
	reload*=1000000/delay_ostickspersec;	//Set according to delay_ostickspersec
													//reload is 24-bit register, max value:16777216, at 180M, about 0.745s	
	fac_ms=1000/delay_ostickspersec;		//Represents the minimum unit of OS delay	   
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;	//Enable SYSTICK interrupt
	SysTick->LOAD=reload; 						//Interrupt every 1/OS_TICKS_PER_SEC	
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk; 	//Enable SYSTICK
#else
#endif
}												    

#if SYSTEM_SUPPORT_OS 							//If OS support is needed.
//Delay nus
//nus: number of us to delay.	
//nus:0~190887435(max value is 2^32/fac_us@fac_us=22.5)													   	
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;					//LOAD value		 
	ticks=nus*fac_us; 							//Number of ticks needed
	delay_osschedlock();						//Disable OS scheduling to prevent interruption of us delay
	told=SysTick->VAL;        					//Counter value when entering
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//Note that SYSTICK is a down counter.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//Exit when time exceeds required delay time.
		}  
	};
	delay_osschedunlock();						//Restore OS scheduling													    
}  
//Delay nms
//nms: number of ms to delay
//nms:0~65535
void delay_ms(u16 nms)
{	
	if(delay_osrunning&&delay_osintnesting==0)//If OS is already running and not in interrupt (interrupt cannot use task scheduling)	    
	{		 
		if(nms>=fac_ms)							//If delay time is greater than OS minimum delay time	 
		{ 
   			delay_ostimedly(nms/fac_ms);	//OS delay
		}
		nms%=fac_ms;							//OS cannot provide such small delay, use normal method	    
	}
	delay_us((u32)(nms*1000));				//Normal delay
}
#else  //When not using ucos

//Delay nus
//nus is the number of us to delay.	
//nus:0~190887435(max value is 2^32/fac_us@fac_us=22.5)	 
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;					//LOAD value		 
	ticks=nus*fac_us; 							//Number of ticks needed
	told=SysTick->VAL;        					//Counter value when entering
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//Note that SYSTICK is a down counter.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//Exit when time exceeds required delay time.
		}  
	};
}

//Delay nms
//nms: number of ms to delay
void delay_ms(u16 nms)
{
	u32 i;
	for(i=0;i<nms;i++) delay_us(1000);
}
#endif
			 