#include "main.h"


snvs_hp_rtc_datetime_t rtcDate = {
        /* 设置日期 */
    .year = 2019,
    .month = 12,
    .day = 9,
    .hour = 10,
    .minute = 0,
    .second = 0,
};/* 定义 rtc 日期配置结构体 */

/**
  * @brief  初始化RTC相关配置
  * @param  无
  * @retval 无
  */
void RTC_Config(void)
{
	/*------------------------------第一部分----------------------------*/
    snvs_hp_rtc_config_t snvsRtcConfig;/* 定义 snvsRtc 配置结构体 */

    /* 获取默认配置 */
    SNVS_HP_RTC_GetDefaultConfig(&snvsRtcConfig);
    /* 初始化RTC */
    SNVS_HP_RTC_Init(SNVS, &snvsRtcConfig);

	/*------------------------------第二部分----------------------------*/
    /* 将RTC时间设置为默认时间和日期并启动RTC */
    SNVS_HP_RTC_SetDatetime(SNVS, &rtcDate);
    SNVS_HP_RTC_StartTimer(SNVS);
}


