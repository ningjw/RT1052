#include "main.h"


snvs_hp_rtc_datetime_t rtcDate = {
        /* �������� */
    .year = 2019,
    .month = 12,
    .day = 9,
    .hour = 10,
    .minute = 0,
    .second = 0,
};/* ���� rtc �������ýṹ�� */

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
    SNVS_HP_RTC_SetDatetime(SNVS, &rtcDate);
    SNVS_HP_RTC_StartTimer(SNVS);
}


