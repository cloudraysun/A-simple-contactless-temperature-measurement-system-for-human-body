#include "SR04.h"
#include "stdio.h"	
#include "delay.h"
#include "usart.h"
#include "timer.h"



float Distance;
 
void HC_SR04_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
 
	RCC_APB2PeriphClockCmd(HC_SR04_TRIG_CLK, ENABLE);	 //使能PC端口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//外部中断，需要使能AFIO时钟
 
	GPIO_InitStructure.GPIO_Pin = HC_SR04_TRIG_PIN;				 // 脉冲触发端口（Trig）配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(HC_SR04_TRIG_PORT, &GPIO_InitStructure);					 //根据设定参数初始化端口
	//GPIO_ResetBits(HC_SR04_TRIG_PORT,HC_SR04_TRIG_PIN);						 //端口初始化为低电平
 
	GPIO_InitStructure.GPIO_Pin = HC_SR04_ECHO_PIN;				 // 回波接收端口（Echo）配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(HC_SR04_ECHO_PORT, &GPIO_InitStructure);					 //根据设定参数初始化端口
 
 
	//接收端口 中断线以及中断初始化配置
	GPIO_EXTILineConfig(HC_SR04_ECHO_Exit_PORT,HC_SR04_ECHO_Exit_PIN);
 
	EXTI_InitStructure.EXTI_Line=HC_SR04_ECHO_Exit_Line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//上升沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器
 
	NVIC_InitStructure.NVIC_IRQChannel = HC_SR04_ECHO_Exit_Channel;			//使能按键所在的外部中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);
 
	//TIM3_Int_Init(49999,7199);  //初始化TIM3定时器，计数一次为1/10000S（0.1ms），每500ms触发一次定时中断
  //TIM_Cmd(TIM3,DISABLE);
}
	
//发送20us的脉冲触发信号
void HC_SR04_start(void)
{
	GPIO_SetBits(HC_SR04_TRIG_PORT,HC_SR04_TRIG_PIN);
	delay_us(20);
	GPIO_ResetBits(HC_SR04_TRIG_PORT,HC_SR04_TRIG_PIN);
	delay_ms(10);
}
 
 
 void EXTI9_5_IRQHandler(void)
{			
	delay_us(10);
	
	if(EXTI_GetITStatus(EXTI_Line6) != RESET)
	{
		//while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5) == RESET);
		TIM_SetCounter(HC_SR04_TIM,0);  //计数清零
		TIM_Cmd(HC_SR04_TIM,ENABLE);  //使能TIM3定时器
		
		while(GPIO_ReadInputDataBit(HC_SR04_ECHO_PORT,HC_SR04_ECHO_PIN));  //等待电平变为低电平
		
		TIM_Cmd(HC_SR04_TIM,DISABLE); //关闭定时器
		
		Distance = TIM_GetCounter(HC_SR04_TIM)*344/200.0;  //计算距离：cnt * 1/10000 * 340 / 2(单位：m)
		
		//printf("Counter:%d\n",TIM_GetCounter(TIM3));
		//printf("Distance:%f cm\r\n",Distance);
			
		EXTI_ClearITPendingBit(HC_SR04_ECHO_Exit_Line);
	}  
}


