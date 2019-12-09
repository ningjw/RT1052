#include "main.h"


snvs_hp_rtc_datetime_t rtcDate;/* ���� rtc �������ýṹ�� */

/**
  * @brief  ��ʼ��RTC�������
  * @param  ��
  * @retval ��
  */
void RTC_Config(void)
{
	/*------------------------------��һ����----------------------------*/
    snvs_hp_rtc_config_t snvsRtcConfig;/* ���� snvsRtc ���ýṹ�� */

    /* ��ʼ��SNVS */
    /*
     * snvsConfig->rtccalenable = false;
     * snvsConfig->rtccalvalue = 0U;
     * snvsConfig->srtccalenable = false;
     * snvsConfig->srtccalvalue = 0U;
     * snvsConfig->PIFreq = 0U;
     */
    /* ��ȡĬ������ */
    SNVS_HP_RTC_GetDefaultConfig(&snvsRtcConfig);
    /* ��ʼ��RTC */
    SNVS_HP_RTC_Init(SNVS, &snvsRtcConfig);

	/*------------------------------�ڶ�����----------------------------*/
    /* �������� */
    rtcDate.year = 2019;
    rtcDate.month = 12;
    rtcDate.day = 9;
    rtcDate.hour = 10;
    rtcDate.minute = 0;
    rtcDate.second = 0;
    
    /* ��RTCʱ������ΪĬ��ʱ������ڲ�����RTC */
    SNVS_HP_RTC_SetDatetime(SNVS, &rtcDate);
    SNVS_HP_RTC_StartTimer(SNVS);
}










