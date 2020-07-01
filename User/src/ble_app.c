#include "main.h"


#define DEVICE_BLE_NAME "BLE Communication"

#define SET_COMMOND_MODE()       GPIO_PinWrite(BOARD_BTM_MODE_GPIO, BOARD_BTM_MODE_PIN, 1);
#define SET_THROUGHPUT_MODE()    GPIO_PinWrite(BOARD_BTM_MODE_GPIO, BOARD_BTM_MODE_PIN, 0);
#define BLE_POWER_ON()           GPIO_PinWrite(BOARD_PWR_WIFI_BLE_GPIO, BOARD_PWR_WIFI_BLE_PIN, 1);
#define BLE_RESET()              GPIO_PinWrite(BOARD_PWR_WIFI_BLE_GPIO, BOARD_PWR_WIFI_BLE_PIN, 0);


extern void LPUART2_init(void);


uint8_t g_lpuart2TxBuf[LPUART2_BUFF_LEN] = {0};//���ڷ��ͻ�����
uint8_t g_lpuart2RxBuf[LPUART2_BUFF_LEN] = {0};//���ڽ��ջ�����

uint16_t g_puart2RxCnt = 0;
uint16_t g_puart2TxCnt = 0;
uint8_t g_puart2StartRx = false;
uint32_t  g_puart2RxTimeCnt = 0;
uint32_t ble_event = 0;
uint32_t BleStartFlag = false;

TaskHandle_t        BLE_TaskHandle = NULL;//����������


/***************************************************************************************
  * @brief   ����һ���ַ���
  * @input   base:ѡ��˿�; data:��Ҫ���͵�����
  * @return
***************************************************************************************/
void LPUART2_SendString(const char *str)
{
    LPUART_WriteBlocking(LPUART2, (uint8_t *)str, strlen(str));
}

/***************************************************************************************
  * @brief   ����WIFIģ��ΪAp����ģʽ
  * @input   
  * @return
***************************************************************************************/
void WIFI_Init(void)
{
	LPUART2_SendString("+++");
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 300);/*wait task notify*/
	
	LPUART2_SendString("a");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 300);/*wait task notify*/
	
//	LPUART2_SendString("AT+RELD\r\n");
//	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 1000);/*wait task notify*/
	
	LPUART2_SendString("AT+E=off\r\n");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);
	
	LPUART2_SendString("AT+WMODE=AP\r\n");
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/
	
	LPUART2_SendString("AT+WAP=USR-C322-,88888888\r\n");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/
	
	LPUART2_SendString("AT+CHANNEL=1\r\n");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/
	
	LPUART2_SendString("AT+LANN=192.168.1.1,255.255.255.0\r\n");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/
	
	LPUART2_SendString("AT+SOCKA=TCPS,192.168.1.1,8899\r\n");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/
	
//	LPUART2_SendString("AT+WKMOD=TRANS\r\n");
//	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/

	LPUART2_SendString("AT+ENTM\r\n");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/
}


/*****************************************************************
* ���ܣ�����ATָ��
* ����: send_buf:���͵��ַ���
		recv_str���ڴ������а��������ַ���
        p_at_cfg��AT����
* �����ִ�н������
******************************************************************/
uint8_t BLE_SendCmd(const char *cmd, const char *recv_str, uint16_t time_out)
{
    uint8_t try_cnt = 0;
	g_puart2RxCnt = 0;
	memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
retry:
    LPUART2_SendString(cmd);//����ATָ��
    /*wait resp_time*/
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, time_out);
    //���յ��������а�����Ӧ������
    if(strstr((char *)g_lpuart2RxBuf, recv_str) != NULL) {
        return true;
    } else {
        if(try_cnt++ > 3) {
            return false;
        }
        goto retry;//����
    }
}
/***************************************************************************************
  * @brief   ��������ģ��
  * @input   
  * @return
***************************************************************************************/
void BLE_Init(void)
{
	SET_COMMOND_MODE();
	BLE_SendCmd("AT\r\n","OK",500);
	BLE_SendCmd("AT+BAUD=230400\r\n","OK",300);
    LPUART_SetBaudRate(LPUART2, 230400, LPUART2_CLOCK_SOURCE);
	BLE_SendCmd("AT+NAME=BLE Communication\r\n","OK",300);/* ������������ */
	BLE_SendCmd("AT+LPM=0\r\n","OK",300);/*�رյ͹���ģʽ*/
    BLE_SendCmd("AT+TPMODE=1\r\n","OK",300);/* ����͸��ģʽ */
	SET_THROUGHPUT_MODE();
	g_sys_para.BleWifiLedStatus = BLE_READY;
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
    PRINTF("BLE/WIFI Task Create and Running\r\n");
    uint8_t* sendBuf = NULL;

	#ifdef BLE_VERSION
	BLE_Init();
	#elif defined WIFI_VERSION
	WIFI_Init();
	#endif
    
	BleStartFlag = true;
    memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
    g_puart2RxCnt = 0;

    while(1)
    {
        /*wait task notify*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, portMAX_DELAY);
        if ( pdTRUE == xReturn && ble_event == EVT_OK) {

            /* ������������Э�� */
            sendBuf = ParseProtocol(g_lpuart2RxBuf);

			/* �Ƿ��������������ݰ� */
			if( g_sys_para.firmUpdate == true) {
				//����������Nor Flash
				NorFlash_SaveUpgradePara();
				//�ر������ж�,����λϵͳ
				NVIC_SystemReset();
			}

			if( NULL != sendBuf )
            {
                LPUART2_SendString((char *)sendBuf);
                PRINTF("%s",sendBuf);
                free(sendBuf);
                sendBuf = NULL;
            }
        }
#ifdef BLE_VERSION
        else if(pdTRUE == xReturn && ble_event == EVT_TIMTOUT) { //�����������ݳ�ʱ
			g_puart2StartRx = false;
			
			uint8_t id = 100;
			if(g_lpuart2RxBuf[7] == ',' && g_lpuart2RxBuf[6] >= '0' && g_lpuart2RxBuf[6] <= '9'){
				id = 100 + (g_lpuart2RxBuf[6]-0x30);
			}else if(g_lpuart2RxBuf[6] >= '0' && g_lpuart2RxBuf[6] <= '9' && 
			         g_lpuart2RxBuf[7] >= '0' && g_lpuart2RxBuf[7] <= '9'){
				id = 100 + (g_lpuart2RxBuf[6] - 0x30) * 10 + (g_lpuart2RxBuf[7] - 0x30);
			}
			/*����cjson��ʽ�Ļظ���Ϣ*/
			cJSON *pJsonRoot = cJSON_CreateObject();
			cJSON_AddNumberToObject(pJsonRoot, "Id", id);
			char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
			LPUART2_SendString(p_reply);
			PRINTF("%s", p_reply);
			
			cJSON_Delete(pJsonRoot);
			free(p_reply);
			p_reply = NULL;
        }
#endif
        //��ս��ܵ�������
        memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
        g_puart2RxCnt = 0;

        /* �ж���������״̬*/
		if(g_sys_para.BleWifiLedStatus != BLE_UPDATE){
			if(!BLE_WIFI_STATUS()) { //Disconnected
				g_sys_para.BleWifiLedStatus = BLE_READY;
			} else { 
				g_sys_para.BleWifiLedStatus = BLE_CONNECT;//Connected
			}
		}
    }
}


/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void LPUART2_TimeTick(void)
{
//	extern volatile uint32_t g_eventTimeMilliseconds;
//	g_eventTimeMilliseconds++;
	
    if(g_puart2StartRx)
    {
        g_puart2RxTimeCnt++;
		if(g_sys_para.BleWifiLedStatus == BLE_UPDATE){
			if(g_puart2RxTimeCnt >= 1000 ){
				g_puart2RxTimeCnt = 0;
				PRINTF("\n�������ݳ�ʱ,��ǰ����%d������\n", g_puart2RxCnt);
				for(uint8_t i = 0;i<g_puart2RxCnt; i++){
					PRINTF("%02x ",g_lpuart2RxBuf[i]);
				}
				xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
			}
		}
		else if(g_puart2RxTimeCnt >= 30) { //30msδ���ܵ�����,��ʾ�������ݳ�ʱ
			g_puart2RxTimeCnt = 0;
			g_puart2StartRx = false;
			xTaskNotify(BLE_TaskHandle, EVT_TIMTOUT, eSetBits);
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
    if ( LPUART_GetStatusFlags(LPUART2) & kLPUART_RxDataRegFullFlag )
    {
        /*��ȡ����*/
        ucTemp = LPUART_ReadByte(LPUART2);
		g_puart2StartRx = true;
		g_puart2RxTimeCnt = 0;
		g_sys_para.inactiveCount = 0;/* ���ܵ��������ݾ���0������*/
		if(g_puart2RxCnt < LPUART2_BUFF_LEN) {
			/* �����ܵ������ݱ��浽����*/
			g_lpuart2RxBuf[g_puart2RxCnt++] = ucTemp;
		}
		
		if(g_sys_para.BleWifiLedStatus != BLE_UPDATE && g_lpuart2RxBuf[g_puart2RxCnt-1] == '}'){
			/* �������,�ñ�־��0*/
			g_puart2StartRx = false;
			xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
		}else if (g_sys_para.BleWifiLedStatus==BLE_UPDATE && g_puart2RxCnt >= FIRM_ONE_PACKE_LEN){
			/* �������,�ñ�־��0*/
			g_puart2StartRx = false;
			xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
		}
    }
	else if ( (LPUART_GetStatusFlags(LPUART2) & kLPUART_RxOverrunFlag)||
		      (LPUART_GetStatusFlags(LPUART2) & kLPUART_FramingErrorFlag) ){
				  
		LPUART_ClearStatusFlags(LPUART2,kLPUART_RxOverrunFlag);
	    LPUART_ClearStatusFlags(LPUART2,kLPUART_FramingErrorFlag);
		LPUART_ClearStatusFlags(LPUART2,kLPUART_NoiseErrorFlag);
		LPUART_ClearStatusFlags(LPUART2,kLPUART_ParityErrorFlag);
	}
    __DSB();
}


