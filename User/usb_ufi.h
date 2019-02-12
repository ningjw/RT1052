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

void USB_HostMsdCommandStart(void);

#endif
