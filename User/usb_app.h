#ifndef USB_APP_H
#define USB_APP_H

#include "stdint.h"

#define usb_echo DbgConsole_Printf

/* USB PHY condfiguration */
#define USB_HOST_INTERRUPT_PRIORITY (3U)
#define USB_HOST_CONFIG_CONFIGURATION_MAX_INTERFACE (5U)


#define MAX_QH (8U)
#define MAX_QTD (8U)
#define MAX_PIPES (16U)
#define FRAME_LIST_SIZE (1024U)

#define BOARD_XTAL0_CLK_HZ                         24000000U  /*!< Board xtal0 frequency in Hz */
/*! @brief Check the port connect state delay if the state is unstable */
#define USB_HOST_EHCI_PORT_CONNECT_DEBOUNCE_DELAY (101U)
/*! @brief Delay for port reset */
#define USB_HOST_EHCI_PORT_RESET_DELAY (11U)
#define EHCI_PORTSC1_W1_BITS (0x0000002AU)
#define EHCI_HOST_POINTER_ADDRESS_MASK (0xFFFFFFE0U)

#define EHCI_HOST_QH_H_SHIFT (15U)

/*! @brief MSD class code */
#define USB_HOST_MSD_CLASS_CODE (8U)
/*! @brief HUB class code */
#define USB_HOST_HUB_CLASS_CODE (9U)
/*! @brief MSD sub-class code */
#define USB_HOST_MSD_SUBCLASS_CODE_UFI (4U)
/*! @brief MSD sub-class code */
#define USB_HOST_MSD_SUBCLASS_CODE_SCSI (6U)
/*! @brief MSD protocol code */
#define USB_HOST_MSD_PROTOCOL_BULK (0x50U)

/*! @brief MSD class-specific request (get maximum logical unit number) */
#define USB_HOST_HID_GET_MAX_LUN (0xFEU)

/* task event */
#define EHCI_TASK_EVENT_DEVICE_ATTACH (0x01U)
#define EHCI_TASK_EVENT_TRANSACTION_DONE (0x02U)
#define EHCI_TASK_EVENT_DEVICE_DETACH (0x04U)
#define EHCI_TASK_EVENT_PORT_CHANGE (0x08U)
#define EHCI_TASK_EVENT_TIMER0 (0x10U)

/* USB speed (the value cannot be changed because EHCI QH use the value directly)*/
#define USB_SPEED_FULL (0x00U)
#define USB_SPEED_LOW (0x01U)
#define USB_SPEED_HIGH (0x02U)
#define USB_SPEED_SUPER (0x04U)

/* USB  standard descriptor endpoint type */
#define USB_ENDPOINT_CONTROL (0x00U)
#define USB_ENDPOINT_ISOCHRONOUS (0x01U)
#define USB_ENDPOINT_BULK (0x02U)
#define USB_ENDPOINT_INTERRUPT (0x03U)
#define EHCI_MAX_UFRAME_VALUE (0x00003FFFU)

/* USB standard descriptor type */
#define USB_DESCRIPTOR_TYPE_DEVICE (0x01U)
#define USB_DESCRIPTOR_TYPE_CONFIGURE (0x02U)
#define USB_DESCRIPTOR_TYPE_STRING (0x03U)
#define USB_DESCRIPTOR_TYPE_INTERFACE (0x04U)
#define USB_DESCRIPTOR_TYPE_ENDPOINT (0x05U)
#define USB_DESCRIPTOR_TYPE_DEVICE_QUALITIER (0x06U)
#define USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION (0x07U)
#define USB_DESCRIPTOR_TYPE_INTERFAACE_POWER (0x08U)
#define USB_DESCRIPTOR_TYPE_OTG (0x09U)
#define USB_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION (0x0BU)
#define USB_DESCRIPTOR_TYPE_BOS (0x0F)
#define USB_DESCRIPTOR_TYPE_DEVICE_CAPABILITY (0x10)

/* USB standard request */
#define USB_REQUEST_STANDARD_GET_STATUS (0x00U)
#define USB_REQUEST_STANDARD_CLEAR_FEATURE (0x01U)
#define USB_REQUEST_STANDARD_SET_FEATURE (0x03U)
#define USB_REQUEST_STANDARD_SET_ADDRESS (0x05U)
#define USB_REQUEST_STANDARD_GET_DESCRIPTOR (0x06U)
#define USB_REQUEST_STANDARD_SET_DESCRIPTOR (0x07U)
#define USB_REQUEST_STANDARD_GET_CONFIGURATION (0x08U)
#define USB_REQUEST_STANDARD_SET_CONFIGURATION (0x09U)
#define USB_REQUEST_STANDARD_GET_INTERFACE (0x0AU)
#define USB_REQUEST_STANDARD_SET_INTERFACE (0x0BU)
#define USB_REQUEST_STANDARD_SYNCH_FRAME (0x0CU)


/* USB standard request type */
#define USB_REQUEST_TYPE_DIR_MASK (0x80U)
#define USB_REQUEST_TYPE_DIR_SHIFT (7U)
#define USB_REQUEST_TYPE_DIR_OUT (0x00U)
#define USB_REQUEST_TYPE_DIR_IN (0x80U)

#define USB_REQUEST_TYPE_TYPE_MASK (0x60U)
#define USB_REQUEST_TYPE_TYPE_SHIFT (5U)
#define USB_REQUEST_TYPE_TYPE_STANDARD (0U)
#define USB_REQUEST_TYPE_TYPE_CLASS (0x20U)
#define USB_REQUEST_TYPE_TYPE_VENDOR (0x40U)

#define USB_REQUEST_TYPE_RECIPIENT_MASK (0x1FU)
#define USB_REQUEST_TYPE_RECIPIENT_SHIFT (0U)
#define USB_REQUEST_TYPE_RECIPIENT_DEVICE (0x00U)
#define USB_REQUEST_TYPE_RECIPIENT_INTERFACE (0x01U)
#define USB_REQUEST_TYPE_RECIPIENT_ENDPOINT (0x02U)
#define USB_REQUEST_TYPE_RECIPIENT_OTHER (0x03U)


/* USB  standard descriptor transfer direction (cannot change the value because iTD use the value directly) */
#define USB_OUT (0U)
#define USB_IN (1U)


#define USB_SHORT_FROM_LITTLE_ENDIAN_DATA(n) ((uint32_t)(((*((uint8_t *)&(n) + 1)) << 8U) | ((*((uint8_t *)&(n))))))

#define USB_SHORT_TO_LITTLE_ENDIAN_DATA(n, m)                       \
    {                                                               \
        *((uint8_t *)&(m) + 1) = ((((uint16_t)(n)) >> 8U) & 0xFFU); \
        *((uint8_t *)&(m)) = ((((uint16_t)(n))) & 0xFFU);           \
    }

    
/*! @brief USB error code */
typedef enum _usb_status
{
    kStatus_USB_Success = 0x00U, /*!< Success */
    kStatus_USB_Error,           /*!< Failed */
    kStatus_USB_Busy,                       /*!< Busy */
    kStatus_USB_NotSupported,   /*!< Configuration is not supported */
    kStatus_USB_Retry,          /*!< Enumeration get configuration retry */
    kStatus_USB_TransferStall,  /*!< Transfer stalled */
    kStatus_USB_TransferFailed, /*!< Transfer failed */
    kStatus_USB_AllocFail,      /*!< Allocation failed */
    kStatus_USB_LackSwapBuffer, /*!< Insufficient swap buffer for KHCI */
    kStatus_USB_TransferCancel, /*!< The transfer cancelled */
    kStatus_USB_BandwidthFail,  /*!< Allocate bandwidth failed */
    kStatus_USB_MSDStatusFail,  /*!< For MSD, the CSW status means fail */
} usb_status_t;


/*! @brief EHCI state for device attachment/detachment. */
typedef enum _host_ehci_device_state_
{
    kEHCIDevicePhyAttached = 1, /*!< Device is physically attached */
    kEHCIDeviceAttached,        /*!< Device is attached and initialized */
    kEHCIDeviceDetached,        /*!< Device is detached and de-initialized */
} host_ehci_device_state_t;


/*! @brief States of device */
typedef enum _usb_host_device_state
{
    kStatus_device_Detached = 0, /*!< Device is used by application */
    kStatus_device_Attached,     /*!< Device's default status */
} usb_host_device_state_t;

/*! @brief EHCI QTD structure. See the USB EHCI specification. */
typedef struct _usb_host_ehci_qtd
{
    uint32_t nextQtdPointer;          /*!< QTD specification filed, the next QTD pointer */
    uint32_t alternateNextQtdPointer; /*!< QTD specification filed, alternate next QTD pointer */
    uint32_t transferResults[2];      /*!< QTD specification filed, transfer results fields */
    uint32_t bufferPointers[4];       /*!< QTD specification filed, transfer buffer fields */
} usb_host_ehci_qtd_t;

/*! @brief EHCI QH structure. See the USB EHCI specification */
typedef struct _usb_host_ehci_qh
{
    uint32_t horizontalLinkPointer; /*!< QH specification filed, queue head a horizontal link pointer 指向下一个数据对象的地址*/
    uint32_t staticEndpointStates[2]; /*!< QH specification filed, static endpoint state and configuration information */
    uint32_t currentQtdPointer;  /*!< QH specification filed, current qTD pointer */
    uint32_t nextQtdPointer;     /*!< QH specification filed, next qTD pointer */
    uint32_t alternateNextQtdPointer; /*!< QH specification filed, alternate next qTD pointer */
    uint32_t transferOverlayResults[6]; /*!< QH specification filed, transfer overlay configuration and transfer results */
    
    /* reserved space */
    uint32_t reservedSpace[2];
    usb_host_ehci_qtd_t *ehciQtdTail; /*!< Transfer list tail on this QH */
    uint16_t timeOutValue; /*!< Its maximum value is USB_HOST_EHCI_CONTROL_BULK_TIME_OUT_VALUE. When the value is zero, the transfer times out. */
    uint16_t timeOutLabel; /*!< It's used to judge the transfer timeout. The EHCI driver maintain the value */
} usb_host_ehci_qh_t;



/*! @brief USB host pipe information structure for opening pipe */
typedef struct _usb_host_pipe_init
{
    uint16_t nakCount;       /*!< Maximum NAK retry count. MUST be zero for interrupt*/
    uint16_t maxPacketSize;  /*!< Pipe's maximum packet size*/
    uint8_t interval;        /*!< Pipe's interval*/
    uint8_t open;                   /*!< 0 - closed, 1 - open*/
    uint8_t nextdata01;             /*!< Data toggle 数据翻转标志*/
    uint8_t endpointAddress; /*!< Endpoint address*/
    uint8_t direction;       /*!< Endpoint direction*/
    uint8_t pipeType;        /*!< Endpoint type, the value is USB_ENDPOINT_INTERRUPT, USB_ENDPOINT_CONTROL,
                                USB_ENDPOINT_ISOCHRONOUS, USB_ENDPOINT_BULK*/
    uint8_t numberPerUframe; /*!< Transaction number for each micro-frame*/
    usb_host_ehci_qh_t *ehciQh;               /*!< Control/bulk/interrupt: QH; ISO: usb_host_ehci_iso_t*/
    struct _usb_host_pipe_init *next;    /*!< Link the idle pipes*/
} usb_host_pipe_init_t;

/*! @brief EHCI data structure */
typedef struct _usb_host_ehci_data
{
    usb_host_ehci_qh_t ehciQh[MAX_QH]; /*!< Idle QH list array*/
    usb_host_ehci_qtd_t ehciQtd[MAX_QTD];
    usb_host_pipe_init_t ehciPipe[MAX_PIPES]; /*!< Idle pipe list array*/
    usb_host_pipe_init_t *ehciRunningPipeList; /*!< Running pipe list pointer*/
}usb_host_ehci_data_t;


typedef struct _usb_descriptor_device
{
    uint8_t bLength;            /* Size of this descriptor in bytes */
    uint8_t bDescriptorType;    /* DEVICE Descriptor Type */
    uint8_t bcdUSB[2];          /* UUSB Specification Release Number in Binary-Coded Decimal, e.g. 0x0200U */
    uint8_t bDeviceClass;       /* Class code */
    uint8_t bDeviceSubClass;    /* Sub-Class code */
    uint8_t bDeviceProtocol;    /* Protocol code */
    uint8_t bMaxPacketSize0;    /* Maximum packet size for endpoint zero */
    uint8_t idVendor[2];        /* Vendor ID (assigned by the USB-IF) */
    uint8_t idProduct[2];       /* Product ID (assigned by the manufacturer) */
    uint8_t bcdDevice[2];       /* Device release number in binary-coded decimal */
    uint8_t iManufacturer;      /* Index of string descriptor describing manufacturer */
    uint8_t iProduct;           /* Index of string descriptor describing product */
    uint8_t iSerialNumber;      /* Index of string descriptor describing the device serial number */
    uint8_t bNumConfigurations; /* Number of possible configurations */
} usb_descriptor_device_t;

//描述符共同点
typedef struct _usb_descriptor_common
{
    uint8_t bLength;         /* Size of this descriptor in bytes */
    uint8_t bDescriptorType; /* DEVICE Descriptor Type */
    uint8_t bData[1];        /* Data */
} usb_descriptor_common_t;

//配置描述符
typedef struct _usb_descriptor_configuration
{
    uint8_t bLength;             /* Descriptor size in bytes = 9U */
    uint8_t bDescriptorType;     /* CONFIGURATION type = 2U or 7U */
    uint8_t wTotalLength[2];     /* Length of concatenated descriptors */
    uint8_t bNumInterfaces;      /* Number of interfaces, this configuration. */
    uint8_t bConfigurationValue; /* Value to set this configuration. */
    uint8_t iConfiguration;      /* Index to configuration string */
    uint8_t bmAttributes;        /* Configuration characteristics */
    uint8_t bMaxPower;           /* Maximum power from bus, 2 mA units */
} usb_descriptor_configuration_t;

//接口描述符
typedef struct _usb_descriptor_interface
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} usb_descriptor_interface_t;

//端点描述符结构
typedef struct _usb_descriptor_endpoint
{
    uint8_t bLength;//该描述符长度
    uint8_t bDescriptorType;//描述符类型:0x05
    uint8_t bEndpointAddress;//该端点地址 D7为该端点传输方向:0为输出;1为输入.
    uint8_t bmAttributes;//该端点属性 D1~D0:0=控制传输;1=等时传输;2=批量传输;3=中断传输
    uint8_t wMaxPacketSize[2];//该端点支持的最大包长度
    uint8_t bInterval;//端口的查询时间
} usb_descriptor_endpoint_t;


/* Set up packet structure */
//标准设备请求的数据结构
typedef struct _usb_setup_struct
{
    uint8_t bmRequestType;//D7数据传输方向:0=主机到设备;1=设备到主机
    uint8_t bRequest;//请求代码
    uint16_t wValue;//
    uint16_t wIndex;
    uint16_t wLength;//数据过程所需要传输的字节数.
} usb_setup_struct_t;

/*!
 * @brief Host stack inner transfer callback function typedef.
 *
 * This callback function is used to notify the upper layer the result of a transfer.
 * This callback pointer is passed when initializing the structure usb_host_transfer_t.
 *
 * @param param     The parameter pointer, which is passed when calling the send/receive APIs.
 * @param transfer  The transfer information; See the structure usb_host_transfer_t.
 * @param status    A USB error code or kStatus_USB_Success.
 */
struct _usb_host_transfer; /* for cross reference */
typedef void (*host_inner_transfer_callback_t)(void);
/*! @brief USB host transfer structure */
typedef struct _usb_host_transfer
{
    uint8_t *transferBuffer;                   /*!< Transfer data buffer*/
    uint32_t transferLength;                   /*!< Transfer data length*/
    host_inner_transfer_callback_t callbackFn; /*!< Transfer callback function*/
    usb_setup_struct_t setupPacket;           /*!< Set up packet buffer*/
    uint8_t direction;                         /*!< Transfer direction; it's values are USB_OUT or USB_IN*/
} usb_host_transfer_t;


/*! @brief USB host configuration information structure */
typedef struct _usb_host_configuration
{
//    usb_host_interface_t interfaceList[USB_HOST_CONFIG_CONFIGURATION_MAX_INTERFACE]; /*!< Interface array*/
//    usb_descriptor_configuration_t *configurationDesc; /*!< Configuration descriptor pointer*/
    uint8_t *configurationExtension;                   /*!< Configuration extended descriptor pointer*/
    uint16_t configurationExtensionLength;             /*!< Extended descriptor length*/
    uint8_t interfaceCount;                            /*!< The configuration's interface number*/
} usb_host_configuration_t;

/*! @brief Device instance */
typedef struct _usb_host_device_instance
{
    struct _usb_host_device_instance *next;    /*!< Next device, or NULL */
    usb_host_configuration_t configuration;    /*!< Parsed configuration information for the device */
    usb_descriptor_device_t deviceDescriptor; /*!< Standard device descriptor */
    uint8_t *configurationDesc;                /*!< Configuration descriptor pointer */
    uint16_t configurationLen;                 /*!< Configuration descriptor length */
    uint16_t configurationValue;               /*!< Configuration index */
    uint8_t interfaceStatus[USB_HOST_CONFIG_CONFIGURATION_MAX_INTERFACE]; /*!< Interfaces' status, please reference to
                                                                             #usb_host_interface_state_t */
    uint8_t enumBuffer[9];                                                  /*!< Buffer for enumeration */
    uint8_t state;                                                        /*!< Device state for enumeration */
    uint8_t enumRetries;       /*!< Re-enumeration when error in control transfer */
    uint8_t stallRetries;      /*!< Re-transfer when stall */
    uint8_t speed;             /*!< Device speed */
    uint8_t allocatedAddress;  /*!< Temporary address for the device. When set address request succeeds, setAddress is
                                  a value, 1 - 127 */
    uint8_t setAddress;        /*!< The address has been set to the device successfully, 1 - 127 */
    uint8_t deviceAttachState; /*!< See the usb_host_device_state_t */
    uint8_t deviceAttached;   /*!< Device attach/detach state, see #host_ehci_device_state_t */
    /* hub related */
    uint8_t hubNumber;   /*!< Device's first connected hub address (root hub = 0) */
    uint8_t portNumber;  /*!< Device's first connected hub's port no (1 - 8) */
    uint8_t hsHubNumber; /*!< Device's first connected high-speed hub's address (1 - 8) */
    uint8_t hsHubPort;   /*!< Device's first connected high-speed hub's port no (1 - 8) */
    uint8_t level;       /*!< Device's level (root device = 0) */
} usb_host_device_instance_t;


extern usb_host_device_instance_t deviceInstance;
//extern usb_host_pipe_init_t       pipeInit;
extern usb_host_transfer_t        transfer;
extern usb_descriptor_device_t deviceDescriptor; /*!< Standard device descriptor */
extern usb_descriptor_configuration_t devConfig;
extern uint8_t enmuBuffer[];
extern __attribute__((aligned(4))) uint8_t s_Setupbuffer[];
extern  USBHS_Type* ehciIpBase; /*!< EHCI IP base address*/
extern  uint8_t USB1_EventType;
extern __attribute__((aligned(64)))    usb_host_ehci_data_t  EhciData;
extern __attribute__((aligned(4096)))  uint8_t               s_UsbHostEhciFrameList1[FRAME_LIST_SIZE * 4];
void USB_HostAppInit(void);
void USB_HostEhciIsrFunc(void);

#endif
