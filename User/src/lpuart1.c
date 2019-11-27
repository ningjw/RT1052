#include "main.h"

volatile bool rxBufferEmpty                                          = true;
volatile bool txBufferFull                                           = false;
volatile bool txOnGoing                                              = false;
volatile bool rxOnGoing                                              = false;

lpuart_transfer_t sendXfer;
lpuart_transfer_t receiveXfer;
    
/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void LPUART1_UserCallback(LPUART_Type *base, lpuart_edma_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_LPUART_TxIdle == status)
    {
        txBufferFull = false;
        txOnGoing    = false;
    }
    
    if (kStatus_LPUART_RxIdle == status)
    {
        rxBufferEmpty = false;
        rxOnGoing     = false;
    }
}


