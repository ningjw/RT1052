#include "main.h"

int main(void)
{
    MPU_Memory_Protection();
    RT1052_Clock_Init();	    //≈‰÷√œµÕ≥ ±÷”
    LPUART1_Init();
    USB_HostAppInit();
    while(1)
    {
        USB_HostEhciIsrFunc();
        if(deviceInstance.state == kStatus_DEV_EnumDone){
            USB_HostMsdCommandStart();
        }
//        USB_HostMsdTask(&g_MsdCommandInstance);
    }
}

