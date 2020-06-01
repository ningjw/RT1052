#include "main.h"



#define LPUART3_BUFF_LEN 1024
uint8_t g_lpuart3RxBuf[LPUART3_BUFF_LEN] = {0};            //串口接收缓冲区

lpuart_transfer_t uart3RevXfer = {
    .data     = g_lpuart3RxBuf,
    .dataSize = LPUART3_BUFF_LEN,
};

uint32_t wifi_event = 0;
TaskHandle_t WIFI_TaskHandle;

/***************************************************************************************
  * @brief   发送一个字符串
  * @input   base:选择端口; data:将要发送的数据
  * @return
***************************************************************************************/
void LPUART3_SendString(const char *str)
{
    LPUART_WriteBlocking(LPUART3, (uint8_t *)str, strlen(str));
}

/***********************************************************************
  * @ 函数名  ： WIFI_AppTask
  * @ 功能说明：
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void WIFI_AppTask(void)
{
	uint8_t xReturn = pdFALSE;
	uint8_t* sendBuf = NULL;
	
	while(1)
	{
		/*wait task notify*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &wifi_event, portMAX_DELAY);
		if ( pdTRUE == xReturn && wifi_event == EVT_OK) {
			/* 处理WIFI数据协议 */
            sendBuf = ParseProtocol(g_lpuart3RxBuf);
			
			/* 是否接受完成整个数据包 */
			if( g_sys_para.firmUpdate == true) {
				//将参数存入Nor Flash
				NorFlash_SaveUpgradePara();
				//关闭所有中断,并复位系统
				NVIC_SystemReset();
			}

			if( NULL != sendBuf )
            {
                LPUART3_SendString((char *)sendBuf);
                free(sendBuf);
                sendBuf = NULL;
            }
			
		}else if(pdTRUE == xReturn && wifi_event == EVT_TIMTOUT) { //接受WIFI数据超时
			uint8_t id = 100;
			if(g_lpuart3RxBuf[7] == ',' && g_lpuart3RxBuf[6] >= '0' && g_lpuart3RxBuf[6] <= '9'){
				id = 100 + (g_lpuart3RxBuf[6]-0x30);
			}else if(g_lpuart3RxBuf[6] >= '0' && g_lpuart3RxBuf[6] <= '9' && 
			         g_lpuart3RxBuf[7] >= '0' && g_lpuart3RxBuf[7] <= '9'){
				id = 100 + (g_lpuart3RxBuf[6] - 0x30) * 10 + (g_lpuart3RxBuf[7] - 0x30);
			}
			/*制作cjson格式的回复消息*/
			cJSON *pJsonRoot = cJSON_CreateObject();
			cJSON_AddNumberToObject(pJsonRoot, "Id", id);
			char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
			LPUART3_SendString(p_reply);
			cJSON_Delete(pJsonRoot);
			free(p_reply);
			p_reply = NULL;
		}
		
		uart3RevXfer.dataSize = LPUART3_BUFF_LEN;
        memset(g_lpuart3RxBuf, 0, LPUART3_BUFF_LEN);
		
		/* 待机条件为1, 接受到wifi数据就清0计数器*/
        if(g_sys_para.inactiveCondition == 1) {
            g_sys_para.inactiveCount = 0;
        }
	}
	
}


/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void LPUART3_IRQHandler(void)
{
    /*	接收到数据满了触发中断	*/
    if ((kLPUART_IdleLineFlag) & LPUART_GetStatusFlags(LPUART3))
    {
		/*清除空闲中断*/
		LPUART3->STAT |= LPUART_STAT_IDLE_MASK; 
        
		/*接收eDMA的数据量*/
		LPUART_TransferGetReceiveCountEDMA(LPUART3, &LPUART3_eDMA_Handle, &uart3RevXfer.dataSize); 
		LPUART_TransferAbortReceiveEDMA(LPUART3, &LPUART3_eDMA_Handle);   //eDMA终止接收数据
		LPUART_ReceiveEDMA(LPUART3, &LPUART3_eDMA_Handle, &uart3RevXfer);  //使用eDMA接收
        
        PRINTF("%s:\r\n",uart3RevXfer.data);
        
        
    }
    __DSB();
}

