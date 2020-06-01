#include "main.h"

extern uint8_t s_nor_program_buffer[];
extern AdcInfoTotal adcInfoTotal;
extern AdcInfo adcInfo;
snvs_lp_srtc_datetime_t sampTime;
uint16_t ble_wait_time = 5;

/***************************************************************************************
  * @brief   ������ϢidΪ1����Ϣ, ����Ϣ���õ����RTCʱ��
  * @input
  * @return
***************************************************************************************/
static char* SetTime(cJSON *pJson, cJSON * pSub)
{
    /*������Ϣ����, ��ȡ���ں�ʱ��*/
    pSub = cJSON_GetObjectItem(pJson, "Y");
    if (NULL != pSub) {
        SNVS_LP_dateTimeStruct.year = pSub->valueint;
    }

    pSub = cJSON_GetObjectItem(pJson, "Mon");
    if (NULL != pSub) {
        SNVS_LP_dateTimeStruct.month = pSub->valueint+1;
    }

    pSub = cJSON_GetObjectItem(pJson, "D");
    if (NULL != pSub) {
        SNVS_LP_dateTimeStruct.day = pSub->valueint;
    }


    pSub = cJSON_GetObjectItem(pJson, "H");
    if (NULL != pSub) {
        SNVS_LP_dateTimeStruct.hour = pSub->valueint;
    }

    pSub = cJSON_GetObjectItem(pJson, "Min");
    if (NULL != pSub) {
        SNVS_LP_dateTimeStruct.minute = pSub->valueint;
    }

    pSub = cJSON_GetObjectItem(pJson, "S");
    if (NULL != pSub) {
        SNVS_LP_dateTimeStruct.second = pSub->valueint;
    }

    /*�������ں�ʱ��*/
    SNVS_LP_SRTC_SetDatetime(SNVS_LP_PERIPHERAL, &SNVS_LP_dateTimeStruct);

    /*����cjson��ʽ�Ļظ���Ϣ*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) return NULL;
    cJSON_AddNumberToObject(pJsonRoot, "Id", 1);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ2����Ϣ, ����Ϣ��ȡ�����RTCʱ��
  * @input
  * @return
***************************************************************************************/
static char * GetTime(void)
{
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 2);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "Y", SNVS_LP_dateTimeStruct.year);
    cJSON_AddNumberToObject(pJsonRoot, "Mon", SNVS_LP_dateTimeStruct.month);
    cJSON_AddNumberToObject(pJsonRoot, "D", SNVS_LP_dateTimeStruct.day);
    cJSON_AddNumberToObject(pJsonRoot, "H", SNVS_LP_dateTimeStruct.hour);
    cJSON_AddNumberToObject(pJsonRoot, "Min", SNVS_LP_dateTimeStruct.minute);
    cJSON_AddNumberToObject(pJsonRoot, "S", SNVS_LP_dateTimeStruct.second);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ3����Ϣ, ����ϢΪ�����Լ�
  * @input
  * @return
***************************************************************************************/
static char * CheckSelf(void)
{
    /*����cjson��ʽ�Ļظ���Ϣ*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }

    //ָʾ���Լ�
    LED_CheckSelf();

    //�������ģ���Լ�
    g_sys_para.objTemp = MXL_ReadObjTemp();

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
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ4����Ϣ, ����ϢΪ��ȡ����
  * @input
  * @return
***************************************************************************************/
static char * GetBatCapacity(void)
{
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 4);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "BatC", (int)g_sys_para.batRemainPercent);//��ص���
    cJSON_AddNumberToObject(pJsonRoot, "PwrV", g_sys_para.batVoltage);//��ص�ѹֵ
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ5����Ϣ, ����ϢΪ��ȡ�汾��
  * @input
  * @return
***************************************************************************************/
static char * GetVersion(void)
{
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 5);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddStringToObject(pJsonRoot, "HV", HARD_VERSION);//Ӳ���汾��
    cJSON_AddStringToObject(pJsonRoot, "SV", SOFT_VERSION);//����汾��

    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ6����Ϣ, ����ϢΪϵͳ��������
  * @input
  * @return
***************************************************************************************/
static char * SetSysPara(cJSON *pJson, cJSON * pSub)
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
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 6);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ7����Ϣ, ����ϢΪ�ɼ���������
  * @input
  * @return
***************************************************************************************/
static char * SetSamplePara(cJSON *pJson, cJSON * pSub)
{
    uint8_t sid = 0;
    /*������Ϣ����,*/
    pSub = cJSON_GetObjectItem(pJson, "SId");
    sid = pSub->valueint;
    switch(pSub->valueint)
    {
    case 0:
        pSub = cJSON_GetObjectItem(pJson, "IP");
        if (NULL != pSub) {
            memset(g_adc_set.IDPath, 0, sizeof(g_adc_set.IDPath));
            strcpy(g_adc_set.IDPath, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pJson, "NP");
        if (NULL != pSub) {
            memset(g_adc_set.NamePath, 0, sizeof(g_adc_set.NamePath));
            strcpy(g_adc_set.NamePath, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pJson, "D");
        if (NULL != pSub) {
            ble_wait_time = pSub->valueint;
        }
        break;
    case 1:
        pSub = cJSON_GetObjectItem(pJson, "SU");
        if (NULL != pSub) {
            memset(g_adc_set.SpeedUnits, 0, sizeof(g_adc_set.SpeedUnits));
            strcpy(g_adc_set.SpeedUnits, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pJson, "PU");
        if (NULL != pSub) {
            memset(g_adc_set.ProcessUnits, 0, sizeof(g_adc_set.ProcessUnits));
            strcpy(g_adc_set.ProcessUnits, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pJson, "D");
        if (NULL != pSub) {
            g_adc_set.DetectionType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "S");
        if (NULL != pSub) {
            g_adc_set.Senstivity = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pJson, "ZD");
        if (NULL != pSub) {
            g_adc_set.Zerodrift = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pJson, "ET");
        if (NULL != pSub) {
            g_adc_set.EUType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "EU");
        if (NULL != pSub) {
            memset(g_adc_set.EU, 0, sizeof(g_adc_set.EU));
            strcpy(g_adc_set.EU, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pJson, "W");
        if (NULL != pSub) {
            g_adc_set.WindowsType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "SF");
        if (NULL != pSub) {
            g_adc_set.StartFrequency = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "EF");
        if (NULL != pSub) {
            g_adc_set.EndFrequency = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "SR");
        if (NULL != pSub) {
            g_adc_set.SampleRate = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "L");
        if (NULL != pSub) {
            g_adc_set.Lines = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "B");
        if (NULL != pSub) {
            g_sys_para.bias = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pJson, "RV");
        if (NULL != pSub) {
            g_sys_para.refV = pSub->valuedouble;
        }
        //�����������
        g_sys_para.sampNumber = 2.56 * g_adc_set.Lines * g_adc_set.Averages * (1 - g_adc_set.AverageOverlap)
                                + 2.56 * g_adc_set.Lines * g_adc_set.AverageOverlap;
        break;
    case 2:
        pSub = cJSON_GetObjectItem(pJson, "A");
        if (NULL != pSub) {
            g_adc_set.Averages = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "OL");
        if (NULL != pSub) {
            g_adc_set.AverageOverlap = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pJson, "AT");
        if (NULL != pSub) {
            g_adc_set.AverageType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "EFL");
        if (NULL != pSub) {
            g_adc_set.EnvFilterLow = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "EFH");
        if (NULL != pSub) {
            g_adc_set.EnvFilterHigh = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "IM");
        if (NULL != pSub) {
            g_adc_set.IncludeMeasurements = pSub->valueint;
        }
        //�����������
        g_sys_para.sampNumber = 2.56 * g_adc_set.Lines * g_adc_set.Averages * (1 - g_adc_set.AverageOverlap)
                                + 2.56 * g_adc_set.Lines * g_adc_set.AverageOverlap;
        break;
    default:
        break;
    }

    /*����cjson��ʽ�Ļظ���Ϣ*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 7);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",sid);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ8����Ϣ, ����ϢΪ��ʼ�����ź�
  * @input
  * @return
***************************************************************************************/
static char * StartSample(cJSON *pJson, cJSON * pSub)
{
    pSub = cJSON_GetObjectItem(pJson, "SampleRate");
    if (NULL != pSub) {
        g_adc_set.SampleRate = pSub->valueint;
    }


    cJSON *pJsonReply = cJSON_CreateObject();
    if(NULL == pJsonReply) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonReply, "Id",  8);
    cJSON_AddNumberToObject(pJsonReply, "Sid", 0);
    char *sendBuf = cJSON_PrintUnformatted(pJsonReply);
    cJSON_Delete(pJsonReply);
    LPUART2_SendString((char *)sendBuf);
    free(sendBuf);
    sendBuf = NULL;
    sampTime = SNVS_LP_dateTimeStruct;
    /*start sample*/
    ADC_SampleStart();

    /*wait task notify*/
    ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

    if(g_sys_para.sampNumber != 0) { //Android�������жϲɼ�����
        cJSON *pJsonRoot = cJSON_CreateObject();
        if(NULL == pJsonRoot) {
            return NULL;
        }
        cJSON_AddNumberToObject(pJsonRoot, "Id",  8);
        cJSON_AddNumberToObject(pJsonRoot, "Sid", 1);
        cJSON_AddStringToObject(pJsonRoot, "F", adcInfo.AdcDataTime);
        cJSON_AddNumberToObject(pJsonRoot, "Kb", adcInfoTotal.freeOfKb);
		cJSON_AddNumberToObject(pJsonRoot,"PK",  g_sys_para.sampPacks);
        cJSON_AddNumberToObject(pJsonRoot, "V", g_sys_para.shkCount);
        cJSON_AddNumberToObject(pJsonRoot, "S", g_sys_para.spdCount);
        sendBuf = cJSON_PrintUnformatted(pJsonRoot);
        cJSON_Delete(pJsonRoot);
    }

    return sendBuf;
}



/***************************************************************************************
  * @brief   ������ϢidΪ9����Ϣ, ����ϢΪ��ȡ��������
  * @input
  * @return
***************************************************************************************/
char * GetSampleData(cJSON *pJson, cJSON * pSub)
{
    uint32_t sid = 0;
    uint32_t index = 0;
    uint32_t flag_get_all_data = 0;
    g_sys_para.sampPacksCnt = 0;
    cJSON *pJsonRoot = NULL;
    char *p_reply = NULL;
    /*������Ϣ����,�������Ҫ�ظ�������*/
    pSub = cJSON_GetObjectItem(pJson, "Sid");
    sid = pSub->valueint;
    if(sid == g_sys_para.sampPacks) {
        flag_get_all_data = 1;
    }

SEND_DATA:
    if(flag_get_all_data) {
        sid = g_sys_para.sampPacksCnt;
    }
    pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 9);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",sid);

    switch(sid)	{
    case 0:
        cJSON_AddNumberToObject(pJsonRoot, "D", ble_wait_time);
        cJSON_AddStringToObject(pJsonRoot, "DP", g_adc_set.IDPath);//Ӳ���汾��
        cJSON_AddStringToObject(pJsonRoot, "NP", g_adc_set.NamePath);//Ӳ���汾��
        p_reply = cJSON_PrintUnformatted(pJsonRoot);
        break;
    case 1:
        cJSON_AddStringToObject(pJsonRoot, "SU", g_adc_set.SpeedUnits);
        cJSON_AddStringToObject(pJsonRoot, "PU", g_adc_set.ProcessUnits);
        cJSON_AddNumberToObject(pJsonRoot, "D", g_adc_set.DetectionType);
        cJSON_AddNumberToObject(pJsonRoot, "S", g_adc_set.Senstivity);
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
        p_reply = cJSON_PrintUnformatted(pJsonRoot);
        break;
    case 2:
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
        p_reply = cJSON_PrintUnformatted(pJsonRoot);
        break;
    default:
        p_reply = malloc(250);
        memset(p_reply, 0, 250);
        char *tempJson = cJSON_PrintUnformatted(pJsonRoot);
        memset(p_reply, 0, LPUART2_BUFF_LEN);
        strcat(p_reply, tempJson);
        p_reply[strlen(p_reply) - 1] = 0x00;//��ȥ��jsong��ʽ�����"}"
        if(sid-3 < g_sys_para.shkPacks)
        {
            strcat(p_reply,",V,");
            index = sid - 3;
            //ÿ������ռ��4��byte;ÿ�������ϴ�40������. 40*4=160
            memcpy(p_reply+strlen(p_reply), VibrateStrADC+index*160, 160);
            strcat(p_reply,"}");
        }
        else if(sid - 3 - g_sys_para.shkPacks < g_sys_para.spdCount)
        {
            strcat((char *)g_lpuart2TxBuf,",S,");
            index = sid - 3 - g_sys_para.shkPacks;
            //ÿ������ռ��4��byte;ÿ�������ϴ�40������. 40*4=160
            memcpy(p_reply+strlen(p_reply), SpeedStrADC+index*160, 160);
            strcat(p_reply, "}");
        }
        free(tempJson);
        tempJson = NULL;
        break;
    }

    cJSON_Delete(pJsonRoot);

    if(flag_get_all_data ) {
        LPUART2_SendString((char *)p_reply);
        PRINTF("%s\r\n",(char *)p_reply);
        vTaskDelay(ble_wait_time);
        free(p_reply);
        p_reply = NULL;
        g_sys_para.sampPacksCnt++;
        if(g_sys_para.sampPacksCnt < g_sys_para.sampPacks) {
            goto SEND_DATA;
        }
    }

    return p_reply;
}


/***************************************************************************************
  * @brief   ������ϢidΪ10����Ϣ, ����ϢΪ��ʼ���������̼���
  * @input
  * @return
***************************************************************************************/
static char * StartUpgrade(cJSON *pJson, cJSON * pSub)
{
    /* ��ʼ�����̼���, ��ʼ��һЩ��Ҫ�ı���*/
    g_sys_para.firmUpdate = false;
    g_sys_para.firmPacksCount = 0;
    g_sys_para.firmSizeCurrent = 0;
    g_sys_para.firmCurrentAddr = APP_START_SECTOR * SECTOR_SIZE;;

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

    /* �����ļ���С������Ӧ��С�Ŀռ� */
    for(int i = 0; i<= g_sys_para.firmSizeTotal/SECTOR_SIZE; i++) {
        FlexSPI_NorFlash_Erase_Sector(FLEXSPI, (APP_START_SECTOR + i)*SECTOR_SIZE);
    }

    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id",  10);
    cJSON_AddNumberToObject(pJsonRoot, "Sid", 0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    g_sys_para.bleLedStatus = BLE_UPDATE;
    g_puart2StartRx = true;//��ʼ���м��,5s��δ���ܵ�������ʱ
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ11����Ϣ, ����ϢΪ��ȡ���⴫�����¶�
  * @input
  * @return
***************************************************************************************/
static char * GetObjTemp(void)
{
    //�������ģ���Լ�
    g_sys_para.objTemp = MXL_ReadObjTemp();
    g_sys_para.envTemp = MXL_ReadEnvTemp();
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 11);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "Temp", g_sys_para.objTemp);
//	cJSON_AddNumberToObject(pJsonRoot, "env", g_sys_para.envTemp);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ12����Ϣ, ����ϢΪ��ֹ����
  * @input
  * @return
***************************************************************************************/
static char* StopSample(void)
{
    //�����ʱ���ڲɼ�����, �ô���ᴥ���ɼ�����ź�
    g_sys_para.sampNumber = 0;
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 12);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}
/***************************************************************************************
  * @brief   ������ϢidΪ13����Ϣ, ����ϢΪ��ȡmanage�ļ�����
  * @input
  * @return
***************************************************************************************/
static char* GetManageInfo(cJSON *pJson, cJSON * pSub)
{
    char   *fileStr;
    int sid = 0, num = 0, si = 0, len = 0;

    /*������Ϣ����,*/
    pSub = cJSON_GetObjectItem(pJson, "S");
    if(pSub != NULL) {
        si = pSub->valueint;
    } else {
        si = 0;
    }

    pSub = cJSON_GetObjectItem(pJson, "N");
    if(pSub != NULL) {
        num = pSub->valueint;
    } else {
        num = 1;
    }
    if(num > 10) num = 10;
    if(num < 1) num = 1;

    //�����ڴ����ڱ����ļ�����
    len = num * 13 + 1;
    fileStr = malloc(len);
    memset(fileStr, 0U, len);

    NorFlash_ReadAdcInfo(si, num, fileStr);

    /*����cjson��ʽ�Ļظ���Ϣ*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 13);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",sid);
    cJSON_AddStringToObject(pJsonRoot, "Name", fileStr);
    cJSON_AddNumberToObject(pJsonRoot, "T",adcInfoTotal.totalAdcInfo);
    cJSON_AddNumberToObject(pJsonRoot, "Kb",adcInfoTotal.freeOfKb);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    free(fileStr);
    fileStr = NULL;
    return p_reply;
}


/***************************************************************************************
  * @brief   ������ϢidΪ14����Ϣ, ����ϢΪͨ���ļ�����ȡ�ɼ�����
  * @input
  * @return
***************************************************************************************/
static char* GetSampleDataInFlash(cJSON *pJson, cJSON * pSub)
{
    extern AdcInfo adcInfo;
    uint32_t addrOfAdcData;
    char fileName[15] = {0};

    /*������Ϣ����,*/
    pSub = cJSON_GetObjectItem(pJson, "fileName");
    if(pSub != NULL && strlen(pSub->valuestring) == 12) {
        strcpy(fileName,pSub->valuestring);
    }

    /*��flash��ȡ�ļ�*/
    addrOfAdcData = NorFlash_ReadAdcData(fileName);

    if(addrOfAdcData != NULL) {
        char* sampJson = malloc(adcInfo.AdcDataLen);
        memcpy(sampJson, NORFLASH_AHB_POINTER(addrOfAdcData), adcInfo.AdcDataLen);

        /*�����ݰ���json��ʽ���н���*/
        cJSON *pFileJson = cJSON_Parse(sampJson);
        if(NULL == pFileJson) {
            return NULL;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "DP");
        if (NULL != pSub) {
            memset(g_adc_set.IDPath, 0, sizeof(g_adc_set.IDPath));
            strcpy(g_adc_set.IDPath, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pFileJson, "NP");
        if (NULL != pSub) {
            memset(g_adc_set.NamePath, 0, sizeof(g_adc_set.NamePath));
            strcpy(g_adc_set.NamePath, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pFileJson, "SU");
        if (NULL != pSub) {
            memset(g_adc_set.SpeedUnits, 0, sizeof(g_adc_set.SpeedUnits));
            strcpy(g_adc_set.SpeedUnits, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pFileJson, "PU");
        if (NULL != pSub) {
            memset(g_adc_set.ProcessUnits, 0, sizeof(g_adc_set.ProcessUnits));
            strcpy(g_adc_set.ProcessUnits, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pFileJson, "DT");
        if (NULL != pSub) {
            g_adc_set.DetectionType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "SEN");
        if (NULL != pSub) {
            g_adc_set.Senstivity = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "ZD");
        if (NULL != pSub) {
            g_adc_set.Zerodrift = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "ET");
        if (NULL != pSub) {
            g_adc_set.EUType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "EU");
        if (NULL != pSub) {
            memset(g_adc_set.EU, 0, sizeof(g_adc_set.EU));
            strcpy(g_adc_set.EU, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pFileJson, "W");
        if (NULL != pSub) {
            g_adc_set.WindowsType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "SF");
        if (NULL != pSub) {
            g_adc_set.StartFrequency = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "EF");
        if (NULL != pSub) {
            g_adc_set.EndFrequency = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "SR");
        if (NULL != pSub) {
            g_adc_set.SampleRate = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "L");
        if (NULL != pSub) {
            g_adc_set.Lines = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "B");
        if (NULL != pSub) {
            g_sys_para.bias = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "RV");
        if (NULL != pSub) {
            g_sys_para.refV = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "A");
        if (NULL != pSub) {
            g_adc_set.Averages = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "OL");
        if (NULL != pSub) {
            g_adc_set.AverageOverlap = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "AT");
        if (NULL != pSub) {
            g_adc_set.AverageType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "EFL");
        if (NULL != pSub) {
            g_adc_set.EnvFilterLow = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "EFH");
        if (NULL != pSub) {
            g_adc_set.EnvFilterHigh = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "IM");
        if (NULL != pSub) {
            g_adc_set.IncludeMeasurements = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "SP");
        if (NULL != pSub) {
            g_adc_set.Speed = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "P");
        if (NULL != pSub) {
            g_adc_set.Process = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "PL");
        if (NULL != pSub) {
            g_adc_set.ProcessMin = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "PH");
        if (NULL != pSub) {
            g_adc_set.ProcessMax = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "PK");
        if (NULL != pSub) {
            g_sys_para.sampPacks = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "Y");
        if (NULL != pSub) {
            sampTime.year = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "M");
        if (NULL != pSub) {
            sampTime.month = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "D");
        if (NULL != pSub) {
            sampTime.day = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "H");
        if (NULL != pSub) {
            sampTime.hour = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "Min");
        if (NULL != pSub) {
            sampTime.minute = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "S");
        if (NULL != pSub) {
            sampTime.second = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "spdCnt");
        if (NULL != pSub) {
            g_sys_para.spdCount = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "vibCnt");
        if (NULL != pSub) {
            g_sys_para.shkCount = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pFileJson, "Vibrate");
        memset(VibrateStrADC, 0, ADC_LEN * 4 + 1);
        if (NULL != pSub) {
            strcpy(VibrateStrADC, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pFileJson, "Speed");
        memset(SpeedStrADC, 0, ADC_LEN * 4 + 1);
        if (NULL != pSub) {
            strcpy(SpeedStrADC, pSub->valuestring);
        }
        free(sampJson);
        sampJson = NULL;
        cJSON_Delete(pFileJson);
    }

    /*����cjson��ʽ�Ļظ���Ϣ*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) return NULL;
    //���㷢�����ź���Ҫ���ٸ���
#ifdef BLE_VERSION
    g_sys_para.shkPacks = (g_sys_para.shkCount / 40) +  (g_sys_para.shkCount%40?1:0);
    g_sys_para.spdPacks = (g_sys_para.spdCount / 40) +  (g_sys_para.spdCount%40?1:0);
	//���㽫һ�βɼ�����ȫ�����͵�Android��Ҫ���ٸ���
    g_sys_para.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 3;
#elif defined WIFI_VERSION
	g_sys_para.shkPacks = (g_sys_para.shkCount / 250) +  (g_sys_para.shkCount%250?1:0);
    g_sys_para.spdPacks = (g_sys_para.spdCount / 250) +  (g_sys_para.spdCount%250?1:0);
	//���㽫һ�βɼ�����ȫ�����͵�Android��Ҫ���ٸ���
    g_sys_para.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 1;
#endif
    
    cJSON_AddNumberToObject(pJsonRoot, "Id", 14);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "PK",g_sys_para.sampPacks);
    cJSON_AddNumberToObject(pJsonRoot, "V", g_sys_para.shkCount);
    cJSON_AddNumberToObject(pJsonRoot, "S", g_sys_para.spdCount);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);

    return p_reply;
}


/***************************************************************************************
  * @brief   ������ϢidΪ15����Ϣ, ����ϢΪ����flash�б�������в�������
  * @input
  * @return
***************************************************************************************/
static char *EraseAdcDataInFlash(void)
{
    for(int i = ADC_INFO_SECTOR; i<ADC_DATA_SECTOR; i++) {
        FlexSPI_NorFlash_Erase_Sector(FLEXSPI, i*SECTOR_SIZE);
    }

    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 15);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddBoolToObject(pJsonRoot, "Status",true);
    uint32_t free = (FLASH_SIZE_BYTE - ADC_DATA_SECTOR*SECTOR_SIZE)/1024;
    cJSON_AddNumberToObject(pJsonRoot, "Kb", free);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}



/***************************************************************************************
  * @brief   ������ϢidΪ16����Ϣ, ����ϢΪ���õ�ص���
  * @input
  * @return
***************************************************************************************/
static char *SetBatCapacity(cJSON *pJson, cJSON * pSub)
{
    uint8_t batC = 0;
    uint8_t batM = 0;
    pSub = cJSON_GetObjectItem(pJson, "BatC");
    if (NULL != pSub) {
        batC = pSub->valueint;
        if( batC != 0) {
            if(batC > 100) batC = 100;
            LTC2942_SetAC(batC / 100 * 0xFFFF);
        }
    }

    pSub = cJSON_GetObjectItem(pJson, "BatM");
    if (NULL != pSub) {
        batM = pSub->valueint;
        if(batM != 0) {
            if(batM == 1) {
                LTC2942_SetPrescaler(LTC2942_PSCM_1);
            } else if(batM == 2) {
                LTC2942_SetPrescaler(LTC2942_PSCM_2);
            } else if(batM == 4) {
                LTC2942_SetPrescaler(LTC2942_PSCM_4);
            } else if(batM == 8) {
                LTC2942_SetPrescaler(LTC2942_PSCM_8);
            } else if(batM == 16) {
                LTC2942_SetPrescaler(LTC2942_PSCM_16);
            } else if(batM == 32) {
                LTC2942_SetPrescaler(LTC2942_PSCM_32);
            } else if(batM == 64) {
                LTC2942_SetPrescaler(LTC2942_PSCM_64);
            } else if(batM == 128) {
                LTC2942_SetPrescaler(LTC2942_PSCM_128);
            }
        }
    }

    /*����cjson��ʽ�Ļظ���Ϣ*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) return NULL;
    cJSON_AddNumberToObject(pJsonRoot, "Id", 16);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}



/***************************************************************************************
  * @brief   ������ϢidΪ17����Ϣ, ����ϢΪͨ��wifi���òɼ�����
  * @input
  * @return
***************************************************************************************/
static char * SetSampleParaByWifi(cJSON *pJson, cJSON * pSub)
{
    pSub = cJSON_GetObjectItem(pJson, "IP");
    if (NULL != pSub) {
        memset(g_adc_set.IDPath, 0, sizeof(g_adc_set.IDPath));
        strcpy(g_adc_set.IDPath, pSub->valuestring);
    }
    pSub = cJSON_GetObjectItem(pJson, "NP");
    if (NULL != pSub) {
        memset(g_adc_set.NamePath, 0, sizeof(g_adc_set.NamePath));
        strcpy(g_adc_set.NamePath, pSub->valuestring);
    }
    pSub = cJSON_GetObjectItem(pJson, "D");
    if (NULL != pSub) {
        ble_wait_time = pSub->valueint;
    }

    pSub = cJSON_GetObjectItem(pJson, "SU");
    if (NULL != pSub) {
        memset(g_adc_set.SpeedUnits, 0, sizeof(g_adc_set.SpeedUnits));
        strcpy(g_adc_set.SpeedUnits, pSub->valuestring);
    }
    pSub = cJSON_GetObjectItem(pJson, "PU");
    if (NULL != pSub) {
        memset(g_adc_set.ProcessUnits, 0, sizeof(g_adc_set.ProcessUnits));
        strcpy(g_adc_set.ProcessUnits, pSub->valuestring);
    }
    pSub = cJSON_GetObjectItem(pJson, "D");
    if (NULL != pSub) {
        g_adc_set.DetectionType = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "S");
    if (NULL != pSub) {
        g_adc_set.Senstivity = pSub->valuedouble;
    }
    pSub = cJSON_GetObjectItem(pJson, "ZD");
    if (NULL != pSub) {
        g_adc_set.Zerodrift = pSub->valuedouble;
    }
    pSub = cJSON_GetObjectItem(pJson, "ET");
    if (NULL != pSub) {
        g_adc_set.EUType = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "EU");
    if (NULL != pSub) {
        memset(g_adc_set.EU, 0, sizeof(g_adc_set.EU));
        strcpy(g_adc_set.EU, pSub->valuestring);
    }
    pSub = cJSON_GetObjectItem(pJson, "W");
    if (NULL != pSub) {
        g_adc_set.WindowsType = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "SF");
    if (NULL != pSub) {
        g_adc_set.StartFrequency = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "EF");
    if (NULL != pSub) {
        g_adc_set.EndFrequency = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "SR");
    if (NULL != pSub) {
        g_adc_set.SampleRate = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "L");
    if (NULL != pSub) {
        g_adc_set.Lines = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "B");
    if (NULL != pSub) {
        g_sys_para.bias = pSub->valuedouble;
    }
    pSub = cJSON_GetObjectItem(pJson, "RV");
    if (NULL != pSub) {
        g_sys_para.refV = pSub->valuedouble;
    }

    pSub = cJSON_GetObjectItem(pJson, "A");
    if (NULL != pSub) {
        g_adc_set.Averages = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "OL");
    if (NULL != pSub) {
        g_adc_set.AverageOverlap = pSub->valuedouble;
    }
    pSub = cJSON_GetObjectItem(pJson, "AT");
    if (NULL != pSub) {
        g_adc_set.AverageType = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "EFL");
    if (NULL != pSub) {
        g_adc_set.EnvFilterLow = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "EFH");
    if (NULL != pSub) {
        g_adc_set.EnvFilterHigh = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "IM");
    if (NULL != pSub) {
        g_adc_set.IncludeMeasurements = pSub->valueint;
    }
    //�����������
    g_sys_para.sampNumber = 2.56 * g_adc_set.Lines * g_adc_set.Averages * (1 - g_adc_set.AverageOverlap)
                            + 2.56 * g_adc_set.Lines * g_adc_set.AverageOverlap;

    /*����cjson��ʽ�Ļظ���Ϣ*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 17);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   ������ϢidΪ18����Ϣ, ����ϢΪͨ��wifi��ȡ��������
  * @input
  * @return
***************************************************************************************/
char * GetSampleDataByWifi(cJSON *pJson, cJSON * pSub)
{
    uint32_t sid = 0;
    uint32_t index = 0;
    uint32_t flag_get_all_data = 0;
    g_sys_para.sampPacksCnt = 0;
    cJSON *pJsonRoot = NULL;
    char *p_reply = NULL;
    /*������Ϣ����,�������Ҫ�ظ�������*/
    pSub = cJSON_GetObjectItem(pJson, "Sid");
    sid = pSub->valueint;
    if(sid == g_sys_para.sampPacks) {
        flag_get_all_data = 1;
    }

SEND_DATA:
    if(flag_get_all_data) {
        sid = g_sys_para.sampPacksCnt;
    }
    pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 18);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",sid);

    switch(sid)	{
    case 0:
        cJSON_AddNumberToObject(pJsonRoot, "D", ble_wait_time);
        cJSON_AddStringToObject(pJsonRoot, "DP", g_adc_set.IDPath);//Ӳ���汾��
        cJSON_AddStringToObject(pJsonRoot, "NP", g_adc_set.NamePath);//Ӳ���汾��
        cJSON_AddStringToObject(pJsonRoot, "SU", g_adc_set.SpeedUnits);
        cJSON_AddStringToObject(pJsonRoot, "PU", g_adc_set.ProcessUnits);
        cJSON_AddNumberToObject(pJsonRoot, "D", g_adc_set.DetectionType);
        cJSON_AddNumberToObject(pJsonRoot, "S", g_adc_set.Senstivity);
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
        p_reply = cJSON_PrintUnformatted(pJsonRoot);
        break;
    default:
        p_reply = malloc(1024);
        memset(p_reply, 0, 1024);
        char *tempJson = cJSON_PrintUnformatted(pJsonRoot);
        strcat(p_reply, tempJson);
        p_reply[strlen(p_reply) - 1] = 0x00;//��ȥ��jsong��ʽ�����"}"
        if(sid-1 < g_sys_para.shkPacks)
        {
            strcat(p_reply,",V,");
            index = sid - 1;
            //ÿ������ռ��4��byte;ÿ�������ϴ�250������. 250*4=1000
            memcpy(p_reply+strlen(p_reply), VibrateStrADC+index*1000, 1000);
            strcat(p_reply,"}");
        }
        else if(sid - 1 - g_sys_para.shkPacks < g_sys_para.spdCount)
        {
            strcat((char *)g_lpuart2TxBuf,",S,");
            index = sid - 1 - g_sys_para.shkPacks;
            //ÿ������ռ��4��byte;ÿ�������ϴ�250������. 250*4=1000
            memcpy(p_reply+strlen(p_reply), SpeedStrADC+index*1000, 1000);
            strcat(p_reply, "}");
        }
        free(tempJson);
        tempJson = NULL;
        break;
    }

    cJSON_Delete(pJsonRoot);

    if(flag_get_all_data ) {
        LPUART3_SendString((char *)p_reply);
        vTaskDelay(6);
        free(p_reply);
        p_reply = NULL;
        g_sys_para.sampPacksCnt++;
        if(g_sys_para.sampPacksCnt < g_sys_para.sampPacks) {
            goto SEND_DATA;
        }
    }

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
    if(NULL == pSub) {
        return NULL;
    }

    switch(pSub->valueint)
    {
    case 1:
        p_reply = SetTime(pJson, pSub);//��������
        break;
    case 2:
        p_reply = GetTime();//��ȡ����
        break;
    case 3:
        p_reply = CheckSelf();//�����Լ�
        break;
    case 4:
        p_reply = GetBatCapacity();//��ȡ����
        break;
    case 5:
        p_reply = GetVersion();//��ȡ�汾��
        break;
    case 6:
        p_reply = SetSysPara(pJson, pSub);//ϵͳ��������
        break;
    case 7:
        p_reply = SetSamplePara(pJson, pSub);//�ɼ���������
        break;
    case 8:
        p_reply = StartSample(pJson, pSub);//��ʼ����
        break;
    case 9:
        p_reply = GetSampleData(pJson, pSub);//��ȡ�������
        break;
    case 10:
        p_reply = StartUpgrade(pJson, pSub);//����
        break;
    case 11:
        p_reply = GetObjTemp();
        break;
    case 12:
        p_reply = StopSample();
        break;
    case 13:
        p_reply = GetManageInfo(pJson, pSub);
        break;
    case 14:
        p_reply = GetSampleDataInFlash(pJson, pSub);
        break;
    case 15:
        p_reply = EraseAdcDataInFlash();
        break;
    case 16:
        p_reply = SetBatCapacity(pJson, pSub);
        break;
	case 17:
		p_reply = SetSampleParaByWifi(pJson, pSub);
		break;
	case 18:
		p_reply = GetSampleDataByWifi(pJson, pSub);
		break;
	case 19:
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

    crc = CRC16(pMsg+4, FIRM_ONE_LEN);//�Լ��������CRC16
    if(pMsg[FIRM_ONE_PACKE_LEN-2] != (uint8_t)crc || pMsg[FIRM_ONE_PACKE_LEN-1] != (crc>>8)) {
        err_code = 1;
    } else {
        /* ��id */
        g_sys_para.firmPacksCount = pMsg[2] | (pMsg[3]<<8);

        g_sys_para.firmCurrentAddr = APP_START_SECTOR * SECTOR_SIZE + g_sys_para.firmPacksCount * FIRM_ONE_LEN;//
//        PRINTF("\nADDR = 0x%x\n",g_sys_para.firmCurrentAddr);
        FlexSPI_FlashWrite(pMsg+4, g_sys_para.firmCurrentAddr, FIRM_ONE_LEN);
    }

    /* ��ǰΪ���һ��,���������̼���crc16�� */
    if(g_sys_para.firmPacksCount == g_sys_para.firmPacksTotal - 1) {

        g_sys_para.bleLedStatus = BLE_CONNECT;
        g_puart2RxTimeCnt = 0;
        g_puart2StartRx = false;

        /* ʹ�������λ������ AHB ������. */
        FLEXSPI_SoftwareReset(FLEXSPI);

//		PRINTF("�����ļ�:\r\n");
//
//		for(uint32_t i = 0;i<g_sys_para.firmSizeTotal; i++){
//			if(i%16 == 0) PRINTF("\n");
//			PRINTF("%02X ",*(uint8_t *)(FlexSPI_AMBA_BASE + APP_START_SECTOR * SECTOR_SIZE+i));
//		}

        crc = CRC16((uint8_t *)(FlexSPI_AMBA_BASE + APP_START_SECTOR * SECTOR_SIZE), g_sys_para.firmSizeTotal);
        PRINTF("\nCRC=%d",crc);
        if(crc != g_sys_para.firmCrc16) {
            g_sys_para.firmUpdate = false;
            err_code = 2;
        } else {
            PRINTF("\n����CRCУ��ͨ��,��ʼ�����豸\n");
            g_sys_para.firmUpdate = true;
        }
    }

    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
#ifdef BLE_VERSION
    cJSON_AddNumberToObject(pJsonRoot, "Id", 10);
#elif defined WIFI_VERSION
	cJSON_AddNumberToObject(pJsonRoot, "Id", 19);
#endif
    cJSON_AddNumberToObject(pJsonRoot, "Sid",1);
    cJSON_AddNumberToObject(pJsonRoot, "P", g_sys_para.firmPacksCount);
    cJSON_AddNumberToObject(pJsonRoot, "E", err_code);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    g_sys_para.firmPacksCount++;
    return (uint8_t*)p_reply;
}


/***************************************************************************************
  * @brief   ����Э����ں���
  * @input
  * @return
***************************************************************************************/
uint8_t* ParseProtocol(uint8_t *pMsg)
{
    if(NULL == pMsg) {
        return NULL;
    }

    if(pMsg[0] == 0xE7 && pMsg[1] == 0xE7 ) { //Ϊ�̼�������
        return ParseFirmPacket(pMsg);
    } else { //Ϊjson���ݰ�
        return ParseJson((char *)pMsg);
    }
}

