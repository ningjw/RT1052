#include "main.h"




/**
  * @brief  ��ʼ��RTC�������
  * @param  ��
  * @retval ��
  */
void RTC_Config(void)
{
	/*------------------------------��һ����----------------------------*/
    snvs_hp_rtc_config_t snvsRtcConfig;/* ���� snvsRtc ���ýṹ�� */
	
    /* ��ȡĬ������ */
    SNVS_HP_RTC_GetDefaultConfig(&snvsRtcConfig);
    /* ��ʼ��RTC */
    SNVS_HP_RTC_Init(SNVS, &snvsRtcConfig);
	
	/*------------------------------�ڶ�����----------------------------*/
    /* ��RTCʱ������ΪĬ��ʱ������ڲ�����RTC */
//    SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
//    SNVS_HP_RTC_SetDatetime(SNVS, &rtcDate);
    SNVS_HP_RTC_StartTimer(SNVS);
}


