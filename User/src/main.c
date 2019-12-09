#include "main.h"


static TaskHandle_t AppTaskCreate_Handle = NULL;      /* 创建任务句柄 */
static void AppTaskCreate(void);               /* 用于创建任务 */




/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
static void AppTaskCreate(void)
{
//    taskENTER_CRITICAL();           //进入临界区
    while(1){
        vTaskDelay(1000);
                /* 获取日期 */
        SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
        /* 打印日期&时间 */ 
        PRINTF("%02d-%02d-%02d  %02d:%02d:%02d \r\n", rtcDate.year,rtcDate.month, rtcDate.day,rtcDate.hour, rtcDate.minute, rtcDate.second);
    }
//    vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务

//    taskEXIT_CRITICAL();            //退出临界区
}



int main(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */

    BOARD_BootClockRUN();
    BOARD_InitBootPins();
    BOARD_InitPeripherals();

    BOARD_InitDebugConsole();
    PRINTF("***** Welcome *****\r\n");

    RTC_Config();
    
    SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);//1ms中断，FreeRTOS使用

    /* 创建AppTaskCreate任务 */
    xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                          (const char*    )"AppTaskCreate",/* 任务名字 */
                          (uint16_t       )512,  /* 任务栈大小 */
                          (void*          )NULL,/* 任务入口函数参数 */
                          (UBaseType_t    )1, /* 任务的优先级 */
                          (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */
    /* 启动任务调度 */
    if(pdPASS == xReturn) {
        vTaskStartScheduler();   /* 启动任务，开启调度 */
    }


    while(1);
}
