#include "main.h"

typedef struct
{
    uint16_t resp_time100ms;  //���ͺ��ѯ������Ϣ����ʱ��100msΪ��λ������Ϊָ�������Ӧʱ�䡣
    uint8_t  try_delay1ms;    //����ʧ�ܺ��ٴη���ʱ����ʱ��1msΪ��λ
    uint8_t  max_try_times;   //������Դ���
    uint8_t  max_reset_times; //�����������
}stcATConfig;

typedef void (*SendMsgFunc_t)(uint8_t * buf, uint32_t len);
SendMsgFunc_t SendMsgCallback;
char atAckBuf[64] = {0};            //���ڽ��ջ�����
SemaphoreHandle_t AckBufMux = NULL; //���ڽ��ջ����������ź���
SemaphoreHandle_t RecvAckSem = NULL;//�����յ�ATָ�������ź��������ڴ��ڽ��տ��к󷢳�

TaskHandle_t AppBLE_TaskHandle = NULL;  /* ���������� */


/***************************************************************************************
  * @brief   ����һ���ַ��� 
  * @input   base:ѡ��˿�; data:��Ҫ���͵�����
  * @return  
***************************************************************************************/
void Uart_SendString( LPUART_Type *base,  const char *str)
{
    LPUART_WriteBlocking( base, (const uint8_t*)str, strlen(str));
}


/***************************************************************************************
  * @brief   �ڷ���ATָ��֮ǰ����Ҫ��ע��ATָ������л���
  * @input   
  * @return  
***************************************************************************************/
void AT_RegisterHandler(SendMsgFunc_t func)
{
	SendMsgCallback = func;                       //���ڷ������ݵĺ���            
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
    
}

/*****************************************************************
* ���ܣ���ѯATָ��Ļ������Ƿ�����Ҫ���ַ���
* ����: recv_str:�ڴ�����ַ�������Ҫ���е����ַ�������"OK\r\n"
        max_resp_time��ָ�������Ӧʱ�䣬��λ100ms
* ��������ҵ������ַ���ָ��
******************************************************************/
char* AT_SearchRecvBuf(const char* recv_str,uint16_t max_resp_time)
{
    
}
/******************************************
* ���ܣ���մ��ڽ��ջ�����
******************************************/
void AT_ClearAckBuff(void)
{
    
}

/***********************************************************************
  * @ ������  �� AppBLE_Task
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void AppBLE_Task(void)
{
    RecvAckSem = xSemaphoreCreateBinary();      //���� ��ֵ �ź��� 
//    xSemaphoreGive( RecvAckSem );             //�����ź���
//    xSemaphoreTake(RecvAckSem, portMAX_DELAY);//��ȡ�ź���
    
    AckBufMux = xSemaphoreCreateMutex();      //���� �����ź���
//    xSemaphoreGive(AckBufMux);              //�ͷŻ�����
//    xSemaphoreTake(AckBufMux,portMAX_DELAY);//��ȡ������
    
    stcATConfig ATCfg;
    ATCfg.resp_time100ms = 1;//�����Ӧʱ��Ϊ100ms
    ATCfg.try_delay1ms   = 100;//��Ӧʧ�ܺ��ٴη���ʱ��ʱ100ms
    ATCfg.max_try_times  = 3; ////������Դ���:3
    ATCfg.max_reset_times =1; //�������������1
    
    while(1)
    {
        /* ��ȡ���� */
        SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
        /* ��ӡ����&ʱ�� */ 
        PRINTF("BLE TASK:%02d-%02d-%02d  %02d:%02d:%02d \r\n", rtcDate.year,rtcDate.month, rtcDate.day,rtcDate.hour, rtcDate.minute, rtcDate.second);
        vTaskDelay(1000);
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
    if ( (kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(LPUART2) )
    {
        /*��ȡ����*/
        ucTemp = LPUART_ReadByte(LPUART2);
        
        /*����ȡ��������д�뵽������*/
//        Uart_SendByte(LPUART2, ucTemp);
    }
    __DSB();
}





