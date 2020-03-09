#include "main.h"


#define DEVICE_BLE_NAME "BLE Communication"

#define SET_COMMOND_MODE()       GPIO_PinWrite(BOARD_BTM_MODE_GPIO, BOARD_BTM_MODE_PIN, 1);
#define SET_THROUGHPUT_MODE()    GPIO_PinWrite(BOARD_BTM_MODE_GPIO, BOARD_BTM_MODE_PIN, 0);
#define BLE_POWER_ON()           GPIO_PinWrite(BOARD_PWR_WIFI_BLE_GPIO, BOARD_PWR_WIFI_BLE_PIN, 1);
#define BLE_RESET()              GPIO_PinWrite(BOARD_PWR_WIFI_BLE_GPIO, BOARD_PWR_WIFI_BLE_PIN, 0);
#define BLE_STATUS()             GPIO_PinRead(BOARD_BLE_STATUS_GPIO, BOARD_BLE_STATUS_PIN)

#define EVT_OK       (1 << 0)
#define ETV_TIMTOUT  (1 << 1)

extern void LPUART2_init(void);


uint8_t g_lpuart2TxBuf[LPUART2_BUFF_LEN] = {0};//���ڷ��ͻ�����
uint8_t g_lpuart2RxBuf[LPUART2_BUFF_LEN] = {0};//���ڽ��ջ�����

uint8_t g_puart2RxCnt = 0;
uint8_t g_puart2TxCnt = 0;
uint8_t g_puart2StartRx = 0;
uint32_t ble_event = 0;
uint32_t  g_puart2RxTimeCnt = 0;
TaskHandle_t        BLE_TaskHandle = NULL;//����������

static char send_str[164] = {0};
ATCfg_t g_at_cfg = {
    .resp_time = 100,//10ms������ܵ�������
    .try_times = 2,
    .try_cnt = 0,
};

/***************************************************************************************
  * @brief   ����һ���ַ��� 
  * @input   base:ѡ��˿�; data:��Ҫ���͵�����
  * @return  
***************************************************************************************/
void LPUART2_SendString(const char *str)
{
    LPUART_WriteBlocking(LPUART2, (uint8_t *)str, strlen(str));
}

/*****************************************************************
* ���ܣ���������
* ����: send_buf:���͵��ַ���
		recv_str���ڴ������а��������ַ���
        p_at_cfg��AT����
* �����ִ�н������
******************************************************************/
uint8_t AT_SendData(const char *send_str, ATCfg_t *p_at_cfg)
{
	
}


/*****************************************************************
* ���ܣ�����ATָ��
* ����: send_buf:���͵��ַ���
		recv_str���ڴ������а��������ַ���
        p_at_cfg��AT����
* �����ִ�н������
******************************************************************/
uint8_t AT_SendCmd(const char *cmd, const char *param, const char *recv_str, ATCfg_t *p_at_cfg)
{
    p_at_cfg->try_cnt = 0;
    
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
    LPUART2_SendString(send_str);//����ATָ��
    
    if (NULL == recv_str ) {
        return true;
    }
    /*wait resp_time*/
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, p_at_cfg->resp_time);
    
	//���յ��������а�����Ӧ������
	if(strstr((char *)g_lpuart2RxBuf, recv_str) != NULL){
		memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
		g_puart2RxCnt = 0;
		return true;
	}else {
		if(p_at_cfg->try_cnt++ > p_at_cfg->try_times){
			return false;
		}
		goto retry;//����
	}
}

/***********************************************************************
  * @ ������  �� BLE_AppTask
  * @ ����˵���� 
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void BLE_AppTask(void)
{
    uint8_t xReturn = pdFALSE;
    PRINTF("BLE Task Create and Running\r\n");
    uint8_t* sendBuf = NULL;

	SET_COMMOND_MODE();
	
	//���ò�����Ϊ230400
	xReturn = AT_SendCmd(BT_BAUD, "230400", BT_BAUD, &g_at_cfg);
	LPUART_SetBaudRate(LPUART2, 230400, LPUART2_CLOCK_SOURCE);
    /* ������������ */
    xReturn = AT_SendCmd(BT_NAME, DEVICE_BLE_NAME, RESP_OK, &g_at_cfg);
    if( xReturn == true ){
        g_sys_para.bleLedStatus = BLE_READY;
    }
	    /* ����͸��ģʽ */
    SET_THROUGHPUT_MODE();
    memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
    g_puart2RxCnt = 0;

    while(1)
    {
        /*wait task notify*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, portMAX_DELAY);
        if ( pdTRUE == xReturn && ble_event == EVT_OK) {
            /* ֻҪ�������ܵ�����,���ʾ��ǰ���豸���� */
            g_sys_para.bleLedStatus = BLE_CONNECT;
            
            /* ������������Э�� */
            sendBuf = ParseProtocol(g_lpuart2RxBuf);
            
            /* �������ݰ� */
            if(g_lpuart2RxBuf[0] == 0xE7 && g_lpuart2RxBuf[1] == 0xE7){
                /* ���ڻظ� */
                LPUART_WriteBlocking(LPUART2, sendBuf, 7);
                
                /* �Ƿ��������������ݰ� */
                if( g_sys_para.firmUpdate == true){
                    //����������Nor Flash
                    NorFlash_SaveFirmPara();
                    //�ر������ж�,����λϵͳ
                    NVIC_SystemReset();
                }
            }
            /* json���ݰ� */
            else if(NULL != sendBuf )
            {
                LPUART2_SendString((char *)sendBuf);
				PRINTF("%s",sendBuf);
				free(sendBuf);
            }
        }
		else if(pdTRUE == xReturn && ble_event == ETV_TIMTOUT){//�����������ݳ�ʱ
			g_puart2StartRx = 0;
		}
		
		//��ս��ܵ�������
		memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
        g_puart2RxCnt = 0;
		
		/* ��������Ϊ1, ���ܵ��������ݾ���0������*/
		if(g_sys_para.inactiveCondition == 1){
			g_sys_para.inactiveCount = 0;
		}
        
        /* �ж���������״̬*/
        if(BLE_STATUS()){//Connected
            g_sys_para.bleLedStatus = BLE_CONNECT;
        }else{//Disconnected
            g_sys_para.bleLedStatus = BLE_READY;
        }
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
    /*���ڽ��յ�����*/
    if ((kLPUART_RxDataRegFullFlag) & LPUART_GetStatusFlags(LPUART2))
    {
         /*��ȡ����*/
        ucTemp = LPUART_ReadByte(LPUART2);
		if( ucTemp == '{'){
			g_puart2StartRx = 1;
			g_puart2RxTimeCnt = 0;
		}else if( ucTemp == '}'){
			/* �������,�ñ�־��0*/
			g_puart2StartRx = 0;
			//���ܵ�Android���͵Ľ����ɼ��ź�
			if(memcmp(g_lpuart2RxBuf,"{\"Id\",12}",g_puart2RxCnt) == 0){
				g_sys_para.sampNumber = 0;//�����ʱ���ڲɼ�����, �ô���ᴥ���ɼ�����ź�
//			}else if(memcmp(g_lpuart2RxBuf,"{\"Id\",10}",g_puart2RxCnt) == 0){//���ܵ�Android���͵Ŀ�ʼ�����ź�
//				
			}else{
				/*���ý�������¼� */
				xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
			}
		}
        if(g_puart2RxCnt < LPUART2_BUFF_LEN){
//            /* �����ʱ��2�ļ�����*/
//            QUADTIMER2_PERIPHERAL->CHANNEL[QUADTIMER2_CHANNEL_0_CHANNEL].CNTR = 0;
//            /* �жϻ�δ������ʱ��2������,������*/
//            if(g_puart2StartRx == 0){
//                g_puart2StartRx++;
//                QTMR_SetTimerPeriod(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL, 16500U);
//                QTMR_StartTimer(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
//            }
            /* �����ܵ������ݱ��浽����*/
            g_lpuart2RxBuf[g_puart2RxCnt++] = ucTemp;
        }
        else/* ���ݳ���ָ������, ��0 */
       {
            g_puart2StartRx = 0;
            g_puart2RxCnt = 0;
       }
    }
    /* ������δ֪�ж�, ��������2*/
    else
    {
        g_puart2StartRx = 0;
        memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
        g_puart2RxCnt = 0;
        LPUART_Deinit(LPUART2);
        LPUART2_init();
		LPUART_SetBaudRate(LPUART2, 230400, LPUART2_CLOCK_SOURCE);
    }
    __DSB();
}

/***************************************************************************************
  * @brief   
  * @input
  * @return
***************************************************************************************/
void LPUART2_TimeTick(void)
{
	if(g_puart2StartRx)
	{
		g_puart2RxTimeCnt++;
		if(g_puart2RxTimeCnt >= 200){//�������ݳ�ʱ
			xTaskNotify(BLE_TaskHandle, ETV_TIMTOUT, eSetBits);
		}
	}
}
/***************************************************************************************
  * @brief   
  * @input
  * @return
***************************************************************************************/
void TMR2_IRQHandler(void)
{
    /* �������,�ñ�־��0*/
    g_puart2StartRx = 0;
    
    /* ��������Ϊ1, ���ܵ��������ݾ���0������*/
    if(g_sys_para.inactiveCondition == 1){
        g_sys_para.inactiveCount = 0;
    }
    /* ���жϱ�־ */
    QTMR_ClearStatusFlags(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL, kQTMR_CompareFlag);
    
    /* �������ݽ������,ֹͣ������ */
    QTMR_StopTimer(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL);
    
    /*���ý�������¼� */
    xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
}


