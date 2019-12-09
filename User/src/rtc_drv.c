#include "main.h"


snvs_hp_rtc_datetime_t rtcDate;/* 定义 rtc 日期配置结构体 */

/**
  * @brief  初始化RTC相关配置
  * @param  无
  * @retval 无
  */
void RTC_Config(void)
{
	/*------------------------------第一部分----------------------------*/
    snvs_hp_rtc_config_t snvsRtcConfig;/* 定义 snvsRtc 配置结构体 */

    /* 初始化SNVS */
    /*
     * snvsConfig->rtccalenable = false;
     * snvsConfig->rtccalvalue = 0U;
     * snvsConfig->srtccalenable = false;
     * snvsConfig->srtccalvalue = 0U;
     * snvsConfig->PIFreq = 0U;
     */
    /* 获取默认配置 */
    SNVS_HP_RTC_GetDefaultConfig(&snvsRtcConfig);
    /* 初始化RTC */
    SNVS_HP_RTC_Init(SNVS, &snvsRtcConfig);

	/*------------------------------第二部分----------------------------*/
    /* 设置日期 */
    rtcDate.year = 2019;
    rtcDate.month = 12;
    rtcDate.day = 9;
    rtcDate.hour = 10;
    rtcDate.minute = 0;
    rtcDate.second = 0;
    
    /* 将RTC时间设置为默认时间和日期并启动RTC */
    SNVS_HP_RTC_SetDatetime(SNVS, &rtcDate);
    SNVS_HP_RTC_StartTimer(SNVS);
}










