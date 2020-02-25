#include "main.h"

#define LPUART3_BUFF_LEN 32
uint8_t g_lpuart3RxBuf[LPUART3_BUFF_LEN] = {0};            //���ڽ��ջ�����

lpuart_transfer_t uart3RevXfer = {
    .data     = g_lpuart3RxBuf,
    .dataSize = LPUART3_BUFF_LEN,
};

    

/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void LPUART1_IRQHandler(void)
{
    uint8_t ucCh = ucCh;

    /*���ڽ��յ�����*/
    if (kLPUART_RxDataRegFullFlag & LPUART_GetStatusFlags(LPUART1) ) {
        /*��ȡ����*/
        ucCh = LPUART_ReadByte( LPUART1 );
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
        if(strstr((char *)uart3RevXfer.data,"ls") != NULL){
            eMMC_ScanFile();
        }else if(strstr((char *)uart3RevXfer.data,"cat") != NULL){
            eMMC_PrintfManageFile();
        }else if(strstr((char *)uart3RevXfer.data,"free") != NULL){
            eMMC_GetFree();
        }else if(strstr((char *)uart3RevXfer.data,"del") != NULL){
            if(strstr((char *)uart3RevXfer.data,"earliest") != NULL){
                eMMC_DelEarliestFile();
            }else{
                f_unlink((char *)&uart3RevXfer.data[4]);
            }
        }else if(strstr((char *)uart3RevXfer.data,"add") != NULL){
            eMMC_AppendmanageFile((char *)&uart3RevXfer.data[4]);
        }
        
        uart3RevXfer.dataSize = LPUART3_BUFF_LEN;
        memset(g_lpuart3RxBuf, 0, LPUART3_BUFF_LEN);
    }
    __DSB();
}

/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void LPUART4_IRQHandler(void)
{
    uint8_t ucTemp;
    /*���ڽ��յ�����*/
    if ((kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(LPUART4))
    {
        /*��ȡ����*/
        ucTemp = LPUART_ReadByte(LPUART4);

        /*����ȡ��������д�뵽������*/
        Uart_SendByte(LPUART4, ucTemp);
    }
    __DSB();
}

/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void LPUART5_IRQHandler(void)
{
    uint8_t ucTemp;
    /*���ڽ��յ�����*/
    if ((kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(LPUART5))
    {
        /*��ȡ����*/
        ucTemp = LPUART_ReadByte(LPUART5);

        /*����ȡ��������д�뵽������*/
        Uart_SendByte(LPUART5, ucTemp);
    }
    __DSB();
}
