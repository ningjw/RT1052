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


void USB_HostMsdCommandStart(void);

#endif
