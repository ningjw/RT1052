#include "interrupt.h"

uint32_t count = 0;

/***************************************************************************************
  * @brief   kPIT_Chnl_0����Ϊ1ms�ж� ��kPIT_Chnl_1 ����Ϊ1s�ж�
  * @input   
  * @return  
***************************************************************************************/
void PIT_IRQHandler(void)
{
    if ( PIT_GetStatusFlags(PIT, kPIT_Chnl_0) == true ){
        /* ����жϱ�־λ.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    }
    
    if( PIT_GetStatusFlags(PIT, kPIT_Chnl_1) == true ){
        /* ����жϱ�־λ.*/
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
    
    /*���ڽ��յ�����*/
    if (kLPUART_RxDataRegFullFlag & LPUART_GetStatusFlags(LPUART1) ) {
        /*��ȡ����*/
        ucCh = LPUART_ReadByte( LPUART1 );
    }
}
