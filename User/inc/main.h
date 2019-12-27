#ifndef __MAIN_H
#define __MAIN_H

#define SOFT_VERSION       "V0.03"
#define POWER_ON_TIMER_ID  1
#define ULONG_MAX        0xFFFFFFFF

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


#include "cJSON.h"
#include "interrupt.h"
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
    uint32_t firmPacksTotal;//�̼��ܰ���
    uint32_t firmPacksCount;//��ǰ���ܵĹ̼�����
    uint32_t firmSizeTotal; //�̼��ܴ�С
    uint32_t firmSizeCurrent;//��ǰ���ܵ��Ĺ̼���С
    uint32_t firmCrc16;      //�̼�У����
    uint8_t  firmUpdate;     //�̼�����
    uint8_t  inactiveTime;     //�������ûʱ��
    uint8_t  batAlarmValue;    //��ص�������ֵ
    uint8_t  inactiveCondition;//�������ô�������
    uint32_t sampBandwidth;   //ȡ������
    uint32_t sampFreq;        //ȡ��Ƶ��
    uint32_t sampClk;         //ȡ��ʱ��Ƶ��
    float    sampTimeSet;     //ȡ��ʱ��
    uint8_t  sampMode;        //ȡ��ģʽ
    
}SysPara1;


//�ýṹ�嶨���˲���Ҫ�����ϵͳ����
typedef struct{
    uint8_t  inactiveCount;//�������ûʱ��
    uint8_t  batLedStatus; //���״̬
    uint8_t  bleLedStatus; //����״̬
    uint8_t  sampLedStatus;//����״̬
    uint8_t  emmcIsOk;     //eMMC�ļ�ϵͳ�Ƿ����
    bool     sampStart;    //��ʼ������־
    uint32_t sampTimeCnt;  //���ڼ�¼��ǰ����ʱ��
    uint32_t sampPacks;    //�ܹ��ɼ���������,��Ҫ�ֶ��ٸ�������Android
    float    batVoltage;   //��ص�ѹ
    float    batTemp;      //����¶�
    float    batChargePercent;//���ٷֱ�
    float    voltageADS1271;  //��ѹ
    float    voltageSpdSignal;//ת���źŵ�ѹ
	uint32_t periodSpdSignal; //ת���ź�����(us)
}SysPara2;

extern SysPara1 g_sys_para1;
extern SysPara2 g_sys_para2;


#endif
