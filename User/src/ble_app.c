#include "main.h"

#define LPUART2_BUFF_LEN 64

SendMsgFunc_t SendMsgCallback;
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_lpuart2RxBuf[LPUART2_BUFF_LEN]) = {0};            //串口接收缓冲区
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_lpuart2TxBuf[LPUART2_BUFF_LEN]) = {0};            //串口发送缓冲区
lpuart_transfer_t receiveXfer;

TaskHandle_t      BLE_TaskHandle = NULL;//蓝牙任务句柄
SemaphoreHandle_t AckBufMux = NULL; //串口接收缓冲区互斥信号量
SemaphoreHandle_t RecvAckSem = NULL;//串口收到AT指令回令的信号量，可在串口接收空闲后发出
TimerHandle_t     Lpuart2Tmr = NULL;//软件定时器句柄,用于AT指令接受超时

/***************************************************************************************
  * @brief   发送一个字符串 
  * @input   base:选择端口; data:将要发送的数据
  * @return  
***************************************************************************************/
void LPUART2_SendString(const char *str)
{
    LPUART_WriteBlocking(LPUART2, (uint8_t *)str, strlen(str));
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
    return 0;
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
    return 0;
}

/***************************************************************************************
  * @brief   定时处理函数
  * @input   
  * @return  
***************************************************************************************/
void SoftTmr_Callback(void* parameter)
{

}

uint8_t flag_rev_data = 0;


/***********************************************************************
  * @ 函数名  ： BLE_AppTask
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void BLE_AppTask(void)
{
    /*使能空闲中断*/
	LPUART_EnableInterrupts(LPUART2, kLPUART_IdleLineInterruptEnable);
	/*使能串口中断**/
	EnableIRQ(LPUART2_IRQn);
    
    RecvAckSem = xSemaphoreCreateBinary();      //创建 二值 信号量 
    
    AckBufMux = xSemaphoreCreateMutex();      //创建 互斥信号量
//    xSemaphoreGive(AckBufMux);              //释放互斥量
//    xSemaphoreTake(AckBufMux,portMAX_DELAY);//获取互斥量
    
    //创建软件定时器。参数一次为：定时器名称、定时周期、周期模式(单次)、唯一id、回调函数
    Lpuart2Tmr = xTimerCreate("BleTimer", 100, pdFALSE,(void*)1, (TimerCallbackFunction_t)SoftTmr_Callback);
    if (Lpuart2Tmr != NULL) {
       xTimerStart(Lpuart2Tmr, 0); // 开启周期定时器
    }
   
    receiveXfer.data     = g_lpuart2RxBuf;
    receiveXfer.dataSize = LPUART2_BUFF_LEN;

    LPUART_ReceiveEDMA(LPUART2, &LPUART2_eDMA_Handle, &receiveXfer);  //使用eDMA接收
    
    PRINTF("BLE Task Create and Running\r\n");
    
    while(1)
    {
        xSemaphoreTake(RecvAckSem, portMAX_DELAY);//获取信号量
        LPUART2_SendString((char *)receiveXfer.data);
    }
}

/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void LPUART2_IRQHandler(void)
{
   /*	接收到数据满了触发中断	*/
    if ((kLPUART_IdleLineFlag) & LPUART_GetStatusFlags(LPUART2))
    {
		/*清除空闲中断*/
		LPUART2->STAT |= LPUART_STAT_IDLE_MASK; 
        
		/*接收eDMA的数据量*/
		LPUART_TransferGetReceiveCountEDMA(LPUART2, &LPUART2_eDMA_Handle, &receiveXfer.dataSize); 
		LPUART_TransferAbortReceiveEDMA(LPUART2, &LPUART2_eDMA_Handle);   //eDMA终止接收数据
		LPUART_ReceiveEDMA(LPUART2, &LPUART2_eDMA_Handle, &receiveXfer);  //使用eDMA接收
        xSemaphoreGive( RecvAckSem );
    }
    __DSB();
}


