#include "main.h"
#include "i2c_eeprom_drv.h"

int main(void)
{
    BOARD_BootClockRUN();
    BOARD_InitBootPins();
    BOARD_InitPeripherals();

    BOARD_InitDebugConsole();
    PRINTF("***** Welcome *****\r\n");
    
    while(1){

    }
}
