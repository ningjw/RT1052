#include "main.h"

#define MLX_ADDR 0x5A

#define ACK           0
#define NACK          1

#define RAM_ACCESS    0x00   //RAM access command
#define EEPROM_ACCESS 0x20   //EEPROM access command

#define EEPROM_SET_TEMP_HIGH  0x00 //100* (Tmax + 273.15)
#define EEPROM_SET_TEMP_LOW   0x01 //100* (Tmin + 273.15)
#define EEPROM_SET_TEMP_ENV   0x03
#define EEPROM_SET_SLAVE_ADDR 0x0E

#define RAM_TEMP_ENV     0x06 
#define RAM_TEMP_OBJ     0x07 

#define SDA_H GPIO_PinWrite( BOARD_SDA_TEMP_GPIO, BOARD_SDA_TEMP_PIN, 1U)
#define SDA_L GPIO_PinWrite( BOARD_SDA_TEMP_GPIO, BOARD_SDA_TEMP_PIN, 0U)
#define SCL_H GPIO_PinWrite( BOARD_SCL_TEMP_GPIO, BOARD_SCL_TEMP_PIN, 1U)
#define SCL_L GPIO_PinWrite( BOARD_SCL_TEMP_GPIO, BOARD_SCL_TEMP_PIN, 0U)
#define SMBUS_SDA_PIN  GPIO_PinRead(BOARD_SDA_TEMP_GPIO, BOARD_SDA_TEMP_PIN)
/*******************************************************************************
* Function Name  : SMBus_Delay
* Description    : 延时  一次循环约1us
*******************************************************************************/
static void SMBus_Delay(uint16_t time)
{
    CPU_TS_Tmr_Delay_US(time);
}


/*******************************************************************************
* Function Name  : PEC_calculation
* Description    : Calculates the PEC of received bytes
* Input          : pec[]
* Output         : None
* Return         : pec[0]-this byte contains calculated crc value
*******************************************************************************/
static uint8_t PEC_Calculation(uint8_t pec[])
{
    uint8_t         crc[6];
    uint8_t        BitPosition = 47;
    uint8_t        shift;
    uint8_t        i;
    uint8_t        j;
    uint8_t        temp;

    do
    {
        /*Load pattern value 0x000000000107*/
        crc[5] = 0;
        crc[4] = 0;
        crc[3] = 0;
        crc[2] = 0;
        crc[1] = 0x01;
        crc[0] = 0x07;
        /*Set maximum bit position at 47 ( six bytes byte5...byte0,MSbit=47)*/
        BitPosition = 47;
        /*Set shift position at 0*/
        shift = 0;
        /*Find first "1" in the transmited message beginning from the MSByte byte5*/
        i = 5;
        j = 0;
        while((pec[i] & (0x80 >> j)) == 0 && i > 0)
        {
            BitPosition--;
            if(j < 7)
            {
                j++;
            }
            else
            {
                j = 0x00;
                i--;
            }
        }/*End of while */


        /*Get shift value for pattern value*/
        shift = BitPosition - 8;
        /*Shift pattern value */
        while(shift)
        {
            for(i = 5; i < 0xFF; i--)
            {
                if((crc[i - 1] & 0x80) && (i > 0))
                {
                    temp = 1;
                }
                else
                {
                    temp = 0;
                }
                crc[i] <<= 1;
                crc[i] += temp;
            }/*End of for*/
            shift--;
        }/*End of while*/
        /*Exclusive OR between pec and crc*/
        for(i = 0; i <= 5; i++)
        {
            pec[i] ^= crc[i];
        }/*End of for*/
    }
    while(BitPosition > 8); /*End of do-while*/

    return pec[0];
}




/*******************************************************************************
* Function Name  : SMBus_SendBit
* Description    : Send a bit on SMBus
* Input          : bit_out
*******************************************************************************/
static void SMBus_SendBit(uint8_t bit_out)
{
    if(bit_out == 0)
    {
        SDA_L;
    }
    else
    {
        SDA_H;
    }
    SMBus_Delay(2);                            // Tsu:dat = 250ns minimum
    SCL_H;                                     // Set SCK line
    SMBus_Delay(10);                           // High Level of Clock Pulse
    SCL_L;                                     // Clear SCK line
    SMBus_Delay(10);                           // Low Level of Clock Pulse
//        SMBUS_SDA_H();                       // Master release SDA line ,
    return;
}
/*******************************************************************************
* Function Name  : SMBus_ReceiveBit
* Description    : Receive a bit on SMBus
* Return         : Ack_bit
*******************************************************************************/
static uint8_t SMBus_ReceiveBit(void)
{
    uint8_t Ack_bit;


    SDA_H;             //引脚靠外部电阻上拉，当作输入
    SCL_H;             // Set SCL line
    SMBus_Delay(2);    // High Level of Clock Pulse
    if (SMBUS_SDA_PIN )
    {
        Ack_bit = 1;
    }
    else
    {
        Ack_bit = 0;
    }
    SCL_L;                    // Clear SCL line
    SMBus_Delay(4);           // Low Level of Clock Pulse
    return   Ack_bit;
}


/*******************************************************************************
* Function Name  : SMBus_StartBit
* Description    : Generate START condition on SMBus
*******************************************************************************/
static void SMBus_StartBit(void)
{
    SDA_H;               // Set SDA line
    SMBus_Delay(1);      // Wait a few microseconds
    SCL_H;               // Set SCK line
    SMBus_Delay(5);      // Generate bus free time between Stop
    SDA_L;               // Clear SDA line
    SMBus_Delay(10);     // Hold time after (Repeated) Start
    // Condition. After this period, the first clock is generated.
    //(Thd:sta=4.0us min)
    SCL_L;               // Clear SCK line
    SMBus_Delay(2);      // Wait a few microseconds
}

/*******************************************************************************
* Function Name  : SMBus_StopBit
* Description    : Generate STOP condition on SMBus
*******************************************************************************/
static void SMBus_StopBit(void)
{
    SCL_L;                // Clear SCK line
    SMBus_Delay(5);       // Wait a few microseconds
    SDA_L;                // Clear SDA line
    SMBus_Delay(5);       // Wait a few microseconds
    SCL_H;                // Set SCK line
    SMBus_Delay(10);      // Stop condition setup time(Tsu:sto=4.0us min)
    SDA_H;                // Set SDA line
}

/*******************************************************************************
* Function Name  : SMBus_SendByte
* Description    : Send a byte on SMBus
* Input          : Tx_buffer
*******************************************************************************/
uint8_t SMBus_SendByte(uint8_t Tx_buffer)
{
    uint8_t        Bit_counter;
    uint8_t        Ack_bit;
    uint8_t        bit_out;


    for(Bit_counter = 8; Bit_counter; Bit_counter--)
    {
        if (Tx_buffer & 0x80)
        {
            bit_out = 1;     // If the current bit of Tx_buffer is 1 set bit_out
        }
        else
        {
            bit_out = 0;    // else clear bit_out
        }
        SMBus_SendBit(bit_out);           // Send the current bit on SDA
        Tx_buffer <<= 1;                  // Get next bit for checking
    }
    Ack_bit = SMBus_ReceiveBit();         // Get acknowledgment bit
    return        Ack_bit;
}


/*******************************************************************************
* Function Name  : SMBus_ReceiveByte
* Description    : Receive a byte on SMBus
* Input          : ack_nack
* Output         : None
* Return         : RX_buffer
*******************************************************************************/
uint8_t SMBus_ReceiveByte(uint8_t ack_nack)
{
    uint8_t        RX_buffer;
    uint8_t        Bit_Counter;
    for(Bit_Counter = 8; Bit_Counter; Bit_Counter--)
    {
        if(SMBus_ReceiveBit())         // Get a bit from the SDA line
        {
            RX_buffer <<= 1;           // If the bit is HIGH save 1  in RX_buffer
            RX_buffer |= 0x01;
        }
        else
        {
            RX_buffer <<= 1;           // If the bit is LOW save 0 in RX_buffer
            RX_buffer &= 0xfe;
        }
    }
    SMBus_SendBit(ack_nack);           // Sends acknowledgment bit
    return RX_buffer;
}


/*******************************************************************************
 * Function Name  : SMBus_ReadMemory
 * Description    : READ DATA FROM RAM/EEPROM
 * Input          : slaveAddress, command
 * Output         : None
 * Return         : Data
*******************************************************************************/
uint16_t SMBus_ReadMemory(uint8_t slaveAddress, uint8_t command)
{
    uint16_t data;               // Data storage (DataH:DataL)
    uint8_t Pec;                 // PEC byte storage
    uint8_t DataL = 0;           // Low data byte storage
    uint8_t DataH = 0;           // High data byte storage
    uint8_t arr[6];              // Buffer for the sent bytes
    uint8_t PecReg;              // Calculated PEC byte storage
    uint8_t ErrorCounter;        // Defines the number of the attempts for communication with MLX90614


    ErrorCounter = 0x00;                              // Initialising of ErrorCounter
    slaveAddress <<= 1;        //2-7位表示从机地址

    do
    {
repeat:
        SMBus_StopBit();                //If slave send NACK stop comunication
        --ErrorCounter;                 //Pre-decrement ErrorCounter
        if(!ErrorCounter)               //ErrorCounter=0?
        {
            break;                      //Yes,go out from do-while{}
        }

        SMBus_StartBit();               //Start condition
        if(SMBus_SendByte(slaveAddress))//Send SlaveAddress 最低位Wr=0表示接下来写命令
        {
            goto  repeat;               //Repeat comunication again
        }
        if(SMBus_SendByte(command))     //Send command
        {
            goto    repeat;             //Repeat comunication again
        }

        SMBus_StartBit();                //Repeated Start condition
        if(SMBus_SendByte(slaveAddress + 1)) //Send SlaveAddress 最低位Rd=1表示接下来读数据
        {
            goto        repeat;           //Repeat comunication again
        }

        DataL = SMBus_ReceiveByte(ACK);   //Read low data,master must send ACK
        DataH = SMBus_ReceiveByte(ACK);   //Read high data,master must send ACK
        Pec = SMBus_ReceiveByte(NACK);    //Read PEC byte, master must send NACK
        SMBus_StopBit();                  //Stop condition

        arr[5] = slaveAddress;
        arr[4] = command;
        arr[3] = slaveAddress + 1;       //Load array arr
        arr[2] = DataL;
        arr[1] = DataH;
        arr[0] = 0;
        PecReg = PEC_Calculation(arr);   //Calculate CRC
    }
    while(PecReg != Pec);                //If received and calculated CRC are equal go out from do-while{}
    data = (DataH << 8) | DataL;     //data=DataH:DataL
    return data;
}





