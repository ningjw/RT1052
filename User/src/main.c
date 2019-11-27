#include "main.h"

int main(void)
{
    BOARD_BootClockRUN();
    BOARD_InitBootPins();
    BOARD_InitPeripherals();
    SysTick_Config(SystemCoreClock / 1000);//1ms ÷–∂œ“ª¥Œ
    
}

