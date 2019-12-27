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

//该结构体定义了需要保存到EEPROM中的参数
typedef struct{
    uint32_t firmPacksTotal;//固件总包数
    uint32_t firmPacksCount;//当前接受的固件包数
    uint32_t firmSizeTotal; //固件总大小
    uint32_t firmSizeCurrent;//当前接受到的固件大小
    uint32_t firmCrc16;      //固件校验码
    uint8_t  firmUpdate;     //固件更新
    uint8_t  inactiveTime;     //用于设置活动时间
    uint8_t  batAlarmValue;    //电池电量报警值
    uint8_t  inactiveCondition;//用于设置触发条件
    uint32_t sampBandwidth;   //取样带宽
    uint32_t sampFreq;        //取样频率
    uint32_t sampClk;         //取样时钟频率
    float    sampTimeSet;     //取样时间
    uint8_t  sampMode;        //取样模式
    
}SysPara1;


//该结构体定义了不需要保存的系统参数
typedef struct{
    uint8_t  inactiveCount;//用于设置活动时间
    uint8_t  batLedStatus; //电池状态
    uint8_t  bleLedStatus; //蓝牙状态
    uint8_t  sampLedStatus;//采样状态
    uint8_t  emmcIsOk;     //eMMC文件系统是否完好
    bool     sampStart;    //开始采样标志
    uint32_t sampTimeCnt;  //用于记录当前采样时间
    uint32_t sampPacks;    //总共采集道的数据,需要分多少个包发给Android
    float    batVoltage;   //电池电压
    float    batTemp;      //电池温度
    float    batChargePercent;//充电百分比
    float    voltageADS1271;  //电压
    float    voltageSpdSignal;//转速信号电压
	uint32_t periodSpdSignal; //转速信号周期(us)
}SysPara2;

extern SysPara1 g_sys_para1;
extern SysPara2 g_sys_para2;


#endif
