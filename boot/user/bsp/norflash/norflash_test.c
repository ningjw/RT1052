/**
  ******************************************************************
  * @file    norflash_test.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   nor flash����
  ******************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��  i.MXRT1052������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************
  */
#include "fsl_flexspi.h"

#include "fsl_debug_console.h"
#include "./norflash/bsp_norflash.h"  
#include "./delay/core_delay.h"



#define EXAMPLE_SECTOR      4096      /* Ҫ���ж�д���Ե������� */
#define EXAMPLE_SIZE        (4*1024)  /* ��д���Ե�����������λΪ�ֽ�*/

/* ��д����ʹ�õĻ����� */
static uint8_t s_nor_program_buffer[EXAMPLE_SIZE];
static uint8_t s_nor_read_buffer[EXAMPLE_SIZE];

extern status_t FlexSPI_NorFlash_Enable_QE(FLEXSPI_Type *base);
extern uint8_t FlexSPI_FlashUUID_Get_ISSI(uint8_t *buf);

/**
* @brief  ʹ��AHB����ķ�ʽ���ж�д����
* @param  ��
* @retval ���Խ��������Ϊ0
*/
int NorFlash_AHBCommand_Test(void)
{
    uint32_t i = 0;
    status_t status;
    uint32_t JedecDeviceID = 0;

    PRINTF("\r\nNorFlash AHB������ʲ���\r\n");

    /***************************��ID����****************************/
    /* ��ȡJedecDevice ID. */
    FlexSPI_NorFlash_Get_JedecDevice_ID(FLEXSPI, &JedecDeviceID);
    
    if(JedecDeviceID != FLASH_WINBOND_JEDECDEVICE_ID && 
			 JedecDeviceID != FLASH_ISSI_JEDECDEVICE_ID)
    {
      PRINTF("FLASH�����󣬶�ȡ����JedecDeviceIDֵΪ: 0x%x\r\n", JedecDeviceID);
      return -1;
    }
    
    PRINTF("��⵽FLASHоƬ��JedecDeviceIDֵΪ: 0x%x\r\n", JedecDeviceID);
    
    /***************************��������****************************/
    PRINTF("������������\r\n");
    
    /* ����ָ������ */
    status = FlexSPI_NorFlash_Erase_Sector(FLEXSPI, EXAMPLE_SECTOR * SECTOR_SIZE);
    if (status != kStatus_Success)
    {
        PRINTF("����flash����ʧ�� !\r\n");
        return -1;
    }
    
    /* ��ȡ���� */
    memcpy(s_nor_read_buffer, 
           NORFLASH_AHB_POINTER(EXAMPLE_SECTOR * SECTOR_SIZE),
           EXAMPLE_SIZE);
    
    /* ������FLASH�е�����ӦΪ0xFF��
       ���ñȽ��õ�s_nor_program_bufferֵȫΪ0xFF */    
    memset(s_nor_program_buffer, 0xFF, EXAMPLE_SIZE);
    /* �Ѷ�����������0xFF�Ƚ� */
    if (memcmp(s_nor_program_buffer, s_nor_read_buffer, EXAMPLE_SIZE))
    {
        PRINTF("�������ݣ��������ݲ���ȷ !\r\n ");
        return -1;
    }
    else
    {
        PRINTF("�������ݳɹ�. \r\n");
    }
    
    /***************************һ��д���������ݲ���****************************/
    PRINTF("8��16��32λд�����ݲ��ԣ�0x12��0x3456��0x789abcde \r\n");
    /*ʹ��AHB���ʽд������*/
    *(uint8_t *)NORFLASH_AHB_POINTER(EXAMPLE_SECTOR * SECTOR_SIZE) = 0x12;
    *(uint16_t *)NORFLASH_AHB_POINTER(EXAMPLE_SECTOR * SECTOR_SIZE + 4) = 0x3456;
    *(uint32_t *)NORFLASH_AHB_POINTER(EXAMPLE_SECTOR * SECTOR_SIZE + 8) = 0x789abcde;

    /*ʹ��AHB���ʽ��ȡ����*/
    PRINTF("8λ��д��� = 0x%x\r\n",
           *(uint8_t *)NORFLASH_AHB_POINTER(EXAMPLE_SECTOR * SECTOR_SIZE));
    PRINTF("16λ��д��� = 0x%x\r\n",
           *(uint16_t *)NORFLASH_AHB_POINTER(EXAMPLE_SECTOR * SECTOR_SIZE+4));
    PRINTF("32λ��д��� = 0x%x\r\n",
           *(uint32_t *)NORFLASH_AHB_POINTER(EXAMPLE_SECTOR * SECTOR_SIZE+8));
    
    /***************************һ��д��һ���������ݲ���****************************/
    PRINTF("\r\n��������д��Ͷ�ȡ���� \r\n");
    
    for (i = 0; i < EXAMPLE_SIZE; i++)
    {
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
    }
    else
    {
        PRINTF("��������д��Ͷ�ȡ���Գɹ�. \r\n");
    }  
    
    PRINTF("NorFlash AHB������ʲ�����ɡ�\r\n");
    
    return 0;  
}

/****************************END OF FILE**********************/
