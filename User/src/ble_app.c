#include "main.h"


#define DEVICE_BLE_NAME "BLE Communication"
#define SET_COMMOND_MODE()       GPIO_PinWrite(BOARD_BTM_MODE_GPIO, BOARD_BTM_MODE_PIN, 1);
#define SET_THROUGHPUT_MODE()    GPIO_PinWrite(BOARD_BTM_MODE_GPIO, BOARD_BTM_MODE_PIN, 0);
#define BLE_POWER_ON()           GPIO_PinWrite(BOARD_PWR_WIFI_BLE_GPIO, BOARD_PWR_WIFI_BLE_PIN, 1);
#define BLE_RESET()              GPIO_PinWrite(BOARD_PWR_WIFI_BLE_GPIO, BOARD_PWR_WIFI_BLE_PIN, 0);
#define EVT_OK       (1 << 0)//���ܵ������¼�



AT_NONCACHEABLE_SECTION_INIT(uint8_t g_lpuart2RxBuf[LPUART2_BUFF_LEN]) = {0};            //���ڽ��ջ�����

uint8_t g_puart2RxCnt = 0;
uint8_t g_puart2TxCnt = 0;
uint8_t g_puart2StartRx = 0;

TaskHandle_t        BLE_TaskHandle = NULL;//����������
EventGroupHandle_t  RecvAckEvt = NULL;//�����յ�ATָ�������ź��������ڴ��ڽ��տ��к󷢳�

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
    xEventGroupClearBits(RecvAckEvt, EVT_OK);//��������ǰ��������¼�
    LPUART2_SendString(send_str);//����ATָ��
    
    if (NULL == recv_str ) {
        return true;
    }
    
    /*  �ȴ������¼���־, */
    at_event = xEventGroupWaitBits(RecvAckEvt,  /*  �¼������� */
                                EVT_OK,      /*  ���� ���� ����Ȥ���¼� */
                                pdFALSE,     /*  �˳�������¼�λ */
                                pdFALSE,     /*  �߼���ȴ��¼� */
                                p_at_cfg->resp_time);/*  ָ����ʱʱ��*/
    
    if( (at_event & EVT_OK) == EVT_OK ){
        xEventGroupClearBits(RecvAckEvt, EVT_OK);//����¼�
        //���յ��������а�����Ӧ������
        if(strstr((char *)g_lpuart2RxBuf, recv_str) != NULL){
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

/*****************************************************************
* ���ܣ�������������
* ����: send_buf:���͵�����
		buf_len�����鳤��
		recv_str���ڴ������а��������ַ���
        p_at_cfg��AT����
* �����ִ�н������
******************************************************************/
uint8_t AT_SendData(const char *send_buf, uint8_t buf_len, const char *recv_str, ATCfg_t *p_at_cfg)
{
    memset(send_str, sizeof(send_str), 0);
    
    sprintf(send_str, "AT+LESEND=%d,", buf_len);
    
    strcat(send_str,send_buf);
    
    LPUART2_SendString(send_str);//��������
    
    return 0;
}


char set_rtc[] = "{\"Id\":1,\"Sid\":0,\"Y\":2019,\"Mon\":12,\"D\":16,\"H\":9,\"Min\":0,\"S\":0}";
char get_rtc[] = "{\"Id\":2,\"Sid\":0}";
/***********************************************************************
  * @ ������  �� BLE_AppTask
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void BLE_AppTask(void)
{
    uint8_t ret = false;

    RecvAckEvt = xEventGroupCreate();/*  ���� �¼��� */
    
    PRINTF("BLE Task Create and Running\r\n");
    
    ret = AT_SendCmd(BT_NAME, DEVICE_BLE_NAME, RESP_OK, &g_at_cfg);//������������
    
    if( ret == true ){
        g_sys_para2.bleLedStatus = BLE_READY;
    }
    SET_THROUGHPUT_MODE();//����͸��ģʽ
    
    while(1)
    {
        /*  �ȴ������¼���־ */
        xEventGroupWaitBits(RecvAckEvt,  /*  �¼������� */
                            EVT_OK,   /*  ���� ���� ����Ȥ���¼� */
                            pdTRUE, /*  �˳�ʱ����¼�λ */
                            pdTRUE, /*  �������Ȥ�������¼� */
                            portMAX_DELAY);/*  ָ����ʱ�¼�, һֱ�� */
        
//        LPUART2_SendString((char *)g_lpuart2RxBuf);
        ParseProtocol(g_lpuart2RxBuf);//������������Э��
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
    if ((kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(LPUART2))
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
    QTMR_ClearStatusFlags(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL, kQTMR_CompareFlag);//���жϱ�־
    QTMR_StopTimer(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL);//ֹͣ������
    xEventGroupSetBitsFromISR(RecvAckEvt, EVT_OK, &xHigherPriorityTaskWoken); /*�����¼� */
}


