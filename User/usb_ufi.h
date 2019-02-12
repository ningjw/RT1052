#ifndef USB_UFI_H
#define USB_UFI_H

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

/*! @brief UFI standard sense data structure */
typedef struct _usb_host_ufi_sense_data
{
    uint8_t errorCode;      /*!< This field shall contain a value of 70h to indicate current errors*/
    uint8_t reserved1;      /*!< Reserved field*/
    uint8_t senseKey;       /*!< Provide a hierarchy of error or command result information*/
    uint8_t information[4]; /*!< This field is command-specific; it is typically used by some commands to return a
                               logical block address denoting where an error occurred*/
    uint8_t additionalSenseLength; /*!< The UFI device sets the value of this field to ten, to indicate that ten more
                                      bytes of sense data follow this field*/
    uint8_t reserved2[4];          /*!< Reserved field*/
    uint8_t additionalSenseCode;   /*!< Provide a hierarchy of error or command result information*/
    uint8_t additionalSenseCodeQualifier; /*!< Provide a hierarchy of error or command result information*/
    uint8_t reserved3[4];                 /*!< Reserved field*/
} usb_host_ufi_sense_data_t;

/*! @brief UFI standard inquiry data structure */
typedef struct _usb_host_ufi_inquiry_data
{
    uint8_t peripheralDeviceType;      /*!< Identifies the device currently connected to the requested logical unit*/
    uint8_t removableMediaBit;         /*!< This shall be set to one to indicate removable media*/
    uint8_t version;                   /*!< Version*/
    uint8_t responseDataFormat;        /*!< A value of 01h shall be used for UFI device*/
    uint8_t additionalLength;          /*!< Specify the length in bytes of the parameters*/
    uint8_t reserved1[3];              /*!< Reserved field*/
    uint8_t vendorInformation[8];      /*!< Contains 8 bytes of ASCII data identifying the vendor of the product*/
    uint8_t productIdentification[16]; /*!< Contains 16 bytes of ASCII data as defined by the vendor*/
    uint8_t productRevisionLevel[4];   /*!< Contains 4 bytes of ASCII data as defined by the vendor*/
} usb_host_ufi_inquiry_data_t;

/*! @brief UFI read capacity data structure */
typedef struct _usb_host_ufi_read_capacity
{
    uint8_t lastLogicalBlockAddress[4]; /*!< The logical block number*/
    uint8_t blockLengthInBytes[4];      /*!< Block size*/
} usb_host_ufi_read_capacity_t;

void USB_HostMsdCommandStart(void);

#endif
