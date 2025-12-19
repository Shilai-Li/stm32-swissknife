#include "usart.h"
#include "delay.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//If using os, include the following header files.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os use	  	  
#endif


//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)	
#if 1
#pragma import(__use_no_semihosting)             
//Standard library support functions required                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//Define _sys_exit() to avoid using semi-hosting mode    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//Redefine fputc function 
int fputc(int ch, FILE *f)
{	
	while((USART1->SR&0X40)==0);//Loop until transmission is complete   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif 

#if EN_USART1_RX   //If receive is enabled
//Serial port 1 interrupt service function
//Note, reading USARTx->SR can avoid inexplicable errors   	
u8 USART_RX_BUF[USART_REC_LEN];     //Receive buffer, maximum USART_REC_LEN bytes.
//Receive status
//bit15:	Receive completion flag
//bit14:	Received 0x0d
//bit13~0:	Number of valid bytes received
u16 USART_RX_STA=0;       //Receive status mark	

u8 aRxBuffer[RXBUFFERSIZE];//HAL library used serial port receive buffer
UART_HandleTypeDef UART1_Handler; //UART handle

//Initialize IO Serial port 1 
//bound: baud rate
void uart_init(u32 bound)
{	
	//UART initialization settings
	UART1_Handler.Instance=USART1;						    //USART1
	UART1_Handler.Init.BaudRate=bound;					    //Baud rate
	UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //8-bit data format
	UART1_Handler.Init.StopBits=UART_STOPBITS_1;		    //One stop bit
	UART1_Handler.Init.Parity=UART_PARITY_NONE;		    //No parity bit
	UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //No hardware flow control
	UART1_Handler.Init.Mode=UART_MODE_TX_RX;		    //Transmit and receive mode
	HAL_UART_Init(&UART1_Handler);					    //HAL_UART_Init() will use UART1
	
	HAL_UART_Receive_IT(&UART1_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE);//This function will enable receive interrupt, flag bit UART_IT_RXNE, and set receive buffer and buffer size
  
}

//UART bottom driver initialization, clock enable, interrupt configuration
//This function will be called by HAL_UART_Init()
//huart: serial port handle

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO port settings
	GPIO_InitTypeDef GPIO_Initure;
	
	if(huart->Instance==USART1)//If it's serial port 1, perform USART1 MSP initialization
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();			//Enable GPIOA clock
		__HAL_RCC_USART1_CLK_ENABLE();			//Enable USART1 clock
	
		GPIO_Initure.Pin=GPIO_PIN_9;			//PA9
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//Alternate function push-pull
		GPIO_Initure.Pull=GPIO_PULLUP;			//Pull-up
		GPIO_Initure.Speed=GPIO_SPEED_FAST;		//Fast
		GPIO_Initure.Alternate=GPIO_AF7_USART1;	//Alternate as USART1
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//Initialize PA9

		GPIO_Initure.Pin=GPIO_PIN_10;			//PA10
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//Initialize PA10
		
#if EN_USART1_RX
		HAL_NVIC_EnableIRQ(USART1_IRQn);				//Enable USART1 interrupt channel
		HAL_NVIC_SetPriority(USART1_IRQn,3,3);			//Preemption priority 3, sub priority 3
#endif	
	}

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//If it's serial port 1
	{
		if((USART_RX_STA&0x8000)==0)//Reception not complete
		{
			if(USART_RX_STA&0x4000)//Received 0x0d
			{
				if(aRxBuffer[0]!=0x0a)USART_RX_STA=0;//Reception error, restart
				else USART_RX_STA|=0x8000;	//Reception completed 
			}
			else //Haven't received 0X0D
			{	
				if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0] ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//Reception data error, restart reception	  
				}		 
			}
		}

	}
}

u32 maxDelay=0x1FFFF;
//Serial port 1 interrupt service function
void USART1_IRQHandler(void)                	
{ 
	u32 timeout=0;
#if SYSTEM_SUPPORT_OS	 	//Use OS
	OSIntEnter();   
#endif
	
	HAL_UART_IRQHandler(&UART1_Handler);	//Call HAL library interrupt handling function
	
	timeout=0;
	
    while (HAL_UART_GetState(&UART1_Handler) != HAL_UART_STATE_READY)//Wait for ready
	{
	 timeout++;////Timeout processing
     if(timeout>maxDelay) break;		
	}
     
	timeout=0;
	while(HAL_UART_Receive_IT(&UART1_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE) != HAL_OK)//After one reception is completed, re-enable interrupt reception, RxXferCount is 1
	{
	 timeout++; //Timeout processing
	 if(timeout>maxDelay) break;	
	}
#if SYSTEM_SUPPORT_OS	 	//Use OS
	OSIntExit();  											 
#endif
} 
#endif	

/*The following code directly affects the interrupt open logic, written in the interrupt service function, not internally*/
/*


//Serial port 1 interrupt service function
void USART1_IRQHandler(void)                	
{ 
	u8 Res;
#if SYSTEM_SUPPORT_OS	 //Use OS
	OSIntEnter();    
#endif
	if((__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_RXNE)!=RESET))  //Receive interrupt (received data must end with 0x0d 0x0a)
	{
        HAL_UART_Receive(&UART1_Handler,&Res,1,1000); 
		if((USART_RX_STA&0x8000)==0)//Receive not complete
		{
			if(USART_RX_STA&0x4000)//Received 0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//Receive error, restart
				else USART_RX_STA|=0x8000;	//Receive completed
			}
			else //Haven't received 0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//Receive data error, restart receive
				}		 
			}
		}   		 
	}
	HAL_UART_IRQHandler(&UART1_Handler);	
#if SYSTEM_SUPPORT_OS	 //Use OS
	OSIntExit();  											 
#endif
} 
#endif	
*/
 




