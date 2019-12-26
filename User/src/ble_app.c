#include "main.h"


#define DEVICE_BLE_NAME "BLE Communication"
#define SET_COMMOND_MODE()       GPIO_PinWrite(BOARD_BTM_MODE_GPIO, BOARD_BTM_MODE_PIN, 1);
#define SET_THROUGHPUT_MODE()    GPIO_PinWrite(BOARD_BTM_MODE_GPIO, BOARD_BTM_MODE_PIN, 0);
#define BLE_POWER_ON()           GPIO_PinWrite(BOARD_PWR_WIFI_BLE_GPIO, BOARD_PWR_WIFI_BLE_PIN, 1);
#define BLE_RESET()              GPIO_PinWrite(BOARD_PWR_WIFI_BLE_GPIO, BOARD_PWR_WIFI_BLE_PIN, 0);
#define EVT_OK       (1 << 0)//接受到数据事件



AT_NONCACHEABLE_SECTION_INIT(uint8_t g_lpuart2RxBuf[LPUART2_BUFF_LEN]) = {0};            //串口接收缓冲区

uint8_t g_puart2RxCnt = 0;
uint8_t g_puart2TxCnt = 0;
uint8_t g_puart2StartRx = 0;

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
    g_puart2StartRx = 0;
    g_puart2RxCnt = 0;
    xEventGroupClearBits(RecvAckEvt, EVT_OK);//发送数据前先清除该事件
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
        if(strstr((char *)g_lpuart2RxBuf, recv_str) != NULL){
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


char set_rtc[] = "{\"Id\":1,\"Sid\":0,\"Y\":2019,\"Mon\":12,\"D\":16,\"H\":9,\"Min\":0,\"S\":0}";
char get_rtc[] = "{\"Id\":2,\"Sid\":0}";
/***********************************************************************
  * @ 函数名  ： BLE_AppTask
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void BLE_AppTask(void)
{
    uint8_t ret = false;

    RecvAckEvt = xEventGroupCreate();/*  创建 事件组 */
    
    PRINTF("BLE Task Create and Running\r\n");
    
    ret = AT_SendCmd(BT_NAME, DEVICE_BLE_NAME, RESP_OK, &g_at_cfg);//设置蓝牙名称
    
    if( ret == true ){
        g_sys_para2.bleLedStatus = BLE_READY;
    }
    SET_THROUGHPUT_MODE();//进入透传模式
    
    while(1)
    {
        /*  等待接收事件标志 */
        xEventGroupWaitBits(RecvAckEvt,  /*  事件对象句柄 */
                            EVT_OK,   /*  接收 任务 感兴趣的事件 */
                            pdTRUE, /*  退出时清除事件位 */
                            pdTRUE, /*  满足感兴趣的所有事件 */
                            portMAX_DELAY);/*  指定超时事件, 一直等 */
        
//        LPUART2_SendString((char *)g_lpuart2RxBuf);
        ParseProtocol(g_lpuart2RxBuf);//处理蓝牙数据协议
        memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
        g_puart2RxCnt = 0;
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
    if ((kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(LPUART2))
    {
         /*读取数据*/
        ucTemp = LPUART_ReadByte(LPUART2);
        if(g_puart2RxCnt < LPUART2_BUFF_LEN){
            QUADTIMER2_PERIPHERAL->CHANNEL[QUADTIMER2_CHANNEL_0_CHANNEL].CNTR = 0;
            if(g_puart2StartRx == 0){
                g_puart2StartRx++;
                QTMR_SetTimerPeriod(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL, 3300U);
                QTMR_StartTimer(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
            }
            g_lpuart2RxBuf[g_puart2RxCnt++] = ucTemp;
        }else{
            g_puart2StartRx = 0;
            g_puart2RxCnt = 0;
        }
        
    }
    __DSB();
}

/***************************************************************************************
  * @brief   
  * @input
  * @return
***************************************************************************************/
void TMR2_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    g_puart2StartRx = 0;
    QTMR_ClearStatusFlags(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL, kQTMR_CompareFlag);//清中断标志
    QTMR_StopTimer(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL);//停止计数器
    xEventGroupSetBitsFromISR(RecvAckEvt, EVT_OK, &xHigherPriorityTaskWoken); /*设置事件 */
}


