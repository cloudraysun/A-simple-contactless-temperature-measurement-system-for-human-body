#ifndef __LED_H
#define __LED_H	 
#include "main.h"
#include "delay.h"


#define red		PBout(5)// PB5
#define green 	PBout(3)// PB3
#define blue 	PBout(4)// PB4

void LED_Init(void);//≥ı ºªØ
void led_test(void);//LED≤‚ ‘
void led_shing(void);	
void LED_RGB_Init(void);//LED_RGB
void LED_RGB(unsigned int led,unsigned int mode);
void LED_shine(unsigned int color);
#endif
