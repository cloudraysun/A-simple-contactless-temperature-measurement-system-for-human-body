/**************************************************
来源：https://blog.csdn.net/baoke485800/article/details/51320740
***************************************************/

#include"MAIN.H"                // 包含全局头文件
#include"MLX90614.H"    // 包含外设头文件 MLX0614-红外测温模块


//开始
void Start_Bit(void)
{
    SDA_H;               // Set SDA line 
    DELAY_nUS(5);      // Wait a few microseconds 
    SCL_H;               // Set SCK line  
    DELAY_nUS(5);      // Generate bus free time between Stop
    SDA_L;               // Clear SDA line
    DELAY_nUS(10);     // Hold time after (Repeated) Start
                         // Condition. After this period, the first clock is generated.
                         //(Thd:sta=4.0us min)
    SCL_L;               // Clear SCK line
    DELAY_nUS(5);      // Wait a few microseconds
}

void Stop_Bit(void)
{
    SCL_L;                // Clear SCK line
    DELAY_nUS(5);       // Wait a few microseconds
    SDA_L;                // Clear SDA line
    DELAY_nUS(5);       // Wait a few microseconds
    SCL_H;                // Set SCK line
    DELAY_nUS(10);      // Stop condition setup time(Tsu:sto=4.0us min)
    SDA_H;                // Set SDA line
}
void PwmToSMBus(void)
{
    SCL_L;
    DELAY_nUS(1500); // greater than 1.44ms
    SCL_H;
}

unsigned char Send_Byte(unsigned char Tx_buffer)
{
    unsigned char Bit_counter;
    unsigned char Ack_bit;
    unsigned char bit_out;

//    SCL_L;
//    SDA_L;
//    DELAY_nUS(2);
    for(Bit_counter=8; Bit_counter; Bit_counter--)
    {
        if (Tx_buffer&0x80)
        {
            bit_out=1;       // If the current bit of Tx_buffer is 1 set bit_out
        }
        else
        {
            bit_out=0;      // else clear bit_out
        }
        Send_Bit(bit_out);      // Send the current bit on SDA
        Tx_buffer<<=1;          // Get next bit for checking
    }
    Ack_bit=Receive_Bit();           // Get acknowledgment bit
    return Ack_bit;
}

void Send_Bit(unsigned char bit_out)
{
    if(bit_out==0)
    {
        SDA_L;   
    }
    else
    {
        SDA_H;
    }
    DELAY_nUS(5);                            // Tsu:dat = 250ns minimum
    SCL_H;                                     // Set SCK line
    DELAY_nUS(10);                           // High Level of Clock Pulse
    SCL_L;                                     // Clear SCK line
    DELAY_nUS(10);                           // Low Level of Clock Pulse
//        SMBUS_SDA_H();                       // Master release SDA line ,
    return;
}
/*******************************************************************************
* Function Name  : SMBus_ReceiveBit
* Description    : Receive a bit on SMBus
* Input          : None
* Output         : None
* Return         : Ack_bit
*******************************************************************************/
unsigned char Receive_Bit(void)
{
    unsigned char Ack_bit;

    SDA_H;             //引脚靠外部电阻上拉，当作输入
    DELAY_nUS(2);
    SCL_H;             // Set SCL line
    DELAY_nUS(5);    // High Level of Clock Pulse
    if(SDA_PIN)
    {
        Ack_bit=1;
    }
    else
    {
        Ack_bit=0;
    }
    SCL_L;                    // Clear SCL line   
    DELAY_nUS(5);           // Low Level of Clock Pulse
    return Ack_bit;
}

/*******************************************************************************
* Function Name  : Receive_Byte
* Description    : Receive a byte on SMBus
* Input          : ack_nack
* Output         : None
* Return         : RX_buffer
*******************************************************************************/
unsigned char Receive_Byte(unsigned char ack_nack)
{
    unsigned char RX_buffer;
    unsigned char Bit_Counter;
    for(Bit_Counter=8; Bit_Counter; Bit_Counter--)
    {
        if(Receive_Bit())         // Get a bit from the SDA line
        {
            RX_buffer <<= 1;           // If the bit is HIGH save 1  in RX_buffer
            RX_buffer |=0x01;
        }
        else
        {
            RX_buffer <<= 1;           // If the bit is LOW save 0 in RX_buffer
            RX_buffer &=0xfe;
        }
    }
    Send_Bit(ack_nack);           // Sends acknowledgment bit
    return RX_buffer;
}

/*******************************************************************************
 * Function Name  : SMBus_ReadMemory
 * Description    : READ DATA FROM RAM/EEPROM
 * Input          : slaveAddress, command
 * Output         : None
 * Return         : Data
*******************************************************************************/
unsigned short Read_Memory(unsigned char slaveAddress, unsigned char command)
{
    unsigned short data=0;               // Data storage (DataH:DataL)
    unsigned char Pec;                 // PEC byte storage
    unsigned char DataL=0;             // Low data byte storage
    unsigned char DataH=0;             // High data byte storage
    unsigned char arr[6];              // Buffer for the sent bytes
    unsigned char PecReg;              // Calculated PEC byte storage
    unsigned char ErrorCounter;        // Defines the number of the attempts for communication with MLX90614


    ErrorCounter=0x00;                  // Initialising of ErrorCounter
    slaveAddress <<= 1;                //2-7位表示从机地址
    do
    {
        repeat:
            Stop_Bit();                //If slave send NACK stop comunication
            --ErrorCounter;                 //Pre-decrement ErrorCounter
            if(!ErrorCounter)               //ErrorCounter=0?
            {
                break;                      //Yes,go out from do-while{}
            }

            Start_Bit();               //Start condition
            if(Send_Byte(slaveAddress))//Send SlaveAddress 最低位Wr=0表示接下来写命令1011 0100
            {
                goto repeat;               //Repeat comunication again
            }
            if(Send_Byte(command))     //Send command
            {
                goto repeat;             //Repeat comunication again
            }

        Start_Bit();                //Repeated Start condition
        if(Send_Byte(0x01))  //Send SlaveAddress+1 最低位Rd=1表示接下来读数据
        {
            goto repeat;           //Repeat comunication again
        }

        DataL = Receive_Byte(ACK);   //Read low data,master must send ACK
        DataH = Receive_Byte(ACK);   //Read high data,master must send ACK
        Pec = Receive_Byte(NACK);    //Read PEC byte, master must send NACK
        Stop_Bit();                  //Stop condition

        arr[5] = slaveAddress;        
        arr[4] = command;
        arr[3] = slaveAddress+1;         //Load array arr
        arr[2] = DataL;                 
        arr[1] = DataH;                
        arr[0] = 0;                   
        PecReg=PEC_Calculation(arr);     //Calculate CRC
    }
    while(PecReg != Pec);                //If received and calculated CRC are equal go out from do-while{}
        data = (DataH<<8) | DataL;       //data=DataH:DataL
    return data;
}

/*******************************************************************************
* Function Name  : PEC_calculation
* Description    : Calculates the PEC of received bytes
* Input          : pec[]
* Output         : None
* Return         : pec[0]-this byte contains calculated crc value
*******************************************************************************/
unsigned char PEC_Calculation(unsigned char pec[])
{
    unsigned char crc[6];
    unsigned char BitPosition=47;
    unsigned char shift;
    unsigned char i;
    unsigned char j;
    unsigned char temp;


    do
    {
        /*Load pattern value 0x000000000107*/
        crc[5]=0;
        crc[4]=0;
        crc[3]=0;
        crc[2]=0;
        crc[1]=0x01;
        crc[0]=0x07;
        /*Set maximum bit position at 47 ( six bytes byte5...byte0,MSbit=47)*/
        BitPosition=47;
        /*Set shift position at 0*/
        shift=0;
        /*Find first "1" in the transmited message beginning from the MSByte byte5*/
        i=5;
        j=0;
        while((pec[i]&(0x80>>j))==0 && i>0)
        {
            BitPosition--;
            if(j<7)
            {
                j++;
            }
            else
            {
                j=0x00;
                i--;
            }
        }/*End of while */


        /*Get shift value for pattern value*/
        shift=BitPosition-8;
        /*Shift pattern value */
        while(shift)
        {
            for(i=5; i<0xFF; i--)
            {
                if((crc[i-1]&0x80) && (i>0))
                {
                    temp=1;
                }
                else
                {
                    temp=0;
                }
                crc[i]<<=1;
                crc[i]+=temp;
            }/*End of for*/
            shift--;
        }/*End of while*/
        /*Exclusive OR between pec and crc*/
        for(i=0; i<=5; i++)
        {
            pec[i] ^=crc[i];
        }/*End of for*/
    }
    while(BitPosition>8); /*End of do-while*/

    return pec[0];
}

 /*******************************************************************************
 * Function Name  : SMBus_ReadTemp
 * Description    : Calculate and return the temperature
 * Input          : None
 * Output         : None
 * Return         : SMBus_ReadMemory(0x00, 0x07)*0.02-273.15
*******************************************************************************/
float Read_Temp(void)
{   
    return Read_Memory(SA, RAM_ACCESS|RAM_TOBJ1)*0.02-273.15;
}
/*********************************END OF FILE*********************************/




