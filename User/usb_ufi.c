#include "main.h"
#include "usb_ufi.h"
#include "usb_app.h"
#include "usb_ehci.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
 
/* UFI command code */
#define UFI_FORMAT_UNIT (0x04U)
#define UFI_INQUIRY (0x12U)
#define UFI_START_STOP (0x1BU)
#define UFI_MODE_SELECT (0x55U)
#define UFI_MODE_SENSE (0x5AU)
#define UFI_MEDIUM_REMOVAL (0x1EU)
#define UFI_READ10 (0x28U)
#define UFI_READ12 (0xA8U)
#define UFI_READ_CAPACITY (0x25U)
#define UFI_READ_FORMAT_CAPACITY (0x23U)
#define UFI_REQUEST_SENSE (0x03U)
#define UFI_REZERO_UINT (0x01U)
#define UFI_SEEK (0x2BU)
#define UFI_SEND_DIAGNOSTIC (0x1DU)
#define UFI_TEST_UNIT_READY (0x00U)
#define UFI_VERIFY (0x2FU)
#define UFI_WRITE10 (0x2AU)
#define UFI_WRITE12 (0xAAU)
#define UFI_WRITE_VERIFY (0x2EU)

/* get the byte from the long value */
#define GET_BYTE_FROM_LE_LONG(b, n) ((uint8_t)((b) >> (n * 8))) 

usb_host_msd_command_t msdCommand;
/* command on-going state. It should set to 1 when start command, it is set to 0 in the callback */
volatile uint8_t ufiIng;
usb_status_t USB_HostMsdProcessCommand(void);

void USB_HostMsdCbwCallback(void)
{
    msdCommand.commandStatus = kMSD_CommandTransferData;
    USB_HostMsdProcessCommand(); /* continue to process ufi command */
}

void USB_HostMsdCswCallback(void)
{
    if(msdCommand.cswBlock.CSWSignature == USB_HOST_MSD_CSW_SIGNATURE)
    {
        switch (msdCommand.cswBlock.CSWStatus)
        {
            case 0:
                msdCommand.commandStatus = kMSD_CommandIdle;
                break;
            case 1:
                break;
            case 2:
                break;
            default:
                break;
        }
    }
}

usb_status_t USB_HostRecv(usb_host_pipe_init_t* pipeHandle)
{
    usb_status_t status = kStatus_USB_Success;
    transfer.direction = USB_IN;
    USB_HostEhciQhQtdListInit(pipeHandle);
    return status;
}


usb_status_t USB_HostMsdProcessCommand(void)
{
    switch ((uint8_t)msdCommand.commandStatus)
    {
        case kMSD_CommandTransferCBW: /* ufi CBW phase */
            transfer.direction = USB_OUT;
            transfer.transferBuffer = (uint8_t *)(&(msdCommand.cbwBlock));
            transfer.transferLength = USB_HOST_UFI_CBW_LENGTH;
            transfer.callbackFn = USB_HostMsdCbwCallback;
            USB_HostEhciQhQtdListInit(&EhciData.ehciPipe[2]);
            break;
        case kMSD_CommandTransferData: /* ufi DATA phase */
             if (msdCommand.dataBuffer != NULL)
            {
                transfer.direction = msdCommand.dataDirection;
                transfer.transferBuffer = msdCommand.dataBuffer;
                transfer.transferLength = msdCommand.dataLength;
                if (transfer.direction == USB_OUT)
                {
//                        USB_HostSend(msdInstance->hostHandle, msdInstance->outPipe, transfer);
                }
                else
                {
//                        USB_HostRecv(msdInstance->hostHandle, msdInstance->inPipe, transfer);
                }
                break;
            }
            else
            {
                /* don't break */
            }
        case kMSD_CommandTransferCSW: /* ufi CSW phase */
            transfer.direction = USB_IN;
            transfer.transferBuffer = (uint8_t *)&msdCommand.cswBlock;
            transfer.transferLength = sizeof(usb_host_csw_t);
            transfer.callbackFn = USB_HostMsdCswCallback;
            USB_HostRecv(&EhciData.ehciPipe[1]);
            break;
        case kMSD_CommandDone:
            break;
    }
    return kStatus_USB_Success;
}



usb_status_t USB_HostMsdCommand(uint8_t *buffer,uint32_t bufferLength, uint8_t direction, uint8_t byteValues[10])
{
    /* initialize CBWCB fields */
    for (uint8_t index = 0; index < USB_HOST_UFI_BLOCK_DATA_VALID_LENGTH; ++index)
    {
        msdCommand.cbwBlock.CBWCB[index] = byteValues[index];
    }
    /* initialize CBW fields */
    msdCommand.cbwBlock.CBWDataTransferLength = bufferLength;
    msdCommand.cbwBlock.CBWFlags = direction;
    msdCommand.cbwBlock.CBWLun = (byteValues[1] >> USB_HOST_UFI_LOGICAL_UNIT_POSITION);
    msdCommand.cbwBlock.CBWCBLength = USB_HOST_UFI_BLOCK_DATA_VALID_LENGTH;
    msdCommand.cbwBlock.CBWSignature = USB_HOST_MSD_CBW_SIGNATURE;//"USBC"
    
    if (direction == USB_HOST_MSD_CBW_FLAGS_DIRECTION_IN){
       msdCommand.dataDirection = USB_IN;
    } else {
       msdCommand.dataDirection = USB_OUT;
    }
    msdCommand.dataBuffer = buffer;
    msdCommand.dataLength = bufferLength;
    msdCommand.retryTime = 1;
    msdCommand.commandStatus = kMSD_CommandTransferCBW;
    
    USB_HostMsdProcessCommand();
    
    return kStatus_USB_Success;
}

usb_status_t USB_HostMsdRead10(uint8_t logicalUnit,
                               uint32_t blockAddress,
                               uint8_t *buffer,
                               uint32_t bufferLength,
                               uint32_t blockNumber)
{
    uint8_t ufiBytes[] = {UFI_READ10,
                          (uint8_t)(logicalUnit << USB_HOST_UFI_LOGICAL_UNIT_POSITION),
                          GET_BYTE_FROM_LE_LONG(blockAddress, 3),
                          GET_BYTE_FROM_LE_LONG(blockAddress, 2),
                          GET_BYTE_FROM_LE_LONG(blockAddress, 1),
                          GET_BYTE_FROM_LE_LONG(blockAddress, 0),
                          0x00,
                          GET_BYTE_FROM_LE_LONG(blockNumber, 1),
                          GET_BYTE_FROM_LE_LONG(blockNumber, 0),
                          0x00};
    return USB_HostMsdCommand(buffer,bufferLength,USB_HOST_MSD_CBW_FLAGS_DIRECTION_IN,ufiBytes);
}


usb_status_t USB_HostMsdRead12(uint8_t logicalUnit,
                               uint32_t blockAddress,
                               uint8_t *buffer,
                               uint32_t bufferLength,
                               uint32_t blockNumber)
{
    uint8_t ufiBytes[] = {UFI_READ12,
                          (uint8_t)(logicalUnit << USB_HOST_UFI_LOGICAL_UNIT_POSITION),
                          GET_BYTE_FROM_LE_LONG(blockAddress, 3),
                          GET_BYTE_FROM_LE_LONG(blockAddress, 2),
                          GET_BYTE_FROM_LE_LONG(blockAddress, 1),
                          GET_BYTE_FROM_LE_LONG(blockAddress, 0),
                          GET_BYTE_FROM_LE_LONG(blockNumber, 3),
                          GET_BYTE_FROM_LE_LONG(blockNumber, 2),
                          GET_BYTE_FROM_LE_LONG(blockNumber, 1),
                          GET_BYTE_FROM_LE_LONG(blockNumber, 0)};
    return USB_HostMsdCommand(buffer,bufferLength,USB_HOST_MSD_CBW_FLAGS_DIRECTION_IN,ufiBytes);
}


usb_status_t USB_HostMsdReadCapacity(uint8_t logicalUnit,
                                     uint8_t *buffer,
                                     uint32_t bufferLength)
{
    uint8_t ufiBytes[] = {UFI_READ_CAPACITY,
                          (uint8_t)(logicalUnit << USB_HOST_UFI_LOGICAL_UNIT_POSITION),
                          0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    return USB_HostMsdCommand(buffer,bufferLength,USB_HOST_MSD_CBW_FLAGS_DIRECTION_IN,ufiBytes);
}

usb_status_t USB_HostMsdTestUnitReady(uint8_t logicalUnit)
{
    uint8_t ufiBytes[] = {UFI_TEST_UNIT_READY,
                          (uint8_t)(logicalUnit << USB_HOST_UFI_LOGICAL_UNIT_POSITION),
                          0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    return USB_HostMsdCommand(NULL,0,USB_HOST_MSD_CBW_FLAGS_DIRECTION_OUT,ufiBytes);
}


usb_status_t USB_HostMsdRequestSense(uint8_t logicalUnit,
                                     uint8_t *buffer,
                                     uint32_t bufferLength)
{
    uint8_t ufiBytes[] = {UFI_REQUEST_SENSE,
                          (uint8_t)(logicalUnit << USB_HOST_UFI_LOGICAL_UNIT_POSITION),
                          0x00,0x00,(uint8_t)bufferLength,0x00,0x00,0x00,0x00,0x00};
    return USB_HostMsdCommand(buffer,bufferLength,USB_HOST_MSD_CBW_FLAGS_DIRECTION_IN,ufiBytes);
}

usb_status_t USB_HostMsdInquiry(uint8_t logicalUnit,
                                uint8_t *buffer,
                                uint32_t bufferLength)
{
    uint8_t ufiBytes[] = {UFI_INQUIRY,
                          (uint8_t)(logicalUnit << USB_HOST_UFI_LOGICAL_UNIT_POSITION),
                          0x00,0x00,(uint8_t)bufferLength,0x00,0x00,0x00,0x00,0x00};
    return USB_HostMsdCommand(buffer,bufferLength,USB_HOST_MSD_CBW_FLAGS_DIRECTION_IN,ufiBytes);
}

usb_status_t USB_HostMsdReadFormatCapacities(uint8_t logicalUnit,
                                             uint8_t *buffer,
                                             uint32_t bufferLength)
{
    uint8_t ufiBytes[] = {UFI_READ_FORMAT_CAPACITY,
                          (uint8_t)(logicalUnit << USB_HOST_UFI_LOGICAL_UNIT_POSITION),
                          0x00,0x00,0x00,0x00,0x00,
                          GET_BYTE_FROM_LE_LONG(bufferLength, 1),
                          GET_BYTE_FROM_LE_LONG(bufferLength, 0),
                          0x00};
    return USB_HostMsdCommand(buffer,bufferLength,USB_HOST_MSD_CBW_FLAGS_DIRECTION_IN,ufiBytes);
}

void USB_HostMsdCommandStart(void)
{
    usb_echo("........................ufi command start....................\r\n");
    USB_HostMsdTestUnitReady(0);
    ufiIng = 1;
    while(ufiIng)
    {
        USB_HostEhciIsrFunc();
    }
}

