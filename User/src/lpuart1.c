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



