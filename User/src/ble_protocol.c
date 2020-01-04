#include "main.h"

/***************************************************************************************
  * @brief   处理消息id为1的消息, 该消息设置点检仪RTC时间
  * @input   
  * @return  
***************************************************************************************/
static char* ParseSetTime(cJSON *pJson, cJSON * pSub)
{
    /*解析消息内容, 获取日期和时间*/
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
    
    /*设置日期和时间*/
    SNVS_HP_RTC_SetDatetime(SNVS, &rtcDate);
    
    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    
    /* 制作回复json包 */
    cJSON_AddNumberToObject(pJsonRoot, "Id", 1);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为2的消息, 该消息获取点检仪RTC时间
  * @input   
  * @return  
***************************************************************************************/
static char * ParseGetTime(void)
{
	/* 获取日期 */
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
  * @brief   处理消息id为3的消息, 该消息为仪器自检
  * @input   
  * @return  
***************************************************************************************/
static char * ParseChkSelf(void)
{
    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    
    //指示灯自检
    LED_CheckSelf();
    
    //电池自检
    g_sys_para.batVoltage = LTC2942_GetVoltage() / 1000.0;// Battery voltage
    g_sys_para.batRemainPercent = LTC2942_GetAC() * 100.0 / 65536; // Accumulated charge
    
    //红外测温模块自检
    g_sys_para.objTemp = MXL_ReadObjTemp();
    
    //文件系统自检
    eMMC_CheckFatfs();
    
    //震动传感器电压
    while (ADC_READY == 0);  //wait ads1271 ready
    g_sys_para.voltageADS1271 = LPSPI4_ReadData() * 2.43f * 2 / 8388607;
    
    cJSON_AddNumberToObject(pJsonRoot, "Id", 3);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "AdcV", g_sys_para.voltageADS1271);  //振动传感器偏置电压
    cJSON_AddNumberToObject(pJsonRoot, "Temp", g_sys_para.objTemp);         //温度传感器的温度
    cJSON_AddNumberToObject(pJsonRoot, "PwrV", g_sys_para.batVoltage);      //电池电压值
    cJSON_AddNumberToObject(pJsonRoot, "BatC", (int)g_sys_para.batRemainPercent);//电池电量
    cJSON_AddNumberToObject(pJsonRoot, "Flash",(uint8_t)g_sys_para.emmcIsOk);//文件系统是否OK
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为4的消息, 该消息为获取电量
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
    cJSON_AddNumberToObject(pJsonRoot, "BatC", g_sys_para.batRemainPercent);//电池电量
 
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为5的消息, 该消息为获取版本号
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
    cJSON_AddStringToObject(pJsonRoot, "HV", "1.0");//硬件版本号
    cJSON_AddStringToObject(pJsonRoot, "SV", SOFT_VERSION);//软件版本号
    
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为6的消息, 该消息为系统参数设置
  * @input   
  * @return  
***************************************************************************************/
static char * ParseSetSysPara(cJSON *pJson, cJSON * pSub)
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
  * @brief   处理消息id为7的消息, 该消息为采集参数设置
  * @input   
  * @return  
***************************************************************************************/
static char * ParseSetSamplePara(cJSON *pJson, cJSON * pSub)
{
    /*解析消息内容,*/
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
    
    /*制作cjson格式的回复消息*/
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
  * @brief   处理消息id为8的消息, 该消息为开始采样信号
  * @input   
  * @return  
***************************************************************************************/
static char * ParseStartSample(void)
{
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id",  8);
    cJSON_AddNumberToObject(pJsonRoot, "Sid", 0);
    cJSON_AddNumberToObject(pJsonRoot, "Packs",g_sys_para.sampPacks);
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    ADC_SampleStart();
    return p_reply;
}


float envValue[5] = {0.1,0.2,0.3,0.4,0.5};
float spdValue[5] = {0.6,0.7,0.8,0.9,1.0};
/***************************************************************************************
  * @brief   处理消息id为9的消息, 该消息为获取采样数据
  * @input   
  * @return  
***************************************************************************************/
char * ParseGetSampleData(void)
{
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id",  9);
    cJSON_AddNumberToObject(pJsonRoot, "Sid", 1);
//    cJSON_AddStringToObject(pJsonRoot, "IDPath", "GroupID\\FactoryID\\EquipmentID\\PointID");
//    cJSON_AddStringToObject(pJsonRoot, "NamePath", "GroupID\\FactoryID\\EquipmentID\\PointID");
//    cJSON_AddNumberToObject(pJsonRoot, "Speed", 1);// 取转速波形平均转速
//    cJSON_AddStringToObject(pJsonRoot, "SpeedUnits", "RPM");
//    cJSON_AddNumberToObject(pJsonRoot, "Process", 1);
//    cJSON_AddNumberToObject(pJsonRoot, "ProcessMin", 65.0);
//    cJSON_AddNumberToObject(pJsonRoot, "ProcessMax", 68.0);
//    cJSON_AddStringToObject(pJsonRoot, "ProcessUnits", "℃");//可以取温度、流量功率等   
//    cJSON_AddStringToObject(pJsonRoot, "DAUID", "");//生成的DAUID
//    cJSON_AddStringToObject(pJsonRoot, "DetectionType", "0");//手动检测0，时间定时检测1
//    cJSON_AddNumberToObject(pJsonRoot, "Senstivity", 100.0);//灵敏度
//    cJSON_AddNumberToObject(pJsonRoot, "Zerodrift", 0.0);//零点偏移，用于将失调电压调节到零
//    cJSON_AddNumberToObject(pJsonRoot, "EUType", 4);//g:0，mm/s2:1，um:2,mm/s:3,, gse:4
//    cJSON_AddStringToObject(pJsonRoot, "EU", "mm/s");
//    cJSON_AddNumberToObject(pJsonRoot, "WindowsType", 1);// 矩形窗0，三角窗1，汉宁窗2，海明窗3，布莱克曼窗4，凯泽窗5
//    cJSON_AddStringToObject(pJsonRoot, "WindowName", "汉宁窗");//
//    cJSON_AddNumberToObject(pJsonRoot, "StartFrequency", 0);//采集起始频率
//    cJSON_AddNumberToObject(pJsonRoot, "EndFrequency", 2000);//采集起始频率
//    cJSON_AddNumberToObject(pJsonRoot, "SampleRate", 2560);//采样频率
//    cJSON_AddNumberToObject(pJsonRoot, "Lines", 1);//线数
//    cJSON_AddNumberToObject(pJsonRoot, "Averages", 1);//平均次数
//    cJSON_AddNumberToObject(pJsonRoot, "AverageOverlap", 0.5);//重叠率
//    cJSON_AddNumberToObject(pJsonRoot, "AverageType", 0);//重叠方式
//    cJSON_AddNumberToObject(pJsonRoot, "EnvFilterLow", 500);//包络滤波频段 低 
//    cJSON_AddNumberToObject(pJsonRoot, "EnvFilterHigh", 10000);//包络滤波频段 高
//    cJSON_AddNumberToObject(pJsonRoot, "StorageReson", 10000);//采集方式     0手动采集，1定时采集
//    cJSON_AddStringToObject(pJsonRoot, "MeasurementComment", "equipment condition description");//
//    cJSON_AddNumberToObject(pJsonRoot, "IncludeMeasurements", 1);
    cJSON_AddStringToObject(pJsonRoot, "Content", " ");
    cJSON_AddNumberToObject(pJsonRoot, "Bias", 10);
    
    cJSON *measureMents = cJSON_AddArrayToObject(pJsonRoot, "MeasureMents");

    cJSON *pJsonSub1 = cJSON_CreateObject();
    cJSON_AddStringToObject(pJsonSub1, "MeasurementType", "VibRawData/EnvData");
    cJSON_AddNumberToObject(pJsonSub1, "Value", envValue[0]);
    cJSON_AddNumberToObject(pJsonSub1, "Value", envValue[1]);
    cJSON_AddItemToArray(measureMents, pJsonSub1);
    
//    pJsonSub = cJSON_CreateObject();
//    cJSON_AddStringToObject(measureMents, "MeasurementType", "RotationSpeed");
//    cJSON_AddNumberToObject(measureMents, "Value", envValue[0]);
//    cJSON_AddItemToArray(measureMents, pJsonSub);
    
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为10的消息, 该消息为开始发送升级固件包
  * @input   
  * @return  
***************************************************************************************/
static char * ParseStartUpdate(cJSON *pJson, cJSON * pSub)
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
  * @brief   解析json数据包
  * @input   
  * @return  
***************************************************************************************/
uint8_t* ParseJson(char *pMsg){
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
            p_reply = ParseSetTime(pJson, pSub);//设置日期
            break;
        case 2:
            p_reply = ParseGetTime();//获取日期
            break;
        case 3:
            p_reply = ParseChkSelf();//控制自检
            break;
        case 4:
            p_reply = ParseGetBatCapacity();//获取电量
            break;
        case 5:
            p_reply = ParseGetVersion();//获取版本号
            break;
        case 6:
            p_reply = ParseSetSysPara(pJson, pSub);//系统参数设置
            break;
        case 7:
            p_reply = ParseSetSamplePara(pJson, pSub);//采集参数设置
            break;
        case 8:
            p_reply = ParseStartSample();//开始采样
            break;
        case 9:
            p_reply = ParseGetSampleData();//获取采样结果
            break;
        case 10:
            p_reply = ParseStartUpdate(pJson, pSub);//升级
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
	if(pMsg[132] != (crc>>8) || pMsg[133] != (uint8_t)crc){
		err_code = 1;
	}else{
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
	if(pMsg[2] == g_sys_para.firmPacksTotal - 1 ){
        crc = CRC16((void *)FIRM_DATA_ADDR, g_sys_para.firmSizeTotal);
        if(crc != g_sys_para.firmCrc16){
            g_sys_para.firmUpdate = false;
            err_code = 2;
        }else{//整包CRC校验通过,开始重启设备更新.
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
    if(NULL == pMsg){
        return NULL;
    }
    
    
    if(pMsg[0] == 0xE7 && pMsg[1] == 0xE7 ){//为固件升级包
        return ParseFirmPacket(pMsg);
    }else{//为json数据包
        return ParseJson((char *)pMsg);
    }
}

