#ifndef __MAIN_H
#define __MAIN_H

#define SOFT_VERSION       "V0.03"
#define POWER_ON_TIMER_ID  1
#define ULONG_MAX          0xFFFFFFFF
#define FIRM_INFO_ADDR    0x6000F000
#define FIRM_DATA_ADDR    0x60010000
#define FIRM_ONE_PACKE_LEN 128

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
#include "norflash_app.h"

//�ýṹ�嶨������Ҫ���浽EEPROM�еĲ���
typedef struct{
    uint8_t  firmUpdate;    //�̼�����
    uint32_t firmSizeTotal; //�̼��ܴ�С
    uint32_t firmCrc16;     //�̼�У����
    uint32_t firmPacksTotal;//�̼��ܰ���
    uint32_t firmPacksCount;//��ǰ���ܵĹ̼�����
    uint32_t firmSizeCurrent;//��ǰ���ܵ��Ĺ̼���С
    uint32_t firmNextAddr;  //��һ��������Ҫ����ĵ�ַ

    uint8_t  inactiveTime;   //�������ûʱ��
    uint8_t  batAlarmValue;  //��ص�������ֵ
    uint8_t  inactiveCondition;//�������ô�������
    
    uint8_t  sampMode;        //ȡ��ģʽ
    uint32_t sampBandwidth;   //ȡ������
    uint32_t sampFreq;        //ȡ��Ƶ��
    float    sampTimeSet;     //ȡ��ʱ��
    
    
    uint8_t  inactiveCount;//�������ûʱ��
    uint8_t  batLedStatus; //���״̬
    uint8_t  bleLedStatus; //����״̬
    uint8_t  sampLedStatus;//����״̬
    bool     ads1271IsOk;  //ADCоƬ�Ƿ����
    bool     emmcIsOk;     //eMMC�ļ�ϵͳ�Ƿ����
    DWORD    emmc_fre_size;//ʣ��ռ�
    DWORD    emmc_tot_size;//�ܿռ��С
    uint32_t sampClk;      //ȡ��ʱ��Ƶ��
    uint32_t sampPacks;    //�ܹ��ɼ���������,��Ҫ�ֶ��ٸ�������Android
    float    batVoltage;   //��ص�ѹ
    float    batTemp;      //����¶�
    float    objTemp;      //�����¶�
    float    batRemainPercent;//���ٷֱ�
    float    voltageSpd;
    float    voltageADS1271;
	uint32_t periodSpdSignal; //ת���ź�����(us)
    uint32_t sampFileSize; //���β����ļ��ܴ�С
    float    bias;         //�𶯴�����ƫ�õ�ѹ
    float    refV;         //1052�Ĳο���ѹֵ
}SysPara;


typedef struct{
    char   start;
    uint32_t sampSpdSize;  //���β����ٶȵ����ݴ�С
    uint32_t sampShakeSize;//���β����𶯵����ݴ�С
    char  IDPath[128];
    char  NamePath[128];
    float Speed;
    char  SpeedUnits[8];
    float Process;//ƽ���¶�
    float ProcessMin;//��Сֵ
    float ProcessMax;//���ֵ
    char  ProcessUnits[8];//�¶ȵ�λ
    char  DAUID[20];
    char  DetectionType;//�ֶ����0,��ʱ���1
    float Senstivity;//������
    float Zerodrift;//���ƫ��
    int   EUType;
    char  EU[8];//��λ
    int   WindowsType;//
    char  WindowName[20];//����������
    int   StartFrequency;//��ʼƵ��
    int   EndFrequency;//��ֹƵ��
    int   SampleRate;//����Ƶ��
    int   Lines;//����
    int   Averages;//ƽ������
    float AverageOverlap;//�ص���
    int   AverageType;//�ص���ʽ
    int   EnvFilterLow;
    int   EnvFilterHigh;//�����˲�Ƶ�� 
    int   StorageReson;//�ɼ���ʽ
    char  MeasurementComment[128];
    int   IncludeMeasurements;//
    char  Content[4];//����
    float Bias;//�𶯴�����ƫ�õ�ѹ
    char  end;
}ADC_Set;

extern SysPara g_sys_para;
extern ADC_Set g_adc_set;

#endif
