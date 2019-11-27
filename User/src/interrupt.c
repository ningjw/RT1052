#include "interrupt.h"


/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void CORE_BOARD_WAUP_KEY_IRQHandler(void)
{
    /* 清除中断标志位 */
    GPIO_PortClearInterruptFlags(BOARD_INITPINS_CORE_BOARD_WAUP_KEY_PORT,1U << BOARD_INITPINS_CORE_BOARD_WAUP_KEY_PIN);
    
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void CORE_BOARD_MODE_KEY_IRQHandler(void)
{
    /* 清除中断标志位 */
    GPIO_PortClearInterruptFlags(BOARD_INITPINS_CORE_BOARD_MODE_KEY_PORT,1U << BOARD_INITPINS_CORE_BOARD_MODE_KEY_PIN);
    
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}


/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void PIT_IRQHandler(void)
{
    /* 清除中断标志位.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    
    
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

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void LPUART2_IRQHandler(void)
{
    uint8_t ucCh = ucCh;
    
    /*串口接收到数据*/
    if (kLPUART_RxDataRegFullFlag & LPUART_GetStatusFlags(LPUART2) ) {
        
        /*读取数据*/
        ucCh = LPUART_ReadByte( LPUART2 );
        
    }
}

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void LPUART3_IRQHandler(void)
{
    uint8_t ucCh = ucCh;
    
    /*串口接收到数据*/
    if (kLPUART_RxDataRegFullFlag & LPUART_GetStatusFlags(LPUART3) ) {
        
        /*读取数据*/
        ucCh = LPUART_ReadByte( LPUART3 );
        
    }
}
