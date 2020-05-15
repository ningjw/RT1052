/**
  ******************************************************************
  * @file    norflash_app.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  */
#include "main.h"

#define MAX_ADC_INFO 200
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
    FlexSPI_NorFlash_Buffer_Program(FLEXSPI, APP_INFO_SECTOR * SECTOR_SIZE, &g_sys_para.firmUpdate, 20);
	
	/* ʹ�������λ������ AHB ������. */
    FLEXSPI_SoftwareReset(FLEXSPI);
	PRINTF("��������:\r\n");
	for (int i=0;i<20;i++){
		PRINTF("%02x ", NORFLASH_AHB_READ_BYTE(APP_INFO_SECTOR * SECTOR_SIZE + i));
	}
}

/***************************************************************************************
  * @brief   ����һ��json��ʽ��ADC����
  * @input   
  * @return  
***************************************************************************************/
void NorFlash_AddAdcInfo(char *adcJsonData)
{
	
	AdcInfoTotal adcInfoTotal;
	AdcInfo adcInfo;
	
	//ǰ12�ֽڱ������ adcInfoTotal �ṹ��
	memcpy(&adcInfoTotal.totalAdcInfo, NORFLASH_AHB_POINTER(ADC_INFO_SECTOR * SECTOR_SIZE) ,sizeof(adcInfoTotal));
	
	//�ж�Ϊ�״��ϵ�����
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){
		//����Ϊ��ʼ��Ϊ0
		adcInfoTotal.totalAdcInfo = 0;
		//���� AdcInfo �ṹ�屣���ַ
		adcInfoTotal.addrOfNewInfo = ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(adcInfoTotal);
		//�������ݵĿ�ʼ��ַ
		adcInfoTotal.addrOfNewData = ADC_DATA_SECTOR * SECTOR_SIZE;
	}
	
	//��ʼ�� adcInfo �ṹ�� ����ʱ��
    SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
	sprintf(adcInfo.AdcDataTime, "%d%02d%02d%02d%02d%02d", 
		                       rtcDate.year%100, rtcDate.month, rtcDate.day, 
	                           rtcDate.hour, rtcDate.minute, rtcDate.second);
	//��ʼ�� adcInfo �ṹ�� ���ݳ���
	adcInfo.AdcDataLen = strlen(adcJsonData) + 1;//��'\0'������ҲҪ���浽flash����
	//��ʼ�� adcInfo �ṹ�� ���ݵ�ַ
	adcInfo.AdcDataAddr = adcInfoTotal.addrOfNewData;
	if((adcInfo.AdcDataAddr % 4) != 0){//�жϵ�ַ�Ƿ����ֽڶ���
		adcInfo.AdcDataAddr = adcInfo.AdcDataAddr + (4 - (adcInfo.AdcDataAddr % 4));
	}
	if((adcInfo.AdcDataAddr + adcInfo.AdcDataLen) > FLASH_SIZE_BYTE){//�жϵ�ַ�Ƿ񳬹�flash��Χ
		adcInfo.AdcDataAddr = ADC_DATA_SECTOR * SECTOR_SIZE;
	}
	
	//���� adcInfo �ṹ��
	FlexSPI_FlashWrite((uint8_t *)&adcInfo.AdcDataAddr, adcInfoTotal.addrOfNewInfo, sizeof(adcInfo));
	
	//���� ��������
	FlexSPI_FlashWrite((uint8_t *)adcJsonData, adcInfo.AdcDataAddr, adcInfo.AdcDataLen);
	
	//��ʼ�� adcInfoTotal �ṹ���е��ܲ�������
	adcInfoTotal.totalAdcInfo++;//���øú���,��ʾ��Ҫ����һ��adc��������
	if(adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){//�жϳ��Ѵﵽ���ֵ
		adcInfoTotal.totalAdcInfo = MAX_ADC_INFO;
	}
	//��ʼ�� adcInfoTotal �ṹ���е��´β�����Ϣ�����ַ
	adcInfoTotal.addrOfNewInfo = adcInfoTotal.addrOfNewInfo + sizeof(adcInfoTotal);
	if(adcInfoTotal.addrOfNewInfo >= ((ADC_INFO_SECTOR+1) * SECTOR_SIZE)){
		adcInfoTotal.addrOfNewInfo = ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(adcInfo);
	}
	//��ʼ�� adcInfoTotal �ṹ���е��´β������ݱ����ַ
	adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + adcInfo.AdcDataLen;
	if( adcInfoTotal.addrOfNewData % 4 != 0){//�ж��µ�ַ����4�ֽڶ����, ��Ҫ����4�ֽڶ���
		adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + (4-adcInfoTotal.addrOfNewData%4);
	}
	if (adcInfoTotal.addrOfNewData > FLASH_SIZE_BYTE){//�жϳ��µ�ַ����flash��Χ,���ͷ��ʼ��¼
		adcInfoTotal.addrOfNewData = ADC_DATA_SECTOR * SECTOR_SIZE;
	}
	
	//���� adcInfoTotal �ṹ��
	FlexSPI_FlashWrite((uint8_t *)adcInfoTotal.totalAdcInfo, ADC_INFO_SECTOR * SECTOR_SIZE, sizeof(AdcInfoTotal));
}



/***************************************************************************************
  * @brief   ͨ��ʱ���ȡһ��adc����
  * @input   
  * @return  �������ݵ�ַ
***************************************************************************************/
uint32_t NorFlash_ReadAdcData(char *adcDataTime)
{
	char *jsonAdcData;
	uint32_t tempAddr;
	AdcInfoTotal adcInfoTotal;
	AdcInfo adcInfo;
	
	//��ȡ���ݵ� adcInfoTotal �ṹ��
	memcpy(&adcInfoTotal.totalAdcInfo, NORFLASH_AHB_POINTER(ADC_INFO_SECTOR * SECTOR_SIZE) ,sizeof(adcInfoTotal));
	
	//flash�л�δ����������,ֱ�ӷ���
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){
		return NULL;
	}
	
	for(uint32_t i = 0; i<adcInfoTotal.totalAdcInfo; i++){
		//��ǰ�����ļ�
		tempAddr = adcInfoTotal.addrOfNewInfo - (i+1)*sizeof(AdcInfo);
		if(tempAddr < (ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(AdcInfoTotal))){
			tempAddr = (ADC_INFO_SECTOR + 1) * SECTOR_SIZE - sizeof(adcInfo);
		}
		//��ȡ���ݵ� adcInfo �ṹ��
		memcpy(&adcInfo.AdcDataAddr, NORFLASH_AHB_POINTER(tempAddr), sizeof(adcInfo));
		
		//�ҵ��ļ�
		if( memcmp(adcDataTime, adcInfo.AdcDataTime, sizeof(adcInfo.AdcDataTime)) == 0){
			return adcInfo.AdcDataAddr;
		}
	}
	
	return NULL;
}

/***************************************************************************************
  * @brief   ��ȡ����������ݲɼ����ļ���
  * @input   
  * @return  �������ݵ�ַ
***************************************************************************************/
void NorFlash_ReadAdcInfo(int si, int num, char *buf)
{
	char *jsonAdcData;
	uint32_t tempAddr;
	AdcInfoTotal adcInfoTotal;
	AdcInfo adcInfo;
	
	//��ȡ���ݵ� adcInfoTotal �ṹ��
	memcpy(&adcInfoTotal.totalAdcInfo, NORFLASH_AHB_POINTER(ADC_INFO_SECTOR * SECTOR_SIZE) ,sizeof(adcInfoTotal));
	
	//flash�л�δ����������,ֱ�ӷ���
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){
		return;
	}
	
	if(si > MAX_ADC_INFO)si= MAX_ADC_INFO;
	if(si <= 0 ) si = 1;
	
	for(uint32_t i = si; i<num+si; i++){
		//��ǰ�����ļ�
		tempAddr = adcInfoTotal.addrOfNewInfo - i*sizeof(AdcInfo);
		if(tempAddr < (ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(AdcInfoTotal))){
			tempAddr = (ADC_INFO_SECTOR + 1) * SECTOR_SIZE - sizeof(adcInfo);
		}
		//��ȡ���ݵ� adcInfo �ṹ��
		memcpy(&adcInfo.AdcDataAddr, NORFLASH_AHB_POINTER(tempAddr), sizeof(adcInfo));
		
		strcat(buf, adcInfo.AdcDataTime);
		strcat(buf,";");//ʹ�ö��ŷָ���
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

	FlexSPI_FlashWrite(s_nor_program_buffer, EXAMPLE_SECTOR * SECTOR_SIZE + FIRM_ONE_LEN, FIRM_ONE_LEN);

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
