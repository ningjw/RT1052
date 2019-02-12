#ifndef USB_EHCI_H
#define USB_EHCI_H

#include "main.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
 
 /* CBW and CSW Macros */
#define USB_HOST_MSD_CBW_FLAGS_DIRECTION_OUT (0x00U)
#define USB_HOST_MSD_CBW_FLAGS_DIRECTION_IN (0x80U)
#define USB_HOST_MSD_CBW_SIGNATURE (0x43425355U)
#define USB_HOST_MSD_CSW_SIGNATURE (0x53425355U)

/* EHCI host macros */
#define EHCI_HOST_T_INVALID_VALUE (1U)
#define EHCI_HOST_POINTER_TYPE_ITD (0x00U)
#define EHCI_HOST_POINTER_TYPE_QH (0x00000002U)
#define EHCI_HOST_POINTER_TYPE_SITD (0x00000004U)
#define EHCI_HOST_POINTER_TYPE_FSTN (0x00000006U)
#define EHCI_HOST_POINTER_TYPE_MASK (0x00000006U)
#define EHCI_HOST_POINTER_ADDRESS_MASK (0xFFFFFFE0U)
#define EHCI_HOST_PID_OUT (0U)
#define EHCI_HOST_PID_IN (1U)
#define EHCI_HOST_PID_SETUP (2U)

#define EHCI_HOST_QH_RL_SHIFT (28U)
#define EHCI_HOST_QH_RL_MASK (0xF0000000U)
#define EHCI_HOST_QH_C_SHIFT (27U)
#define EHCI_HOST_QH_MAX_PACKET_LENGTH_SHIFT (16U)
#define EHCI_HOST_QH_MAX_PACKET_LENGTH_MASK (0x07FF0000U)
#define EHCI_HOST_QH_H_SHIFT (15U)
#define EHCI_HOST_QH_DTC_SHIFT (14U)
#define EHCI_HOST_QH_EPS_SHIFT (12U)
#define EHCI_HOST_QH_ENDPT_SHIFT (8U)
#define EHCI_HOST_QH_I_SHIFT (7U)
#define EHCI_HOST_QH_DEVICE_ADDRESS_SHIFT (0U)
#define EHCI_HOST_QH_MULT_SHIFT (30U)
#define EHCI_HOST_QH_PORT_NUMBER_SHIFT (23U)
#define EHCI_HOST_QH_HUB_ADDR_SHIFT (16U)
#define EHCI_HOST_QH_UFRAME_CMASK_SHIFT (8U)
#define EHCI_HOST_QH_UFRAME_SMASK_SHIFT (0U)
#define EHCI_HOST_QH_STATUS_ERROR_MASK (0x0000007EU)
#define EHCI_HOST_QH_STATUS_NOSTALL_ERROR_MASK (0x0000003EU)

#define EHCI_HOST_QTD_DT_SHIFT (31U) //Data Toggle
#define EHCI_HOST_QTD_DT_MASK (0x80000000U)
#define EHCI_HOST_QTD_TOTAL_BYTES_SHIFT (16U)
#define EHCI_HOST_QTD_TOTAL_BYTES_MASK (0x7FFF0000U)
#define EHCI_HOST_QTD_IOC_MASK (0x00008000U)
#define EHCI_HOST_QTD_C_PAGE_SHIFT (12U)
#define EHCI_HOST_QTD_CERR_SHIFT (10U)
#define EHCI_HOST_QTD_CERR_MAX_VALUE (0x00000003U)
#define EHCI_HOST_QTD_PID_CODE_SHIFT (8U)
#define EHCI_HOST_QTD_STATUS_SHIFT (0U)
#define EHCI_HOST_QTD_CURRENT_OFFSET_MASK (0x00000FFFU)
#define EHCI_HOST_QTD_BUFFER_POINTER_SHIFT (12U)
#define EHCI_HOST_QTD_STATUS_ACTIVE_MASK (0x00000080U)
#define EHCI_HOST_QTD_STATUS_MASK (0x000000ffU)
#define EHCI_HOST_QTD_STATUS_ERROR_MASK (0x0000007EU)
#define EHCI_HOST_QTD_STATUS_STALL_ERROR_MASK (0x00000040U)

/* CBW and CSW Macros */
#define USB_HOST_MSD_CBW_FLAGS_DIRECTION_OUT (0x00U)
#define USB_HOST_MSD_CBW_FLAGS_DIRECTION_IN (0x80U)
#define USB_HOST_MSD_CBW_SIGNATURE (0x43425355U)
#define USB_HOST_MSD_CSW_SIGNATURE (0x53425355U)

/* UFI data bit macro */
#define USB_HOST_UFI_BLOCK_DATA_VALID_LENGTH (10U)
#define USB_HOST_UFI_LOGICAL_UNIT_POSITION (5U)
#define USB_HOST_UFI_CBW_LENGTH (31U)
#define USB_HOST_UFI_CSW_LENGTH (13U)
#define USB_HOST_UFI_MODE_SENSE_PAGE_CONTROL_SHIFT (6U)
#define USB_HOST_UFI_MODE_SENSE_PAGE_CODE_SHIFT (0U)
#define USB_HOST_UFI_START_STOP_UNIT_LOEJ_SHIFT (1U)
#define USB_HOST_UFI_START_STOP_UNIT_START_SHIFT (0U)
#define USB_HOST_UFI_SEND_DIAGNOSTIC_SELF_TEST_SHIFT (2U)


/*! @brief States of device instances enumeration 枚举时使用的状态机*/
typedef enum _usb_host_device_enumeration_status
{
    kStatus_DEV_Notinit = 0, /*!< Device is invalid */
    kStatus_DEV_Initial,     /*!< Device has been processed by host driver */
    kStatus_DEV_GetDes8,     /*!< Enumeration process: get 8 bytes' device descriptor */
    kStatus_DEV_SetAddress,  /*!< Enumeration process: set device address */
    kStatus_DEV_GetDes,      /*!< Enumeration process: get device descriptor */
    kStatus_DEV_GetCfg9,     /*!< Enumeration process: get 9 bytes' configuration descriptor */
    kStatus_DEV_GetCfg,      /*!< Enumeration process: get configuration descriptor */
    kStatus_DEV_SetCfg,      /*!< Enumeration process: set configuration */
    kStatus_DEV_GetMaxLun,   /*!< This device has been used by application */
    kStatus_DEV_SetInterface,
    kStatus_DEV_EnumDone,    /*!< Enumeration is done */
} usb_host_device_enumeration_status_t;


/*! @brief For USB_REQUEST_STANDARD_GET_DESCRIPTOR and USB_REQUEST_STANDARD_SET_DESCRIPTOR */
typedef struct _usb_host_process_descriptor_param
{
    uint8_t descriptorType;    /*!< See the usb_spec.h, such as the USB_DESCRIPTOR_TYPE_DEVICE */
    uint8_t descriptorIndex;   /*!< The descriptor index is used to select a specific descriptor (only for configuration
                                  and string descriptors) when several descriptors of the same type are implemented in a
                                  device */
    uint8_t languageId;        /*!< It specifies the language ID for string descriptors or is reset to zero for other
                                  descriptors */
    uint8_t *descriptorBuffer; /*!< Buffer pointer */
    uint16_t descriptorLength; /*!< Buffer data length */
} usb_host_process_descriptor_param_t;

/*! @brief MSC Bulk-Only command block wrapper (CBW) */
typedef struct _usb_host_cbw
{
    uint32_t CBWSignature; /*!< Signature that helps identify this data packet as a CBW. The signature field shall
                              contain the value 43425355h (little endian), indicating a CBW */
    uint32_t CBWTag; /*!< A Command Block Tag sent by the host. The device shall echo the contents of this field back to the
                          host in the dCSWTag field of the associated CSW */
    uint32_t CBWDataTransferLength; /*!< The number of bytes of data that the host expects to transfer on the Bulk-In or
                                       Bulk-Out endpoint during the execution of this command */
    uint8_t  CBWFlags;               /*!<
                                         Bit 7 Direction - the device shall ignore this bit if the dCBWDataTransferLength field is
                                       zero, otherwise:
                                             0 = Data-Out from host to the device,
                                             1 = Data-In from the device to the host.
                                         Bit 6 Obsolete. The host shall set this bit to zero.
                                         Bits 5..0 Reserved - the host shall set these bits to zero.
                                     */
    uint8_t  CBWLun;      /*!< The device Logical Unit Number (LUN) to which the command block is being sent */
    uint8_t  CBWCBLength; /*!< The valid length of the CBWCB in bytes. This defines the valid length of the command
                            block. The only legal values are 1 through 16 (01h through 10h).*/
    uint8_t  CBWCB[16];   /*!< The command block to be executed by the device*/
} usb_host_cbw_t;

/*! @brief MSC Bulk-Only command status wrapper (CSW) */
typedef struct _usb_host_csw
{
    uint32_t CSWSignature; /*!< Signature that helps identify this data packet as a CSW. The signature field shall
                              contain the value 53425355h (little endian), indicating CSW.*/
    uint32_t CSWTag; /*!< The device shall set this field to the value received in the dCBWTag of the associated CBW*/
    uint32_t CSWDataResidue; /*!< the difference between the amount of data expected as stated in the
                                dCBWDataTransferLength and the actual amount of relevant data processed by the device.*/
    uint8_t CSWStatus;       /*!<
                                   bCSWStatus indicates the success or failure of the command.
                                   00h - Command passed.
                                   01h - Command Failed.
                                   02h - Phase error.
                                   others - Reserved.
                             */
} usb_host_csw_t;

/*! @brief UFI command process status */
typedef enum _usb_host_msd_command_status
{
    kMSD_CommandIdle = 0,
    kMSD_CommandTransferCBW,
    kMSD_CommandTransferData,
    kMSD_CommandTransferCSW,
    kMSD_CommandDone,
    kMSD_CommandCancel,
    kMSD_CommandErrorDone,
} usb_host_msd_command_status_t;

/*! @brief MSC UFI command information structure */
typedef struct _usb_host_msd_command
{
    usb_host_cbw_t cbwBlock;       /*!< CBW data block*/
    usb_host_csw_t cswBlock;       /*!< CSW data block*/
    uint8_t *dataBuffer;           /*!< Data buffer pointer*/
    uint32_t dataLength;           /*!< Data buffer length*/
    uint8_t retryTime;     /*!< The UFI command residual retry time, when it reduce to zero the UFI command fail */
    uint8_t dataDirection; /*!< The data direction, its value is USB_OUT or USB_IN*/
    usb_host_msd_command_status_t commandStatus;
} usb_host_msd_command_t;


void USB_HostEhciPortStaChange(void);
usb_status_t USB_HostEhciQhQtdListInit(usb_host_pipe_init_t *ehciPipePointer);
usb_status_t USB_HostProcessState(void);
usb_status_t USB_HostAttachDevice(uint8_t speed, uint8_t hubNumber, uint8_t level);
void USB_HostEhciTransactionDone(void);

#endif





