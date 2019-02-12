#ifndef USB_UTIL_H
#define USB_UTIL_H

#include "main.h"

usb_status_t USB_HostEhciStartIP(void);
void USB_HostEhciStartAsync(void);
void USB_HostEhciStopAsync(void);
void USB_HostEhciDelay(USBHS_Type *ehciIpBase, uint32_t ms);
void USB_HostEhciStartPeriodic(void);
void USB_HostEhciStopPeriodic(void);


#endif

