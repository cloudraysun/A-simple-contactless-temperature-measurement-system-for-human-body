#ifndef __USART_H
#define __USART_H
#include "main.h"	 

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART_RX 			1		//使能（1）/禁止（0）串口1接收
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义
void USART1_Init(u32 bound);
	  	
extern u8  USART3_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART3_RX_STA;         		//接收状态标记
void USART3_Init(u32 bound);

void UART1_Init(u32 bound);
void USART_SendChar(USART_TypeDef* USARTx,u8 data);
void USART_SendString(USART_TypeDef* USARTx,u8 *DAT);
void USART_SendLine(USART_TypeDef* USARTx);
unsigned char USART_ReceiveChar(USART_TypeDef* USARTx);
void USART_SendNumber_Float(USART_TypeDef* USARTx, float Number, uint8_t Count1, uint8_t Count2);

extern unsigned int face_pass;
extern unsigned int mask_pass;
extern unsigned int face_pass2;
extern unsigned int mask_pass2;
extern unsigned char face_num[5];
extern unsigned int test; //调试模式
extern u8 Res;	
#endif

