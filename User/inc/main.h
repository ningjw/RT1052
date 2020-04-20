#ifndef __MAIN_H
#define __MAIN_H

#define SOFT_VERSION       "0.09"
#define HARD_VERSION       "1.0"
#define POWER_ON_TIMER_ID  1
#define ULONG_MAX          0xFFFFFFFF
//#define FIRM_INFO_ADDR    (0x6001F000U)
//#define FIRM_DATA_ADDR    (0x60020000U)

#define APP_INFO_SECTOR    63 /* 升级信息保存在NorFlash的第63个扇区*/
#define APP_START_SECTOR   64 /* App数据从第64个扇区开始保存 */
#define FIRM_ONE_PACKE_LEN 176
#define FIRM_ONE_LEN (FIRM_ONE_PACKE_LEN - 6)

#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "time.h"

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
#include "fsl_pwm.h"
#include "fsl_gpc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "queue.h"
#include "timers.h"

#include "lpm_app.h"
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
#include "norflash_app.h"

//该结构体定义了需要保存到EEPROM中的参数
typedef struct{
    uint8_t  firmUpdate;    //固件更新
    uint32_t firmSizeTotal; //固件总大小
    uint32_t firmCrc16;     //固件校验码
    uint32_t firmPacksTotal;//固件总包数
    uint32_t firmPacksCount;//当前接受的固件包数
    uint32_t firmSizeCurrent;//当前接受到的固件大小
    uint32_t firmNextAddr;  //下一次数据需要保存的地址
	
	uint32_t inactiveCount;//用于设置活动时间
    uint8_t  inactiveTime;   //用于设置活动时间
    uint8_t  batAlarmValue;  //电池电量报警值
    uint8_t  inactiveCondition;//用于设置触发条件

    uint8_t  batLedStatus; //电池状态
    uint8_t  bleLedStatus; //蓝牙状态
    uint8_t  sampLedStatus;//采样状态
    bool     ads1271IsOk;  //ADC芯片是否完好
    bool     emmcIsOk;     //eMMC文件系统是否完好
	float    batVoltage;   //电池电压
    float    batTemp;      //电池温度
    float    objTemp;      //物体温度
	float    envTemp;      //环境温度
    float    batRemainPercent;//充电百分比
	
    uint32_t sampNumber;  //取样时间
    uint32_t Ltc1063Clk;      //取样时钟频率
	uint32_t sampSize;     //将采集到的数据,打包成json格式后的总长度
    char*    sampJson;     //已经打包成json格式的数据的首地址
	
	uint32_t sampPacksCnt; //计数器
    uint32_t sampPacks;	   //总共采集道的数据,需要分多少个包发给Android
	uint32_t spdPacks;     //转速信号需要分多少个包发送完成
	uint32_t shkPacks;     //震动信号需要分多少个包发送完成
	
    float    voltageADS1271;
	uint32_t periodSpdSignal;//转速信号周期(us)
    uint32_t spdCount;   //转速信号采集到的个数
    uint32_t shkCount;   //震动信号采集到的个数
	uint32_t tempCount;  //当前记录的温度个数
	bool     WorkStatus; //用于指示当前是否正在采集.
	
    float    bias;         //震动传感器偏置电压
    float    refV;         //1052的参考电压值
	uint8_t  enterLPMCount;
    DWORD    emmc_fre_size;//剩余空间
    DWORD    emmc_tot_size;//总空间大小
    char     fileName[20];
    char     earliestFile[20];
}SysPara;


typedef struct{
    char  start;
    char  IDPath[128];
    char  NamePath[128];
	char  SpeedUnits[8];
	char  ProcessUnits[8];//温度单位
	char  DetectionType;//手动检测0,定时检测1
	float Senstivity;//灵敏度
    float Zerodrift;//零点偏移
    int   EUType;
    char  EU[8];//单位
	int   WindowsType;//
    char  WindowName[20];//窗函数名称
	int   StartFrequency;//起始频率
    int   EndFrequency;//截止频率
    int   SampleRate;//采样频率
    int   Lines;//线数
	int   Averages;//平均次数
    float AverageOverlap;//重叠率
    int   AverageType;//重叠方式
    int   EnvFilterLow;
    int   EnvFilterHigh;//包络滤波频段 
	int   IncludeMeasurements;//
	
    float Speed;//平均转速
    float Process;//平均温度
    float ProcessMin;//最小值
    float ProcessMax;//最大值
    
    int   StorageReson;//采集方式
    char  MeasurementComment[128];
    char  DAUID[20];
    char  Content[4];//保留
    char  end;
}ADC_Set;

extern SysPara g_sys_para;
extern ADC_Set g_adc_set;

#endif
