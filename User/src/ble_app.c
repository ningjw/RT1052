#include "main.h"

#define LPUART2_BUFF_LEN 64

#define EVT_OK       (1 << 0)//���ܵ������¼�


SendMsgFunc_t SendMsgCallback;
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_lpuart2RxBuf[LPUART2_BUFF_LEN]) = {0};            //���ڽ��ջ�����
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_lpuart2TxBuf[LPUART2_BUFF_LEN]) = {0};            //���ڷ��ͻ�����
lpuart_transfer_t receiveXfer;

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
        if(strstr((char *)receiveXfer.data, recv_str) != NULL){
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





/***********************************************************************
  * @ ������  �� BLE_AppTask
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void BLE_AppTask(void)
{
    uint8_t ret = false;
    /*ʹ�ܿ����ж�*/
	LPUART_EnableInterrupts(LPUART2, kLPUART_IdleLineInterruptEnable);
	/*ʹ�ܴ����ж�**/
	EnableIRQ(LPUART2_IRQn);
    
    RecvAckEvt = xEventGroupCreate();/*  ���� �¼��� */
    
    receiveXfer.data     = g_lpuart2RxBuf;
    receiveXfer.dataSize = LPUART2_BUFF_LEN;

    LPUART_ReceiveEDMA(LPUART2, &LPUART2_eDMA_Handle, &receiveXfer);  //ʹ��eDMA����
    
    PRINTF("BLE Task Create and Running\r\n");
    
    AT_SendCmd(BT_NAME, "NINGJW", RESP_OK, &g_at_cfg);//������������
    
    AT_SendCmd(BT_NAME, NULL, BT_NAME, &g_at_cfg);//��ȡ��������,�ж��Ƿ����óɹ�
    if( ret == false ){//��ʾ��������״ָ̬ʾ��.
        
    }
    while(1)
    {
        /*  �ȴ������¼���־ */
        xEventGroupWaitBits(RecvAckEvt,  /*  �¼������� */
                            EVT_OK,   /*  ���� ���� ����Ȥ���¼� */
                            pdTRUE, /*  �˳�ʱ����¼�λ */
                            pdTRUE, /*  �������Ȥ�������¼� */
                            portMAX_DELAY);/*  ָ����ʱ�¼�, һֱ�� */

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
   /*	���յ��������˴����ж�	*/
    if ((kLPUART_IdleLineFlag) & LPUART_GetStatusFlags(LPUART2))
    {
		/*��������ж�*/
		LPUART2->STAT |= LPUART_STAT_IDLE_MASK; 
        
		/*����eDMA��������*/
		LPUART_TransferGetReceiveCountEDMA(LPUART2, &LPUART2_eDMA_Handle, &receiveXfer.dataSize); 
		LPUART_TransferAbortReceiveEDMA(LPUART2, &LPUART2_eDMA_Handle);   //eDMA��ֹ��������
		LPUART_ReceiveEDMA(LPUART2, &LPUART2_eDMA_Handle, &receiveXfer);  //ʹ��eDMA����
        g_sys_para2.inactiveCount = 0;//���ܵ��������ݣ����������
        xEventGroupSetBitsFromISR(RecvAckEvt, EVT_OK, &xHigherPriorityTaskWoken); /*�����¼� */
    }
    __DSB();
}


