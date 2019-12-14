#include "main.h"

#define LPUART2_BUFF_LEN 64

#define EVT_OK       (1 << 0)//接受到数据事件


SendMsgFunc_t SendMsgCallback;
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_lpuart2RxBuf[LPUART2_BUFF_LEN]) = {0};            //串口接收缓冲区
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_lpuart2TxBuf[LPUART2_BUFF_LEN]) = {0};            //串口发送缓冲区
lpuart_transfer_t receiveXfer;

TaskHandle_t        BLE_TaskHandle = NULL;//蓝牙任务句柄
EventGroupHandle_t  RecvAckEvt = NULL;//串口收到AT指令回令的信号量，可在串口接收空闲后发出

static char send_str[164] = {0};
ATCfg_t g_at_cfg = {
    .resp_time = 2000,
    .try_times = 2,
    .try_cnt = 0,
};

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
        p_at_cfg：AT配置
* 输出：执行结果代码
******************************************************************/
uint8_t AT_SendCmd(const char *cmd, const char *param, const char *recv_str, ATCfg_t *p_at_cfg)
{
    p_at_cfg->try_cnt = 0;
    EventBits_t at_event;
    
    memset(send_str, sizeof(send_str), 0);
    strcpy(send_str, "AT");
    strcat(send_str, cmd);
    if (NULL != param){
        strcat(send_str, "=");
        strcat(send_str, param);
    }
    strcat(send_str, "\r\n");
    
retry:
    LPUART2_SendString(send_str);//发送AT指令
    
    if (NULL == recv_str ) {
        return true;
    }
    
    /*  等待接收事件标志, */
    at_event = xEventGroupWaitBits(RecvAckEvt,  /*  事件对象句柄 */
                                EVT_OK,      /*  接收 任务 感兴趣的事件 */
                                pdFALSE,     /*  退出不清除事件位 */
                                pdFALSE,     /*  逻辑或等待事件 */
                                p_at_cfg->resp_time);/*  指定超时时间*/
    
    if( (at_event & EVT_OK) == EVT_OK ){
        xEventGroupClearBits(RecvAckEvt, EVT_OK);//清除事件
        //接收到的数据中包含响应的数据
        if(strstr((char *)receiveXfer.data, recv_str) != NULL){
            return true;
        }else {
            if(p_at_cfg->try_cnt++ > p_at_cfg->try_times){
                return false;
            }
            goto retry;//重试
        }
        
    }else{//回复超时
        if(p_at_cfg->try_cnt++ > p_at_cfg->try_times){
            return false;
        }
        goto retry;
    }
}

/*****************************************************************
* 功能：发送数组数据
* 输入: send_buf:发送的数组
		buf_len：数组长度
		recv_str：期待回令中包含的子字符串
        p_at_cfg：AT配置
* 输出：执行结果代码
******************************************************************/
uint8_t AT_SendData(const char *send_buf, uint8_t buf_len, const char *recv_str, ATCfg_t *p_at_cfg)
{
    memset(send_str, sizeof(send_str), 0);
    
    sprintf(send_str, "AT+LESEND=%d,", buf_len);
    
    strcat(send_str,send_buf);
    
    LPUART2_SendString(send_str);//发送数据
    
    return 0;
}





/***********************************************************************
  * @ 函数名  ： BLE_AppTask
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void BLE_AppTask(void)
{
    uint8_t ret = false;
    /*使能空闲中断*/
	LPUART_EnableInterrupts(LPUART2, kLPUART_IdleLineInterruptEnable);
	/*使能串口中断**/
	EnableIRQ(LPUART2_IRQn);
    
    RecvAckEvt = xEventGroupCreate();/*  创建 事件组 */
    
    receiveXfer.data     = g_lpuart2RxBuf;
    receiveXfer.dataSize = LPUART2_BUFF_LEN;

    LPUART_ReceiveEDMA(LPUART2, &LPUART2_eDMA_Handle, &receiveXfer);  //使用eDMA接收
    
    PRINTF("BLE Task Create and Running\r\n");
    
    AT_SendCmd(BT_NAME, "NINGJW", RESP_OK, &g_at_cfg);//设置蓝牙名称
    
    AT_SendCmd(BT_NAME, NULL, BT_NAME, &g_at_cfg);//获取蓝牙名称,判断是否设置成功
    if( ret == false ){//显示蓝牙错误状态指示等.
        
    }
    while(1)
    {
        /*  等待接收事件标志 */
        xEventGroupWaitBits(RecvAckEvt,  /*  事件对象句柄 */
                            EVT_OK,   /*  接收 任务 感兴趣的事件 */
                            pdTRUE, /*  退出时清除事件位 */
                            pdTRUE, /*  满足感兴趣的所有事件 */
                            portMAX_DELAY);/*  指定超时事件, 一直等 */

//        LPUART2_SendString((char *)receiveXfer.data);
    }
}

/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void LPUART2_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
   /*	接收到数据满了触发中断	*/
    if ((kLPUART_IdleLineFlag) & LPUART_GetStatusFlags(LPUART2))
    {
		/*清除空闲中断*/
		LPUART2->STAT |= LPUART_STAT_IDLE_MASK; 
        
		/*接收eDMA的数据量*/
		LPUART_TransferGetReceiveCountEDMA(LPUART2, &LPUART2_eDMA_Handle, &receiveXfer.dataSize); 
		LPUART_TransferAbortReceiveEDMA(LPUART2, &LPUART2_eDMA_Handle);   //eDMA终止接收数据
		LPUART_ReceiveEDMA(LPUART2, &LPUART2_eDMA_Handle, &receiveXfer);  //使用eDMA接收
        g_sys_para2.inactiveCount = 0;//接受到蓝牙数据，计数器清空
        xEventGroupSetBitsFromISR(RecvAckEvt, EVT_OK, &xHigherPriorityTaskWoken); /*设置事件 */
    }
    __DSB();
}


