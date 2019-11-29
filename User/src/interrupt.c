#include "interrupt.h"

uint32_t count = 0;

/***************************************************************************************
  * @brief   kPIT_Chnl_0配置为1ms中断 ；kPIT_Chnl_1 配置为1s中断
  * @input   
  * @return  
***************************************************************************************/
void PIT_IRQHandler(void)
{
    if ( PIT_GetStatusFlags(PIT, kPIT_Chnl_0) == true ){
        /* 清除中断标志位.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    }
    
    if( PIT_GetStatusFlags(PIT, kPIT_Chnl_1) == true ){
        /* 清除中断标志位.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag);
        
    }
    
    __DSB();
}


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
