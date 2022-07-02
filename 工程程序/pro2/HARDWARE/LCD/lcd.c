#include "lcd.h"
#include "lcd_init.h"
#include "lcdfont.h"
#include "delay.h"


/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{          
	u16 i,j; 
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);//设置显示范围
	for(i=ysta;i<yend;i++)
	{													   	 	
		for(j=xsta;j<xend;j++)
		{
			LCD_WR_DATA(color);
		}
	} 					  	    
}

/******************************************************************************
      函数说明：在指定位置画点
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_Address_Set(x,y,x,y);//设置光标位置 
	LCD_WR_DATA(color);
} 


/******************************************************************************
      函数说明：画线
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   线的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1;
	uRow=x1;//画线起点坐标
	uCol=y1;
	if(delta_x>0)incx=1; //设置单步方向 
	else if (delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//水平线 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_DrawPoint(uRow,uCol,color);//画点
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}


/******************************************************************************
      函数说明：画矩形
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   矩形的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}


/******************************************************************************
      函数说明：画圆
      入口数据：x0,y0   圆心坐标
                r       半径
                color   圆的颜色
      返回值：  无
******************************************************************************/
void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color)
{
	int a,b;
	a=0;b=r;	  
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,color);             //3           
		LCD_DrawPoint(x0+b,y0-a,color);             //0           
		LCD_DrawPoint(x0-a,y0+b,color);             //1                
		LCD_DrawPoint(x0-a,y0-b,color);             //2             
		LCD_DrawPoint(x0+b,y0+a,color);             //4               
		LCD_DrawPoint(x0+a,y0-b,color);             //5
		LCD_DrawPoint(x0+a,y0+b,color);             //6 
		LCD_DrawPoint(x0-b,y0+a,color);             //7
		a++;
		if((a*a+b*b)>(r*r))//判断要画的点是否过远
		{
			b--;
		}
	}
}


/******************************************************************************
      函数说明：显示单个字符
      入口数据：x,y显示坐标
                num 要显示的字符
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 temp,sizex,t,m=0;
	u16 i,TypefaceNum;//一个字符所占字节大小
	u16 x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //得到偏移后的值
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //设置光标位置 
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==12)temp=ascii_1206[num][i];		       //调用6x12字体
		else if(sizey==16)temp=ascii_1608[num][i];		 //调用8x16字体
		else if(sizey==24)temp=ascii_2412[num][i];		 //调用12x24字体
		else if(sizey==32)temp=ascii_3216[num][i];		 //调用16x32字体
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//叠加模式
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}


/******************************************************************************
      函数说明：显示字符串
      入口数据：x,y显示坐标
                *p 要显示的字符串
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowString(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 sizey,u8 mode)
{         
	while(*p!='\0')
	{       
		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=sizey/2;
		p++;
	}  
}


/******************************************************************************
      函数说明：显示数字
      入口数据：m底数，n指数
      返回值：  无
******************************************************************************/
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;
	return result;
}


/******************************************************************************
      函数说明：显示整数变量
      入口数据：x,y显示坐标
                num 要显示整数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowIntNum(u16 x,u16 y,u16 num,u8 len,u16 fc,u16 bc,u8 sizey)
{         	
	u8 t,temp;
	u8 enshow=0;
	u8 sizex=sizey/2;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+t*sizex,y,' ',fc,bc,sizey,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
} 


/******************************************************************************
      函数说明：显示两位小数变量
      入口数据：x,y显示坐标
                num 要显示小数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u16 fc,u16 bc,u8 sizey)
{         	
	u8 t,temp,sizex;
	u16 num1;
	sizex=sizey/2;
	num1=num*100;
	for(t=0;t<len;t++)
	{
		temp=(num1/mypow(10,len-t-1))%10;
		if(t==(len-2))
		{
			LCD_ShowChar(x+(len-2)*sizex,y,'.',fc,bc,sizey,0);
			t++;
			len+=1;
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
}

//==================================================================================================
//  实现功能: TFT 指定位置 显示浮点数字 16x16像素
//  函数标记: 外设驱动函数
//  函数说明: 1. 位置选取不恰当 可能造成数字显示不完全
//            2. float型数据的有效数字7位 double型数据的有效数字16位
//--------------------------------------------------------------------------------------------------
//  输入参量: X - X方向坐标  取值范围 - 详情见宏定义TFT_SHOWSIZE_X
//            Y - Y方向坐标  取值范围 - 详情见宏定义TFT_SHOWSIZE_Y
//            FloatNumber - 待显示浮点型数字  取值范围 - -99999.99999~99999.99999
//            Count1 - 整数显示位数  取值范围 - 0~5
//            Count2 - 小数显示位数  取值范围 - 0~5
//            FontColor - 字体颜色  取值范围 - 详情见枚举类型enumTFT_Color
//            BackgroundColor - 背景颜色 取值范围 - 详情见枚举类型enumTFT_Color
//  输出参量: 无
//--------------------------------------------------------------------------------------------------
//  |   -   |   -   |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   
//==================================================================================================
void TFT_ShowNumber_Float_16x16(unsigned short X, unsigned short Y, float FloatNumber, unsigned char Count1, unsigned char Count2,u16 fc,u16 bc,u8 sizey,u8 mode)
{
    unsigned char Number_Integer_Array[6];          // 定义局部数组 用于存储整数位各位数据
    unsigned char Number_Decimal_Array[6];          // 定义局部数组 用于存储小数位各位数据

    unsigned long int Number_Integer = 0;           // 定义局部变量 表示浮点数的 整数部分
    unsigned long int Number_Decimal = 0;           // 定义局部变量 表示浮点数的 小数部分
    

    // 限制 显示字符数
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

    // 计算浮点数字的整数部分与小数部分
    Number_Integer = (unsigned long int)(FloatNumber);                                          // 取整数部分
    Number_Decimal = (unsigned long int)((FloatNumber - Number_Integer + 0.000005) * 1e5);      // 取小数部分，1e5科学计数法

    // 装载显示字符
    Number_Integer_Array[0] = Number_Integer/10000 % 10 + 0x30;     // 计算整数部分
    Number_Integer_Array[1] = Number_Integer/ 1000 % 10 + 0x30;
    Number_Integer_Array[2] = Number_Integer/  100 % 10 + 0x30;
    Number_Integer_Array[3] = Number_Integer/   10 % 10 + 0x30;
    Number_Integer_Array[4] = Number_Integer/    1 % 10 + 0x30;
    Number_Integer_Array[5] = '\0';

    Number_Decimal_Array[0] = Number_Decimal/10000 % 10 + 0x30;     // 计算小数部分
    Number_Decimal_Array[1] = Number_Decimal/ 1000 % 10 + 0x30;
    Number_Decimal_Array[2] = Number_Decimal/  100 % 10 + 0x30;
    Number_Decimal_Array[3] = Number_Decimal/   10 % 10 + 0x30;
    Number_Decimal_Array[4] = Number_Decimal/    1 % 10 + 0x30;
    Number_Decimal_Array[Count2] = '\0';
    
    // 执行显示操作
	LCD_ShowString(X+8,Y,&Number_Integer_Array[5-Count1],fc,bc,sizey,mode);
	
    LCD_ShowChar( X+(1+Count1)*8, Y, '.',fc,bc,sizey,mode);                                           
    LCD_ShowString(X+(2+Count1)*8, Y,&Number_Decimal_Array[0],fc,bc,sizey,mode);
}


//==================================================================================================
//  实现功能: TFT 指定位置 显示中文 16x16像素
//  函数标记: 外设驱动函数
//  函数说明: 1. 位置选取不恰当 可能造成字符串显示不完全
//            2. 如果字库不存在该文字则不显示
//            3. 1个汉字等同2个字节组成
//--------------------------------------------------------------------------------------------------
//  输入参量: X - X方向坐标  取值范围 - 详情见宏定义TFT_SHOWSIZE_X
//            Y - Y方向坐标  取值范围 - 详情见宏定义TFT_SHOWSIZE_Y
//            Chinese - 待显示汉字  取值范围 - 单个汉字组成的字符串
//            FontColor - 字体颜色  取值范围 - 详情见枚举类型enumTFT_Color
//            BackgroundColor - 背景颜色 取值范围 - 详情见枚举类型enumTFT_Color
//  输出参量: 无
//--------------------------------------------------------------------------------------------------
//  |   -   |   -   |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   
//==================================================================================================
void LCD_ShowChinese_16x16(uint16_t X, uint16_t Y, uint8_t *Chinese, u16 FontColor, u16 BackgroundColor)
{
    uint8_t i,j,k;                            // 定义局部变量 用于循环计数
    uint8_t Data;                             // 定义局部变量 用于临时存储

    uint8_t FontCount;                        // 定义局部变量 用于记录字符字库个数    
    
	FontCount = sizeof(TFT_CHINESE_16x16_FontCode)/sizeof(TFT_CHINESE_16x16_FontCode[0]);

    for(i=0; i<FontCount; i++)                      // 开始字库内容匹配                             
    {
        if((*Chinese     == TFT_CHINESE_16x16_FontCode[i].Char[0])
        && (*(Chinese+1) == TFT_CHINESE_16x16_FontCode[i].Char[1]))
        {
            LCD_Address_Set(X, Y, X+16-1, Y+16-1);      // 设置显示位置
            for(j=0; j<32; j++)                     // 循环写入32字节 一个汉字信息包含32字节
            {
                Data = TFT_CHINESE_16x16_FontCode[i].Code[j];
                for(k=0; k<8; k++)
                {
                    if((Data&0x80) == 0x80)         // 判断最高位是否为1
                    {
						//LCD_WR_DATA(Data);
                        LCD_WR_DATA(FontColor);             // 最高位为1 写入字符颜色
                    }
                    else
                    {
                        LCD_WR_DATA(BackgroundColor);       // 最高位为0 写入背景颜色
                    }
                    Data <<= 1;
                }
            }
            break;                                  // 结束字库内容匹配                              
        }
    }
}


