#ifndef __I2C_EEPROM_DRV_H
#define __I2C_EEPROM_DRV_H

#include "stdint.h"
#include "fsl_lpi2c.h"


/* lpi2c������ */
#define EEPROM_I2C_MASTER_BASE   (LPI2C1_BASE)
#define EEPROM_I2C_MASTER        ((LPI2C_Type *)EEPROM_I2C_MASTER_BASE)
/* lpi2c������ */
#define EEPROM_I2C_BAUDRATE      400000U

/*! @brief I2C���Ŷ��� */
#define EEPROM_SCL_IOMUXC        IOMUXC_GPIO_AD_B1_00_LPI2C1_SCL
#define EEPROM_SDA_IOMUXC        IOMUXC_GPIO_AD_B1_01_LPI2C1_SDA

/* �ȴ���ʱʱ�� */
#define I2CT_FLAG_TIMEOUT        ((uint32_t)0x100)
#define I2CT_LONG_TIMEOUT        ((uint32_t)(10 * I2CT_FLAG_TIMEOUT))

/* EEPROM ���ܿռ��С*/
#define EEPROM_SIZE                    256
/* AT24C01/02ÿҳ��8���ֽ� */
#define EEPROM_PAGE_SIZE               8
/* EEPROM�豸��ַ */
#define EEPROM_ADDRESS_7_BIT           (0xA0>>1)   
#define EEPROM_WRITE_ADDRESS_8_BIT     (0xA0)   
#define EEPROM_READ_ADDRESS_8_BIT      (0xA1)   

/* EEPROM�ڲ��洢��Ԫ��ַ�Ĵ�С����λ���ֽ�*/
#define EEPROM_INER_ADDRESS_SIZE            0x01   


//�����ӿ�
uint8_t EEPROM_Test(void);

uint32_t I2C_EEPROM_BufferRead( uint8_t ClientAddr, uint8_t ReadAddr, uint8_t* pBuffer, uint16_t NumByteToRead);                                       

uint32_t I2C_EEPROM_Page_Write( uint8_t ClientAddr, uint8_t WriteAddr, uint8_t* pBuffer, uint8_t NumByteToWrite);

void I2C_EEPROM_Buffer_Write( uint8_t ClientAddr, uint8_t WriteAddr, uint8_t* pBuffer, uint16_t NumByteToWrite);

uint8_t I2C_EEPROM_WaitStandbyState(uint8_t ClientAddr);


#endif
