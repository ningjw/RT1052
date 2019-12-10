#include "main.h"

typedef struct
{
    uint16_t resp_time100ms;  //发送后查询返回信息的延时，100ms为单位。可设为指令最大响应时间。
    uint8_t  try_delay1ms;    //发送失败后再次发送时的延时，1ms为单位
    uint8_t  max_try_times;   //最大重试次数
    uint8_t  max_reset_times; //最大重启次数
}stcATConfig;

typedef void (*SendMsgFunc_t)(uint8_t * buf, uint32_t len);
SendMsgFunc_t SendMsgCallback;
char atAckBuf[64] = {0};            //串口接收缓冲区
SemaphoreHandle_t AckBufMux = NULL; //串口接收缓冲区互斥信号量
SemaphoreHandle_t RecvAckSem = NULL;//串口收到AT指令回令的信号量，可在串口接收空闲后发出

TaskHandle_t AppBLE_TaskHandle = NULL;  /* 蓝牙任务句柄 */


/***************************************************************************************
  * @brief   发送一个字符串 
  * @input   base:选择端口; data:将要发送的数据
  * @return  
***************************************************************************************/
void Uart_SendString( LPUART_Type *base,  const char *str)
{
    LPUART_WriteBlocking( base, (const uint8_t*)str, strlen(str));
}


/***************************************************************************************
  * @brief   在发送AT指令之前，需要先注册AT指令的运行环境
  * @input   
  * @return  
***************************************************************************************/
void AT_RegisterHandler(SendMsgFunc_t func)
{
	SendMsgCallback = func;                       //串口发送数据的函数            
}


/*****************************************************************
* 功能：发送AT指令
* 输入: send_buf:发送的字符串
		recv_str：期待回令中包含的子字符串
        p_at_config：AT配置
* 输出：执行结果代码
******************************************************************/
uint8_t AT_SendCmd(const char *send_str,const char *recv_str,stcATConfig *p_at_config)
{
    
}
 
/*****************************************************************
* 功能：发送数组数据
* 输入: send_buf:发送的数组
		buf_len：数组长度
		recv_str：期待回令中包含的子字符串
        p_at_config：AT配置
* 输出：执行结果代码
******************************************************************/
uint8_t AT_SendData(const char *send_buf,const uint16_t buf_len,const char *recv_str,stcATConfig *p_at_config)
{
    
}

/*****************************************************************
* 功能：查询AT指令的回令中是否有需要的字符串
* 输入: recv_str:期待输出字符串中需要含有的子字符串，如"OK\r\n"
        max_resp_time：指令最大响应时间，单位100ms
* 输出：查找到的子字符串指针
******************************************************************/
char* AT_SearchRecvBuf(const char* recv_str,uint16_t max_resp_time)
{
    
}
/******************************************
* 功能：清空串口接收缓冲区
******************************************/
void AT_ClearAckBuff(void)
{
    
}

/***********************************************************************
  * @ 函数名  ： AppBLE_Task
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void AppBLE_Task(void)
{
    RecvAckSem = xSemaphoreCreateBinary();      //创建 二值 信号量 
//    xSemaphoreGive( RecvAckSem );             //发送信号量
//    xSemaphoreTake(RecvAckSem, portMAX_DELAY);//获取信号量
    
    AckBufMux = xSemaphoreCreateMutex();      //创建 互斥信号量
//    xSemaphoreGive(AckBufMux);              //释放互斥量
//    xSemaphoreTake(AckBufMux,portMAX_DELAY);//获取互斥量
    
    stcATConfig ATCfg;
    ATCfg.resp_time100ms = 1;//最大相应时间为100ms
    ATCfg.try_delay1ms   = 100;//响应失败后再次发送时延时100ms
    ATCfg.max_try_times  = 3; ////最大重试次数:3
    ATCfg.max_reset_times =1; //最大重启次数：1
    
    while(1)
    {
        /* 获取日期 */
        SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
        /* 打印日期&时间 */ 
        PRINTF("BLE TASK:%02d-%02d-%02d  %02d:%02d:%02d \r\n", rtcDate.year,rtcDate.month, rtcDate.day,rtcDate.hour, rtcDate.minute, rtcDate.second);
        vTaskDelay(1000);
    }
}


/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void LPUART2_IRQHandler(void)
{
    uint8_t ucTemp;
    /*串口接收到数据*/
    if ( (kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(LPUART2) )
    {
        /*读取数据*/
        ucTemp = LPUART_ReadByte(LPUART2);
        
        /*将读取到的数据写入到缓冲区*/
//        Uart_SendByte(LPUART2, ucTemp);
    }
    __DSB();
}





