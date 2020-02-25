#include "main.h"

#define LPUART3_BUFF_LEN 32
uint8_t g_lpuart3RxBuf[LPUART3_BUFF_LEN] = {0};            //串口接收缓冲区

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

    /*串口接收到数据*/
    if (kLPUART_RxDataRegFullFlag & LPUART_GetStatusFlags(LPUART1) ) {
        /*读取数据*/
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
    /*串口接收到数据*/
    if ((kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(LPUART4))
    {
        /*读取数据*/
        ucTemp = LPUART_ReadByte(LPUART4);

        /*将读取到的数据写入到缓冲区*/
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
    /*串口接收到数据*/
    if ((kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(LPUART5))
    {
        /*读取数据*/
        ucTemp = LPUART_ReadByte(LPUART5);

        /*将读取到的数据写入到缓冲区*/
        Uart_SendByte(LPUART5, ucTemp);
    }
    __DSB();
}
