#include "main.h"

TaskHandle_t AppBLE_TaskHandle = NULL;  /* 蓝牙任务句柄 */
TaskHandle_t AppADC_TaskHandle = NULL;  /* ADC任务句柄 */
TaskHandle_t AppEMMC_TaskHandle = NULL; /* eMMC任务句柄 */




/***********************************************************************
  * @ 函数名  ： AppBLE_Task
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void AppBLE_Task(void)
{
    
    while(1)
    {
        /* 获取日期 */
        SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
        /* 打印日期&时间 */ 
        PRINTF("BLE TASK:%02d-%02d-%02d  %02d:%02d:%02d \r\n", rtcDate.year,rtcDate.month, rtcDate.day,rtcDate.hour, rtcDate.minute, rtcDate.second);
        vTaskDelay(1000);
    }
}

/***********************************************************************
  * @ 函数名  ： AppADC_Task
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void AppADC_Task(void)
{
    while(1)
    {
        PRINTF("ADC TASK\r\n");
        vTaskDelay(1000);
    }
}

/***********************************************************************
  * @ 函数名  ： AppADC_Task
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void AppEMMC_Task(void)
{
    while(1)
    {
        PRINTF("eMMC TASK\r\n");
        vTaskDelay(1000);
    }
}



