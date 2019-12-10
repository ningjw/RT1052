#include "main.h"


static TaskHandle_t AppTaskCreate_Handle = NULL;      /* 创建任务句柄 */
static void AppTaskCreate(void);                      /* 用于创建任务 */


/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
static void AppTaskCreate(void)
{
    taskENTER_CRITICAL();           //进入临界区
    
    /* 创建BLE_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )AppBLE_Task,"BLE_Task",512,NULL,1,&AppBLE_TaskHandle);
    
    /* 创建ADC_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )AppADC_Task, "ADC_Task",512,NULL, 2,&AppADC_TaskHandle);
                          
    /* 创建eMMC_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )AppEMMC_Task,"eMMC_Task",512,NULL,3,&AppEMMC_TaskHandle);

    vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务

    taskEXIT_CRITICAL();            //退出临界区
}





/***************************************************************************************
  * @brief   入口函数
  * @input   
  * @return  
***************************************************************************************/
int main(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */

    BOARD_BootClockRUN();
    BOARD_InitBootPins();
    BOARD_InitPeripherals();

    BOARD_InitDebugConsole();
    PRINTF("***** Welcome *****\r\n");
//    EEPROM_Test();
    RTC_Config();//实时时钟初始化
    
    SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);//1ms中断，FreeRTOS使用
    
    /* 创建AppTaskCreate任务。参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate, "AppTaskCreate",512,NULL,1,&AppTaskCreate_Handle);
    /* 启动任务调度 */
    if(pdPASS == xReturn) {
        vTaskStartScheduler();   /* 启动任务，开启调度 */
    }else PRINTF("任务创建失败");

    while(1);
}
