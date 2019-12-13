#include "main.h"

#define LPUART2_BUFF_LEN 64

SendMsgFunc_t SendMsgCallback;
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_lpuart2RxBuf[LPUART2_BUFF_LEN]) = {0};            //���ڽ��ջ�����
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_lpuart2TxBuf[LPUART2_BUFF_LEN]) = {0};            //���ڷ��ͻ�����
lpuart_transfer_t receiveXfer;

TaskHandle_t      BLE_TaskHandle = NULL;//����������
SemaphoreHandle_t AckBufMux = NULL; //���ڽ��ջ����������ź���
SemaphoreHandle_t RecvAckSem = NULL;//�����յ�ATָ�������ź��������ڴ��ڽ��տ��к󷢳�
TimerHandle_t     Lpuart2Tmr = NULL;//�����ʱ�����,����ATָ����ܳ�ʱ

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
        p_at_config��AT����
* �����ִ�н������
******************************************************************/
uint8_t AT_SendCmd(const char *send_str,const char *recv_str,stcATConfig *p_at_config)
{
    return 0;
}
 
/*****************************************************************
* ���ܣ�������������
* ����: send_buf:���͵�����
		buf_len�����鳤��
		recv_str���ڴ������а��������ַ���
        p_at_config��AT����
* �����ִ�н������
******************************************************************/
uint8_t AT_SendData(const char *send_buf,const uint16_t buf_len,const char *recv_str,stcATConfig *p_at_config)
{
    return 0;
}

/***************************************************************************************
  * @brief   ��ʱ������
  * @input   
  * @return  
***************************************************************************************/
void SoftTmr_Callback(void* parameter)
{

}

uint8_t flag_rev_data = 0;


/***********************************************************************
  * @ ������  �� BLE_AppTask
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void BLE_AppTask(void)
{
    /*ʹ�ܿ����ж�*/
	LPUART_EnableInterrupts(LPUART2, kLPUART_IdleLineInterruptEnable);
	/*ʹ�ܴ����ж�**/
	EnableIRQ(LPUART2_IRQn);
    
    RecvAckSem = xSemaphoreCreateBinary();      //���� ��ֵ �ź��� 
    
    AckBufMux = xSemaphoreCreateMutex();      //���� �����ź���
//    xSemaphoreGive(AckBufMux);              //�ͷŻ�����
//    xSemaphoreTake(AckBufMux,portMAX_DELAY);//��ȡ������
    
    //���������ʱ��������һ��Ϊ����ʱ�����ơ���ʱ���ڡ�����ģʽ(����)��Ψһid���ص�����
    Lpuart2Tmr = xTimerCreate("BleTimer", 100, pdFALSE,(void*)1, (TimerCallbackFunction_t)SoftTmr_Callback);
    if (Lpuart2Tmr != NULL) {
       xTimerStart(Lpuart2Tmr, 0); // �������ڶ�ʱ��
    }
   
    receiveXfer.data     = g_lpuart2RxBuf;
    receiveXfer.dataSize = LPUART2_BUFF_LEN;

    LPUART_ReceiveEDMA(LPUART2, &LPUART2_eDMA_Handle, &receiveXfer);  //ʹ��eDMA����
    
    PRINTF("BLE Task Create and Running\r\n");
    
    while(1)
    {
        xSemaphoreTake(RecvAckSem, portMAX_DELAY);//��ȡ�ź���
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
   /*	���յ��������˴����ж�	*/
    if ((kLPUART_IdleLineFlag) & LPUART_GetStatusFlags(LPUART2))
    {
		/*��������ж�*/
		LPUART2->STAT |= LPUART_STAT_IDLE_MASK; 
        
		/*����eDMA��������*/
		LPUART_TransferGetReceiveCountEDMA(LPUART2, &LPUART2_eDMA_Handle, &receiveXfer.dataSize); 
		LPUART_TransferAbortReceiveEDMA(LPUART2, &LPUART2_eDMA_Handle);   //eDMA��ֹ��������
		LPUART_ReceiveEDMA(LPUART2, &LPUART2_eDMA_Handle, &receiveXfer);  //ʹ��eDMA����
        xSemaphoreGive( RecvAckSem );
    }
    __DSB();
}


