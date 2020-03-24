#include "main.h"

extern float ShakeADC[];
extern float SpeedADC[];
extern char  SpeedStrADC[];
extern char  ShakeStrADC[];
snvs_hp_rtc_datetime_t sampTime;
uint16_t ble_wait_time = 35;
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
    if (NULL != pSub)
        rtcDate.year = pSub->valueint;

    pSub = cJSON_GetObjectItem(pJson, "Mon");
    if (NULL != pSub)
        rtcDate.month = pSub->valueint+1;

    pSub = cJSON_GetObjectItem(pJson, "D");
    if (NULL != pSub)
        rtcDate.day = pSub->valueint;

    pSub = cJSON_GetObjectItem(pJson, "is24Hour");
    if (NULL != pSub)
        is24Hour = pSub->valueint;

    pSub = cJSON_GetObjectItem(pJson, "isAm");
    if (NULL != pSub)
        isAm = pSub->valueint;

    pSub = cJSON_GetObjectItem(pJson, "H");
    if (NULL != pSub) {
        rtcDate.hour = pSub->valueint;
        if(is24Hour ) {
            rtcDate.hour = pSub->valueint;
        } else {
            if(isAm) {
                rtcDate.hour = pSub->valueint;
            } else {
                rtcDate.hour = pSub->valueint + 12;
            }

        }
    }

    pSub = cJSON_GetObjectItem(pJson, "Min");
    if (NULL != pSub)
        rtcDate.minute = pSub->valueint;

    pSub = cJSON_GetObjectItem(pJson, "S");
    if (NULL != pSub)
        rtcDate.second = pSub->valueint;

    /*设置日期和时间*/
    SNVS_HP_RTC_SetDatetime(SNVS, &rtcDate);

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
	for(uint8_t i=0;i<3;i++){
		g_sys_para.objTemp = MXL_ReadObjTemp();
		if(g_sys_para.objTemp > -200){
			break;
		}
	}

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
    cJSON_AddNumberToObject(pJsonRoot, "PwrV", g_sys_para.batVoltage);      //电池电压值
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
    cJSON_AddNumberToObject(pJsonRoot, "HV", HARD_VERSION);//硬件版本号
    cJSON_AddNumberToObject(pJsonRoot, "SV", SOFT_VERSION);//软件版本号

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
        pSub = cJSON_GetObjectItem(pJson, "IDPath");
        if (NULL != pSub) {
            memset(g_adc_set.IDPath, 0, sizeof(g_adc_set.IDPath));
            strcpy(g_adc_set.IDPath, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pJson, "NamePath");
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
        pSub = cJSON_GetObjectItem(pJson, "SpeedUnits");
        if (NULL != pSub) {
            memset(g_adc_set.SpeedUnits, 0, sizeof(g_adc_set.SpeedUnits));
            strcpy(g_adc_set.SpeedUnits, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pJson, "ProcessUnits");
        if (NULL != pSub) {
            memset(g_adc_set.ProcessUnits, 0, sizeof(g_adc_set.ProcessUnits));
            strcpy(g_adc_set.ProcessUnits, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pJson, "DetectionType");
        if (NULL != pSub) {
            g_adc_set.DetectionType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "Senstivity");
        if (NULL != pSub) {
            g_adc_set.Senstivity = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pJson, "Zerodrift");
        if (NULL != pSub) {
            g_adc_set.Zerodrift = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pJson, "EUType");
        if (NULL != pSub) {
            g_adc_set.EUType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "EU");
        if (NULL != pSub) {
            memset(g_adc_set.EU, 0, sizeof(g_adc_set.EU));
            strcpy(g_adc_set.EU, pSub->valuestring);
        }
        break;
    case 2:
        pSub = cJSON_GetObjectItem(pJson, "WindowsType");
        if (NULL != pSub) {
            g_adc_set.WindowsType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "StartFrequency");
        if (NULL != pSub) {
            g_adc_set.StartFrequency = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "EndFrequency");
        if (NULL != pSub) {
            g_adc_set.EndFrequency = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "SampleRate");
        if (NULL != pSub) {
            g_adc_set.SampleRate = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "Lines");
        if (NULL != pSub) {
            g_adc_set.Lines = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "Bias");
        if (NULL != pSub) {
            g_sys_para.bias = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pJson, "RefV");
        if (NULL != pSub) {
            g_sys_para.refV = pSub->valuedouble;
        }
		//计算采样点数
        g_sys_para.sampNumber = 2.56 * g_adc_set.Lines * g_adc_set.Averages * (1 - g_adc_set.AverageOverlap)
                                + 2.56 * g_adc_set.Lines * g_adc_set.AverageOverlap;
        break;
    case 3:
        pSub = cJSON_GetObjectItem(pJson, "Averages");
        if (NULL != pSub) {
            g_adc_set.Averages = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "AverageOverlap");
        if (NULL != pSub) {
            g_adc_set.AverageOverlap = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pJson, "AverageType");
        if (NULL != pSub) {
            g_adc_set.AverageType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "EnvFilterLow");
        if (NULL != pSub) {
            g_adc_set.EnvFilterLow = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "EnvFilterHigh");
        if (NULL != pSub) {
            g_adc_set.EnvFilterHigh = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "IncludeMeasurements");
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

    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }

    if(g_sys_para.sampNumber == 0) { //Android发送了中断采集命令
        cJSON_AddNumberToObject(pJsonRoot, "Id",  12);
        cJSON_AddNumberToObject(pJsonRoot, "Sid", 0);
    } else {
        cJSON_AddNumberToObject(pJsonRoot, "Id",  8);
        cJSON_AddNumberToObject(pJsonRoot, "Sid", 1);
        cJSON_AddStringToObject(pJsonRoot, "fileName", g_sys_para.fileName);
        cJSON_AddNumberToObject(pJsonRoot, "size", g_sys_para.sampSize);
        cJSON_AddNumberToObject(pJsonRoot,"packs",  g_sys_para.sampPacks);
        cJSON_AddNumberToObject(pJsonRoot, "shkNum", g_sys_para.shkCount);
        cJSON_AddNumberToObject(pJsonRoot, "spdNum", g_sys_para.spdCount);
    }
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}



/***************************************************************************************
  * @brief   处理消息id为9的消息, 该消息为获取采样数据
  * @input
  * @return
***************************************************************************************/
char * PacketSampleData(void)
{
    free(g_sys_para.sampJson);
    g_sys_para.sampJson = NULL;
    g_sys_para.sampSize = 0;
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id",  9);
    cJSON_AddNumberToObject(pJsonRoot, "Sid", 0);
    cJSON_AddStringToObject(pJsonRoot, "IDPath", g_adc_set.IDPath);
    cJSON_AddStringToObject(pJsonRoot, "NamePath", g_adc_set.NamePath);
    cJSON_AddNumberToObject(pJsonRoot, "Speed", g_adc_set.Speed);// 取转速波形平均转速
    cJSON_AddStringToObject(pJsonRoot, "SpeedUnits", g_adc_set.SpeedUnits);
    cJSON_AddNumberToObject(pJsonRoot, "Process", g_adc_set.Process);//取平均温度
    cJSON_AddNumberToObject(pJsonRoot, "ProcessMin", g_adc_set.ProcessMin);
    cJSON_AddNumberToObject(pJsonRoot, "ProcessMax", g_adc_set.ProcessMax);
    cJSON_AddStringToObject(pJsonRoot, "ProcessUnits", g_adc_set.ProcessUnits);//可以取温度、流量功率等
    cJSON_AddStringToObject(pJsonRoot, "DAUID", g_adc_set.DAUID);//生成的DAUID
    cJSON_AddNumberToObject(pJsonRoot, "DetectionType", g_adc_set.DetectionType);//手动检测0，时间定时检测1
    cJSON_AddNumberToObject(pJsonRoot, "Senstivity", g_adc_set.Senstivity);//灵敏度
    cJSON_AddNumberToObject(pJsonRoot, "Zerodrift", g_adc_set.Zerodrift);//零点偏移，用于将失调电压调节到零
    cJSON_AddNumberToObject(pJsonRoot, "EUType", g_adc_set.EUType);//g:0，mm/s2:1，um:2,mm/s:3,, gse:4
    cJSON_AddStringToObject(pJsonRoot, "EU", g_adc_set.EU);
    cJSON_AddNumberToObject(pJsonRoot, "WindowsType", g_adc_set.WindowsType);// 矩形窗0，三角窗1，汉宁窗2，海明窗3，布莱克曼窗4，凯泽窗5
    cJSON_AddStringToObject(pJsonRoot, "WindowName", g_adc_set.WindowName);//
    cJSON_AddNumberToObject(pJsonRoot, "StartFrequency", g_adc_set.StartFrequency);//采集起始频率
    cJSON_AddNumberToObject(pJsonRoot, "EndFrequency", g_adc_set.EndFrequency);//采集结束频率
    cJSON_AddNumberToObject(pJsonRoot, "SampleRate", g_adc_set.SampleRate);//采样率
    cJSON_AddNumberToObject(pJsonRoot, "Lines", g_adc_set.Lines);   //线数
    cJSON_AddNumberToObject(pJsonRoot, "Averages", g_adc_set.Averages);//平均次数
    cJSON_AddNumberToObject(pJsonRoot, "AverageOverlap", g_adc_set.AverageOverlap);//重叠率
    cJSON_AddNumberToObject(pJsonRoot, "AverageType", g_adc_set.AverageType);//重叠方式
    cJSON_AddNumberToObject(pJsonRoot, "EnvFilterLow", g_adc_set.EnvFilterLow);//包络滤波频段 低
    cJSON_AddNumberToObject(pJsonRoot, "EnvFilterHigh", g_adc_set.EnvFilterHigh);//包络滤波频段 高
    cJSON_AddNumberToObject(pJsonRoot, "StorageReson", g_adc_set.StorageReson);//采集方式     0手动采集，1定时采集
    cJSON_AddStringToObject(pJsonRoot, "MeasurementComment", g_adc_set.MeasurementComment);//
    cJSON_AddNumberToObject(pJsonRoot, "IncludeMeasurements", g_adc_set.IncludeMeasurements);
    cJSON_AddStringToObject(pJsonRoot, "Content", g_adc_set.Content);

    cJSON *measureMents = cJSON_AddArrayToObject(pJsonRoot, "MeasureMents");

    cJSON *pJsonSub1 = cJSON_CreateObject();
    cJSON_AddStringToObject(pJsonSub1, "MeasurementType", "VibRawData/EnvData");
    cJSON_AddStringToObject(pJsonSub1, "Value", ShakeStrADC);
    cJSON_AddItemToArray(measureMents, pJsonSub1);

    pJsonSub1 = cJSON_CreateObject();
    cJSON_AddStringToObject(pJsonSub1, "MeasurementType", "RotationSpeed");
    cJSON_AddStringToObject(pJsonSub1, "Value", SpeedStrADC);
    cJSON_AddItemToArray(measureMents, pJsonSub1);

    g_sys_para.sampJson = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);

    /*将打包好的数据保存到文件 */
//    if (NULL != g_sys_para.sampJson) {
//        g_sys_para.sampSize = strlen(g_sys_para.sampJson);
//        eMMC_SaveSampleData(g_sys_para.sampJson, g_sys_para.sampSize);
        //将数据通过串口打印出来
//        PRINTF("%s", g_sys_para.sampJson);
//    }

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
        cJSON_AddStringToObject(pJsonRoot, "IDPath", g_adc_set.IDPath);//硬件版本号
        cJSON_AddStringToObject(pJsonRoot, "NamePath", g_adc_set.NamePath);//硬件版本号
        break;
    case 1:
        cJSON_AddStringToObject(pJsonRoot, "SpeedUnits", g_adc_set.SpeedUnits);
        cJSON_AddStringToObject(pJsonRoot, "ProcessUnits", g_adc_set.ProcessUnits);
        cJSON_AddNumberToObject(pJsonRoot, "DetectionType", g_adc_set.DetectionType);
        cJSON_AddNumberToObject(pJsonRoot, "Senstivity", g_adc_set.Senstivity);
        cJSON_AddNumberToObject(pJsonRoot, "Zerodrift", g_adc_set.Zerodrift);
        cJSON_AddNumberToObject(pJsonRoot, "EUType", g_adc_set.EUType);
        cJSON_AddStringToObject(pJsonRoot, "EU", g_adc_set.EU);
        break;
    case 2:
        cJSON_AddNumberToObject(pJsonRoot, "WindowsType", g_adc_set.WindowsType);
        cJSON_AddNumberToObject(pJsonRoot, "StartFrequency", g_adc_set.StartFrequency);
        cJSON_AddNumberToObject(pJsonRoot, "EndFrequency", g_adc_set.EndFrequency);
        cJSON_AddNumberToObject(pJsonRoot, "SampleRate", g_adc_set.SampleRate);
        cJSON_AddNumberToObject(pJsonRoot, "Lines", g_adc_set.Lines);
        cJSON_AddNumberToObject(pJsonRoot, "Bias", g_sys_para.bias);
        cJSON_AddNumberToObject(pJsonRoot, "RefV", g_sys_para.refV);
        break;
    case 3:
        cJSON_AddNumberToObject(pJsonRoot, "Averages", g_adc_set.Averages);
        cJSON_AddNumberToObject(pJsonRoot, "AverageOverlap", g_adc_set.AverageOverlap);
        cJSON_AddNumberToObject(pJsonRoot, "AverageType", g_adc_set.AverageType);
        cJSON_AddNumberToObject(pJsonRoot, "EnvFilterLow", g_adc_set.EnvFilterLow);
        cJSON_AddNumberToObject(pJsonRoot, "EnvFilterHigh", g_adc_set.EnvFilterHigh);
        cJSON_AddNumberToObject(pJsonRoot, "IncludeMeasurements", g_adc_set.IncludeMeasurements);
        break;
    case 4:
        cJSON_AddNumberToObject(pJsonRoot, "Speed", g_adc_set.Speed);
        cJSON_AddNumberToObject(pJsonRoot, "Process", g_adc_set.Process);
        cJSON_AddNumberToObject(pJsonRoot, "ProcessMin", g_adc_set.ProcessMin);
        cJSON_AddNumberToObject(pJsonRoot, "ProcessMax", g_adc_set.ProcessMax);
        cJSON_AddNumberToObject(pJsonRoot, "packs", g_sys_para.sampPacks);
        cJSON_AddNumberToObject(pJsonRoot, "Y", sampTime.year);
        cJSON_AddNumberToObject(pJsonRoot, "Mon", sampTime.month);
        cJSON_AddNumberToObject(pJsonRoot, "D", sampTime.day);
        cJSON_AddNumberToObject(pJsonRoot, "H", sampTime.hour);
        cJSON_AddNumberToObject(pJsonRoot, "Min", sampTime.minute);
        cJSON_AddNumberToObject(pJsonRoot, "S", sampTime.second);
        break;
    default:
        memset(g_lpuart2TxBuf, 0, LPUART2_BUFF_LEN);
        if(sid-5 < g_sys_para.shkPacks)
        {
//            cJSON_AddStringToObject(pJsonRoot, "T", "V");
            g_lpuart2TxBuf[0] = 'V';
            index = sid - 5;
            //每个数据占用4个byte;每包可以上传38个数据. 38*4=152
            memcpy(g_lpuart2TxBuf+1, ShakeStrADC+index*152, 152);

            //最后一个数据的逗号分隔符需要去掉.
//            g_lpuart2TxBuf[strlen((char *)g_lpuart2TxBuf)-1] = 0x00;
            cJSON_AddStringToObject(pJsonRoot, "V", (char *)g_lpuart2TxBuf);
        }
        else if(sid - 5 - g_sys_para.shkPacks < g_sys_para.spdCount)
        {
//            cJSON_AddStringToObject(pJsonRoot, "T", "S");
            g_lpuart2TxBuf[0] = 'S';
            index = sid - 5 - g_sys_para.shkPacks;
            //每个数据占用4个byte;每包可以上传38个数据. 38*4=152
            memcpy(g_lpuart2TxBuf, SpeedStrADC+index*152, 152);
            //最后一个数据的逗号分隔符需要去掉.
//            g_lpuart2TxBuf[strlen((char *)g_lpuart2TxBuf)-1] = 0x00;
            cJSON_AddStringToObject(pJsonRoot, "V", (char *)g_lpuart2TxBuf);
        }
        break;
    }

    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
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
    g_sys_para.firmNextAddr = FIRM_DATA_ADDR;

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

    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id",  10);
    cJSON_AddNumberToObject(pJsonRoot, "Sid", 0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    g_sys_para.bleLedStatus = BLE_UPDATE;
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
	for(uint8_t i=0;i<3;i++){
		g_sys_para.objTemp = MXL_ReadObjTemp();
		if(g_sys_para.objTemp > -200){
			break;
		}
	}
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

    crc = CRC16(pMsg, g_puart2RxCnt);//自己计算出的CRC16
    if(pMsg[132] != (crc>>8) || pMsg[133] != (uint8_t)crc) {
        err_code = 1;
    } else {
        /* 包id */
        g_sys_para.firmPacksCount = pMsg[2];

        /* 根据包id,计算出该包需要保存的地址*/
        g_sys_para.firmNextAddr = FIRM_DATA_ADDR + g_sys_para.firmPacksCount * FIRM_ONE_PACKE_LEN;

        /* 当前接受到的包长度 */
        g_sys_para.firmSizeCurrent += pMsg[3];

        /* 保存固件数据到Nor Flash*/
        NorFlash_WriteApp(&pMsg[4], FIRM_ONE_PACKE_LEN);
    }

    /* 当前为最后一包,计算整个固件的crc16码 */
    if(pMsg[2] == g_sys_para.firmPacksTotal - 1 ) {
        crc = CRC16((void *)FIRM_DATA_ADDR, g_sys_para.firmSizeTotal);
        if(crc != g_sys_para.firmCrc16) {
            g_sys_para.firmUpdate = false;
            err_code = 2;
        } else { //整包CRC校验通过,开始重启设备更新.
            g_sys_para.firmUpdate = true;
        }
    }

    /* 固件包,点检仪固定回复7Byte数据 */
    g_puart2TxCnt = 7;
    memcpy(g_lpuart2TxBuf, pMsg, 3);
    g_lpuart2TxBuf[3] = g_puart2RxCnt - 6;//接受到的有效数据个数
    g_lpuart2TxBuf[4] = err_code;         //错误码
    crc = CRC16(g_lpuart2TxBuf, g_puart2TxCnt);
    g_lpuart2TxBuf[5] = crc>>8;           //CRC H
    g_lpuart2TxBuf[6] = (uint8_t)crc;     //CRC L
    g_sys_para.bleLedStatus = BLE_UPDATE;
    return g_lpuart2TxBuf;
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

