#include "main.h"

TimerHandle_t       InactiveTmr = NULL;//软件定时器句柄,用于AT指令接受超时
static TaskHandle_t AppTaskCreate_Handle = NULL;      /* 创建任务句柄 */
static void AppTaskCreate(void);                      /* 用于创建任务 */

SysPara1 g_sys_para1;
SysPara2 g_sys_para2;
/***************************************************************************************
  * @brief   定时处理函数,该参数用于记录开机多久了，每1分钟中断一次
  * @input   
  * @return  
***************************************************************************************/
static void InactiveTmr_Callback(void* parameter)
{
    PRINTF("1分钟定时到了");
    if(g_sys_para2.inactiveCount++ >= g_sys_para1.inactiveTime + 1){//自动关机
        
    }
}



/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
static void AppTaskCreate(void)
{
    taskENTER_CRITICAL();           //进入临界区
    
    /* 创建LED_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )LED_AppTask,"LED_Task",128,NULL, 1,&LED_TaskHandle);
    
    /* 创建Battery_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )BAT_AppTask,"BAT_Task",512,NULL, 2,&BAT_TaskHandle);

    /* 创建BLE_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )BLE_AppTask,"BLE_Task",512,NULL, 3,&BLE_TaskHandle);
    
    /* 创建ADC_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )ADC_AppTask, "ADC_Task",512,NULL, 4,&ADC_TaskHandle);
    
    //创建软件定时器。参数一次为：定时器名称、定时周期、周期模式、唯一id、回调函数
    InactiveTmr = xTimerCreate("PwrOnTmr", 60*1000, pdTRUE, (void*)POWER_ON_TIMER_ID, (TimerCallbackFunction_t)InactiveTmr_Callback);
    
    vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
    taskEXIT_CRITICAL();            //退出临界区
}


/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
static void InitSysPara()
{
    g_sys_para1.inactiveTime = 15;//默认15分钟没有活动后，自动关机。
    g_sys_para2.inactiveCount = 0;
    g_sys_para2.sampLedStatus = WORK_FINE;
    g_sys_para2.batLedStatus = BAT_FULL;
    g_sys_para2.bleLedStatus = BLE_CLOSE;
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
    InitSysPara();
    RTC_Config();//实时时钟初始化
    FlexSPI_NorFlash_Init();
    SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);//1ms中断，FreeRTOS使用
    
    EEPROM_Test();
    NorFlash_IPCommand_Test();
    
    /* 创建AppTaskCreate任务。参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate, "AppTaskCreate",512,NULL,1,&AppTaskCreate_Handle);
    /* 启动任务调度 */
    if(pdPASS == xReturn) {
        vTaskStartScheduler();   /* 启动任务，开启调度 */
    }else PRINTF("任务创建失败");

    while(1);
}
