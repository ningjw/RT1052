#ifndef __MAIN_H
#define __MAIN_H

#define SOFT_VERSION       "V0.01"
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
#include "emmc_drv.h"
#include "adc_drv.h"
#include "battery_drv.h"

#include "ble_app.h"
#include "adc_app.h"
#include "battery_app.h"
#include "led_app.h"

//�ýṹ�嶨������Ҫ���浽EEPROM�еĲ���
typedef struct{
    uint8_t inactiveTime;//�������ûʱ��
    uint8_t batAlarmValue; //
}SysPara1;


//�ýṹ�嶨���˲���Ҫ�����ϵͳ����
typedef struct{
    uint8_t inactiveCount;//�������ûʱ��
    uint8_t batStatus;//���״̬
    uint8_t bleStatus;//����״̬
    uint8_t sysStatus;
}SysPara2;

extern SysPara1 g_sys_para1;
extern SysPara2 g_sys_para2;


#endif
