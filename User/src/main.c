#include "main.h"

uint8_t txbuff[] = "¿ª»ú\r\n";
extern uint8_t rxData;
extern uint8_t rxFlag;
int main(void)
{
    BOARD_BootClockRUN();
    BOARD_InitBootPins();
    BOARD_InitPeripherals();

    LPUART_WriteBlocking(LPUART1, txbuff, sizeof(txbuff) - 1);
    
    while(1){

    }
}

