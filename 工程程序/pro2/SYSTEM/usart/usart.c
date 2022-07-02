#include "sys.h"
#include "usart.h"	  
#include <stdio.h>

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 


void USART_SendChar(USART_TypeDef* USARTx,u8 data)
{
	//这行加了可以防止一些错误，但对发送频率和数据量要求不大的情况下，错误较少
	//while((USARTx->SR&0X40)==0); //用蓝牙后，这里卡住，不确定为什么
		USARTx->DR = data;
}

void USART_SendString(USART_TypeDef* USARTx,u8 *DAT)
{
	while(*DAT != '\0')
	{
	 	USART_SendChar(USARTx,*DAT++);
		DAT++;
	}	
}
unsigned char USART_ReceiveChar(USART_TypeDef* USARTx)
{
    return USARTx->DR;
}
void USART_SendLine(USART_TypeDef* USARTx)
{
    USART_SendChar(USARTx, 0x0D);                   // 换行
    USART_SendChar(USARTx, 0x0A);                   // 回车
}

//==================================================================================================
//  函数功能: USART 发送数组
//  函数标记: 内设驱动函数
//  函数说明: 无
//--------------------------------------------------------------------------------------------------
//  输入参量: USARTx - 通道号  取值范围 - USARTx1/USARTx2/USARTx3/UARTx4/UARTx5/USARTx6
//            Array - 待发送数组的指针  取值范围 - 指针类型
//            Count - 待发送数组的数量  取值范围 - 0~255
//  输出参量: 无
//--------------------------------------------------------------------------------------------------
//  |   -   |   -   |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   
//==================================================================================================
void USART_SendArray(USART_TypeDef* USARTx, uint8_t *Array, uint16_t Count)
{
    uint8_t i = 0;                            // 定义局部变量 用于函数循环
    for(i=0; i<Count; i++)
    {
        USART_SendChar(USARTx, Array[i]);    
    }
}
//==================================================================================================
//  函数功能: USART 发送数字 浮点类型
//  函数标记: 内设驱动函数
//  函数说明: 本函数打印浮点数字与实际输入数字存在偏差
//--------------------------------------------------------------------------------------------------
//  输入参量: USARTx - 通道号  取值范围 - USARTx1/USARTx2/USARTx3/UARTx4/UARTx5
//            Number - 待发送浮点型数字  取值范围 - -99999.99999~99999.99999
//            Count1 - 整数显示位数  取值范围 - 0~5
//            Count2 - 小数显示位数  取值范围 - 0~5
//  输出参量: 无
//--------------------------------------------------------------------------------------------------
//  |   -   |   -   |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   
//==================================================================================================
void USART_SendNumber_Float(USART_TypeDef* USARTx, float Number, uint8_t Count1, uint8_t Count2)
{
    uint8_t Number_Integer_Array[5];          // 定义局部数组 用于存储整数位各位数据
    uint8_t Number_Decimal_Array[5];          // 定义局部数组 用于存储小数位各位数据

    int32_t  Number_Integer = 0;          // 定义局部变量 表示浮点数的整数部分
    int32_t  Number_Decimal = 0;          // 定义局部变量 表示浮点数的小数部分

    // 判断 浮点数字正负
    if(Number < 0)
    {
        Number = 0 - Number;
        USART_SendChar(USARTx, '-');
    }
    else
    {
        USART_SendChar(USARTx, '+');
    }
    
    // 限制 发送位数
    // 由于由于float型的有效十进制数值最多为7位，即整数位 + 小数位 数量一定小于等于7
    while((Count1 + Count2 > 7 ))
    {
        if((Count1 > 5) && (Count1 != 0))
        {
            --Count1;
        }
        else
        {
            --Count2;
        }
    }
    
    // 分离 整数位与小数位
    // 取整数部分
    Number_Integer = (uint32_t)(Number);  
    // 取小数部分 1e5科学计数法表示10000
    Number_Decimal = (uint32_t)((Number - Number_Integer + 0.000005) * 1e5);
        
    // 计算各位数值的字符
    if(Count1>4) Number_Integer_Array[0]=(Number_Integer/10000) % 10 + 0x30; 
    if(Count1>3) Number_Integer_Array[1]=(Number_Integer/1000 ) % 10 + 0x30; 
    if(Count1>2) Number_Integer_Array[2]=(Number_Integer/100  ) % 10 + 0x30; 
    if(Count1>1) Number_Integer_Array[3]=(Number_Integer/10   ) % 10 + 0x30; 
    if(Count1>0) Number_Integer_Array[4]=(Number_Integer/1    ) % 10 + 0x30; 
    
    if(Count2>0) Number_Decimal_Array[0]=(Number_Decimal/10000) % 10 + 0x30; 
    if(Count2>1) Number_Decimal_Array[1]=(Number_Decimal/1000 ) % 10 + 0x30; 
    if(Count2>2) Number_Decimal_Array[2]=(Number_Decimal/100  ) % 10 + 0x30; 
    if(Count2>3) Number_Decimal_Array[3]=(Number_Decimal/10   ) % 10 + 0x30; 
    if(Count2>4) Number_Decimal_Array[4]=(Number_Decimal/1    ) % 10 + 0x30; 
   
    USART_SendArray(USARTx, &Number_Integer_Array[5-Count1], Count1);       // 显示整数位
    USART_SendChar(USARTx, '.');
    USART_SendArray(USARTx, &Number_Decimal_Array[0],        Count2);       // 显示小数位
}

#if EN_USART_RX   
	
u8 USART_RX_BUF[USART_REC_LEN];    

u16 USART_RX_STA=0;       	  
  

void USART1_Init(u32 bound)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	

	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1, ENABLE);                   

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//开启中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);	

}
unsigned int face_pass=0;//人脸识别通过
unsigned int face_pass2=0;//人脸识别通过
unsigned int mask_pass=0;
unsigned int mask_pass2=0;
unsigned char face_num[5];//编号
unsigned int test; //调试模式
u8 Res;	
void USART1_IRQHandler(void)                
{
	    
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//接收到数据
	{	 
		
	    Res = USART_ReceiveData(USART1);
		if(Res== 'a') //人脸识别通过
		{
			face_pass=1;		
		}
		else if(Res=='b' && face_pass==1)//口罩识别通过
		{
			mask_pass=1;	
		}
		else if(Res=='t' && test==0)//进入调试模式
		{
			face_pass=0;
			mask_pass=0;
			test=1;
		}
		else if(Res=='f' && test==1)
		{ 
			test=0;
		}
//		if(Res== '1' && face_pass==1)
//		{
//			face_pass2=1;
//		}
//		if(Res== '2' && mask_pass==1)
//		{
//			mask_pass2=1;
//		}
//		else if(face_pass>0 && face_pass<=5)
//		{			
//			face_num[face_pass-1] = Res;//添加人脸识别编号字符数组
//		}
		USART_ClearFlag(USART1,USART_FLAG_TC);
	}
} 
#endif	


#if EN_USART3_RX  
 	
u8 USART3_RX_BUF[USART_REC_LEN];    

u16 USART3_RX_STA=0;       
  
void UART3_Init(u32 bound)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
   

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	

	USART_Init(USART3, &USART_InitStructure); 
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART3, ENABLE);     
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);	
  
	
}

void USART3_IRQHandler(void) 
{  
    if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET) 
    {
        USART_ClearFlag(USART3, USART_IT_RXNE);
    }  
}

#endif

