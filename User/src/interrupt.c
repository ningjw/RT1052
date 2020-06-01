#include "main.h"



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
void GPIO2_Combined_16_31_IRQHandler(void)
{
	/* ����жϱ�־λ */
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
