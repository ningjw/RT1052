#include  "main.h"
#include "usb_ehci.h"
#include "usb_util.h"

usb_descriptor_interface_t *descInterface;
usb_descriptor_endpoint_t *descEndpoint1;
usb_descriptor_endpoint_t *descEndpoint2;
uint8_t logicalUnitNumber;


void USB_HostEhciPortStaChange(void)
{
    int32_t sofStart = 0;
    int32_t sofCount = 0;
    uint32_t index;

    if (ehciIpBase->PORTSC1 & USBHS_PORTSC1_CSC_MASK) /*port status change */
    {
        sofStart = (int32_t)(ehciIpBase->FRINDEX & EHCI_MAX_UFRAME_VALUE);
        /* process CSC bit */
        while (1)
        {
            if (ehciIpBase->PORTSC1 & USBHS_PORTSC1_CSC_MASK){
                /* clear csc bit */
                ehciIpBase->PORTSC1 &= (~EHCI_PORTSC1_W1_BITS);
                ehciIpBase->PORTSC1 |=  USBHS_PORTSC1_CSC_MASK;
            }
            sofCount = (int32_t)(ehciIpBase->FRINDEX & EHCI_MAX_UFRAME_VALUE);
             /* delay 1ms to clear CSC */
            if (((sofCount - sofStart + EHCI_MAX_UFRAME_VALUE + 1) & EHCI_MAX_UFRAME_VALUE) > (1 * 8)){
                break;
            }
        }
    }

    /* process CCS bit */
    if (ehciIpBase->PORTSC1 & USBHS_PORTSC1_CCS_MASK) /* process attach */
    {
        for (index = 0; index < USB_HOST_EHCI_PORT_CONNECT_DEBOUNCE_DELAY; ++index){//延时101ms,等待USB设备稳定.
            USB_HostEhciDelay(ehciIpBase, 1);//延时1ms
            if (!(ehciIpBase->PORTSC1 & USBHS_PORTSC1_CCS_MASK)) {//如果在延时过程中,检测到拔出USB,则跳出
                break;
            }
        }
        if (index < USB_HOST_EHCI_PORT_CONNECT_DEBOUNCE_DELAY) /* CCS is cleared 延时过程中,USB拔出*/
        {
            deviceInstance.deviceAttached = kEHCIDeviceDetached;
            return;
        }
        /* reset port */
        ehciIpBase->PORTSC1 &= (~EHCI_PORTSC1_W1_BITS);
        ehciIpBase->PORTSC1 = (ehciIpBase->PORTSC1 | USBHS_PORTSC1_PR_MASK);
        while (ehciIpBase->PORTSC1 & USBHS_PORTSC1_PR_MASK){}
        /* get device speed*/
        deviceInstance.speed = ((ehciIpBase->PORTSC1 & USBHS_PORTSC1_PSPD_MASK) >> USBHS_PORTSC1_PSPD_SHIFT);
        /* enable ehci phy disconnection */
        if (deviceInstance.speed == USB_SPEED_HIGH){
            USBPHY1->CTRL |= USBPHY_CTRL_ENHOSTDISCONDETECT_MASK;
            usb_echo("High speed device attached\r\n");
        }
        else{
            usb_echo("Full speed device attached\r\n");
        }
        /* wait for reset finish*/
        USB_HostEhciDelay(ehciIpBase, USB_HOST_EHCI_PORT_RESET_DELAY);
        /* gpt timer0 start */
        ehciIpBase->GPTIMER0CTL |= (USBHS_GPTIMER0CTL_RUN_MASK | USBHS_GPTIMER0CTL_MODE_MASK | USBHS_GPTIMER0CTL_RST_MASK);
        /* process attach */
        USB1_EventType =  EHCI_TASK_EVENT_DEVICE_ATTACH;
        deviceInstance.deviceAttached = kEHCIDevicePhyAttached;
    }
    else
    {
        if ((deviceInstance.deviceAttached == kEHCIDevicePhyAttached) || 
            (deviceInstance.deviceAttached == kEHCIDeviceAttached))
        {
            usb_echo("device detached\r\n");
            /* disable ehci phy disconnection */
            USBPHY1->CTRL &= (~USBPHY_CTRL_ENHOSTDISCONDETECT_MASK);
            /* disable async and periodic */
            USB_HostEhciStopAsync();
            USB_HostEhciStopPeriodic();
            USB1_EventType = EHCI_TASK_EVENT_DEVICE_DETACH;
            deviceInstance.state = kStatus_DEV_Notinit;
        }
    }
}

usb_status_t USB_HostEhciQhQtdListInit(usb_host_pipe_init_t *ehciPipePointer)
{
    uint32_t qtdNumber,qtd_index;
    uint32_t dataAddress,endAddress;
    usb_host_ehci_qtd_t *qtdPointer = NULL;
    EhciData.ehciRunningPipeList = ehciPipePointer;
    /* compute the qtd number */
    if (ehciPipePointer->pipeType == USB_ENDPOINT_CONTROL)//控制传输
    {
        /* assume setup data don't exceed one qtd data size, one qtd can transfer least 16k data */
        if (transfer.transferLength == 0){
            qtdNumber = 2;
            /* init qtd list */
            EhciData.ehciQtd[0].nextQtdPointer = (uint32_t)&EhciData.ehciQtd[1];
            EhciData.ehciQtd[1].nextQtdPointer = 0;
//            EhciData.ehciQh[1].ehciQtdTail = &EhciData.ehciQtd[1];
            ehciPipePointer->ehciQh->ehciQtdTail = &EhciData.ehciQtd[1];
        }
        else{
            qtdNumber = 3;
            /* init qtd list */
            EhciData.ehciQtd[0].nextQtdPointer = (uint32_t)&EhciData.ehciQtd[1];
            EhciData.ehciQtd[1].nextQtdPointer = (uint32_t)&EhciData.ehciQtd[2];
            EhciData.ehciQtd[2].nextQtdPointer = 0;
//            EhciData.ehciQh[1].ehciQtdTail = &EhciData.ehciQtd[2];
            ehciPipePointer->ehciQh->ehciQtdTail = &EhciData.ehciQtd[2];
        }
    }
    else//发送UFI命令
    {
        qtdNumber = (((transfer.transferLength) & 0xFFFFC000U) >> 14) + (((transfer.transferLength) & 0x00003FFF) ? 1 : 0);
        if(qtdNumber == 1){
            EhciData.ehciQtd[0].nextQtdPointer = 0;
        }
    }
    
    if (ehciPipePointer->pipeType == USB_ENDPOINT_CONTROL)//控制传输
    {
        /* setup transaction qtd setup的qtd初始化 */
        qtd_index = 0;
        EhciData.ehciQtd[qtd_index].alternateNextQtdPointer = EHCI_HOST_T_INVALID_VALUE;
        /* dt: need set; ioc: 0; C_Page: 0; PID Code: SETUP; Status: Active */
        EhciData.ehciQtd[qtd_index].transferResults[0] = EhciData.ehciQtd[0].transferResults[1] = 0;
        EhciData.ehciQtd[qtd_index].transferResults[0] = 
            ((0x00000000 << EHCI_HOST_QTD_DT_SHIFT) | (8 << EHCI_HOST_QTD_TOTAL_BYTES_SHIFT) |
             (EHCI_HOST_PID_SETUP << EHCI_HOST_QTD_PID_CODE_SHIFT) | (EHCI_HOST_QTD_STATUS_ACTIVE_MASK));
        
        dataAddress = ((uint32_t)&transfer.setupPacket);//需要4字节对齐
        EhciData.ehciQtd[qtd_index].transferResults[1] = dataAddress; /* current offset is set too */
        /* set buffer pointer no matter data length */
        for (uint8_t index = 0; index < 4; ++index){
            //&0xFFFFF000U:低12bit清零
            EhciData.ehciQtd[qtd_index].bufferPointers[index] = ((dataAddress + (index + 1) * 4 * 1024) & 0xFFFFF000U);
        }
        
        /* data transaction qtd 数据过程qtd初始化*/
        if(transfer.transferLength != 0)
        {
            qtd_index++;
            EhciData.ehciQtd[qtd_index].alternateNextQtdPointer = EHCI_HOST_T_INVALID_VALUE;
            /* dt: need set; ioc: 0; C_Page: 0; PID Code: IN/OUT; Status: Active */
            EhciData.ehciQtd[qtd_index].transferResults[0] = EhciData.ehciQtd[1].transferResults[1] = 0;
            if (transfer.direction == USB_OUT)
            {
                EhciData.ehciQtd[qtd_index].transferResults[0] =
                    ((0x00000001U << EHCI_HOST_QTD_DT_SHIFT) | (transfer.transferLength << EHCI_HOST_QTD_TOTAL_BYTES_SHIFT) |
                     (EHCI_HOST_PID_OUT << EHCI_HOST_QTD_PID_CODE_SHIFT) | (EHCI_HOST_QTD_STATUS_ACTIVE_MASK));
            }
            else
            {
                EhciData.ehciQtd[qtd_index].transferResults[0] =
                    ((0x00000001U << EHCI_HOST_QTD_DT_SHIFT) | (transfer.transferLength << EHCI_HOST_QTD_TOTAL_BYTES_SHIFT) |
                     (EHCI_HOST_PID_IN << EHCI_HOST_QTD_PID_CODE_SHIFT) | (EHCI_HOST_QTD_STATUS_ACTIVE_MASK));
            }
            dataAddress = (uint32_t)transfer.transferBuffer;
            EhciData.ehciQtd[qtd_index].transferResults[1] = dataAddress; /* current offset is set too */
            /* set buffer pointer no matter data length */
            for (uint8_t index = 0; index < 4; ++index)
            {
                EhciData.ehciQtd[qtd_index].bufferPointers[index] = ((dataAddress + (index + 1) * 4 * 1024) & 0xFFFFF000U);
            }
        }
        qtd_index++;
        /* status transaction qtd 状态过程qtd*/
        EhciData.ehciQtd[qtd_index].alternateNextQtdPointer = EHCI_HOST_T_INVALID_VALUE;
        /* dt: dont care; ioc: 1; C_Page: 0; PID Code: IN/OUT; Status: Active */
        EhciData.ehciQtd[qtd_index].transferResults[0] = EhciData.ehciQtd[2].transferResults[1] = 0;
        if ((transfer.transferLength == 0) || (transfer.direction == USB_OUT))
        {
            EhciData.ehciQtd[qtd_index].transferResults[0] =
                ((0x00000001U << EHCI_HOST_QTD_DT_SHIFT) | (EHCI_HOST_PID_IN << EHCI_HOST_QTD_PID_CODE_SHIFT) |
                 (EHCI_HOST_QTD_IOC_MASK) | (EHCI_HOST_QTD_STATUS_ACTIVE_MASK));
        }
        else
        {
            EhciData.ehciQtd[qtd_index].transferResults[0] =
                ((0x00000001U << EHCI_HOST_QTD_DT_SHIFT) | (EHCI_HOST_PID_OUT << EHCI_HOST_QTD_PID_CODE_SHIFT) |
                 (EHCI_HOST_QTD_IOC_MASK) | (EHCI_HOST_QTD_STATUS_ACTIVE_MASK));
        }
        EhciData.ehciQtd[qtd_index].nextQtdPointer |= EHCI_HOST_T_INVALID_VALUE;
        /* link qtd to qh (link to end) */
        EhciData.ehciQh[1].nextQtdPointer = (uint32_t)&EhciData.ehciQtd[0];
    }
    else //发送UFI命令
    {
        qtd_index = 0;
        dataAddress = (uint32_t)transfer.transferBuffer;
        qtdPointer = (usb_host_ehci_qtd_t *)&EhciData.ehciQtd[0];
        while(1)
        {
            qtdPointer->alternateNextQtdPointer = EHCI_HOST_T_INVALID_VALUE;
            /* dt: set; ioc: 0; C_Page: 0; PID Code: IN/OUT; Status: Active */
            qtdPointer->transferResults[0] = EhciData.ehciQtd[qtd_index].transferResults[1] = 0;
            if (transfer.direction == USB_OUT)
            {
                qtdPointer->transferResults[0] =
                    (((transfer.transferLength) << EHCI_HOST_QTD_TOTAL_BYTES_SHIFT) |
                     ((uint32_t)ehciPipePointer->nextdata01 << EHCI_HOST_QTD_DT_SHIFT) |
                     (EHCI_HOST_QTD_CERR_MAX_VALUE << EHCI_HOST_QTD_CERR_SHIFT) |
                     (EHCI_HOST_PID_OUT << EHCI_HOST_QTD_PID_CODE_SHIFT) | (EHCI_HOST_QTD_STATUS_ACTIVE_MASK));
            }
            else
            {
                qtdPointer->transferResults[0] =
                    (((transfer.transferLength) << EHCI_HOST_QTD_TOTAL_BYTES_SHIFT) |
                     ((uint32_t)ehciPipePointer->nextdata01 << EHCI_HOST_QTD_DT_SHIFT) |
                     (EHCI_HOST_QTD_CERR_MAX_VALUE << EHCI_HOST_QTD_CERR_SHIFT) |
                     (EHCI_HOST_PID_IN << EHCI_HOST_QTD_PID_CODE_SHIFT) | (EHCI_HOST_QTD_STATUS_ACTIVE_MASK));
            }
            qtdPointer->transferResults[1] = dataAddress; /* current offset is set too */
            /* set buffer pointer no matter data length */
            for (uint8_t index = 0; index < 4; ++index)
            {
                qtdPointer->bufferPointers[index] = ((dataAddress + (index + 1) * 4 * 1024) & 0xFFFFF000U);
            }
            dataAddress = endAddress; /* for next qtd */

            if (qtdPointer->nextQtdPointer == 0)
            {
                break;
            }
            qtdPointer = (usb_host_ehci_qtd_t *)(qtdPointer->nextQtdPointer);
        }
        qtdPointer->nextQtdPointer |= EHCI_HOST_T_INVALID_VALUE;
        qtdPointer->transferResults[0] |= EHCI_HOST_QTD_IOC_MASK; /* last one set IOC */
        /* save qtd to transfer */
        ehciPipePointer->ehciQh->ehciQtdTail = qtdPointer;
        /* link qtd to qh (link to end) */
        ehciPipePointer->ehciQh->nextQtdPointer = (uint32_t)&EhciData.ehciQtd[0];
    }
    
    USB_HostEhciDelay(ehciIpBase, 2U);
    USB_HostEhciStartAsync();
    
    return kStatus_USB_Success;
}


usb_status_t USB_HostEhciQhInit(usb_host_ehci_qh_t * qhPoint, usb_host_pipe_init_t *ehciPipePointer)
{
    uint32_t controlBits1 = 0;
    uint32_t controlBits2 = 0;
    
    qhPoint->currentQtdPointer = EHCI_HOST_T_INVALID_VALUE;//bit4-bit0:Reserved
    qhPoint->nextQtdPointer = EHCI_HOST_T_INVALID_VALUE;
    qhPoint->alternateNextQtdPointer = EHCI_HOST_T_INVALID_VALUE;
    qhPoint->timeOutLabel = 0;
    qhPoint->timeOutValue = 50;
    
    /* initialize staticEndpointStates[0] */
    if (ehciPipePointer->pipeType == USB_ENDPOINT_INTERRUPT){
        /* Software should set the RL field to zero if the queue head is an interrupt endpoint. */
        controlBits1 |= ((0U << EHCI_HOST_QH_RL_SHIFT) & EHCI_HOST_QH_RL_MASK);
    }
    else{
        if (ehciPipePointer->nakCount >= 16){
            controlBits1 |= ((15U << EHCI_HOST_QH_RL_SHIFT) & EHCI_HOST_QH_RL_MASK);
        } else {
            controlBits1 |= (((uint32_t)ehciPipePointer->nakCount << EHCI_HOST_QH_RL_SHIFT) & EHCI_HOST_QH_RL_MASK);
        }
    }
    if (ehciPipePointer->pipeType == USB_ENDPOINT_CONTROL)
    {
        if (deviceInstance.speed != USB_SPEED_HIGH){
            controlBits1 |= (1 << EHCI_HOST_QH_C_SHIFT);
        }
        controlBits1 |= (1 << EHCI_HOST_QH_DTC_SHIFT);
    }
    controlBits1 |= ((uint32_t)ehciPipePointer->maxPacketSize << EHCI_HOST_QH_MAX_PACKET_LENGTH_SHIFT);//最大包长
    controlBits1 |= (deviceInstance.speed << EHCI_HOST_QH_EPS_SHIFT);//速度
    controlBits1 |= ((uint32_t)ehciPipePointer->endpointAddress << EHCI_HOST_QH_ENDPT_SHIFT);//端点地址
    controlBits1 |= (deviceInstance.setAddress << EHCI_HOST_QH_DEVICE_ADDRESS_SHIFT);//设备地址
    qhPoint->staticEndpointStates[0] = controlBits1;
    /*initialize staticEndpointStates[1] */
    if (deviceInstance.speed == USB_SPEED_HIGH){
        controlBits2 |= ((uint32_t)ehciPipePointer->numberPerUframe << EHCI_HOST_QH_MULT_SHIFT);
    } else {
        controlBits2 |= (0x00000001U << EHCI_HOST_QH_MULT_SHIFT);
    }
    if (deviceInstance.speed != USB_SPEED_HIGH){
        controlBits2 |= (deviceInstance.hsHubPort << EHCI_HOST_QH_PORT_NUMBER_SHIFT);
        controlBits2 |= (deviceInstance.hsHubNumber << EHCI_HOST_QH_HUB_ADDR_SHIFT);
    } else {
        controlBits2 |= (deviceInstance.hsHubPort << EHCI_HOST_QH_PORT_NUMBER_SHIFT);
        controlBits2 |= (deviceInstance.hubNumber << EHCI_HOST_QH_HUB_ADDR_SHIFT);
    }
    controlBits2 |= ((uint32_t)0 << EHCI_HOST_QH_UFRAME_CMASK_SHIFT);
    controlBits2 |= ((uint32_t)0 << EHCI_HOST_QH_UFRAME_SMASK_SHIFT);
    qhPoint->staticEndpointStates[1] = controlBits2;
    
    ehciPipePointer->ehciQh = qhPoint;
    return kStatus_USB_Success;
}



void USB_HostEhciTransactionDone(void)
{
    usb_host_pipe_init_t *ehciPipePointer;
    volatile usb_host_ehci_qh_t *vltQhPointer;
    volatile usb_host_ehci_qtd_t *vltQtdPointer;
    ehciPipePointer = EhciData.ehciRunningPipeList;
    while(ehciPipePointer != NULL)
    {
        switch(ehciPipePointer->pipeType)
        {
            case USB_ENDPOINT_BULK:
            case USB_ENDPOINT_INTERRUPT:
            case USB_ENDPOINT_CONTROL:
                vltQhPointer = ehciPipePointer->ehciQh;
                vltQtdPointer = ehciPipePointer->ehciQh->ehciQtdTail;
                //IOC=1:qTD is completed && QTD_STATUS_ERROR_MASK == 0
                if( (vltQtdPointer->transferResults[0] & EHCI_HOST_QTD_IOC_MASK) &&
                    (!(vltQtdPointer->transferResults[0] & EHCI_HOST_QTD_STATUS_ERROR_MASK))){
                    
                    switch (deviceInstance.state)
                    {
                        case kStatus_DEV_GetDes8:
                            EhciData.ehciQh[1].staticEndpointStates[0] &= ~EHCI_HOST_QH_MAX_PACKET_LENGTH_MASK;
                            EhciData.ehciQh[1].staticEndpointStates[0] |= (deviceDescriptor.bMaxPacketSize0 << EHCI_HOST_QH_MAX_PACKET_LENGTH_SHIFT);
                            USB_HostEhciStartAsync();
                        break;
                        case kStatus_DEV_SetAddress:
                        {
                            EhciData.ehciQh[1].staticEndpointStates[0] &= ~0x7F;//低7位清零
                            EhciData.ehciQh[1].staticEndpointStates[0] |= deviceInstance.setAddress;
                            USB_HostEhciDelay(ehciIpBase, 2U);
                            break;
                        }
                        case kStatus_DEV_GetDes:
                            break;
                        case kStatus_DEV_GetCfg9:
                            break;
                        case kStatus_DEV_GetCfg:
                            descInterface = (usb_descriptor_interface_t *)(enmuBuffer + 9);
                            descEndpoint1 = (usb_descriptor_endpoint_t *)(enmuBuffer + 18);
                            descEndpoint2 = (usb_descriptor_endpoint_t *)(enmuBuffer + 25);
                            break;
                        case kStatus_DEV_GetMaxLun:
                            usb_echo("maxLun: %d\r\n", logicalUnitNumber);
                            break;
                        case kStatus_DEV_EnumDone:
                            if(transfer.callbackFn != NULL)
                                transfer.callbackFn();
                            break;
                        default :
                            break;
                    }
                    if(deviceInstance.state >= kStatus_DEV_GetDes8 && deviceInstance.state < kStatus_DEV_EnumDone){
                        deviceInstance.state++;
                        USB_HostProcessState();
                    }
                }
            break;
        }
        ehciPipePointer = ehciPipePointer->next;
        if(ehciPipePointer->ehciQh == NULL){
            ehciPipePointer = NULL;
            break;//跳出while循环
        }
    }
}

usb_status_t USB_HostProcessState(void)
{
    usb_status_t status = kStatus_USB_Success;
    
    /* reset transfer fields */
    transfer.setupPacket.bmRequestType = 0x00;
    transfer.setupPacket.wIndex = 0;
    transfer.setupPacket.wLength = 0;
    transfer.setupPacket.wValue = 0;
    
    switch (deviceInstance.state)
    {
        case kStatus_DEV_GetDes8:
        case kStatus_DEV_GetDes: /* get descriptor state */
            /* initialize transfer transfer.setupPacket在初始化qtd的时候使用到*/
            transfer.setupPacket.wLength = sizeof(usb_descriptor_device_t);
            if (deviceInstance.state == kStatus_DEV_GetDes8)//第一次获取设备描述符时获取8byte,是为了获取端点0的最大包长
            {
                transfer.setupPacket.wLength = 8;
            }
            transfer.setupPacket.bmRequestType |= USB_REQUEST_TYPE_DIR_IN;
            transfer.setupPacket.bRequest = USB_REQUEST_STANDARD_GET_DESCRIPTOR;
            transfer.setupPacket.wValue = (uint16_t)(USB_DESCRIPTOR_TYPE_DEVICE << 8) | 0;
            transfer.setupPacket.wIndex = 0;
            transfer.transferBuffer = (uint8_t *)&deviceDescriptor;//将接受到的数据保存在deviceDescriptor结构体中.
            transfer.transferLength = transfer.setupPacket.wLength;
            if ((transfer.setupPacket.bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN){
                transfer.direction = USB_IN;
            } else {
                transfer.direction = USB_OUT;
            }
            status = USB_HostEhciQhQtdListInit(&EhciData.ehciPipe[0]);
            break;
        case kStatus_DEV_SetAddress: /* set address state */
            deviceInstance.setAddress = 1;
            transfer.setupPacket.bRequest = USB_REQUEST_STANDARD_SET_ADDRESS;
            transfer.setupPacket.wValue = deviceInstance.setAddress;//设置设备的地址为1
            transfer.setupPacket.wLength = 0;
            transfer.transferBuffer = NULL;
            transfer.transferLength = 0;
            if ((transfer.setupPacket.bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN){
                transfer.direction = USB_IN;
            } else {
                transfer.direction = USB_OUT;
            }
            status = USB_HostEhciQhQtdListInit(&EhciData.ehciPipe[0]);
            break;

        case kStatus_DEV_GetCfg9: /* get 9 bytes configuration state 这里可以获取配置描述符总长度*/
            transfer.setupPacket.bmRequestType |= USB_REQUEST_TYPE_DIR_IN;
            transfer.setupPacket.bRequest = USB_REQUEST_STANDARD_GET_DESCRIPTOR;
            transfer.setupPacket.wValue =(USB_DESCRIPTOR_TYPE_CONFIGURE << 8) | deviceInstance.configurationValue;
            transfer.setupPacket.wLength = 9;
            transfer.setupPacket.wIndex = 0;
            transfer.transferBuffer = (uint8_t *)&devConfig;//将接受到的数据保存在deviceDescriptor结构体中.
            transfer.transferLength = transfer.setupPacket.wLength;
            if ((transfer.setupPacket.bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN){
                transfer.direction = USB_IN;
            } else {
                transfer.direction = USB_OUT;
            }
            status = USB_HostEhciQhQtdListInit(&EhciData.ehciPipe[0]);
            break;
        case kStatus_DEV_GetCfg: /* get configuration state */
            transfer.setupPacket.bmRequestType |= USB_REQUEST_TYPE_DIR_IN;
            transfer.setupPacket.bRequest = USB_REQUEST_STANDARD_GET_DESCRIPTOR;
            transfer.setupPacket.wValue =(USB_DESCRIPTOR_TYPE_CONFIGURE << 8) | deviceInstance.configurationValue;
            transfer.setupPacket.wIndex = 0;
            transfer.setupPacket.wLength = (devConfig.wTotalLength[1]<<8) | devConfig.wTotalLength[0];
            transfer.transferBuffer = enmuBuffer;//将接受到的数据保存在deviceDescriptor结构体中.
            transfer.transferLength = transfer.setupPacket.wLength;
            if ((transfer.setupPacket.bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN){
                transfer.direction = USB_IN;
            } else {
                transfer.direction = USB_OUT;
            }
            status = USB_HostEhciQhQtdListInit(&EhciData.ehciPipe[0]);
            break;

        case kStatus_DEV_SetCfg: /* set configuration state */
            transfer.setupPacket.wValue = devConfig.bConfigurationValue;
            transfer.setupPacket.bRequest = USB_REQUEST_STANDARD_SET_CONFIGURATION;
            transfer.transferBuffer = NULL;
            transfer.transferLength = 0;
            if ((transfer.setupPacket.bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN){
                transfer.direction = USB_IN;
            } else {
                transfer.direction = USB_OUT;
            }
            status = USB_HostEhciQhQtdListInit(&EhciData.ehciPipe[0]);
            break;
        case kStatus_DEV_GetMaxLun:
            transfer.setupPacket.bmRequestType = (USB_REQUEST_TYPE_DIR_IN | USB_REQUEST_TYPE_TYPE_CLASS | USB_REQUEST_TYPE_RECIPIENT_INTERFACE);
            transfer.setupPacket.bRequest = USB_HOST_HID_GET_MAX_LUN;
            transfer.setupPacket.wValue = 0x0000;
            transfer.setupPacket.wIndex = 0;
            transfer.setupPacket.wLength = 1;
            transfer.transferBuffer = &logicalUnitNumber;//将接受到的数据保存在deviceDescriptor结构体中.
            transfer.transferLength = transfer.setupPacket.wLength;
            if ((transfer.setupPacket.bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN){
                transfer.direction = USB_IN;
            } else {
                transfer.direction = USB_OUT;
            }
            status = USB_HostEhciQhQtdListInit(&EhciData.ehciPipe[0]);
            break;
        case kStatus_DEV_SetInterface:
            EhciData.ehciPipe[1].pipeType = USB_ENDPOINT_BULK;
            EhciData.ehciPipe[1].direction = USB_IN;
            EhciData.ehciPipe[1].endpointAddress = (descEndpoint1->bEndpointAddress & 0x0F);//低4位表示端口地址
            EhciData.ehciPipe[1].interval = descEndpoint1->bInterval;//端口查询时间
            EhciData.ehciPipe[1].maxPacketSize = (uint16_t)((descEndpoint1->wMaxPacketSize[1]<<8 | descEndpoint1->wMaxPacketSize[0]) & 0x07FF);
            EhciData.ehciPipe[1].numberPerUframe = 1;
            EhciData.ehciPipe[1].nakCount = 3000;
            EhciData.ehciPipe[1].nextdata01 = 0;
        
            
            EhciData.ehciQh[2].horizontalLinkPointer = ((uint32_t)&EhciData.ehciQh[1] | EHCI_HOST_POINTER_TYPE_QH);
            
            EhciData.ehciPipe[2].pipeType = USB_ENDPOINT_BULK;
            EhciData.ehciPipe[2].direction = USB_IN;
            EhciData.ehciPipe[2].endpointAddress = (descEndpoint2->bEndpointAddress & 0x0F);//低4位表示端口地址
            EhciData.ehciPipe[2].interval = descEndpoint2->bInterval;//端口查询时间
            EhciData.ehciPipe[2].maxPacketSize = (uint16_t)((descEndpoint2->wMaxPacketSize[1]<<8 | descEndpoint2->wMaxPacketSize[0]) & 0x07FF);
            EhciData.ehciPipe[2].numberPerUframe = 1;
            EhciData.ehciPipe[2].nakCount = 3000;
            EhciData.ehciPipe[2].nextdata01 = 0;
        
            USB_HostEhciQhInit(&EhciData.ehciQh[1],&EhciData.ehciPipe[0]);
            USB_HostEhciQhInit(&EhciData.ehciQh[2],&EhciData.ehciPipe[1]);
            USB_HostEhciQhInit(&EhciData.ehciQh[3],&EhciData.ehciPipe[2]);
            
            EhciData.ehciQh[3].horizontalLinkPointer = ((uint32_t)&EhciData.ehciQh[2] | EHCI_HOST_POINTER_TYPE_QH);
            EhciData.ehciQh[2].horizontalLinkPointer = ((uint32_t)&EhciData.ehciQh[1] | EHCI_HOST_POINTER_TYPE_QH);
            EhciData.ehciQh[1].horizontalLinkPointer = ((uint32_t)&EhciData.ehciQh[0] | EHCI_HOST_POINTER_TYPE_QH);
            EhciData.ehciQh[0].horizontalLinkPointer = ((uint32_t)&EhciData.ehciQh[3] | EHCI_HOST_POINTER_TYPE_QH);
            deviceInstance.state++;
        case kStatus_DEV_EnumDone: /* enumeration done state */
            if(descInterface->bInterfaceClass == USB_HOST_MSD_CLASS_CODE && 
                (descInterface->bInterfaceSubClass == USB_HOST_MSD_SUBCLASS_CODE_UFI || 
                 descInterface->bInterfaceSubClass == USB_HOST_MSD_SUBCLASS_CODE_SCSI))
            {
                usb_echo("pid=0x%x ", (deviceDescriptor.idProduct[0] | deviceDescriptor.idProduct[1]<<8));
                usb_echo("vid=0x%x ", (deviceDescriptor.idVendor[0]  | deviceDescriptor.idVendor[1]<<8));
                usb_echo("address=%d\r\n", deviceInstance.setAddress);
            }
            else if(descInterface->bInterfaceClass == USB_HOST_HUB_CLASS_CODE)
            {
                usb_echo("hub attached:level=%d ", deviceInstance.level);
                usb_echo("address=%d\r\n", deviceInstance.allocatedAddress);
            }
            else
            {
                usb_echo("Not supported device\r\n");
            }
            break;
        default:
            break;
    }

    return status;
}

usb_status_t USB_HostAttachDevice(uint8_t speed, uint8_t hubNumber, uint8_t level)
{
    /* new instance fields init */
    deviceInstance.speed = speed;   //设备速度
    deviceInstance.stallRetries = 1;//挂起重试次数
    deviceInstance.enumRetries = 3; //枚举重试次数
    deviceInstance.setAddress = 0;  //端点地址为0
    deviceInstance.deviceAttachState = kStatus_device_Attached;
    deviceInstance.hsHubNumber = 0;
    deviceInstance.portNumber = 0;
    deviceInstance.level = 1;//hub级数,没有hub时该值为1.
    deviceInstance.allocatedAddress = 1;
    deviceInstance.state = kStatus_DEV_Initial;
    deviceInstance.next = 0;
    
    /* new pipeInit fields init 获取最大数据包长度*/
    EhciData.ehciPipe[0].pipeType = USB_ENDPOINT_CONTROL;//控制传输
    EhciData.ehciPipe[0].direction = USB_OUT;
    EhciData.ehciPipe[0].endpointAddress = 0;//USB设备在总线复位后其地址为0
    EhciData.ehciPipe[0].maxPacketSize = 8;//端点0最大包长度在的偏移为7,所以首先获取设备描述符的前8字节,
    EhciData.ehciPipe[0].numberPerUframe = 1;
    EhciData.ehciPipe[0].nakCount = 3000U; //the max NAK count for one transaction.
    EhciData.ehciPipe[0].nextdata01 = 0;//使用DATA0
    
    /* initialize qh */
    USB_HostEhciQhInit(&EhciData.ehciQh[1],&EhciData.ehciPipe[0]);
    
    /* link qh*/
    EhciData.ehciQh[1].horizontalLinkPointer = ((uint32_t)&EhciData.ehciQh[0] | EHCI_HOST_POINTER_TYPE_QH);
    EhciData.ehciQh[0].horizontalLinkPointer = ((uint32_t)&EhciData.ehciQh[1] | EHCI_HOST_POINTER_TYPE_QH);
    
    /* start enumeration 开始枚举*/
    deviceInstance.state = kStatus_DEV_GetDes8;
    USB_HostProcessState();
    return kStatus_USB_Success;
}
