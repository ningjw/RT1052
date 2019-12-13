#include "main.h"


/**
* @brief  ����һ���ַ�
* @param  base:ѡ��˿�
* @param  data:��Ҫ���͵�����
* @retval ��
*/
void Uart_SendByte(LPUART_Type *base, uint8_t data)
{
    LPUART_WriteByte( base, data);
    while (!(base->STAT & LPUART_STAT_TDRE_MASK));
}

/***************************************************************************************
  * @brief   kPIT_Chnl_0����Ϊ1ms�ж� ��kPIT_Chnl_1 ����Ϊ1s�ж�
  * @input
  * @return
***************************************************************************************/
void PIT_IRQHandler(void)
{
    if ( PIT_GetStatusFlags(PIT, kPIT_Chnl_0) == true ) {
        /* ����жϱ�־λ.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    }

    if( PIT_GetStatusFlags(PIT, kPIT_Chnl_1) == true ) {
        /* ����жϱ�־λ.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag);
        //����led����˸
        BOARD_LED_PORT->DR ^= (1 << BOARD_LED_PIN);
        
        /* ��ȡ���� */
        SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
        /* ��ӡ����&ʱ�� */ 
        PRINTF("%02d-%02d-%02d  %02d:%02d:%02d \r\n", 
               rtcDate.year,rtcDate.month, rtcDate.day,rtcDate.hour,
               rtcDate.minute, rtcDate.second);
    }

    __DSB();
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
