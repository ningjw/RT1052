/**
  ******************************************************************
  * @file    norflash_app.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  */
#include "main.h"


#define EXAMPLE_SECTOR      4096      /* Ҫ���ж�д���Ե������� */
#define EXAMPLE_SIZE        128  /* ��д���Ե�����������λΪ�ֽ�*/

/* ��д����ʹ�õĻ����� */
static uint8_t s_nor_program_buffer[EXAMPLE_SIZE];
static uint8_t s_nor_read_buffer[EXAMPLE_SIZE];

extern status_t FlexSPI_NorFlash_Enable_QE(FLEXSPI_Type *base);
extern uint8_t  FlexSPI_FlashUUID_Get_ISSI(uint8_t *buf);

/***************************************************************************************
  * @brief   �����ܵ��Ĺ̼�����д��Nor Flash
  * @input   buff:��Ҫд��������׵�ַ
             len:��Ҫд������ݳ���
  * @return  
***************************************************************************************/
int NorFlash_WriteApp(uint8_t* buff, uint8_t len)
{
    /* д�����ݵ�Nor Flash */
    FlexSPI_NorFlash_Buffer_Program(FLEXSPI, g_sys_para.firmNextAddr, buff, FIRM_ONE_PACKE_LEN);
    
    return 0;
}

/***************************************************************************************
  * @brief   ����̼���������,�ڽ��ܵ������Ĺ̼�����,���øú���
  * @input   
  * @return  
***************************************************************************************/
void NorFlash_SaveFirmPara(void)
{
    FlexSPI_NorFlash_Erase_Sector(FLEXSPI, FIRM_INFO_ADDR);
    FlexSPI_NorFlash_Buffer_Program(FLEXSPI, FIRM_INFO_ADDR, &g_sys_para.firmUpdate, 16);
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
    for (i = 0; i < EXAMPLE_SIZE; i++){
        s_nor_program_buffer[i] = (uint8_t)i;
    }
    
    /* ����ָ������ */
    status = FlexSPI_NorFlash_Erase_Sector(FLEXSPI, EXAMPLE_SECTOR * SECTOR_SIZE);
    if (status != kStatus_Success)
    {
        PRINTF("����flash����ʧ�� !\r\n");
        return -1;
    }

    /* д��һ������������ */
    status = FlexSPI_NorFlash_Buffer_Program(FLEXSPI,
                                             EXAMPLE_SECTOR * SECTOR_SIZE,
                                             (void *)s_nor_program_buffer,
                                             EXAMPLE_SIZE);
    if (status != kStatus_Success)
    {
        PRINTF("д��ʧ�� !\r\n");
        return -1;
    }
    
    /* ʹ�������λ������ AHB ������. */
    FLEXSPI_SoftwareReset(FLEXSPI);

    /* ��ȡ���� */
    memcpy(s_nor_read_buffer, 
           NORFLASH_AHB_POINTER(EXAMPLE_SECTOR * SECTOR_SIZE),
           EXAMPLE_SIZE);
    
    /* �Ѷ�����������д��ıȽ� */
    if (memcmp(s_nor_program_buffer, s_nor_read_buffer,EXAMPLE_SIZE))
    {
        PRINTF("д�����ݣ��������ݲ���ȷ !\r\n ");
        return -1;
    } else {
        PRINTF("��������д��Ͷ�ȡ���Գɹ�. \r\n");
    }

    return 0;  
}

/****************************END OF FILE**********************/
