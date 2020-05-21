#ifndef __BLE_PROTOCOL_H
#define __BLE_PROTOCOL_H

extern snvs_hp_rtc_datetime_t sampTime;

uint8_t* ParseProtocol(uint8_t *pMsg);

#endif
