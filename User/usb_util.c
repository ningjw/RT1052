#include "main.h"


USBHS_Type* ehciIpBase; /*!< EHCI IP base address*/

usb_status_t USB_HostEhciStartIP(void)
{
    uint32_t tmp = 0;
    
    /* reset controller */
    ehciIpBase->USBCMD = USBHS_USBCMD_RST_MASK;
    //This bit is set to zero by the Host/Device Controller when the reset process is complete
    while (ehciIpBase->USBCMD & USBHS_USBCMD_RST_MASK)
    {
    }
    /* set host mode Host*/
    ehciIpBase->USBMODE |= 0x03;
    
    if (ehciIpBase->HCSPARAMS & USBHS_HCSPARAMS_PPC_MASK) /* Ports have power port switches */
    {
        /* only has one port */
        tmp = ehciIpBase->PORTSC1 & (~EHCI_PORTSC1_W1_BITS);
        ehciIpBase->PORTSC1 = (tmp | USBHS_PORTSC1_PP_MASK); /* turn on port power */
    }

    /* set frame list size : The frame list must always be aligned on a 4K-page boundary */
    if (ehciIpBase->HCCPARAMS & USBHS_HCCPARAMS_PFL_MASK)
    {
        ehciIpBase->USBCMD |= (0x00 << USBHS_USBCMD_FS_SHIFT);
    }

    /* start the controller */
    ehciIpBase->USBCMD = USBHS_USBCMD_RS_MASK;
    
    /* set timer0 */
    ehciIpBase->GPTIMER0LD = (100 * 1000 - 1); /* 100ms */
    
    /* enable interrupt (USB interrupt enable + USB error interrupt enable + port change detect enable + system error
     * enable + interrupt on async advance enable) + general purpos Timer 0 Interrupt enable */
    ehciIpBase->USBINTR |= (0x1000037);

    return kStatus_USB_Success;
}


void USB_HostEhciStartAsync(void)
{
    uint32_t stateSync;
    
    if (!(ehciIpBase->USBSTS & USBHS_USBSTS_AS_MASK))
    {    //USBCMD->ASE = 1 :Use the ASYNCLISTADDR register to access the Asynchronous Schedule.
        /* the status must be same when change USBCMD->ASE */
        do
        {
            stateSync = ((ehciIpBase->USBSTS & USBHS_USBSTS_AS_MASK) |
                         (ehciIpBase->USBCMD & USBHS_USBCMD_ASE_MASK));
        } while ((stateSync == USBHS_USBSTS_AS_MASK) || (stateSync == USBHS_USBCMD_ASE_MASK));

        ehciIpBase->ASYNCLISTADDR = (uint32_t)(&EhciData.ehciQh[0]);
        ehciIpBase->USBCMD |= USBHS_USBCMD_ASE_MASK;
        while (!(ehciIpBase->USBSTS & USBHS_USBSTS_AS_MASK))
        {
        }
    }
}

void USB_HostEhciStopAsync(void)
{
    uint32_t stateSync;

    /* the status must be same when change USBCMD->ASE */
    do
    {
        stateSync = ((ehciIpBase->USBSTS & USBHS_USBSTS_AS_MASK) | (ehciIpBase->USBCMD & USBHS_USBCMD_ASE_MASK));
    } while ((stateSync == USBHS_USBSTS_AS_MASK) || (stateSync == USBHS_USBCMD_ASE_MASK));

    ehciIpBase->USBCMD &= (uint32_t)(~(uint32_t)USBHS_USBCMD_ASE_MASK); /* disable async schedule */
    while (ehciIpBase->USBSTS & USBHS_USBSTS_AS_MASK)
    {
    }
}

void USB_HostEhciDelay(USBHS_Type *ehciIpBase, uint32_t ms)
{
    /* note: the max delay time cannot exceed half of max value (0x4000) */
    int32_t sofStart;
    int32_t SofEnd;
    uint32_t distance;
    //FRINDEX register update every 125us
    sofStart = (int32_t)(ehciIpBase->FRINDEX & EHCI_MAX_UFRAME_VALUE);
    do
    {
        SofEnd = (int32_t)(ehciIpBase->FRINDEX & EHCI_MAX_UFRAME_VALUE);
        distance = (uint32_t)(SofEnd - sofStart + EHCI_MAX_UFRAME_VALUE + 1);
    } while ((distance & EHCI_MAX_UFRAME_VALUE) < (ms * 8)); /* compute the distance between sofStart and SofEnd */
}

uint8_t USB_HostEhciGet2PowerValue(uint8_t value)
{
    if ((value == 0) || (value == 1))
    {
        return value;
    }
    if (value & 0xf0)
    {
        if (value & 0x80)
        {
            return 128;
        }
        else if (value & 0x40)
        {
            return 64;
        }
        else if (value & 0x20)
        {
            return 32;
        }
        else
        {
            return 16;
        }
    }
    else
    {
        if (value & 0x08)
        {
            return 8;
        }
        else if (value & 0x04)
        {
            return 4;
        }
        else if (value & 0x02)
        {
            return 2;
        }
        else
        {
            return 1;
        }
    }
}

void USB_HostEhciStartPeriodic(void)
{
    uint32_t stateSync;

    if (!(ehciIpBase->USBSTS & USBHS_USBSTS_PS_MASK))
    {
        /* the status must be same when change USBCMD->PSE */
        do
        {
            stateSync = ((ehciIpBase->USBSTS & USBHS_USBSTS_PS_MASK) |
                         (ehciIpBase->USBCMD & USBHS_USBCMD_PSE_MASK));
        } while ((stateSync == USBHS_USBSTS_PS_MASK) || (stateSync == USBHS_USBCMD_PSE_MASK));
        ehciIpBase->PERIODICLISTBASE = (uint32_t)(&s_UsbHostEhciFrameList1);
        if (!(ehciIpBase->USBCMD & USBHS_USBCMD_PSE_MASK))
        {
            ehciIpBase->USBCMD |= USBHS_USBCMD_PSE_MASK; /* start periodic schedule */
        }
        while (!(ehciIpBase->USBSTS & USBHS_USBSTS_PS_MASK))
        {
        }
    }
    return;
}

void USB_HostEhciStopPeriodic(void)
{
    uint32_t stateSync;

    /* the status must be same when change USBCMD->PSE */
    do
    {
        stateSync = ((ehciIpBase->USBSTS & USBHS_USBSTS_PS_MASK) | (ehciIpBase->USBCMD & USBHS_USBCMD_PSE_MASK));
    } while ((stateSync == USBHS_USBSTS_PS_MASK) || (stateSync == USBHS_USBCMD_PSE_MASK));

    ehciIpBase->USBCMD &= (~USBHS_USBCMD_PSE_MASK); /* stop periodic schedule */
    while (ehciIpBase->USBSTS & USBHS_USBSTS_PS_MASK)
    {
    }
}


