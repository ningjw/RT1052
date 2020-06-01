#include "main.h"

extern uint8_t s_nor_program_buffer[];
extern AdcInfoTotal adcInfoTotal;
extern AdcInfo adcInfo;
snvs_lp_srtc_datetime_t sampTime;
uint16_t ble_wait_time = 5;

/***************************************************************************************
  * @brief   处理消息id为1的消息, 该消息设置点检仪RTC时间
  * @input
  * @return
***************************************************************************************/
static char* SetTime(cJSON *pJson, cJSON * pSub)
{
    /*解析消息内容, 获取日期和时间*/
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

    /*设置日期和时间*/
    SNVS_LP_SRTC_SetDatetime(SNVS_LP_PERIPHERAL, &SNVS_LP_dateTimeStruct);

    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) return NULL;
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
    sampTime = SNVS_LP_dateTimeStruct;
    /*start sample*/
    ADC_SampleStart();

    /*wait task notify*/
    ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

    if(g_sys_para.sampNumber != 0) { //Android发送了中断采集命令
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
            //每个数据占用4个byte;每包可以上传40个数据. 40*4=160
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
    g_sys_para.firmCurrentAddr = APP_START_SECTOR * SECTOR_SIZE;;

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
  * @brief   处理消息id为12的消息, 该消息为终止采样
  * @input
  * @return
***************************************************************************************/
static char* StopSample(void)
{
    //如果此时正在采集数据, 该代码会触发采集完成信号
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
  * @brief   处理消息id为13的消息, 该消息为获取manage文件内容
  * @input
  * @return
***************************************************************************************/
static char* GetManageInfo(cJSON *pJson, cJSON * pSub)
{
    char   *fileStr;
    int sid = 0, num = 0, si = 0, len = 0;

    /*解析消息内容,*/
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

    //申请内存用于保存文件内容
    len = num * 13 + 1;
    fileStr = malloc(len);
    memset(fileStr, 0U, len);

    NorFlash_ReadAdcInfo(si, num, fileStr);

    /*制作cjson格式的回复消息*/
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
  * @brief   处理消息id为14的消息, 该消息为通过文件名获取采集数据
  * @input
  * @return
***************************************************************************************/
static char* GetSampleDataInFlash(cJSON *pJson, cJSON * pSub)
{
    extern AdcInfo adcInfo;
    uint32_t addrOfAdcData;
    char fileName[15] = {0};

    /*解析消息内容,*/
    pSub = cJSON_GetObjectItem(pJson, "fileName");
    if(pSub != NULL && strlen(pSub->valuestring) == 12) {
        strcpy(fileName,pSub->valuestring);
    }

    /*从flash读取文件*/
    addrOfAdcData = NorFlash_ReadAdcData(fileName);

    if(addrOfAdcData != NULL) {
        char* sampJson = malloc(adcInfo.AdcDataLen);
        memcpy(sampJson, NORFLASH_AHB_POINTER(addrOfAdcData), adcInfo.AdcDataLen);

        /*将数据按照json格式进行解析*/
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

    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) return NULL;
    //计算发送震动信号需要多少个包
#ifdef BLE_VERSION
    g_sys_para.shkPacks = (g_sys_para.shkCount / 40) +  (g_sys_para.shkCount%40?1:0);
    g_sys_para.spdPacks = (g_sys_para.spdCount / 40) +  (g_sys_para.spdCount%40?1:0);
	//计算将一次采集数据全部发送到Android需要多少个包
    g_sys_para.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 3;
#elif defined WIFI_VERSION
	g_sys_para.shkPacks = (g_sys_para.shkCount / 250) +  (g_sys_para.shkCount%250?1:0);
    g_sys_para.spdPacks = (g_sys_para.spdCount / 250) +  (g_sys_para.spdCount%250?1:0);
	//计算将一次采集数据全部发送到Android需要多少个包
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
  * @brief   处理消息id为15的消息, 该消息为擦除flash中保存的所有采样数据
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
  * @brief   处理消息id为16的消息, 该消息为设置电池电量
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

    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) return NULL;
    cJSON_AddNumberToObject(pJsonRoot, "Id", 16);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}



/***************************************************************************************
  * @brief   处理消息id为17的消息, 该消息为通过wifi设置采集参数
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
    //计算采样点数
    g_sys_para.sampNumber = 2.56 * g_adc_set.Lines * g_adc_set.Averages * (1 - g_adc_set.AverageOverlap)
                            + 2.56 * g_adc_set.Lines * g_adc_set.AverageOverlap;

    /*制作cjson格式的回复消息*/
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
  * @brief   处理消息id为18的消息, 该消息为通过wifi获取采样数据
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
    cJSON_AddNumberToObject(pJsonRoot, "Id", 18);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",sid);

    switch(sid)	{
    case 0:
        cJSON_AddNumberToObject(pJsonRoot, "D", ble_wait_time);
        cJSON_AddStringToObject(pJsonRoot, "DP", g_adc_set.IDPath);//硬件版本号
        cJSON_AddStringToObject(pJsonRoot, "NP", g_adc_set.NamePath);//硬件版本号
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
        p_reply[strlen(p_reply) - 1] = 0x00;//先去掉jsong格式后面的"}"
        if(sid-1 < g_sys_para.shkPacks)
        {
            strcat(p_reply,",V,");
            index = sid - 1;
            //每个数据占用4个byte;每包可以上传250个数据. 250*4=1000
            memcpy(p_reply+strlen(p_reply), VibrateStrADC+index*1000, 1000);
            strcat(p_reply,"}");
        }
        else if(sid - 1 - g_sys_para.shkPacks < g_sys_para.spdCount)
        {
            strcat((char *)g_lpuart2TxBuf,",S,");
            index = sid - 1 - g_sys_para.shkPacks;
            //每个数据占用4个byte;每包可以上传250个数据. 250*4=1000
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
  * @brief   解析固件包
  * @input
  * @return
***************************************************************************************/
uint8_t*  ParseFirmPacket(uint8_t *pMsg)
{
    uint16_t crc = 0;
    uint8_t  err_code = 0;

    crc = CRC16(pMsg+4, FIRM_ONE_LEN);//自己计算出的CRC16
    if(pMsg[FIRM_ONE_PACKE_LEN-2] != (uint8_t)crc || pMsg[FIRM_ONE_PACKE_LEN-1] != (crc>>8)) {
        err_code = 1;
    } else {
        /* 包id */
        g_sys_para.firmPacksCount = pMsg[2] | (pMsg[3]<<8);

        g_sys_para.firmCurrentAddr = APP_START_SECTOR * SECTOR_SIZE + g_sys_para.firmPacksCount * FIRM_ONE_LEN;//
//        PRINTF("\nADDR = 0x%x\n",g_sys_para.firmCurrentAddr);
        FlexSPI_FlashWrite(pMsg+4, g_sys_para.firmCurrentAddr, FIRM_ONE_LEN);
    }

    /* 当前为最后一包,计算整个固件的crc16码 */
    if(g_sys_para.firmPacksCount == g_sys_para.firmPacksTotal - 1) {

        g_sys_para.bleLedStatus = BLE_CONNECT;
        g_puart2RxTimeCnt = 0;
        g_puart2StartRx = false;

        /* 使用软件复位来重置 AHB 缓冲区. */
        FLEXSPI_SoftwareReset(FLEXSPI);

//		PRINTF("升级文件:\r\n");
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
            PRINTF("\n整包CRC校验通过,开始重启设备\n");
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

