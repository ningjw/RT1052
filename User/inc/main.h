#ifndef __MAIN_H
#define __MAIN_H

#define SOFT_VERSION       "0.15"
#define HARD_VERSION       "1.1"

#define BLE_VERSION
//#define WIFI_VERSION 
#define HDV_1_0

#define ULONG_MAX          0xFFFFFFFF
#define EVT_OK       (1 << 0)
#define EVT_TIMTOUT  (1 << 1)


#define APP_INFO_SECTOR    128 /* ������Ϣ������NorFlash�ĵ�63������*/
#define APP_START_SECTOR   129 /* App���ݴӵ�64��������ʼ���� */
#define ADC_INFO_SECTOR    256 //���ڹ���ADC����
#define MAX_ADC_INFO       2047//��10��sector���ڹ���ADC��������, ÿ����������ռ��20byte, �����Ա���40960/20=2048��
#define ADC_DATA_SECTOR    266
#define MAX_SECTOR         8192

#ifdef BLE_VERSION
	#define FIRM_ONE_PACKE_LEN 166 
	#define FIRM_ONE_LEN (FIRM_ONE_PACKE_LEN - 6)
#elif defined WIFI_VERSION
	#define FIRM_ONE_PACKE_LEN 1006
	#define FIRM_ONE_LEN (FIRM_ONE_PACKE_LEN - 6)
#endif

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
#include "fsl_dcdc.h"
#include "fsl_src.h"
#include "fsl_pwm.h"
#include "fsl_gpc.h"
#include "fsl_gpt.h"


#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "queue.h"
#include "timers.h"

#include "cJSON.h"
#include "interrupt.h"
#include "adc_drv.h"
#include "battery_drv.h"
#include "iic_temp_drv.h"
#include "norflash_drv.h"
#include "utility.h"
#include "flexspi.h"
#include "si5351_drv.h"
#include "wifi_app.h"

#include "protocol.h"
#include "ble_app.h"
#include "adc_app.h"
#include "battery_app.h"
#include "led_app.h"
#include "norflash_app.h"

#include "lpm.h"
#include "power_mode_switch_bm.h"
#include "specific.h"

typedef struct{
	uint32_t totalAdcInfo;
	uint32_t addrOfNewInfo;
	uint32_t addrOfNewData;
	uint32_t freeOfKb;
	uint32_t bakup;//��
}AdcInfoTotal;

typedef struct{
	uint32_t AdcDataAddr;//ADC���ݵ�ַ
	uint32_t AdcDataLen; //ADC���ݳ���
	char  AdcDataTime[12];//ADC���ݲɼ�ʱ��
}AdcInfo;

//�ýṹ�嶨������Ҫ���浽EEPROM�еĲ���
typedef struct{
    uint8_t  firmUpdate;     //�̼�����
    uint32_t firmSizeTotal;  //�̼��ܴ�С
    uint32_t firmCrc16;      //�̼�У����
    uint32_t firmPacksTotal; //�̼��ܰ���
//	uint32_t batEnergyInFlash;
	
    uint32_t firmPacksCount; //��ǰ���ܵĹ̼�����
    uint32_t firmSizeCurrent;//��ǰ���ܵ��Ĺ̼���С
    uint32_t firmCurrentAddr;//��һ��������Ҫ����ĵ�ַ
	uint32_t firmByteCount;  //��ǰ���ܵ����ֽ���
	
	uint32_t inactiveCount;  //�������ûʱ��
    uint8_t  inactiveTime;   //�������ûʱ��
    uint8_t  batAlarmValue;  //��ص�������ֵ
    uint8_t  inactiveCondition;//�������ô�������

    uint8_t  batLedStatus; //���״̬
    uint8_t  BleWifiLedStatus; //����״̬
    uint8_t  sampLedStatus;//����״̬
    bool     ads1271IsOk;  //ADCоƬ�Ƿ����
    bool     emmcIsOk;     //eMMC�ļ�ϵͳ�Ƿ����
	float    batVoltage;   //��ص�ѹ
    float    batTemp;      //����¶�
    float    objTemp;      //�����¶�
	float    envTemp;      //�����¶�
    float    batRemainPercent;//���ٷֱ�
	
    uint32_t sampNumber;  //ȡ��ʱ��
    uint32_t Ltc1063Clk;  //ȡ��ʱ��Ƶ��
    char*    sampJson;     //�Ѿ������json��ʽ�����ݵ��׵�ַ
	
	uint32_t sampPacksCnt; //������
    
	uint32_t spdPacks;     //ת���ź���Ҫ�ֶ��ٸ����������
	uint32_t shkPacks;     //���ź���Ҫ�ֶ��ٸ����������
	
    float    voltageADS1271;
	uint32_t periodSpdSignal;//ת���ź�����(us)
    
	uint32_t tempCount;  //��ǰ��¼���¶ȸ���
	bool     WorkStatus; //����ָʾ��ǰ�Ƿ����ڲɼ�.
	
    
    char     fileName[20];
    char     earliestFile[20];
}SysPara;


typedef struct{
    char  IDPath[128];
    char  NamePath[128];
	char  SpeedUnits[8];
	char  ProcessUnits[8];//�¶ȵ�λ
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
	int   IncludeMeasurements;//
	
    float Speed;//ƽ��ת��
    float Process;//ƽ���¶�
    float ProcessMin;//��Сֵ
    float ProcessMax;//���ֵ
    
    int   StorageReson;//�ɼ���ʽ
    char  MeasurementComment[128];
    char  DAUID[20];
    char  Content[4];//����
	
	float    bias;       //�𶯴�����ƫ�õ�ѹ
    float    refV;       //1052�Ĳο���ѹֵ
	uint32_t sampPacks;	   //�ܹ��ɼ���������,��Ҫ�ֶ��ٸ�������Android
	uint32_t spdCount;   //ת���źŲɼ����ĸ���
    uint32_t shkCount;   //���źŲɼ����ĸ���
}ADC_Set;

extern SysPara g_sys_para;
extern ADC_Set g_adc_set;
extern snvs_lp_srtc_datetime_t SNVS_LP_dateTimeStruct;
void BOARD_InitDebugConsole(void);
#endif
