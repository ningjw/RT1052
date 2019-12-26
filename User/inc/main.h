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
#include "fsl_flexspi.h"
#include "fsl_flexio.h"
#include "fsl_mmc.h"
#include "fsl_dcdc.h"
#include "fsl_src.h"

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
#include "norflash_drv.h"
#include "utility.h"

#include "ff.h"
#include "diskio.h"


#include "ble_protocol.h"
#include "ble_app.h"
#include "adc_app.h"
#include "battery_app.h"
#include "led_app.h"

//�ýṹ�嶨������Ҫ���浽EEPROM�еĲ���
typedef struct{
    uint8_t  inactiveTime;//�������ûʱ��
    uint8_t  batAlarmValue; //��ص�������ֵ
    uint8_t  inactiveCondition;//�������ô�������
    uint8_t  sampMode;//ȡ��ģʽ
    uint8_t  sampFreq;//ȡ��Ƶ��
    uint8_t  sampBandwidth;//ȡ������
    uint8_t  sampTime;//ȡ��ʱ��
    uint8_t  firmUpdate;//�̼�����
    uint32_t firmPacksTotal;//�̼��ܰ���
    uint32_t firmPacksCount;//��ǰ���ܵĹ̼�����
    uint32_t firmSizeTotal;//�̼��ܴ�С
    uint32_t firmSizeCurrent;//��ǰ���ܵ��Ĺ̼���С
    uint32_t firmCrc16;//�̼�У����
}SysPara1;


//�ýṹ�嶨���˲���Ҫ�����ϵͳ����
typedef struct{
    uint8_t inactiveCount;//�������ûʱ��
    uint8_t batLedStatus;//���״̬
    uint8_t bleLedStatus;//����״̬
    uint8_t sampLedStatus;//����״̬
    uint8_t emmcIsOk;//eMMC�ļ�ϵͳ
    bool     sampStart;
    uint32_t sampPacks;
    float   batVoltage;//��ص�ѹ
    float   batTemp;   //����¶�
    float batChargePercent;//���ٷֱ�
}SysPara2;

extern SysPara1 g_sys_para1;
extern SysPara2 g_sys_para2;


#endif
