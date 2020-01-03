#include "main.h"


#define DEVICE_BLE_NAME "BLE Communication"
#define SET_COMMOND_MODE()       GPIO_PinWrite(BOARD_BTM_MODE_GPIO, BOARD_BTM_MODE_PIN, 1);
#define SET_THROUGHPUT_MODE()    GPIO_PinWrite(BOARD_BTM_MODE_GPIO, BOARD_BTM_MODE_PIN, 0);
#define BLE_POWER_ON()           GPIO_PinWrite(BOARD_PWR_WIFI_BLE_GPIO, BOARD_PWR_WIFI_BLE_PIN, 1);
#define BLE_RESET()              GPIO_PinWrite(BOARD_PWR_WIFI_BLE_GPIO, BOARD_PWR_WIFI_BLE_PIN, 0);

#define EVT_OK       (1 << 0)//���ܵ������¼�

extern void LPUART2_init(void);

char set_rtc[] = "{\"Id\":1,\"Sid\":0,\"Y\":2019,\"Mon\":12,\"D\":16,\"H\":9,\"Min\":0,\"S\":0}";
char get_rtc[] = "{\"Id\":2,\"Sid\":0}";

AT_NONCACHEABLE_SECTION_INIT(uint8_t g_lpuart2TxBuf[LPUART2_BUFF_LEN]) = {0};            //���ڷ��ͻ�����
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_lpuart2RxBuf[LPUART2_BUFF_LEN]) = {0};            //���ڽ��ջ�����

uint8_t g_puart2RxCnt = 0;
uint8_t g_puart2TxCnt = 0;
uint8_t g_puart2StartRx = 0;
uint32_t ble_event = 0;

TaskHandle_t        BLE_TaskHandle = NULL;//����������

static char send_str[164] = {0};
ATCfg_t g_at_cfg = {
    .resp_time = 2000,
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
* ���ܣ�����ATָ��
* ����: send_buf:���͵��ַ���
		recv_str���ڴ������а��������ַ���
        p_at_cfg��AT����
* �����ִ�н������
******************************************************************/
uint8_t AT_SendCmd(const char *cmd, const char *param, const char *recv_str, ATCfg_t *p_at_cfg)
{
    uint8_t at_ret;
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
    /*wait task notify*/
    at_ret = xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, p_at_cfg->resp_time);
    if( pdTRUE == at_ret ){
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
    }else{//�ظ���ʱ
        if(p_at_cfg->try_cnt++ > p_at_cfg->try_times){
            return false;
        }
        goto retry;
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
    xReturn = AT_SendCmd(BT_NAME, DEVICE_BLE_NAME, RESP_OK, &g_at_cfg);//������������
//    if( xReturn == true ){
        g_sys_para.bleLedStatus = BLE_READY;
//    }
    SET_THROUGHPUT_MODE();//����͸��ģʽ
    memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
    g_puart2RxCnt = 0;
    while(1)
    {
        /*wait task notify*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, portMAX_DELAY);
        if ( pdTRUE == xReturn ) {
            sendBuf = ParseProtocol(g_lpuart2RxBuf);//������������Э��
            if(g_lpuart2RxBuf[0] == 0xE7 && g_lpuart2RxBuf[1] == 0xE7){//�������ݰ�
                LPUART_WriteBlocking(LPUART2, sendBuf, 7);
                if( g_sys_para.firmUpdate == true){
                    //����������Nor Flash
                    NorFlash_SaveFirmPara();
                    //�ر������ж�,����λϵͳ
                    NVIC_SystemReset();
                }
            }else if(NULL != sendBuf){//json���ݰ�
                LPUART2_SendString((char *)sendBuf);
                free(sendBuf);
            }
        }
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
    /*���ڽ��յ�����*/
    if ((kLPUART_RxDataRegFullFlag) & LPUART_GetStatusFlags(LPUART2))
    {
         /*��ȡ����*/
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
    }else{
        g_puart2StartRx = 0;
        memset(g_lpuart2RxBuf, 0, LPUART2_BUFF_LEN);
        g_puart2RxCnt = 0;
        LPUART_Deinit(LPUART2);
        LPUART2_init();
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
    g_puart2StartRx = 0;
    if(g_sys_para.inactiveCondition == 1){
        g_sys_para.inactiveCount = 0;
    }
    QTMR_ClearStatusFlags(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL, kQTMR_CompareFlag);//���жϱ�־
    QTMR_StopTimer(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL);//ֹͣ������
    xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);/*�����¼� */
}


