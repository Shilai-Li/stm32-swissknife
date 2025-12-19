#ifndef _USART_H
#define _USART_H
#include "sys.h"
#include "stdio.h"	

////////////////////////////////////////////////////////////////////////////////// 	
#define USART_REC_LEN  			200  	//Maximum receive data bytes 200
#define EN_USART1_RX 			1		//Enable (1)/Disable (0) serial port 1 receive
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //Receive buffer, maximum USART_REC_LEN bytes. Last byte is newline character 
extern u16 USART_RX_STA;         		//Receive status mark	
extern UART_HandleTypeDef UART1_Handler; //UART handle

#define RXBUFFERSIZE   1 //Buffer size
extern u8 aRxBuffer[RXBUFFERSIZE];//HAL library USART receive Buffer

//If you want to use serial port interrupt reception, please don't comment out the following macros
void uart_init(u32 bound);


#endif
