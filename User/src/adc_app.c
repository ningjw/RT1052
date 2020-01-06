#include "main.h"

#define ADC_LEN      40000
#define ADC_STR_LEN  200000

AT_NONCACHEABLE_SECTION_INIT(float SpeedADC[ADC_LEN]);
AT_NONCACHEABLE_SECTION_INIT(float ShakeADC[ADC_LEN]);
AT_NONCACHEABLE_SECTION_INIT(char  StrSpeedADC[ADC_STR_LEN]);
AT_NONCACHEABLE_SECTION_INIT(char  StrShakeADC[ADC_STR_LEN]);

#define ADC_MODE_LOW_POWER       GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1)  //低功耗模式
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 0)   //高速模式
#define ADC_MODE_HIGH_RESOLUTION //高精度模式(浮空)


TaskHandle_t ADC_TaskHandle = NULL;  /* ADC任务句柄 */


static uint32_t counterClock = 0;
uint32_t timeCapt = 0;
char str[12];

/***************************************************************************************
  * @brief   kPIT_Chnl_0用于触发ADC采样 ；kPIT_Chnl_1 用于定时采样; kPIT_Chnl_2用于定时关机1分钟中断
  * @input
  * @return
***************************************************************************************/
void PIT_IRQHandler(void)
{
    if( PIT_GetStatusFlags(PIT, kPIT_Chnl_1) == true ) {
        /* 清除中断标志位.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag);
        ADC_SampleStop();
    }
    else if( PIT_GetStatusFlags(PIT, kPIT_Chnl_2) == true){
        /* 清除中断标志位.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_2, kPIT_TimerFlag);
         if(g_sys_para.inactiveCount++ >= g_sys_para.inactiveTime + 1){//定时时间到
            GPIO_PinWrite(BOARD_SYS_PWR_OFF_GPIO,BOARD_SYS_PWR_OFF_PIN,1);
    //        SNVS->LPSR |= SNVS_LPCR_DP_EN(1);
    //        SNVS->LPSR |= SNVS_LPCR_TOP(1);
        }
    }
    
    __DSB();
}

/***************************************************************************************
  * @brief  定时器用于获取转速信号的周期(频率)
  * @input
  * @return
***************************************************************************************/
void TMR1_IRQHandler(void)
{
    /* 清除中断标志 */
    QTMR_ClearStatusFlags(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_EdgeFlag);
    timeCapt = QUADTIMER1_PERIPHERAL->CHANNEL[QUADTIMER1_CHANNEL_0_CHANNEL].CAPT;//读取寄存器值
}


/***************************************************************************************
  * @brief   用于获取转速信号的电压值,该采集通过PIT1的Channel0触发
  * @input
  * @return
***************************************************************************************/
void ADC_ETC_IRQ0_IRQHandler(void)
{
    /*清除转换完成中断标志位*/
    ADC_ETC_ClearInterruptStatusFlags(ADC_ETC, (adc_etc_external_trigger_source_t)0U, kADC_ETC_Done0StatusFlagMask);
    /*读取转换结果*/
    if(g_sys_para.ADC_SpdCnt < ADC_LEN){
        SpeedADC[g_sys_para.ADC_SpdCnt++] = ADC_ETC_GetADCConversionValue(ADC_ETC, 0U, 0U); /* Get trigger0 chain0 result. */
    }
#if 0
    if( g_sys_para.ADC_ShakeCnt < ADC_LEN && ADC_READY == 0){
        ShakeADC[g_sys_para.ADC_ShakeCnt++] = LPSPI4_ReadData();
    }
//    else if(g_sys_para.ADC_ShakeCnt > 0){
//        ShakeADC[g_sys_para.ADC_ShakeCnt] = ShakeADC[g_sys_para.ADC_ShakeCnt-1];
//        g_sys_para.ADC_ShakeCnt++;
//    }
#else
    uint32_t wait_time = 0;
    while (1){//wait ads1271 ready
        if(ADC_READY == 0) break;
        if(wait_time++ >= 100) break;
    }
    if( g_sys_para.ADC_ShakeCnt < ADC_LEN && wait_time < 100 ){
        ShakeADC[g_sys_para.ADC_ShakeCnt++] = LPSPI4_ReadData();
    }
#endif
}


/***************************************************************************************
  * @brief   start adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStart(void)
{
    g_sys_para.ADC_SpdCnt = 0;
    g_sys_para.ADC_ShakeCnt = 0;
    g_sys_para.sampClk = 1000 * g_sys_para.sampFreq / 25;

    if(g_sys_para.inactiveCondition != 1){
        g_sys_para.inactiveCount = 0;
    }
    
    vTaskSuspend(BAT_TaskHandle);
    vTaskSuspend(LED_TaskHandle);
    /* Setup the PWM mode of the timer channel */
    QTMR_SetupPwm(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, g_sys_para.sampClk, 50U, false, QUADTIMER3_CHANNEL_0_CLOCK_SOURCE);
    /* Set channel 0 period (66000000 ticks). */
    PIT_SetTimerPeriod(PIT1_PERIPHERAL, kPIT_Chnl_0, PIT1_CLK_FREQ/g_sys_para.sampFreq - 1);
    /* Set channel 1 period (66000000 ticks). */
    PIT_SetTimerPeriod(PIT1_PERIPHERAL, kPIT_Chnl_1, PIT1_CLK_FREQ/g_sys_para.sampTimeSet - 1);
    /* Start the timer - select the timer counting mode */
    QTMR_StartTimer(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
    /* Start the timer - select the timer counting mode */
    QTMR_StartTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
    /* Start channel 0. */
    PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
    /* Start channel 1. */
    PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_1);
}


/***************************************************************************************
  * @brief   stop adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStop(void)
{
    /* Stop the timer */
    QTMR_StopTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL);
//    QTMR_StopTimer(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL);
    /* Stop channel 0. */
    PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
    /* Stop channel 1. */
    PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_1);
    vTaskResume(BAT_TaskHandle);
    vTaskResume(LED_TaskHandle);
    /* 触发ADC采样完成事件  */
    xTaskNotify(ADC_TaskHandle, NOTIFY_FINISH, eSetBits);
}


/***********************************************************************
  * @ 函数名  ： ADC采集任务
  * @ 功能说明：
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void ADC_AppTask(void)
{
    uint32_t r_event;
    BaseType_t xReturn = pdTRUE;
    /* 配置ADC的外部触摸模式 */
    ADC_ETC_Config();
    XBARA_Configuration();
    
    /* 使能LPSPI4用于读取ADS1271的值*/
    LPSPI_Enable(LPSPI4, true); 
    
    /* 初始化counterClock,用于计算捕获周期 */
    counterClock = QUADTIMER1_CHANNEL_0_CLOCK_SOURCE / 1000;
    
    ADC_MODE_HIGH_SPEED;
    
    /* 等待ADS1271 ready,并读取电压值,如果没有成功获取电压值, 则闪灯提示 */
    while (ADC_READY == 0);  //wait ads1271 ready
    if(LPSPI4_ReadData() == 0){
        g_sys_para.sampLedStatus = WORK_FATAL_ERR;
    }

    PRINTF("ADC Task Create and Running\r\n");
    while(1)
    {
        /*等待ADC完成采样事件*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &r_event, portMAX_DELAY);
        
        /* 判断是否成功等待到事件 */
        if ( pdTRUE == xReturn ) {
            
            /* 完成采样事件*/
            if(r_event & NOTIFY_FINISH){
                /* 计算转速信号周期 */
                g_sys_para.periodSpdSignal = (timeCapt * 1000) / counterClock;
                
                PRINTF("设置的采样频率为:%d Hz\r\n", g_sys_para.sampFreq);
                PRINTF("设置的采样时间:%d s\r\n", g_sys_para.sampTimeSet);
                PRINTF("计算出转速信号周期:%d us\r\n",g_sys_para.periodSpdSignal);
                PRINTF("共采样到 %d 个震动信号\r\n", g_sys_para.ADC_ShakeCnt);
                PRINTF("共采样到 %d 个转速信号\r\n", g_sys_para.ADC_SpdCnt);
                
                /* 将震动信号转换*/
                memset(StrShakeADC, 0, sizeof(StrShakeADC));
                for(uint32_t i = 0; i< g_sys_para.ADC_ShakeCnt; i++){
                    ShakeADC[i] = ShakeADC[i] * g_sys_para.bias  / 0x400000;
                    memset(str, 0, sizeof(str));
                    sprintf(str,"%1.6f,",ShakeADC[i]);
                    strcat(StrShakeADC,str);
                }
                
                /* 将转速信号转换*/
                memset(StrSpeedADC, 0, sizeof(StrSpeedADC));
                for(uint32_t i = 0; i< g_sys_para.ADC_SpdCnt; i++){
                    SpeedADC[i] = SpeedADC[i] * g_sys_para.refV / 4096;
                    memset(str, 0, sizeof(str));
                    sprintf(str,"%1.6f,",SpeedADC[i]);
                    strcat(StrSpeedADC,str);
                }
                
                /* 将采用数据打包成json格式,并保存到文件中*/
                ParseSampleData();
                
                /* 发送任务通知，并解锁阻塞在该任务通知下的任务 */
                vTaskNotifyGiveFromISR( BLE_TaskHandle, (void *)pdFALSE);
            }
        }
    }
}



