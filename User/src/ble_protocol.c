#include "main.h"

/***************************************************************************************
  * @brief   处理消息id为1的消息, 该消息设置点检仪RTC时间
  * @input   
  * @return  
***************************************************************************************/
static char* ParseMsg1(cJSON *pJson, cJSON * pSub)
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
static char * ParseMsg2(void)
{
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
static char * ParseMsg3(void)
{
    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 3);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "AdcV",3.3);//振动传感器偏置电压
    cJSON_AddNumberToObject(pJsonRoot, "Temp",25.5);//温度传感器的温度
    cJSON_AddNumberToObject(pJsonRoot, "PwrV",3.3);//电源电压值
    cJSON_AddNumberToObject(pJsonRoot, "BatC", 100);//电池电量
    cJSON_AddNumberToObject(pJsonRoot, "Flash",1);
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为4的消息, 该消息为获取电量
  * @input   
  * @return  
***************************************************************************************/
static char * ParseMsg4(void)
{
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 4);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "BatC", 100);//电池电量
 
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为5的消息, 该消息为获取电量
  * @input   
  * @return  
***************************************************************************************/
static char * ParseMsg5(void)
{
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 5);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddStringToObject(pJsonRoot, "HV", "1.0");//硬件版本号
    cJSON_AddStringToObject(pJsonRoot, "SV", "1.0");//软件版本号
 
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为6的消息, 该消息为系统参数设置
  * @input   
  * @return  
***************************************************************************************/
static char * ParseMsg6(cJSON *pJson, cJSON * pSub)
{
    /*解析消息内容,*/
    pSub = cJSON_GetObjectItem(pJson, "OffT");
    if (NULL != pSub)
        g_sys_para1.inactiveTime = pSub->valueint;
    
    pSub = cJSON_GetObjectItem(pJson, "OffC");
    if (NULL != pSub)
        g_sys_para1.inactiveCondition = pSub->valueint;
    
    pSub = cJSON_GetObjectItem(pJson, "BatL");
    if (NULL != pSub)
        g_sys_para1.batAlarmValue = pSub->valueint;
    
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
static char * ParseMsg7(cJSON *pJson, cJSON * pSub)
{
    /*解析消息内容,*/
    pSub = cJSON_GetObjectItem(pJson, "Mode");
    if (NULL != pSub)
        g_sys_para1.sampMode = pSub->valueint;
    
    pSub = cJSON_GetObjectItem(pJson, "Freq");
    if (NULL != pSub)
        g_sys_para1.sampFreq = pSub->valueint;
    
    pSub = cJSON_GetObjectItem(pJson, "Bw");
    if (NULL != pSub)
        g_sys_para1.sampBandwidth = pSub->valueint;
    
    pSub = cJSON_GetObjectItem(pJson, "Time");
    if (NULL != pSub)
        g_sys_para1.sampTime = pSub->valueint;
    
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
static char * ParseMsg8(void)
{
    g_sys_para2.sampStart = true;
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id",  8);
    cJSON_AddNumberToObject(pJsonRoot, "Sid", 0);
    cJSON_AddNumberToObject(pJsonRoot, "Packs",g_sys_para2.sampPacks);
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为9的消息, 该消息为获取采样数据
  * @input   
  * @return  
***************************************************************************************/
static char * ParseMsg9(void)
{
    g_sys_para2.sampStart = true;
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id",  9);
    cJSON_AddNumberToObject(pJsonRoot, "Sid", 1);
    
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为10的消息, 该消息为开始发送升级固件包
  * @input   
  * @return  
***************************************************************************************/
static char * ParseMsg10(cJSON *pJson, cJSON * pSub)
{
    /*解析消息内容,*/
    pSub = cJSON_GetObjectItem(pJson, "Packs");
    if (NULL != pSub)
        g_sys_para1.firmPacks = pSub->valueint;
    
    pSub = cJSON_GetObjectItem(pJson, "Size");
    if (NULL != pSub)
        g_sys_para1.firmSize = pSub->valueint;
    
    pSub = cJSON_GetObjectItem(pJson, "CRC16");
    if (NULL != pSub)
        g_sys_para1.firmCrc16 = pSub->valueint;
    
    
    g_sys_para2.sampStart = true;
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id",  10);
    cJSON_AddNumberToObject(pJsonRoot, "Sid", 0);
    char *p_reply = cJSON_Print(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
char* ParseProtocol(char *pMsg)
{
    char *p_reply = NULL;
    if(NULL == pMsg){
        return NULL;
    }
    
    cJSON *pJson = cJSON_Parse(pMsg);
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
            p_reply = ParseMsg1(pJson, pSub);
            break;
        case 2:
            p_reply = ParseMsg2();
            break;
        case 3:
            p_reply = ParseMsg3();
            break;
        case 4:
            p_reply = ParseMsg4();
            break;
        case 5:
            p_reply = ParseMsg5();
            break;
        case 6:
            p_reply = ParseMsg6(pJson, pSub);
            break;
        case 7:
            p_reply = ParseMsg7(pJson, pSub);
            break;
        case 8:
            p_reply = ParseMsg8();
            break;
        case 9:
            p_reply = ParseMsg9();
            break;
        case 10:
            p_reply = ParseMsg10(pJson, pSub);
            break;
        default:
            break;
    }
    
    cJSON_Delete(pJson);
    
    return p_reply;
}

