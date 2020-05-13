/**
  ******************************************************************
  * @file    norflash_app.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  */
#include "main.h"


#define EXAMPLE_SECTOR      64//4096      /* Ҫ���ж�д���Ե������� */

/* ��д����ʹ�õĻ����� */
extern uint8_t s_nor_program_buffer[];
extern uint8_t s_nor_read_buffer[];

extern status_t FlexSPI_NorFlash_Enable_QE(FLEXSPI_Type *base);
extern uint8_t  FlexSPI_FlashUUID_Get_ISSI(uint8_t *buf);



/***************************************************************************************
  * @brief   ����̼���������,�ڽ��ܵ������Ĺ̼�����,���øú���
  * @input   
  * @return  
***************************************************************************************/
void NorFlash_SaveFirmPara(void)
{
    FlexSPI_NorFlash_Erase_Sector(FLEXSPI, APP_INFO_SECTOR * SECTOR_SIZE);
    FlexSPI_NorFlash_Buffer_Program(FLEXSPI, APP_INFO_SECTOR * SECTOR_SIZE, &g_sys_para.firmUpdate, 16);

	/* ʹ�������λ������ AHB ������. */
    FLEXSPI_SoftwareReset(FLEXSPI);
	PRINTF("��������:\r\n");
	for (int i=0;i<16;i++){
		PRINTF("%02x ", NORFLASH_AHB_READ_BYTE(APP_INFO_SECTOR * SECTOR_SIZE + i));
	}
}


/**
* @brief  ʹ��AHB����ķ�ʽ���ж�д����
* @param  ��
* @retval ���Խ��������Ϊ0
*/
int NorFlash_ChkSelf(void)
{
    uint32_t i = 0;
    status_t status;
    uint32_t JedecDeviceID = 0;

    /***************************��ID����****************************/
    /* ��ȡJedecDevice ID. */
    FlexSPI_NorFlash_Get_JedecDevice_ID(FLEXSPI, &JedecDeviceID);
    
    if(JedecDeviceID != FLASH_WINBOND_JEDECDEVICE_ID && 
			 JedecDeviceID != FLASH_ISSI_JEDECDEVICE_ID)
    {
      PRINTF("FLASH�����󣬶�ȡ����JedecDeviceIDֵΪ: 0x%x\r\n", JedecDeviceID);
      return -1;
    }
    
    /***************************һ��д��һ���������ݲ���****************************/
    for (i = 0; i < FIRM_ONE_LEN; i++){
        s_nor_program_buffer[i] = (uint8_t)i;
    }

	FlexSPI_FlashWrite(s_nor_program_buffer, EXAMPLE_SECTOR * SECTOR_SIZE + FIRM_ONE_LEN*i, FIRM_ONE_LEN);

    /* ��ȡ���� */
    memcpy(s_nor_read_buffer, 
           NORFLASH_AHB_POINTER(EXAMPLE_SECTOR * SECTOR_SIZE + FIRM_ONE_LEN),
           FIRM_ONE_LEN);
    
    /* �Ѷ�����������д��ıȽ� */
    if (memcmp(s_nor_program_buffer, s_nor_read_buffer,FIRM_ONE_LEN))
    {
        PRINTF("д�����ݣ��������ݲ���ȷ !\r\n ");
        return -1;
    } else {
        PRINTF("��������д��Ͷ�ȡ���Գɹ�. \r\n");
    }

    return 0;  
}

/****************************END OF FILE**********************/
