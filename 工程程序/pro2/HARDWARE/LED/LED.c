#include "led.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	   

//初始化PB5和PE5为输出口.并使能这两个口的时钟		    
//LED IO初始化
void LED_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);	 //使能PB,PE端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 //LED0-->PB.5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
	GPIO_SetBits(GPIOB,GPIO_Pin_5);						 //PB.5 输出高

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	    		 //LED1-->PE.5 端口配置, 推挽输出
	GPIO_Init(GPIOE, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
	GPIO_SetBits(GPIOE,GPIO_Pin_5); 						 //PE.5 输出高 
}
void LED_RGB_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB,PE端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 //LED0-->PB.5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
	GPIO_ResetBits(GPIOB,GPIO_Pin_5);						 //PB.5 输出高

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	    		 //LED1-->PE.5 端口配置, 推挽输出
	GPIO_Init(GPIOE, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
	GPIO_ResetBits(GPIOB,GPIO_Pin_3); 						 //PE.5 输出高 .
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	    		 //LED1-->PE.5 端口配置, 推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
	GPIO_ResetBits(GPIOB,GPIO_Pin_5); 						 //PE.5 输出高 .
}
//led：
//0：红，1：绿，2：蓝
//mode
//0：亮，其它：灭

void LED_RGB(unsigned int led,unsigned int mode)
{
	if(mode==0)
	{
		
		switch(led)
		{
			case 0:{red=1;green=0;blue=0;}break;
			case 1:{red=0;green=1;blue=0;}break;
			case 2:{red=0;green=0;blue=1;}break;	
		}
		
	}
	else
	{
		red=0;
		green=0;
		blue=0;
	}
}
//LED闪烁，高电平点亮
//0：红
//1：绿
//2：蓝
void LED_shine(unsigned int color)
{
	switch(color)
	{
		case 0:{LED_RGB(0,1);delay_ms(100);	LED_RGB(0,0);delay_ms(100);	}break;
		case 1:{LED_RGB(1,1);delay_ms(100);	LED_RGB(1,0);delay_ms(100);	}break;
		case 2:{LED_RGB(2,1);delay_ms(100);	LED_RGB(2,0);delay_ms(100);	}break;
	}
}
void led_test()
{
	LED_RGB(0,1);delay_ms(10000);	LED_RGB(0,0);delay_ms(10000);
	LED_RGB(0,1);delay_ms(10000);	LED_RGB(0,0);delay_ms(10000);
}


//void led_shing(void)
//{
//	LED0=0;
//	delay_ms(100);
//	LED0=1;
//	delay_ms(100);
//}
