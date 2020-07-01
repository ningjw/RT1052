#include "main.h"



/* ��д����ʹ�õĻ����� */
extern uint8_t s_nor_program_buffer[];
extern uint8_t s_nor_read_buffer[];

extern status_t FlexSPI_NorFlash_Enable_QE(FLEXSPI_Type *base);
extern uint8_t  FlexSPI_FlashUUID_Get_ISSI(uint8_t *buf);

AdcInfoTotal adcInfoTotal;
AdcInfo adcInfo;

/***************************************************************************************
  * @brief   ����̼���������,�ڽ��ܵ������Ĺ̼�����,���øú���
  * @input   
  * @return  
***************************************************************************************/
void NorFlash_SaveUpgradePara(void)
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
  * @brief   ����һ��ADC����
  * @input   
  * @return  
***************************************************************************************/
void NorFlash_AddAdcData(void)
{
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
	sprintf(adcInfo.AdcDataTime, "%d%02d%02d%02d%02d%02d", 
		                       sampTime.year%100, sampTime.month, sampTime.day, 
	                           sampTime.hour, sampTime.minute, sampTime.second);
	//��ʼ�� adcInfo �ṹ�� ���ݳ���
	adcInfo.AdcDataLen = sizeof(ADC_Set) + g_adc_set.shkCount*4 + g_adc_set.spdCount*4;
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
	
	//���� ADC_Set �ṹ��
	FlexSPI_FlashWrite((uint8_t *)&g_adc_set.IDPath[0], adcInfo.AdcDataAddr, sizeof(ADC_Set));
	
	//���� ������
	FlexSPI_FlashWrite((uint8_t *)&ShakeADC[0], adcInfo.AdcDataAddr+sizeof(ADC_Set), g_adc_set.shkCount*4);
	
	//���� ת������
	FlexSPI_FlashWrite((uint8_t *)&SpeedADC[0], adcInfo.AdcDataAddr+sizeof(ADC_Set)+g_adc_set.shkCount*4, g_adc_set.spdCount*4);
	
	//���� adcInfoTotal �ṹ���е��ܲ�������
	adcInfoTotal.totalAdcInfo++;//���øú���,��ʾ��Ҫ����һ��adc��������
	if(adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){//�жϳ��Ѵﵽ���ֵ
		adcInfoTotal.totalAdcInfo = MAX_ADC_INFO;
	}
	//���� adcInfoTotal �ṹ���е��´β�����Ϣ�����ַ
	adcInfoTotal.addrOfNewInfo = adcInfoTotal.addrOfNewInfo + sizeof(adcInfoTotal);
	if(adcInfoTotal.addrOfNewInfo >= ((ADC_INFO_SECTOR+10) * SECTOR_SIZE)){
		adcInfoTotal.addrOfNewInfo = ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(AdcInfoTotal);
	}
	//���� adcInfoTotal �ṹ���е��´β������ݱ����ַ
	adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + adcInfo.AdcDataLen;
	if( adcInfoTotal.addrOfNewData % 4 != 0){//�ж��µ�ַ����4�ֽڶ����, ��Ҫ����4�ֽڶ���
		adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + (4-adcInfoTotal.addrOfNewData%4);
	}
	if (adcInfoTotal.addrOfNewData > FLASH_SIZE_BYTE){//�жϳ��µ�ַ����flash��Χ,���ͷ��ʼ��¼
		adcInfoTotal.addrOfNewData = ADC_DATA_SECTOR * SECTOR_SIZE;
		adcInfoTotal.freeOfKb = 0;
	}else{
		adcInfoTotal.freeOfKb = (FLASH_SIZE_BYTE - adcInfoTotal.addrOfNewData)/1024;
	}

	//���� adcInfoTotal �ṹ��
	FlexSPI_FlashWrite((uint8_t *)&adcInfoTotal.totalAdcInfo, ADC_INFO_SECTOR * SECTOR_SIZE, sizeof(AdcInfoTotal));
}



/***************************************************************************************
  * @brief   ͨ��ʱ���ȡһ��adc����
  * @input   
  * @return  �������ݵ�ַ
***************************************************************************************/
char NorFlash_ReadAdcData(char *adcDataTime)
{
	uint32_t tempAddr;
	uint8_t  ret = false;
	
	//��ȡ���ݵ� adcInfoTotal �ṹ��
	memcpy(&adcInfoTotal.totalAdcInfo, NORFLASH_AHB_POINTER(ADC_INFO_SECTOR * SECTOR_SIZE) ,sizeof(adcInfoTotal));
	
	//flash�л�δ����������,ֱ�ӷ���
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){
		return ret;
	}
	
	for(uint32_t i = 0; i<adcInfoTotal.totalAdcInfo; i++){
		//��ǰ�����ļ�
		tempAddr = adcInfoTotal.addrOfNewInfo - (i+1)*sizeof(AdcInfo);
		if(tempAddr < (ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(AdcInfoTotal))){
			tempAddr = (ADC_INFO_SECTOR + 10) * SECTOR_SIZE - sizeof(adcInfo);
		}
		//��ȡ���ݵ� adcInfo �ṹ��
		memcpy(&adcInfo.AdcDataAddr, NORFLASH_AHB_POINTER(tempAddr), sizeof(adcInfo));
		
		//�ҵ��ļ�
		if( memcmp(adcDataTime, adcInfo.AdcDataTime, sizeof(adcInfo.AdcDataTime)) == 0){
			ret = true;
			break;
		}
	}
	if ( ret == true){
		//��ȡ ADC_Set �ṹ������
		memcpy((uint8_t *)&g_adc_set.IDPath[0], NORFLASH_AHB_POINTER(adcInfo.AdcDataAddr), sizeof(ADC_Set));
		
		//��ȡ ������
		memcpy(ShakeADC, NORFLASH_AHB_POINTER(adcInfo.AdcDataAddr+sizeof(ADC_Set)), g_adc_set.shkCount);
		
		//��ȡ ת������
		if (g_adc_set.spdCount != 0){
			memcpy(SpeedADC, NORFLASH_AHB_POINTER(adcInfo.AdcDataAddr+sizeof(ADC_Set)+g_adc_set.shkCount), g_adc_set.spdCount);
		}
	}
	return ret;
}

/***************************************************************************************
  * @brief   ��ȡ����������ݲɼ����ļ���
  * @input   
  * @return  �������ݵ�ַ
***************************************************************************************/
void NorFlash_ReadAdcInfo(int si, int num, char *buf)
{
	uint32_t tempAddr,ei;
	
	//��ȡ���ݵ� adcInfoTotal �ṹ��
	memcpy(&adcInfoTotal.totalAdcInfo, NORFLASH_AHB_POINTER(ADC_INFO_SECTOR * SECTOR_SIZE) ,sizeof(adcInfoTotal));
	
	//flash�л�δ����������,ֱ�ӷ���
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){
		adcInfoTotal.totalAdcInfo = 0;
		adcInfoTotal.freeOfKb = (FLASH_SIZE_BYTE - ADC_DATA_SECTOR*SECTOR_SIZE)/1024;
		return;
	}
	
	if(si > MAX_ADC_INFO)si= MAX_ADC_INFO;
	if(si <= 0 ) si = 1;
	
	ei = num+si;
	if(ei > adcInfoTotal.totalAdcInfo+1) ei=adcInfoTotal.totalAdcInfo+1;
	
	for(uint32_t i = si; i<ei; i++){
		//��ǰ�����ļ�
		tempAddr = adcInfoTotal.addrOfNewInfo - i*sizeof(AdcInfo);
		if(tempAddr < (ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(AdcInfoTotal))){
			tempAddr = (ADC_INFO_SECTOR + 10) * SECTOR_SIZE - sizeof(adcInfo);
		}
		//��ȡ���ݵ� adcInfo �ṹ��
		memcpy(&adcInfo.AdcDataAddr, NORFLASH_AHB_POINTER(tempAddr), sizeof(adcInfo));
		strncat(buf, adcInfo.AdcDataTime, 12);
		strcat(buf, ",");//��ӷָ���
	}
	if(strlen(buf) > 0){//ȥ�����һ���ָ���
		buf[strlen(buf)-1] = 0x00;
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

	FlexSPI_FlashWrite(s_nor_program_buffer, APP_START_SECTOR * SECTOR_SIZE + (FIRM_ONE_LEN), FIRM_ONE_LEN);

    /* ��ȡ���� */
    memcpy(s_nor_read_buffer, 
           NORFLASH_AHB_POINTER(APP_START_SECTOR * SECTOR_SIZE + FIRM_ONE_LEN),
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
