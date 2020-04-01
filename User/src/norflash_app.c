/**
  ******************************************************************
  * @file    norflash_app.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  */
#include "main.h"


#define EXAMPLE_SECTOR      4096      /* 要进行读写测试的扇区号 */
#define EXAMPLE_SIZE        128  /* 读写测试的数据量，单位为字节*/

/* 读写测试使用的缓冲区 */
static uint8_t s_nor_program_buffer[EXAMPLE_SIZE];
static uint8_t s_nor_read_buffer[EXAMPLE_SIZE];

extern status_t FlexSPI_NorFlash_Enable_QE(FLEXSPI_Type *base);
extern uint8_t  FlexSPI_FlashUUID_Get_ISSI(uint8_t *buf);

/***************************************************************************************
  * @brief   将接受到的固件数据写入Nor Flash
  * @input   buff:需要写入的数据首地址
             len:需要写入的数据长度
  * @return  
***************************************************************************************/
int NorFlash_WriteApp(uint8_t* buff, uint8_t len)
{
    /* 写入数据到Nor Flash */
    FlexSPI_NorFlash_Buffer_Program(FLEXSPI, g_sys_para.firmNextAddr, buff, FIRM_ONE_PACKE_LEN);
    
    return 0;
}

/***************************************************************************************
  * @brief   保存固件升级参数,在接受到完整的固件包后,调用该函数
  * @input   
  * @return  
***************************************************************************************/
void NorFlash_SaveFirmPara(void)
{
    FlexSPI_NorFlash_Erase_Sector(FLEXSPI, FIRM_INFO_ADDR);
    FlexSPI_NorFlash_Buffer_Program(FLEXSPI, FIRM_INFO_ADDR, &g_sys_para.firmUpdate, 16);
}


/**
* @brief  使用AHB命令的方式进行读写测试
* @param  无
* @retval 测试结果，正常为0
*/
int NorFlash_ChkSelf(void)
{
    uint32_t i = 0;
    status_t status;
    uint32_t JedecDeviceID = 0;

    /***************************读ID测试****************************/
    /* 获取JedecDevice ID. */
    FlexSPI_NorFlash_Get_JedecDevice_ID(FLEXSPI, &JedecDeviceID);
    
    if(JedecDeviceID != FLASH_WINBOND_JEDECDEVICE_ID && 
			 JedecDeviceID != FLASH_ISSI_JEDECDEVICE_ID)
    {
      PRINTF("FLASH检测错误，读取到的JedecDeviceID值为: 0x%x\r\n", JedecDeviceID);
      return -1;
    }
    
    /***************************一次写入一个扇区数据测试****************************/
    for (i = 0; i < EXAMPLE_SIZE; i++){
        s_nor_program_buffer[i] = (uint8_t)i;
    }
    
    /* 擦除指定扇区 */
    status = FlexSPI_NorFlash_Erase_Sector(FLEXSPI, EXAMPLE_SECTOR * SECTOR_SIZE);
    if (status != kStatus_Success)
    {
        PRINTF("擦除flash扇区失败 !\r\n");
        return -1;
    }

    /* 写入一个扇区的数据 */
    status = FlexSPI_NorFlash_Buffer_Program(FLEXSPI,
                                             EXAMPLE_SECTOR * SECTOR_SIZE,
                                             (void *)s_nor_program_buffer,
                                             EXAMPLE_SIZE);
    if (status != kStatus_Success)
    {
        PRINTF("写入失败 !\r\n");
        return -1;
    }
    
    /* 使用软件复位来重置 AHB 缓冲区. */
    FLEXSPI_SoftwareReset(FLEXSPI);

    /* 读取数据 */
    memcpy(s_nor_read_buffer, 
           NORFLASH_AHB_POINTER(EXAMPLE_SECTOR * SECTOR_SIZE),
           EXAMPLE_SIZE);
    
    /* 把读出的数据与写入的比较 */
    if (memcmp(s_nor_program_buffer, s_nor_read_buffer,EXAMPLE_SIZE))
    {
        PRINTF("写入数据，读出数据不正确 !\r\n ");
        return -1;
    } else {
        PRINTF("大量数据写入和读取测试成功. \r\n");
    }

    return 0;  
}

/****************************END OF FILE**********************/
