#include "main.h"

float Temperature;//测试温度

int main()
{
	float up,down;
//	unsigned int i;
	delay_init();
	LED_Init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	TIM2_Int_Init(4999,7199);//用于SR04
	TIM3_Int_Init(4999,7199);//10Khz的计数频率，计数到5000为50ms ，5000x7200/72=50ms
	USART1_Init(9600);
//	LED_RGB_Init();
//	key4x4_Init();
	MLX90614_Init();
	LCD_Init();//LCD初始化
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	HC_SR04_init();
	up = 37.50;
	down = 10.00;
	USART_SendChar(USART1,'A');//口罩识别通过，切换回人脸识别	

	while(1)
	{	
		LCD_ShowChinese_16x16(5,2,"体",BLACK,WHITE);
		LCD_ShowChinese_16x16(20,2,"温",BLACK,WHITE);
//		LCD_ShowString(0,130,"distence:",BLACK,WHITE,16,1);
//		TFT_ShowNumber_Float_16x16(70,130,Distance,3,2,BLACK,WHITE,16,0);
		if(Temperature>25)//临时测试，当接近时，才启动距离测量
			HC_SR04_start();
		if(Distance<1.72 || Distance>100)//说明在1cm以内
			Temperature=Read_Temp()+2;
		else if(Distance<=3.44)//说明在3~5cm
			Temperature=Read_Temp()+2.5;
		else if(Distance<=5.16)//说明在5~8cm
			Temperature=Read_Temp()+3.4;
		else if(Distance<=6.88)//说明在8~10cm
			Temperature=Read_Temp()+5;
		else if(Distance>6.88)//说明在10cm以外
			Temperature=Read_Temp();
		if(Temperature>up)
		{			
			TFT_ShowNumber_Float_16x16(70,2,Temperature,2,1,RED,WHITE,16,0);
			//LED_shine(0); //亮红灯
		}
		else
		{
			TFT_ShowNumber_Float_16x16(70,2,Temperature,2,1,GREEN,WHITE,16,0);
		}
		LCD_ShowChinese_16x16(110,2,"℃",BLACK,WHITE);
		LCD_DrawRectangle(2,0,126,20,BLUE);
		
		LCD_ShowChinese_16x16(5,40,"人",BLACK,WHITE);
		LCD_ShowChinese_16x16(20,40,"脸",BLACK,WHITE);
		LCD_ShowChinese_16x16(35,40,"识",BLACK,WHITE);
		LCD_ShowChinese_16x16(50,40,"别",BLACK,WHITE);
		LCD_DrawRectangle(2,35,126,60,BLUE);
		if(test==1)
		{		
			LCD_ShowString(100,22,"TEST",RED,WHITE,12,1);
			LCD_ShowString(0,22,"FORMAL",WHITE,WHITE,12,1);
			face_pass=0;
			mask_pass=0;
		}
		else
		{
			LCD_ShowString(100,22,"TEST",WHITE,WHITE,12,1);
			LCD_ShowString(0,22,"FORMAL",GREEN,WHITE,12,1);
		}
		if(face_pass==1)//
		{
			LCD_ShowChinese_16x16(80,40,"已",GREEN,WHITE);
			LCD_ShowChinese_16x16(95,40,"通",GREEN,WHITE);
			LCD_ShowChinese_16x16(110,40,"过",GREEN,WHITE);
			face_pass=1;
			USART_SendChar(USART1,'B');
		}
		else
		{
			LCD_ShowChinese_16x16(80,40,"未",RED,WHITE);
			LCD_ShowChinese_16x16(95,40,"通",RED,WHITE);
			LCD_ShowChinese_16x16(110,40,"过",RED,WHITE);			
		}
		
		LCD_ShowChinese_16x16(5,70,"口",BLACK,WHITE);
		LCD_ShowChinese_16x16(20,70,"罩",BLACK,WHITE);
		LCD_ShowChinese_16x16(35,70,"识",BLACK,WHITE);
		LCD_ShowChinese_16x16(50,70,"别",BLACK,WHITE);
		LCD_DrawRectangle(2,35,126,90,BLUE);	
		if(mask_pass==1)
		{
			LCD_ShowChinese_16x16(80,70,"已",GREEN,WHITE);
			LCD_ShowChinese_16x16(95,70,"通",GREEN,WHITE);
			LCD_ShowChinese_16x16(110,70,"过",GREEN,WHITE);
			mask_pass=1;
			USART_SendChar(USART1,'A');		
		}
		else
		{
			LCD_ShowChinese_16x16(80,70,"未",RED,WHITE);
			LCD_ShowChinese_16x16(95,70,"通",RED,WHITE);
			LCD_ShowChinese_16x16(110,70,"过",RED,WHITE);			
		}
		
		LCD_ShowChinese_16x16(5,100,"识",BLACK,WHITE);
		LCD_ShowChinese_16x16(20,100,"别",BLACK,WHITE);
		LCD_ShowChinese_16x16(35,100,"结",BLACK,WHITE);
		LCD_ShowChinese_16x16(50,100,"果",BLACK,WHITE);
		LCD_DrawRectangle(2,35,126,120,BLUE);	

		if(face_pass==1 && mask_pass==1 &&Temperature<up)
		{
			LCD_ShowChinese_16x16(80,100,"已",GREEN,WHITE);
			LCD_ShowChinese_16x16(95,100,"通",GREEN,WHITE);
			LCD_ShowChinese_16x16(110,100,"过",GREEN,WHITE);			
			USART_SendChar(USART1,'A');//口罩识别通过，切换回人脸识别，容易失效
			face_pass=0;
			mask_pass=0;
			delay_ms(200);
			delay_ms(2000);//用来观察效果
			USART_SendChar(USART1,'A');
		}
		else
		{
			LCD_ShowChinese_16x16(80,100,"未",RED,WHITE);
			LCD_ShowChinese_16x16(95,100,"通",RED,WHITE);
			LCD_ShowChinese_16x16(110,100,"过",RED,WHITE);			
		}			
	}
	return 0;
}



