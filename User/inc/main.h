#ifndef __MAIN_H
#define __MAIN_H

#define SOFT_VERSION       "V0.02"
#define POWER_ON_TIMER_ID  1

#include "stdint.h"
#include "string.h"
#include "stdio.h"

#include "clock_config.h"
#include "peripherals.h"
#include "pin_mux.h"

#include "fsl_gpio.h"
#include "fsl_lpuart.h"
#include "fsl_debug_console.h"
#include "fsl_snvs_hp.h"
#include "fsl_iomuxc.h"
#include "fsl_xbara.h"


#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "queue.h"
#include "timers.h"
#include "task_app.h"

#include "cJSON.h"
#include "interrupt.h"
#include "lpuart1.h"
#include "i2c_eeprom_drv.h"
#include "rtc_drv.h"
#include "adc_drv.h"
#include "battery_drv.h"
#include "iic_temp_drv.h"
#include "core_delay.h"

#include "ble_protocol.h"
#include "ble_app.h"
#include "adc_app.h"
#include "battery_app.h"
#include "led_app.h"

//该结构体定义了需要保存到EEPROM中的参数
typedef struct{
    uint8_t  inactiveTime;//用于设置活动时间
    uint8_t  batAlarmValue; //
    uint8_t  inactiveCondition;
    uint8_t  sampMode;
    uint8_t  sampFreq;
    uint8_t  sampBandwidth;
    uint8_t  sampTime;
    uint8_t  firmUpdate;
    uint32_t firmPacks;
    uint32_t firmSize;
    uint32_t firmCrc16;
}SysPara1;


//该结构体定义了不需要保存的系统参数
typedef struct{
    uint8_t inactiveCount;//用于设置活动时间
    uint8_t batLedStatus;//电池状态
    uint8_t bleLedStatus;//蓝牙状态
    uint8_t sampLedStatus;
    uint32_t sampPacks;
    bool     sampStart;
}SysPara2;

extern SysPara1 g_sys_para1;
extern SysPara2 g_sys_para2;


#endif
