//==================================================================================================
//  文件名称: MLX90614.H
//  文件功能: 红外测温模块
//  文件说明: 无
//  文件类型: Board-板载设备驱动代码
//==================================================================================================
#ifndef __MLX90614__H__
#define __MLX90614__H__


#define ACK           0
#define NACK          1
#define SA            0x00 //Slave address 单个MLX90614时地址为0x00,多个时地址默认为0x5a
#define RAM_ACCESS    0x00 //RAM access command
#define EEPROM_ACCESS 0x20 //EEPROM access command
#define RAM_TOBJ1     0x07 //To1 address in the eeprom

//SDA：上拉开漏输出
//SCL：上拉开漏输出

#define SDA_L     GPIO_ResetBit(GPIOA, GPIO_Pin_4)
#define SDA_H     GPIO_SetBit(GPIOA, GPIO_Pin_4)  
#define SCL_H     GPIO_SetBit(GPIOA, GPIO_Pin_5)
#define SCL_L     GPIO_ResetBit(GPIOA, GPIO_Pin_5)
            

#define SDA_PIN   GPIO_GetBit(GPIOA, GPIO_Pin_4) //读取引脚电平

//GPIO_PuPd_NOPULL
void Start_Bit(void);
void Stop_Bit(void);
unsigned char Send_Byte(unsigned char Tx_buffer);
void Send_Bit(unsigned char bit_out);
unsigned char Receive_Bit(void);
unsigned char Receive_Byte(unsigned char ack_nack);
unsigned short Read_Memory(unsigned char slaveAddress, unsigned char command);
unsigned char PEC_Calculation(unsigned char pec[]);
float Read_Temp(void);
#endif  
