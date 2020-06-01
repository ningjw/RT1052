#include "main.h"



#define LPUART3_BUFF_LEN 1024
uint8_t g_lpuart3RxBuf[LPUART3_BUFF_LEN] = {0};            //���ڽ��ջ�����

lpuart_transfer_t uart3RevXfer = {
    .data     = g_lpuart3RxBuf,
    .dataSize = LPUART3_BUFF_LEN,
};

uint32_t wifi_event = 0;
TaskHandle_t WIFI_TaskHandle;

/***************************************************************************************
  * @brief   ����һ���ַ���
  * @input   base:ѡ��˿�; data:��Ҫ���͵�����
  * @return
***************************************************************************************/
void LPUART3_SendString(const char *str)
{
    LPUART_WriteBlocking(LPUART3, (uint8_t *)str, strlen(str));
}

/***********************************************************************
  * @ ������  �� WIFI_AppTask
  * @ ����˵����
  * @ ����    �� ��
  * @ ����ֵ  �� ��
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
			/* ����WIFI����Э�� */
            sendBuf = ParseProtocol(g_lpuart3RxBuf);
			
			/* �Ƿ��������������ݰ� */
			if( g_sys_para.firmUpdate == true) {
				//����������Nor Flash
				NorFlash_SaveUpgradePara();
				//�ر������ж�,����λϵͳ
				NVIC_SystemReset();
			}

			if( NULL != sendBuf )
            {
                LPUART3_SendString((char *)sendBuf);
                free(sendBuf);
                sendBuf = NULL;
            }
			
		}else if(pdTRUE == xReturn && wifi_event == EVT_TIMTOUT) { //����WIFI���ݳ�ʱ
			uint8_t id = 100;
			if(g_lpuart3RxBuf[7] == ',' && g_lpuart3RxBuf[6] >= '0' && g_lpuart3RxBuf[6] <= '9'){
				id = 100 + (g_lpuart3RxBuf[6]-0x30);
			}else if(g_lpuart3RxBuf[6] >= '0' && g_lpuart3RxBuf[6] <= '9' && 
			         g_lpuart3RxBuf[7] >= '0' && g_lpuart3RxBuf[7] <= '9'){
				id = 100 + (g_lpuart3RxBuf[6] - 0x30) * 10 + (g_lpuart3RxBuf[7] - 0x30);
			}
			/*����cjson��ʽ�Ļظ���Ϣ*/
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
		
		/* ��������Ϊ1, ���ܵ�wifi���ݾ���0������*/
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
    /*	���յ��������˴����ж�	*/
    if ((kLPUART_IdleLineFlag) & LPUART_GetStatusFlags(LPUART3))
    {
		/*��������ж�*/
		LPUART3->STAT |= LPUART_STAT_IDLE_MASK; 
        
		/*����eDMA��������*/
		LPUART_TransferGetReceiveCountEDMA(LPUART3, &LPUART3_eDMA_Handle, &uart3RevXfer.dataSize); 
		LPUART_TransferAbortReceiveEDMA(LPUART3, &LPUART3_eDMA_Handle);   //eDMA��ֹ��������
		LPUART_ReceiveEDMA(LPUART3, &LPUART3_eDMA_Handle, &uart3RevXfer);  //ʹ��eDMA����
        
        PRINTF("%s:\r\n",uart3RevXfer.data);
        
        
    }
    __DSB();
}

