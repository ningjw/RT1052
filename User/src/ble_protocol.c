#include "main.h"

extern float ShakeADC[];
extern float SpeedADC[];
extern char  SpeedStrADC[];
extern char  VibrateStrADC[];
extern const TCHAR driverNumberBuffer[];
extern BYTE g_data_read[FF_MAX_SS];
snvs_hp_rtc_datetime_t sampTime;
uint16_t ble_wait_time = 60;
uint8_t appBuf[8192];
uint16_t i_appBuf = 0;
/***************************************************************************************
  * @brief   处理消息id为1的消息, 该消息设置点检仪RTC时间
  * @input
  * @return
***************************************************************************************/
static char* SetTime(cJSON *pJson, cJSON * pSub)
{
    /*解析消息内容, 获取日期和时间*/
    char isAm = 0;
    char is24Hour = 0;
    pSub = cJSON_GetObjectItem(pJson, "Y");
    if (NULL != pSub){
        rtcDate.year = pSub->valueint;
		SNVS_LP_dateTimeStruct.year = pSub->valueint;
	}

    pSub = cJSON_GetObjectItem(pJson, "Mon");
    if (NULL != pSub){
        rtcDate.month = pSub->valueint+1;
		SNVS_LP_dateTimeStruct.month = pSub->valueint+1;
	}

    pSub = cJSON_GetObjectItem(pJson, "D");
    if (NULL != pSub){
        rtcDate.day = pSub->valueint;
		SNVS_LP_dateTimeStruct.day = pSub->valueint;
	}

    pSub = cJSON_GetObjectItem(pJson, "is24Hour");
    if (NULL != pSub){
        is24Hour = pSub->valueint;
	}

    pSub = cJSON_GetObjectItem(pJson, "isAm");
    if (NULL != pSub)
        isAm = pSub->valueint;

    pSub = cJSON_GetObjectItem(pJson, "H");
    if (NULL != pSub) {
        rtcDate.hour = pSub->valueint;
		SNVS_LP_dateTimeStruct.hour = pSub->valueint;
//        if(is24Hour ) {
//            rtcDate.hour = pSub->valueint;
//        } else {
//            if(isAm) {
//                rtcDate.hour = pSub->valueint;
//            } else {
//                rtcDate.hour = pSub->valueint + 12;
//            }

//        }
    }

    pSub = cJSON_GetObjectItem(pJson, "Min");
    if (NULL != pSub){
        rtcDate.minute = pSub->valueint;
		SNVS_LP_dateTimeStruct.minute = pSub->valueint;
	}

    pSub = cJSON_GetObjectItem(pJson, "S");
    if (NULL != pSub){
        rtcDate.second = pSub->valueint;
		SNVS_LP_dateTimeStruct.second = pSub->valueint;
	}

    /*设置日期和时间*/
    SNVS_HP_RTC_SetDatetime(SNVS, &rtcDate);
    SNVS_LP_SRTC_SetDatetime(SNVS_LP_PERIPHERAL, &SNVS_LP_dateTimeStruct);
    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }

    /* 制作回复json包 */
    cJSON_AddNumberToObject(pJsonRoot, "Id", 1);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为2的消息, 该消息获取点检仪RTC时间
  * @input
  * @return
***************************************************************************************/
static char * GetTime(void)
{
    /* 获取日期 */
    SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);

    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
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
    cJSON_AddBoolToObject(pJsonRoot,"is24Hour",true);
    if (rtcDate.hour < 12) {
        cJSON_AddBoolToObject(pJsonRoot,"isAm",true);
    } else {
        cJSON_AddBoolToObject(pJsonRoot,"isAm",false);
    }

    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为3的消息, 该消息为仪器自检
  * @input
  * @return
***************************************************************************************/
static char * CheckSelf(void)
{
    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }

    //指示灯自检
    LED_CheckSelf();

    //红外测温模块自检
    g_sys_para.objTemp = MXL_ReadObjTemp();

    //文件系统自检
    eMMC_CheckFatfs();

    //震动传感器电压
    while (ADC_READY == 0);  //wait ads1271 ready
    g_sys_para.voltageADS1271 = LPSPI4_ReadData() * g_sys_para.bias * 1.0f / 0x800000;

    cJSON_AddNumberToObject(pJsonRoot, "Id", 3);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "AdcV", g_sys_para.voltageADS1271);  //振动传感器偏置电压
    cJSON_AddNumberToObject(pJsonRoot, "Temp", g_sys_para.objTemp);         //温度传感器的温度
    cJSON_AddNumberToObject(pJsonRoot, "PwrV", g_sys_para.batVoltage);      //电池电压值
    cJSON_AddNumberToObject(pJsonRoot, "BatC", (int)g_sys_para.batRemainPercent);//电池电量
    cJSON_AddNumberToObject(pJsonRoot, "Flash",(uint8_t)g_sys_para.emmcIsOk);//文件系统是否OK
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为4的消息, 该消息为获取电量
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
    cJSON_AddNumberToObject(pJsonRoot, "BatC", (int)g_sys_para.batRemainPercent);//电池电量
    cJSON_AddNumberToObject(pJsonRoot, "PwrV", g_sys_para.batVoltage);//电池电压值
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为5的消息, 该消息为获取版本号
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
    cJSON_AddStringToObject(pJsonRoot, "HV", HARD_VERSION);//硬件版本号
    cJSON_AddStringToObject(pJsonRoot, "SV", SOFT_VERSION);//软件版本号

    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为6的消息, 该消息为系统参数设置
  * @input
  * @return
***************************************************************************************/
static char * SetSysPara(cJSON *pJson, cJSON * pSub)
{
    /*解析消息内容,*/
    pSub = cJSON_GetObjectItem(pJson, "OffT");
    if (NULL != pSub)
        g_sys_para.inactiveTime = pSub->valueint;//触发自动关机时间

    pSub = cJSON_GetObjectItem(pJson, "OffC");
    if (NULL != pSub)
        g_sys_para.inactiveCondition = pSub->valueint;//触发自动关机条件

    pSub = cJSON_GetObjectItem(pJson, "BatL");
    if (NULL != pSub)
        g_sys_para.batAlarmValue = pSub->valueint;//电池电量报警值

    /*制作cjson格式的回复消息*/
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
  * @brief   处理消息id为7的消息, 该消息为采集参数设置
  * @input
  * @return
***************************************************************************************/
static char * SetSamplePara(cJSON *pJson, cJSON * pSub)
{
    uint8_t sid = 0;
    /*解析消息内容,*/
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
        //计算采样点数
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
        //计算采样点数
        g_sys_para.sampNumber = 2.56 * g_adc_set.Lines * g_adc_set.Averages * (1 - g_adc_set.AverageOverlap)
                                + 2.56 * g_adc_set.Lines * g_adc_set.AverageOverlap;
        break;
    default:
        break;
    }

    /*制作cjson格式的回复消息*/
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
  * @brief   处理消息id为8的消息, 该消息为开始采样信号
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
    sampTime = rtcDate;
    /*start sample*/
    ADC_SampleStart();

    /*wait task notify*/
    ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

    char *p_reply = NULL;
    if(g_sys_para.sampNumber != 0) { //Android发送了中断采集命令
        cJSON *pJsonRoot = cJSON_CreateObject();
        if(NULL == pJsonRoot) {
            return NULL;
        }
        cJSON_AddNumberToObject(pJsonRoot, "Id",  8);
        cJSON_AddNumberToObject(pJsonRoot, "Sid", 1);
        cJSON_AddStringToObject(pJsonRoot, "F", g_sys_para.fileName);
        cJSON_AddNumberToObject(pJsonRoot, "Si", g_sys_para.sampSize);
        cJSON_AddNumberToObject(pJsonRoot,"PK",  g_sys_para.sampPacks);
        cJSON_AddNumberToObject(pJsonRoot, "V", g_sys_para.shkCount);
        cJSON_AddNumberToObject(pJsonRoot, "S", g_sys_para.spdCount);
        p_reply = cJSON_PrintUnformatted(pJsonRoot);
        cJSON_Delete(pJsonRoot);
    }

    return p_reply;
}



/***************************************************************************************
  * @brief   处理消息id为9的消息, 该消息为获取采样数据
  * @input
  * @return
***************************************************************************************/
char *PacketSampleData(void)
{
    free(g_sys_para.sampJson);
    g_sys_para.sampJson = NULL;
    g_sys_para.sampSize = 0;
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddStringToObject(pJsonRoot, "DP", g_adc_set.IDPath);//硬件版本号
    cJSON_AddStringToObject(pJsonRoot, "NP", g_adc_set.NamePath);//硬件版本号
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

    /*将打包好的数据保存到文件 */
    if (NULL != g_sys_para.sampJson) {
        g_sys_para.sampSize = strlen(g_sys_para.sampJson);
        eMMC_SaveSampleData(g_sys_para.sampJson, g_sys_para.sampSize);
//        //将数据通过串口打印出来
        PRINTF("%s", g_sys_para.sampJson);
    }

    return g_sys_para.sampJson;
}


/***************************************************************************************
  * @brief   处理消息id为9的消息, 该消息为获取采样数据
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
    /*解析消息内容,并打包需要回复的内容*/
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
        cJSON_AddStringToObject(pJsonRoot, "DP", g_adc_set.IDPath);//硬件版本号
        cJSON_AddStringToObject(pJsonRoot, "NP", g_adc_set.NamePath);//硬件版本号
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
        p_reply[strlen(p_reply) - 1] = 0x00;//先去掉jsong格式后面的"}"
        if(sid-3 < g_sys_para.shkPacks)
        {
            strcat(p_reply,",V,");
            index = sid - 3;
            //每个数据占用4个byte;每包可以上传40个数据. 40*4=160
            memcpy(p_reply+strlen(p_reply), VibrateStrADC+index*160, 160);
            strcat(p_reply,"}");

        }
        else if(sid - 3 - g_sys_para.shkPacks < g_sys_para.spdCount)
        {
            strcat((char *)g_lpuart2TxBuf,",S,");
            index = sid - 3 - g_sys_para.shkPacks;
            //每个数据占用4个byte;每包可以上传38个数据. 40*4=160
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
  * @brief   处理消息id为10的消息, 该消息为开始发送升级固件包
  * @input
  * @return
***************************************************************************************/
static char * StartUpgrade(cJSON *pJson, cJSON * pSub)
{
    /* 开始升级固件后, 初始化一些必要的变量*/
    g_sys_para.firmUpdate = false;
    g_sys_para.firmPacksCount = 0;
    g_sys_para.firmSizeCurrent = 0;
    g_sys_para.firmNextAddr = APP_START_SECTOR * SECTOR_SIZE;;

    /*解析消息内容,*/
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
	
	/* 按照文件大小擦除对应大小的空间 */
    for(int i = 0; i<= g_sys_para.firmSizeTotal/SECTOR_SIZE; i++){
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
	g_puart2StartRx = true;//开始超市检测,5s中未接受到数据则超时
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为11的消息, 该消息为获取红外传感器温度
  * @input
  * @return
***************************************************************************************/
static char * GetObjTemp(void)
{
    //红外测温模块自检
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
  * @brief   处理消息id为13的消息, 该消息为获取manage文件内容
  * @input
  * @return
***************************************************************************************/
static char* GetManageFile(cJSON *pJson, cJSON * pSub)
{
    FRESULT res;
    UINT    br;
    char   *fileStr;
    extern FIL   g_fileObject ;
    int sid = 0, num = 0, offset = 0;

	eMMC_GetFree();
	
    /*解析消息内容,*/
    pSub = cJSON_GetObjectItem(pJson, "Sid");
    if(pSub != NULL) {
        sid = pSub->valueint;
    }

    //申请内存用于保存文件内容
    fileStr = malloc(200);
    memset(fileStr, 0U, 200);

    /*以可写方式打开文件*/
    res = f_open(&g_fileObject, _T("manage.txt"), (FA_READ));
    if(res == FR_OK) {
        num = g_fileObject.obj.objsize / 13 + (g_fileObject.obj.objsize%13?1:0);//文件名长度12个字符+1个逗号分隔符
    }

    if(sid <= num && res == FR_OK) {
        /*定位到指定位置,并读取文件的内容到 fileStr 缓冲区*/
		if (g_fileObject.obj.objsize > ((sid +1)* 130)){
			offset = g_fileObject.obj.objsize - ((sid +1)* 130);
		}else{
			offset = 0;
		}
        f_lseek(&g_fileObject, offset);
        res = f_read(&g_fileObject, fileStr, 130, &br);//每次读取10个文件名
    } else {
        fileStr = NULL;
    }
    f_close(&g_fileObject);

    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 13);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",sid);
    cJSON_AddNumberToObject(pJsonRoot, "num",num);
    cJSON_AddStringToObject(pJsonRoot, "content", fileStr);
	cJSON_AddNumberToObject(pJsonRoot, "total",g_sys_para.emmc_tot_size);
	cJSON_AddNumberToObject(pJsonRoot, "free",g_sys_para.emmc_fre_size);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    free(fileStr);
    fileStr = NULL;
    return p_reply;
}


/***************************************************************************************
  * @brief   处理消息id为14的消息, 该消息为通过文件名获取采集数据
  * @input
  * @return
***************************************************************************************/
static char* GetSampleFile(cJSON *pJson, cJSON * pSub)
{
    FRESULT res;
    UINT    br;
    char   *fileStr;
    extern FIL   g_fileObject ;
    char fileName[15] = {0};

    /*解析消息内容,*/
    pSub = cJSON_GetObjectItem(pJson, "fileName");
    if(pSub != NULL && strlen(pSub->valuestring) == 12) {
        strcpy(fileName,pSub->valuestring);
    }

    /*以可写方式打开文件*/
    res = f_open(&g_fileObject, fileName, (FA_READ));
    if(res == FR_OK) {
        //申请内存用于保存文件内容
        fileStr = malloc(g_fileObject.obj.objsize);
        memset(fileStr, 0U, g_fileObject.obj.objsize);

        /*读取文件的内容到 fileStr 缓冲区*/
        for (int i=0;; i++) {
            res = f_read(&g_fileObject, fileStr+i*4096, 4096, &br);
            if (res || br == 0) break; /* error or eof */
        }

        /*将读出来的文件数据按照json格式进行解析*/
        cJSON *pFileJson = cJSON_Parse((char *)fileStr);
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

        cJSON_Delete(pFileJson);
    }
    f_close(&g_fileObject);


    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
	//计算发送震动信号需要多少个包
	g_sys_para.shkPacks = (g_sys_para.shkCount / 40) +  (g_sys_para.shkCount%40?1:0);
	g_sys_para.spdPacks = (g_sys_para.spdCount / 40) +  (g_sys_para.spdCount%40?1:0);
	//计算将一次采集数据全部发送到Android需要多少个包
	g_sys_para.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 3;
    cJSON_AddNumberToObject(pJsonRoot, "Id", 14);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "packs",g_sys_para.sampPacks);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);

    free(fileStr);
    fileStr = NULL;

    return p_reply;
}


/***************************************************************************************
  * @brief   处理消息id为15的消息, 该消息为格式化eMMC文件系统
  * @input
  * @return
***************************************************************************************/
static char *FormatEmmc(void)
{
	FRESULT ret = FR_OK;

	ret = f_mkfs(driverNumberBuffer, FM_FAT32, 0U, g_data_read, sizeof g_data_read);//制作文件系统
	if (ret) {//格式化失败
		ret = f_mkfs(driverNumberBuffer, FM_FAT32, 0U, g_data_read, sizeof g_data_read);//重试
		if(ret){//再次格式化失败
			g_sys_para.emmcIsOk = false;
		}else{//格式化成功,挂载文件系统
			eMMC_Init();
		}
	}else{
		eMMC_Init();
	}
	
	cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
	cJSON_AddNumberToObject(pJsonRoot, "Id", 15);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
	cJSON_AddBoolToObject(pJsonRoot, "status",g_sys_para.emmcIsOk);
	cJSON_AddNumberToObject(pJsonRoot, "total",g_sys_para.emmc_tot_size);
	cJSON_AddNumberToObject(pJsonRoot, "free",g_sys_para.emmc_fre_size);
	char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为16的消息, 该消息为删除最早创建的文件
  * @input
  * @return
***************************************************************************************/
static char *DelEarliestFile(void)
{
	cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
	cJSON_AddNumberToObject(pJsonRoot, "Id", 16);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);

	eMMC_DelEarliestFile();

	cJSON_AddStringToObject(pJsonRoot, "fileName", g_sys_para.earliestFile);
	char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   解析json数据包
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
        p_reply = SetTime(pJson, pSub);//设置日期
        break;
    case 2:
        p_reply = GetTime();//获取日期
        break;
    case 3:
        p_reply = CheckSelf();//控制自检
        break;
    case 4:
        p_reply = GetBatCapacity();//获取电量
        break;
    case 5:
        p_reply = GetVersion();//获取版本号
        break;
    case 6:
        p_reply = SetSysPara(pJson, pSub);//系统参数设置
        break;
    case 7:
        p_reply = SetSamplePara(pJson, pSub);//采集参数设置
        break;
    case 8:
        p_reply = StartSample(pJson, pSub);//开始采样
        break;
    case 9:
        p_reply = GetSampleData(pJson, pSub);//获取采样结果
        break;
    case 10:
        p_reply = StartUpgrade(pJson, pSub);//升级
        break;
    case 11:
        p_reply = GetObjTemp();
        break;
    case 13:
        p_reply = GetManageFile(pJson, pSub);
        break;
    case 14:
        p_reply = GetSampleFile(pJson, pSub);
        break;
	case 15:
		p_reply = FormatEmmc();
		break;
	case 16:
		p_reply = DelEarliestFile();
		break;
    }

    cJSON_Delete(pJson);

    return (uint8_t *)p_reply;
}



/***************************************************************************************
  * @brief   解析固件包
  * @input
  * @return
***************************************************************************************/
uint8_t*  ParseFirmPacket(uint8_t *pMsg)
{
    uint16_t crc = 0;
    uint8_t  err_code = 0;
	status_t status;
	
    crc = CRC16(pMsg+4, FIRM_ONE_LEN);//自己计算出的CRC16
    if(pMsg[FIRM_ONE_PACKE_LEN-2] != (uint8_t)crc || pMsg[FIRM_ONE_PACKE_LEN-1] != (crc>>8)) {
        err_code = 1;
    } else {
        /* 包id */
        g_sys_para.firmPacksCount = pMsg[2] | (pMsg[3]<<8);
		
		i_appBuf = g_sys_para.firmPacksCount * FIRM_ONE_LEN;
		//将数据现存入appBuf当中
		memcpy(appBuf+i_appBuf%sizeof(appBuf), g_lpuart2RxBuf+4 , FIRM_ONE_LEN);
		//每25个包可以填满一个sector数据, 就可将app_buf中的数据写入spi Flash当中
		if(g_sys_para.firmPacksCount % 24 == 0 && g_sys_para.firmPacksCount/24 >=1){
			/* 计算出该包需要保存的SECTOR */
			g_sys_para.firmNextAddr = (APP_START_SECTOR + g_sys_para.firmPacksCount/24 - 1) * SECTOR_SIZE;
			if((g_sys_para.firmPacksCount/24) % 2 == 1 ){//奇数
				status = FlexSPI_NorFlash_Buffer_Program(FLEXSPI, g_sys_para.firmNextAddr, appBuf, SECTOR_SIZE);
			}else{//偶数
				status = FlexSPI_NorFlash_Buffer_Program(FLEXSPI, g_sys_para.firmNextAddr, appBuf+4096, SECTOR_SIZE);
			}
			if (status != kStatus_Success){
				PRINTF("写入失败 !\r\n");
			}
			/* 使用软件复位来重置 AHB 缓冲区. */
			FLEXSPI_SoftwareReset(FLEXSPI);
		}
	}

    /* 当前为最后一包,计算整个固件的crc16码 */
    if(g_sys_para.firmPacksCount == g_sys_para.firmPacksTotal - 1) {
	
		g_sys_para.bleLedStatus = BLE_CONNECT;
		g_puart2RxTimeCnt = 0;
		g_puart2StartRx = false;
		
		/* 计算出该包需要保存的SECTOR */
		g_sys_para.firmNextAddr = (APP_START_SECTOR + g_sys_para.firmPacksCount/25) * SECTOR_SIZE;
		if((g_sys_para.firmPacksCount/24) % 2 == 0 ){//偶数
			status = FlexSPI_NorFlash_Buffer_Program(FLEXSPI, g_sys_para.firmNextAddr, appBuf, SECTOR_SIZE);
		}else{//奇数
			status = FlexSPI_NorFlash_Buffer_Program(FLEXSPI, g_sys_para.firmNextAddr, appBuf+4096, SECTOR_SIZE);
		}
		if (status != kStatus_Success){
			PRINTF("写入失败 !\r\n");
		}
		
		/* 使用软件复位来重置 AHB 缓冲区. */
		FLEXSPI_SoftwareReset(FLEXSPI);
		
//		PRINTF("升级文件:\r\n");
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
			PRINTF("\n整包CRC校验通过,开始重启设备\n");
            g_sys_para.firmUpdate = true;
        }
    }

    /* 固件包,点检仪固定回复7Byte数据 */
//    g_puart2TxCnt = 7;
//    memcpy(g_lpuart2TxBuf, pMsg, 4);
//    g_lpuart2TxBuf[4] = err_code;         //错误码
//    crc = CRC16(g_lpuart2TxBuf, g_puart2TxCnt-2);
//    g_lpuart2TxBuf[5] = crc>>8;           //CRC H
//    g_lpuart2TxBuf[6] = (uint8_t)crc;     //CRC L
//    return g_lpuart2TxBuf;
	
	cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 10);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",1);
	cJSON_AddNumberToObject(pJsonRoot, "P", g_sys_para.firmPacksCount);
    cJSON_AddNumberToObject(pJsonRoot, "E", err_code);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
	g_sys_para.firmPacksCount++;
    return (uint8_t*)p_reply;
}

/***************************************************************************************
  * @brief   解析协议入口函数
  * @input
  * @return
***************************************************************************************/
uint8_t* ParseProtocol(uint8_t *pMsg)
{
    if(NULL == pMsg) {
        return NULL;
    }

    if(pMsg[0] == 0xE7 && pMsg[1] == 0xE7 ) { //为固件升级包
        return ParseFirmPacket(pMsg);
    } else { //为json数据包
        return ParseJson((char *)pMsg);
    }
}

