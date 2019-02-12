#include "main.h"
#include "usb_ehci.h"
#include "usb_util.h"
/*******************************************************************************
 * Variables
 ******************************************************************************/

__attribute__((aligned(4096)))  uint8_t               s_UsbHostEhciFrameList1[FRAME_LIST_SIZE * 4];
__attribute__((aligned(64)))    usb_host_ehci_data_t  EhciData;

uint8_t USB1_EventType = 0;
usb_host_device_instance_t     deviceInstance;
usb_descriptor_device_t        deviceDescriptor; /*!< Standard device descriptor */
usb_descriptor_configuration_t devConfig;
uint8_t enmuBuffer[64];

//usb_host_pipe_init_t           pipeInit;
usb_host_transfer_t            transfer;

/*!
 * @brief ehci phy initialization.
 */
static uint32_t USB_EhciPhyInit(void)
{
    USBPHY_Type *usbPhyBase = (USBPHY_Type *)USBPHY1_BASE;
    
    // Do not check whether a charger is connected to the USB port.
    // Disable the charger detector.
    USB_ANALOG->INSTANCE[0].CHRG_DETECT_SET = USB_ANALOG_CHRG_DETECT_CHK_CHRG_B(1) | USB_ANALOG_CHRG_DETECT_EN_B(1);
    
    usbPhyBase->CTRL |= USBPHY_CTRL_SET_ENUTMILEVEL2_MASK; /* support LS device. bit14*/
    usbPhyBase->CTRL |= USBPHY_CTRL_SET_ENUTMILEVEL3_MASK; /* support external FS Hub with LS device connected. bit15*/
    
    /* PWD register provides overall control of the PHY power state */
    usbPhyBase->PWD = 0U;
    
    /* Decode to trim the nominal 17.78mA current source for the High Speed TX drivers on USB_DP and USB_DM. */
    usbPhyBase->TX =
        ((usbPhyBase->TX & (~(USBPHY_TX_D_CAL_MASK | USBPHY_TX_TXCAL45DM_MASK | USBPHY_TX_TXCAL45DP_MASK))) |//对应bit位清零
         (USBPHY_TX_D_CAL(0x0CU) | USBPHY_TX_TXCAL45DP(0x06U) | USBPHY_TX_TXCAL45DM(0x06U)));

    return kStatus_USB_Success;
}


void USB_HostAppInit(void)
{
    //初始化全局变量
    ehciIpBase = (USBHS_Type *)USB1;
    deviceInstance.deviceAttached = kEHCIDeviceDetached;
    
    /* initialize first qh */
    for (uint8_t index = 1; index < MAX_QH; ++index)
    {
        EhciData.ehciQh[index - 1].horizontalLinkPointer = (uint32_t)&EhciData.ehciQh[index];
    }
    EhciData.ehciQh[MAX_QH - 1].horizontalLinkPointer = (uint32_t)NULL;
    EhciData.ehciQh[0].horizontalLinkPointer = (uint32_t)&EhciData.ehciQh[0] | EHCI_HOST_POINTER_TYPE_QH;
    EhciData.ehciQh[0].currentQtdPointer = EHCI_HOST_T_INVALID_VALUE;//bit4-bit0:Reserved
    EhciData.ehciQh[0].nextQtdPointer = EHCI_HOST_T_INVALID_VALUE;
    EhciData.ehciQh[0].alternateNextQtdPointer = EHCI_HOST_T_INVALID_VALUE;
    EhciData.ehciQh[0].staticEndpointStates[0] = (1 << EHCI_HOST_QH_H_SHIFT);
    
    /* initialize qtd list */
    for (uint8_t index = 1; index < MAX_QTD; ++index)
    {
        EhciData.ehciQtd[index - 1].nextQtdPointer = (uint32_t)(&EhciData.ehciQtd[index]);
    }
    EhciData.ehciQtd[MAX_QTD - 1].nextQtdPointer = (uint32_t)NULL;
    
    /* initialize pipe*/
    for (uint8_t index = 1; index < MAX_PIPES; ++index)
    {
        EhciData.ehciPipe[index - 1].next = &EhciData.ehciPipe[index];
    }
    
    /* initialize periodic list */
    for (int32_t index = 0; index < FRAME_LIST_SIZE; ++index)
    {
        s_UsbHostEhciFrameList1[index*4] = 1;
    }
    
    //初始化时钟为480M
    CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
    CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, 480000000U);
    
    //USB初始化
    USB_EhciPhyInit();
    USB_HostEhciStartIP();
    
    //配置并开启USB1中断
    NVIC_SetPriority(USB_OTG1_IRQn, USB_HOST_INTERRUPT_PRIORITY);
    EnableIRQ(USB_OTG1_IRQn);
    
    USB1_EventType |= EHCI_TASK_EVENT_PORT_CHANGE;
    usb_echo("host init done\r\n");
}


void USB_HostEhciIsrFunc(void)
{
    //处理中断
    if(USB1_EventType)
    {
        if (USB1_EventType & EHCI_TASK_EVENT_PORT_CHANGE) /* port state change */
        {
            USB_HostEhciPortStaChange();
        }
        
        if (USB1_EventType & EHCI_TASK_EVENT_TIMER0) /* timer0 */
        {
            
        }
        
        if (deviceInstance.deviceAttached == kEHCIDeviceAttached)
        {
            if (USB1_EventType & EHCI_TASK_EVENT_TRANSACTION_DONE) /* transaction done */
            {
                USB1_EventType &= ~EHCI_TASK_EVENT_TRANSACTION_DONE;
                USB_HostEhciTransactionDone();
            }

            if (USB1_EventType & EHCI_TASK_EVENT_DEVICE_DETACH) /* device detach */
            {
                /* disable attach, enable when the detach process is done */
                ehciIpBase->USBINTR &= (~USBHS_USBINTR_PCE_MASK); 
                deviceInstance.deviceAttached = kEHCIDeviceDetached;
            }
        }
        else if(deviceInstance.deviceAttached != kEHCIDeviceAttached)
        {
            if (USB1_EventType & EHCI_TASK_EVENT_DEVICE_ATTACH) /* device is attached */
            {
                USB_HostEhciStartAsync();
                USB_HostEhciStartPeriodic();
                //进行一系列的初始化
                if (USB_HostAttachDevice(deviceInstance.speed, 0, 1) == kStatus_USB_Success)
                {
                    deviceInstance.deviceAttached = kEHCIDeviceAttached;
                }
            }
        }
    }
}

void USB_OTG1_IRQHandler(void)
{
    static uint32_t interruptStatus = 0;
    
    interruptStatus = ehciIpBase->USBSTS;
    interruptStatus &= ehciIpBase->USBINTR;
    while (interruptStatus) /* there are usb interrupts */
    {
        ehciIpBase->USBSTS = interruptStatus; /* clear interrupt */
        
        if ((interruptStatus & USBHS_USBSTS_UI_MASK) ||
            (interruptStatus & USBHS_USBSTS_UEI_MASK)) /* USB interrupt or USB error interrupt */
        {
            USB1_EventType |= EHCI_TASK_EVENT_TRANSACTION_DONE;
        }
        
        if (interruptStatus & USBHS_USBSTS_PCI_MASK) /* port change detect interrupt */
        {
            USB1_EventType |= EHCI_TASK_EVENT_PORT_CHANGE;
        }

        if (interruptStatus & USBHS_USBSTS_TI0_MASK) /* timer 0 interrupt */
        {
            USB1_EventType |= EHCI_TASK_EVENT_TIMER0;
        }
        
        if (interruptStatus & USBHS_USBSTS_SRI_MASK) /* Start Of Frame Interrupt */
        {
        }

        if (interruptStatus & USBHS_USBSTS_SEI_MASK) /* system error interrupt */
        {
        }
        interruptStatus = ehciIpBase->USBSTS;
        interruptStatus &= ehciIpBase->USBINTR;
    }
}


