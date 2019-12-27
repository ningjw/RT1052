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
void LPUART3_IRQHandler(void)
{
    uint8_t ucTemp;
    /*���ڽ��յ�����*/
    if ((kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(LPUART3))
    {
        /*��ȡ����*/
        ucTemp = LPUART_ReadByte(LPUART3);

        /*����ȡ��������д�뵽������*/
        Uart_SendByte(LPUART3, ucTemp);
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
