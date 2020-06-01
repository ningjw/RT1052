#include "main.h"



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
void GPIO2_Combined_16_31_IRQHandler(void)
{
	/* 清除中断标志位 */
	GPIO_PortClearInterruptFlags(BOARD_KEY_OFF_GPIO, 1<<BOARD_KEY_OFF_PIN);
	g_sys_para.inactiveCount = 0;
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
