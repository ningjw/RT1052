/**
  ******************************************************************
  * @file    norflash_app.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  */
#include "main.h"
//��10��sector���ڹ���ADC��������, ÿ����������ռ��20byte, �����Ա���40960/20=2048��
#define MAX_ADC_INFO 2047


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
  * @brief   ����һ��json��ʽ��ADC����
  * @input   
  * @return  
***************************************************************************************/
void NorFlash_AddAdcData(char *adcJsonData)
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
	if(adcInfoTotal.addrOfNewInfo >= ((ADC_INFO_SECTOR+10) * SECTOR_SIZE)){
		adcInfoTotal.addrOfNewInfo = ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(AdcInfoTotal);
	}
	//��ʼ�� adcInfoTotal �ṹ���е��´β������ݱ����ַ
	adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + adcInfo.AdcDataLen;
	if( adcInfoTotal.addrOfNewData % 4 != 0){//�ж��µ�ַ����4�ֽڶ����, ��Ҫ����4�ֽڶ���
		adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + (4-adcInfoTotal.addrOfNewData%4);
	}
	if (adcInfoTotal.addrOfNewData > FLASH_SIZE_BYTE){//�жϳ��µ�ַ����flash��Χ,���ͷ��ʼ��¼
		adcInfoTotal.addrOfNewData = ADC_DATA_SECTOR * SECTOR_SIZE;
		adcInfoTotal.freeOfKb = 0;
	}else{
		adcInfoTotal.freeOfKb = (FLASH_SIZE_BYTE - adcInfoTotal.addrOfNewInfo)/1024;
	}

	//���� adcInfoTotal �ṹ��
	FlexSPI_FlashWrite((uint8_t *)&adcInfoTotal.totalAdcInfo, ADC_INFO_SECTOR * SECTOR_SIZE, sizeof(AdcInfoTotal));
}



/***************************************************************************************
  * @brief   ͨ��ʱ���ȡһ��adc����
  * @input   
  * @return  �������ݵ�ַ
***************************************************************************************/
uint32_t NorFlash_ReadAdcData(char *adcDataTime)
{
	uint32_t tempAddr;
	
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
			tempAddr = (ADC_INFO_SECTOR + 10) * SECTOR_SIZE - sizeof(adcInfo);
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
		
		strcat(buf, adcInfo.AdcDataTime);
		strcat(buf,",");//��ӷָ���
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
//	for(i = 0; i<100;i++){
		FlexSPI_FlashWrite(s_nor_program_buffer, APP_START_SECTOR * SECTOR_SIZE + (FIRM_ONE_LEN), FIRM_ONE_LEN);
//	}
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


/***************************************************************************************
  * @brief   �����ݴ����json��ʽ��,���浽flash����
  * @input
  * @return
***************************************************************************************/
void SaveSampleData(void)
{
    free(g_sys_para.sampJson);
    g_sys_para.sampJson = NULL;
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return;
    }
    cJSON_AddStringToObject(pJsonRoot, "DP", g_adc_set.IDPath);//Ӳ���汾��
    cJSON_AddStringToObject(pJsonRoot, "NP", g_adc_set.NamePath);//Ӳ���汾��
    cJSON_AddStringToObject(pJsonRoot, "SU", g_adc_set.SpeedUnits);
    cJSON_AddStringToObject(pJsonRoot, "PU", g_adc_set.ProcessUnits);
    cJSON_AddNumberToObject(pJsonRoot, "DT", g_adc_set.DetectionType);
    cJSON_AddNumberToObject(pJsonRoot, "SEN", g_adc_set.Senstivity);
    cJSON_AddNumberToObject(pJsonRoot, "ZD", g_adc_set.Zerodrift);
    cJSON_AddNumberToObject(pJsonRoot, "ET", g_adc_set.EUType);
    cJSON_AddStringToObject(pJsonRoot, "EU", g_adc_set.EU);
    cJSON_AddNumberToObject(pJsonRoot, "W", g_adc_set.WindowsType);
    cJSON_AddNumberToObject(pJsonRoot, "SF", g_adc_set.StartFrequency);
    cJSON_AddNumberToObject(pJsonRoot, "EF", g_adc_set.EndFrequency);
    cJSON_AddNumberToObject(pJsonRoot, "SR", g_adc_set.SampleRate);
    cJSON_AddNumberToObject(pJsonRoot, "L", g_adc_set.Lines);
    cJSON_AddNumberToObject(pJsonRoot, "B", g_sys_para.bias);
    cJSON_AddNumberToObject(pJsonRoot, "RV", g_sys_para.refV);
    cJSON_AddNumberToObject(pJsonRoot, "A", g_adc_set.Averages);
    cJSON_AddNumberToObject(pJsonRoot, "OL", g_adc_set.AverageOverlap);
    cJSON_AddNumberToObject(pJsonRoot, "AT", g_adc_set.AverageType);
    cJSON_AddNumberToObject(pJsonRoot, "EFL", g_adc_set.EnvFilterLow);
    cJSON_AddNumberToObject(pJsonRoot, "EFH", g_adc_set.EnvFilterHigh);
    cJSON_AddNumberToObject(pJsonRoot, "IM", g_adc_set.IncludeMeasurements);
    cJSON_AddNumberToObject(pJsonRoot, "SP", g_adc_set.Speed);
    cJSON_AddNumberToObject(pJsonRoot, "P", g_adc_set.Process);
    cJSON_AddNumberToObject(pJsonRoot, "PL", g_adc_set.ProcessMin);
    cJSON_AddNumberToObject(pJsonRoot, "PH", g_adc_set.ProcessMax);
    cJSON_AddNumberToObject(pJsonRoot, "PK", g_sys_para.sampPacks);
    cJSON_AddNumberToObject(pJsonRoot, "Y", sampTime.year);
    cJSON_AddNumberToObject(pJsonRoot, "M", sampTime.month);
    cJSON_AddNumberToObject(pJsonRoot, "D", sampTime.day);
    cJSON_AddNumberToObject(pJsonRoot, "H", sampTime.hour);
    cJSON_AddNumberToObject(pJsonRoot, "Min", sampTime.minute);
    cJSON_AddNumberToObject(pJsonRoot, "S", sampTime.second);
	cJSON_AddNumberToObject(pJsonRoot, "spdCnt", g_sys_para.spdCount);
	cJSON_AddNumberToObject(pJsonRoot, "vibCnt", g_sys_para.shkCount);
    cJSON_AddStringToObject(pJsonRoot, "Vibrate", VibrateStrADC);
    cJSON_AddStringToObject(pJsonRoot, "Speed", SpeedStrADC);

    g_sys_para.sampJson = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
	
    /*������õ����ݱ��浽�ļ� */
    if (NULL != g_sys_para.sampJson) {
        NorFlash_AddAdcData(g_sys_para.sampJson);
//        //������ͨ�����ڴ�ӡ����
//        PRINTF("%s", g_sys_para.sampJson);
    }
}

/****************************END OF FILE**********************/
