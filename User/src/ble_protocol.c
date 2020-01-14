#include "main.h"

#define ONE_PACK_LEN 150

extern float ShakeADC[];
extern float SpeedADC[];
extern char  SpeedStrADC[];
extern char  ShakeStrADC[];
time_t seconds;
struct tm *data_time;
/***************************************************************************************
  * @brief   ������ϢidΪ1����Ϣ, ����Ϣ���õ����RTCʱ��
  * @input   
  * @return  
***************************************************************************************/
static char* ParseSetTime(cJSON *pJson, cJSON * pSub)
{
    /*������Ϣ����, ��ȡ���ں�ʱ��*/
    pSub = cJSON_GetObjectItem(pJson, "LongTime");
    if(NULL != pSub){
        seconds = pSub->valueint;
        data_time = gmtime(&seconds);
        rtcDate.year = 1900 + data_time->tm_year;
        rtcDate.month = 1+ data_time->tm_mon;
        rtcDate.day = data_time->tm_mday;
        rtcDate.hour = data_time->tm_hour;
        rtcDate.minute = data_time->tm_min;
        rtcDate.second = data_time->tm_sec;
    }else{
        pSub = cJSON_GetObjectItem(pJson, "Y");
        if (NULL != pSub)
            rtcDate.year = pSub->valueint;
        
        pSub = cJSON_GetObjectItem(pJson, "Mon");
        if (NULL != pSub)
            rtcDate.month = pSub->valueint;
        
        pSub = cJSON_GetObjectItem(pJson, "D");
        if (NULL != pSub)
            rtcDate.day = pSub->valueint;
        
         pSub = cJSON_GetObjectItem(pJson, "H");
        if (NULL != pSub)
            rtcDate.hour = pSub->valueint;
        
        pSub = cJSON_GetObjectItem(pJson, "Min");
        if (NULL != pSub)
            rtcDate.minute = pSub->valueint;
        
        pSub = cJSON_GetObjectItem(pJson, "S");
        if (NULL != pSub)
            rtcDate.second = pSub->valueint;
    }
    /*�������ں�ʱ��*/
    SNVS_HP_RTC_SetDatetime(SNVS, &rtcDate);
    
    /*����cjson��ʽ�Ļظ���Ϣ*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    
    /* �����ظ�json�� */
    cJSON_AddNumberToObject(pJsonRoot, "Id", 1);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ2����Ϣ, ����Ϣ��ȡ�����RTCʱ��
  * @input   
  * @return  
***************************************************************************************/
static char * ParseGetTime(void)
{
	/* ��ȡ���� */
    SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
	
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 2);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "Y", rtcDate.year);
    cJSON_AddNumberToObject(pJsonRoot, "Mon", rtcDate.month);
    cJSON_AddNumberToObject(pJsonRoot, "D", rtcDate.day);
    cJSON_AddNumberToObject(pJsonRoot, "H", rtcDate.hour);
    cJSON_AddNumberToObject(pJsonRoot, "Min", rtcDate.minute);
    cJSON_AddNumberToObject(pJsonRoot, "S", rtcDate.second);
 
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ3����Ϣ, ����ϢΪ�����Լ�
  * @input   
  * @return  
***************************************************************************************/
static char * ParseChkSelf(void)
{
    /*����cjson��ʽ�Ļظ���Ϣ*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    
    //ָʾ���Լ�
    LED_CheckSelf();
    
    //����Լ�
    g_sys_para.batVoltage = LTC2942_GetVoltage() / 1000.0;// Battery voltage
    g_sys_para.batRemainPercent = LTC2942_GetAC() * 100.0 / 65536; // Accumulated charge
    
    //�������ģ���Լ�
    g_sys_para.objTemp = MXL_ReadObjTemp();
    
    //�ļ�ϵͳ�Լ�
    eMMC_CheckFatfs();
    
    //�𶯴�������ѹ
    while (ADC_READY == 0);  //wait ads1271 ready
    g_sys_para.voltageADS1271 = LPSPI4_ReadData() * g_sys_para.bias * 1.0f / 0x800000;
    
    cJSON_AddNumberToObject(pJsonRoot, "Id", 3);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "AdcV", g_sys_para.voltageADS1271);  //�񶯴�����ƫ�õ�ѹ
    cJSON_AddNumberToObject(pJsonRoot, "Temp", g_sys_para.objTemp);         //�¶ȴ��������¶�
    cJSON_AddNumberToObject(pJsonRoot, "PwrV", g_sys_para.batVoltage);      //��ص�ѹֵ
    cJSON_AddNumberToObject(pJsonRoot, "BatC", (int)g_sys_para.batRemainPercent);//��ص���
    cJSON_AddNumberToObject(pJsonRoot, "Flash",(uint8_t)g_sys_para.emmcIsOk);//�ļ�ϵͳ�Ƿ�OK
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ4����Ϣ, ����ϢΪ��ȡ����
  * @input   
  * @return  
***************************************************************************************/
static char * ParseGetBatCapacity(void)
{
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 4);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "BatC", (int)g_sys_para.batRemainPercent);//��ص���
    cJSON_AddNumberToObject(pJsonRoot, "PwrV", g_sys_para.batVoltage);      //��ص�ѹֵ
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ5����Ϣ, ����ϢΪ��ȡ�汾��
  * @input   
  * @return  
***************************************************************************************/
static char * ParseGetVersion(void)
{
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 5);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddStringToObject(pJsonRoot, "HV", "1.0");//Ӳ���汾��
    cJSON_AddStringToObject(pJsonRoot, "SV", SOFT_VERSION);//����汾��
    
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ6����Ϣ, ����ϢΪϵͳ��������
  * @input   
  * @return  
***************************************************************************************/
static char * ParseSetSysPara(cJSON *pJson, cJSON * pSub)
{
    /*������Ϣ����,*/
    pSub = cJSON_GetObjectItem(pJson, "OffT");
    if (NULL != pSub)
        g_sys_para.inactiveTime = pSub->valueint;//�����Զ��ػ�ʱ��
    
    pSub = cJSON_GetObjectItem(pJson, "OffC");
    if (NULL != pSub)
        g_sys_para.inactiveCondition = pSub->valueint;//�����Զ��ػ�����
    
    pSub = cJSON_GetObjectItem(pJson, "BatL");
    if (NULL != pSub)
        g_sys_para.batAlarmValue = pSub->valueint;//��ص�������ֵ
    
    /*����cjson��ʽ�Ļظ���Ϣ*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 6);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ7����Ϣ, ����ϢΪ�ɼ���������
  * @input   
  * @return  
***************************************************************************************/
static char * ParseSetSamplePara(cJSON *pJson, cJSON * pSub)
{
    /*������Ϣ����,*/
    pSub = cJSON_GetObjectItem(pJson, "Mode");
    if (NULL != pSub)
        g_sys_para.sampMode = pSub->valueint;
    
    pSub = cJSON_GetObjectItem(pJson, "Freq");
    if (NULL != pSub){
        g_sys_para.sampFreq = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "Bw");
    if (NULL != pSub)
        g_sys_para.sampBandwidth = pSub->valueint;
    
    pSub = cJSON_GetObjectItem(pJson, "Time");
    if (NULL != pSub){
        g_sys_para.sampTimeSet = pSub->valueint;
    }
    
    pSub = cJSON_GetObjectItem(pJson, "Bias");
    if (NULL != pSub){
        g_sys_para.bias = pSub->valuedouble;
    }
    
    pSub = cJSON_GetObjectItem(pJson, "RefV");
    if (NULL != pSub){
        g_sys_para.refV = pSub->valuedouble;
    }
    
    /*����cjson��ʽ�Ļظ���Ϣ*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 7);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ8����Ϣ, ����ϢΪ��ʼ�����ź�
  * @input   
  * @return  
***************************************************************************************/
static char * ParseStartSample(void)
{
    cJSON *pJson = cJSON_CreateObject();
    if(NULL == pJson){
        return NULL;
    }
    cJSON_AddNumberToObject(pJson, "Id",  8);
    cJSON_AddNumberToObject(pJson, "Sid", 0);
    char *sendBuf = cJSON_Print(pJson);
    cJSON_Delete(pJson);
    LPUART2_SendString((char *)sendBuf);
    free(sendBuf);
    sendBuf = NULL;
    
    /*start sample*/
    ADC_SampleStart();
    
    /*wait task notify*/
    ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
    
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id",  8);
    cJSON_AddNumberToObject(pJsonRoot, "Sid", 0);
    cJSON_AddStringToObject(pJsonRoot, "fileName", g_sys_para.fileName);
    cJSON_AddBoolToObject(pJsonRoot, "saveOk", g_sys_para.saveOk);
    cJSON_AddNumberToObject(pJsonRoot, "size", g_sys_para.sampPacks);
    cJSON_AddNumberToObject(pJsonRoot, "shkNum", g_sys_para.ADC_ShakeCnt);
    cJSON_AddNumberToObject(pJsonRoot, "shkV", g_sys_para.voltageADS1271);
    cJSON_AddNumberToObject(pJsonRoot, "spdNum", g_sys_para.ADC_SpdCnt);
    cJSON_AddNumberToObject(pJsonRoot, "spdV", g_sys_para.voltageSpd);
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}


float envValue[5] = {0.1,0.2,0.3,0.4,0.5};
float spdValue[5] = {0.6,0.7,0.8,0.9,1.0};
/***************************************************************************************
  * @brief   ������ϢidΪ9����Ϣ, ����ϢΪ��ȡ��������
  * @input   
  * @return  
***************************************************************************************/
char * ParseSampleData(void)
{
    free(g_sys_para.sampJson);
    g_sys_para.sampJson = NULL;
    g_sys_para.sampJsonSize = 0;
    g_sys_para.sampJsonPacks = 0;
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id",  9);
    cJSON_AddNumberToObject(pJsonRoot, "Sid", 0);
    cJSON_AddStringToObject(pJsonRoot, "IDPath", "GroupID\\FactoryID\\EquipmentID\\PointID");
    cJSON_AddStringToObject(pJsonRoot, "NamePath", "GroupID\\FactoryID\\EquipmentID\\PointID");
    cJSON_AddNumberToObject(pJsonRoot, "Speed", 1);// ȡת�ٲ���ƽ��ת��
    cJSON_AddStringToObject(pJsonRoot, "SpeedUnits", "RPM");
    cJSON_AddNumberToObject(pJsonRoot, "Process", 1);//ȡƽ���¶�
    cJSON_AddNumberToObject(pJsonRoot, "ProcessMin", 65.0);
    cJSON_AddNumberToObject(pJsonRoot, "ProcessMax", 68.0);
    cJSON_AddStringToObject(pJsonRoot, "ProcessUnits", "��");//����ȡ�¶ȡ��������ʵ�   
    cJSON_AddStringToObject(pJsonRoot, "DAUID", "");//���ɵ�DAUID
    cJSON_AddStringToObject(pJsonRoot, "DetectionType", "0");//�ֶ����0��ʱ�䶨ʱ���1
    cJSON_AddNumberToObject(pJsonRoot, "Senstivity", 100.0);//������
    cJSON_AddNumberToObject(pJsonRoot, "Zerodrift", 0.0);//���ƫ�ƣ����ڽ�ʧ����ѹ���ڵ���
    cJSON_AddNumberToObject(pJsonRoot, "EUType", 4);//g:0��mm/s2:1��um:2,mm/s:3,, gse:4
    cJSON_AddStringToObject(pJsonRoot, "EU", "mm/s");
    cJSON_AddNumberToObject(pJsonRoot, "WindowsType", 1);// ���δ�0�����Ǵ�1��������2��������3������������4������5
    cJSON_AddStringToObject(pJsonRoot, "WindowName", "������");//
    cJSON_AddNumberToObject(pJsonRoot, "StartFrequency", 0);//�ɼ���ʼƵ��
    cJSON_AddNumberToObject(pJsonRoot, "EndFrequency", 2000);//�ɼ�����Ƶ��
    cJSON_AddNumberToObject(pJsonRoot, "SampleRate", 2560);//������
    cJSON_AddNumberToObject(pJsonRoot, "Lines", 1);   //����
    cJSON_AddNumberToObject(pJsonRoot, "Averages", 1);//ƽ������
    cJSON_AddNumberToObject(pJsonRoot, "AverageOverlap", 0.5);//�ص���
    cJSON_AddNumberToObject(pJsonRoot, "AverageType", 0);//�ص���ʽ
    cJSON_AddNumberToObject(pJsonRoot, "EnvFilterLow", 500);//�����˲�Ƶ�� �� 
    cJSON_AddNumberToObject(pJsonRoot, "EnvFilterHigh", 10000);//�����˲�Ƶ�� ��
    cJSON_AddNumberToObject(pJsonRoot, "StorageReson", 10000);//�ɼ���ʽ     0�ֶ��ɼ���1��ʱ�ɼ�
    cJSON_AddStringToObject(pJsonRoot, "MeasurementComment", "equipment condition description");//
    cJSON_AddNumberToObject(pJsonRoot, "IncludeMeasurements", 1);
    cJSON_AddStringToObject(pJsonRoot, "Content", " ");
    cJSON_AddNumberToObject(pJsonRoot, "Bias", 10);
    
    cJSON *measureMents = cJSON_AddArrayToObject(pJsonRoot, "MeasureMents");

    cJSON *pJsonSub1 = cJSON_CreateObject();
    cJSON_AddStringToObject(pJsonSub1, "MeasurementType", "VibRawData/EnvData");
    cJSON_AddStringToObject(pJsonSub1, "Value", ShakeStrADC);
//    cJSON *shkValues = cJSON_AddArrayToObject(pJsonRoot, "Value");
//    for(uint32_t i = 0; i< g_sys_para.ADC_ShakeCnt; i++){
//        cJSON *shkValue = cJSON_CreateNumber(ShakeADC[i]);
//        cJSON_AddItemToArray(shkValues,shkValue);
//    }
    cJSON_AddItemToArray(measureMents, pJsonSub1);
    
    pJsonSub1 = cJSON_CreateObject();
    cJSON_AddStringToObject(pJsonSub1, "MeasurementType", "RotationSpeed");
    cJSON_AddStringToObject(pJsonSub1, "Value", SpeedStrADC);
//    cJSON *spdValues = cJSON_AddArrayToObject(pJsonRoot, "Value");
//    for(uint32_t i = 0; i< g_sys_para.ADC_SpdCnt; i++){
//        cJSON *spdValue = cJSON_CreateNumber(SpeedADC[i]);
//        cJSON_AddItemToArray(spdValues,spdValue);
//    }
    cJSON_AddItemToArray(measureMents, pJsonSub1);
    
    g_sys_para.sampJson = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    
    /*������õ����ݱ��浽�ļ� */
    if (NULL != g_sys_para.sampJson){
        g_sys_para.sampJsonSize = strlen(g_sys_para.sampJson);
        g_sys_para.sampJsonPacks = g_sys_para.sampJsonSize / ONE_PACK_LEN + ((g_sys_para.sampJsonSize%ONE_PACK_LEN) ? 1 : 0);
        eMMC_SaveSampleData(g_sys_para.sampJson, g_sys_para.sampJsonSize);
        PRINTF("%s", g_sys_para.sampJson);
    }
    
    return g_sys_para.sampJson;
}


/***************************************************************************************
  * @brief   ������ϢidΪ9����Ϣ, ����ϢΪ��ȡ��������
  * @input   
  * @return  
***************************************************************************************/
char * ParseGetSampleData(cJSON *pJson, cJSON * pSub)
{
    uint32_t sid = 0;
    uint8_t  slen = 0;
    
    /*������Ϣ����,*/
    pSub = cJSON_GetObjectItem(pJson, "Sid");
    if (NULL != pSub)
        sid = pSub->valueint;
    if(sid >= g_sys_para.sampJsonPacks){
        memset(g_lpuart2TxBuf, 0, LPUART2_BUFF_LEN);
        return (char *)g_lpuart2TxBuf;
    }
    else if(sid == g_sys_para.sampJsonPacks - 1){//���һ��������
        slen = g_sys_para.sampJsonSize % ONE_PACK_LEN;
    }else{
        slen = ONE_PACK_LEN;
    }
    memset(g_lpuart2TxBuf, 0, LPUART2_BUFF_LEN);
    memcpy(g_lpuart2TxBuf, (g_sys_para.sampJson+sid*ONE_PACK_LEN), slen);
    return (char *)g_lpuart2TxBuf;
}
/***************************************************************************************
  * @brief   ������ϢidΪ10����Ϣ, ����ϢΪ��ʼ���������̼���
  * @input   
  * @return  
***************************************************************************************/
static char * ParseStartUpdate(cJSON *pJson, cJSON * pSub)
{
    /* ��ʼ�����̼���, ��ʼ��һЩ��Ҫ�ı���*/
    g_sys_para.firmUpdate = false;
    g_sys_para.firmPacksCount = 0;
    g_sys_para.firmSizeCurrent = 0;
    g_sys_para.firmNextAddr = FIRM_DATA_ADDR;
    
    /*������Ϣ����,*/
    pSub = cJSON_GetObjectItem(pJson, "Packs");
    if (NULL != pSub)
        g_sys_para.firmPacksTotal = pSub->valueint;
    
    pSub = cJSON_GetObjectItem(pJson, "Size");
    if (NULL != pSub)
        g_sys_para.firmSizeTotal = pSub->valueint;
    
    pSub = cJSON_GetObjectItem(pJson, "CRC16");
    if (NULL != pSub)
        g_sys_para.firmCrc16 = pSub->valueint;
    
    g_sys_para.firmPacksCount = 0;
	g_sys_para.firmSizeCurrent = 0;
	g_sys_para.firmUpdate = false;
    
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id",  10);
    cJSON_AddNumberToObject(pJsonRoot, "Sid", 0);
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    g_sys_para.bleLedStatus = BLE_UPDATE;
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ11����Ϣ, ����ϢΪ��ȡ���⴫�����¶�
  * @input   
  * @return  
***************************************************************************************/
static char * ParseGetObjTemp(void)
{
    //�������ģ���Լ�
    g_sys_para.objTemp = MXL_ReadObjTemp();
    
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 11);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "Temp", g_sys_para.objTemp);
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}



/***************************************************************************************
  * @brief   ����json���ݰ�
  * @input   
  * @return  
***************************************************************************************/
uint8_t* ParseJson(char *pMsg)
{
    char *p_reply = NULL;
    
     cJSON *pJson = cJSON_Parse((char *)pMsg);
    if(NULL == pJson) {
        return NULL;
    }
    
    // get string from json
    cJSON * pSub = cJSON_GetObjectItem(pJson, "Id");
    if(NULL == pSub){
        return NULL;
    }
    
    switch(pSub->valueint)
    {
        case 1:
            p_reply = ParseSetTime(pJson, pSub);//��������
            break;
        case 2:
            p_reply = ParseGetTime();//��ȡ����
            break;
        case 3:
            p_reply = ParseChkSelf();//�����Լ�
            break;
        case 4:
            p_reply = ParseGetBatCapacity();//��ȡ����
            break;
        case 5:
            p_reply = ParseGetVersion();//��ȡ�汾��
            break;
        case 6:
            p_reply = ParseSetSysPara(pJson, pSub);//ϵͳ��������
            break;
        case 7:
            p_reply = ParseSetSamplePara(pJson, pSub);//�ɼ���������
            break;
        case 8:
            p_reply = ParseStartSample();//��ʼ����
            break;
        case 9:
            p_reply = ParseGetSampleData(pJson, pSub);//��ȡ�������
            break;
        case 10:
            p_reply = ParseStartUpdate(pJson, pSub);//����
            break;
        case 11:
            p_reply = ParseGetObjTemp();
            break;
    }
    
    cJSON_Delete(pJson);
    
    return (uint8_t *)p_reply;
}



/***************************************************************************************
  * @brief   �����̼���
  * @input   
  * @return  
***************************************************************************************/
uint8_t*  ParseFirmPacket(uint8_t *pMsg)
{
    uint16_t crc = 0;
	uint8_t  err_code = 0;
    
    crc = CRC16(pMsg, g_puart2RxCnt);//�Լ��������CRC16
	if(pMsg[132] != (crc>>8) || pMsg[133] != (uint8_t)crc){
		err_code = 1;
	}else{
        /* ��id */
        g_sys_para.firmPacksCount = pMsg[2];
        
        /* ���ݰ�id,������ð���Ҫ����ĵ�ַ*/
        g_sys_para.firmNextAddr = FIRM_DATA_ADDR + g_sys_para.firmPacksCount * FIRM_ONE_PACKE_LEN;
        
        /* ��ǰ���ܵ��İ����� */
        g_sys_para.firmSizeCurrent += pMsg[3];
        
        /* ����̼����ݵ�Nor Flash*/
        NorFlash_WriteApp(&pMsg[4], FIRM_ONE_PACKE_LEN);
    }
    
    /* ��ǰΪ���һ��,���������̼���crc16�� */
	if(pMsg[2] == g_sys_para.firmPacksTotal - 1 ){
        crc = CRC16((void *)FIRM_DATA_ADDR, g_sys_para.firmSizeTotal);
        if(crc != g_sys_para.firmCrc16){
            g_sys_para.firmUpdate = false;
            err_code = 2;
        }else{//����CRCУ��ͨ��,��ʼ�����豸����.
            g_sys_para.firmUpdate = true;
        }
	}
	
    /* �̼���,����ǹ̶��ظ�7Byte���� */
    g_puart2TxCnt = 7;
    memcpy(g_lpuart2TxBuf, pMsg, 3);
    g_lpuart2TxBuf[3] = g_puart2RxCnt - 6;//���ܵ�����Ч���ݸ���
    g_lpuart2TxBuf[4] = err_code;         //������
	crc = CRC16(g_lpuart2TxBuf, g_puart2TxCnt);
    g_lpuart2TxBuf[5] = crc>>8;           //CRC H
    g_lpuart2TxBuf[6] = (uint8_t)crc;     //CRC L
    g_sys_para.bleLedStatus = BLE_UPDATE;
    return g_lpuart2TxBuf;
}

/***************************************************************************************
  * @brief   ����Э����ں���
  * @input   
  * @return  
***************************************************************************************/
uint8_t* ParseProtocol(uint8_t *pMsg)
{
    if(NULL == pMsg){
        return NULL;
    }
    
    if(pMsg[0] == 0xE7 && pMsg[1] == 0xE7 ){//Ϊ�̼�������
        return ParseFirmPacket(pMsg);
    }else{//Ϊjson���ݰ�
        return ParseJson((char *)pMsg);
    }
}

