#include "main.h"


/**
* @brief  发送一个字符
* @param  base:选择端口
* @param  data:将要发送的数据
* @retval 无
*/
void Uart_SendByte(LPUART_Type *base, uint8_t data)
{
    LPUART_WriteByte( base, data);
    while (!(base->STAT & LPUART_STAT_TDRE_MASK));
}

/***************************************************************************************
  * @brief   kPIT_Chnl_0配置为1ms中断 ；kPIT_Chnl_1 配置为1s中断
  * @input
  * @return
***************************************************************************************/
void PIT_IRQHandler(void)
{
    if ( PIT_GetStatusFlags(PIT, kPIT_Chnl_0) == true ) {
        /* 清除中断标志位.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    }

    if( PIT_GetStatusFlags(PIT, kPIT_Chnl_1) == true ) {
        /* 清除中断标志位.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag);
        //控制led灯闪烁
        BOARD_LED_PORT->DR ^= (1 << BOARD_LED_PIN);
        
        /* 获取日期 */
        SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
        /* 打印日期&时间 */ 
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
    /*串口接收到数据*/
    if ((kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(LPUART3))
    {
        /*读取数据*/
        ucTemp = LPUART_ReadByte(LPUART3);

        /*将读取到的数据写入到缓冲区*/
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
