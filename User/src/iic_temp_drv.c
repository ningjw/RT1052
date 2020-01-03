#include "main.h"

#define MLX_I2C_MASTER_BASE         LPI2C3_BASE
#define MLX_I2C_MASTER              ((LPI2C_Type *)MLX_I2C_MASTER_BASE)

#define MLX_ADDR     0x5A

#define RAM_ACCESS    0x00   //RAM access command
#define EEPROM_ACCESS 0x20   //EEPROM access command

#define EEPROM_SET_TEMP_HIGH  0x00 //100* (Tmax + 273.15)
#define EEPROM_SET_TEMP_LOW   0x01 //100* (Tmin + 273.15)
#define EEPROM_SET_TEMP_ENV   0x03
#define EEPROM_SET_SLAVE_ADDR 0x0E

#define RAM_TEMP_ENV     0x06 
#define RAM_TEMP_OBJ     0x07 

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void MLX_WriteReg(uint8_t reg, uint8_t value) {
    lpi2c_master_transfer_t masterXfer = {0};
    uint8_t data = value;
    masterXfer.slaveAddress = MLX_ADDR;
    masterXfer.direction = kLPI2C_Write;
    masterXfer.subaddress = reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data  = &data;;
    masterXfer.dataSize = 1;
    masterXfer.flags = kLPI2C_TransferDefaultFlag;
    
	LPI2C_MasterTransferBlocking(MLX_I2C_MASTER, &masterXfer);
}


/***************************************************************************************
  * @brief   Read MLX register
  * @input   reg - register number
  * @return  register value
***************************************************************************************/
uint16_t MLX_ReadReg(uint8_t reg) 
{
	lpi2c_master_transfer_t masterXfer = {0};
    uint8_t value[3];
    uint16_t ret = 0;
    masterXfer.slaveAddress = MLX_ADDR;
    masterXfer.direction = kLPI2C_Read;
    masterXfer.subaddress = (uint32_t)reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data = value;
    masterXfer.dataSize = 3;
    masterXfer.flags = kLPI2C_TransferRepeatedStartFlag;
    
    LPI2C_MasterTransferBlocking(MLX_I2C_MASTER, &masterXfer);
    ret = (value[1]<<8) | value[0];
	return ret;
}

/*******************************************************************************
* Function Name  : SMBus_ReadTemp
* Description    : Calculate and return the temperature
* Input          : None
* Output         : None
* Return         : SMBus_ReadMemory(0x00, 0x07)*0.02-273.15
*******************************************************************************/
float MXL_ReadEnvTemp(void)
{
    float    temp = 0;
    uint16_t regValue = 0;
    regValue = MLX_ReadReg(RAM_ACCESS | RAM_TEMP_ENV);
    temp = regValue * 0.02f - 273.15f;
    return  temp;
}

float MXL_ReadObjTemp(void)
{
    float    temp = 0;
    uint16_t regValue = 0;
    regValue = MLX_ReadReg(RAM_ACCESS | RAM_TEMP_OBJ);
    temp = regValue * 0.02f - 273.15f;
    return  temp;
}



/*******************************************************************************
* Function Name  : PEC_calculation
* Description    : Calculates the PEC of received bytes
* Input          : pec[]
* Output         : None
* Return         : pec[0]-this byte contains calculated crc value
*******************************************************************************/
uint8_t PEC_Calculation(uint8_t pec[])
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




