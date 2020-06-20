#include "main.h"

#define LPUART3_BUFF_LEN 10
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
//		LPUART_TransferGetReceiveCountEDMA(LPUART3, &LPUART3_eDMA_Handle, &uart3RevXfer.dataSize); 
//		LPUART_TransferAbortReceiveEDMA(LPUART3, &LPUART3_eDMA_Handle);   //eDMA终止接收数据
//		LPUART_ReceiveEDMA(LPUART3, &LPUART3_eDMA_Handle, &uart3RevXfer);  //使用eDMA接收
        
//        PRINTF("%s:\r\n",uart3RevXfer.data);
    }
    __DSB();
}

/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void GPIO2_Combined_16_31_IRQHandler(void)
{
	/* 清除中断标志位 */
	GPIO_PortClearInterruptFlags(BOARD_KEY_OFF_GPIO, 1<<BOARD_KEY_OFF_PIN);
	g_sys_para.inactiveCount = 0;
	
	/* Disable interrupt. */
//	GPIO_DisableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
//	GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
	LPM_DisableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
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
