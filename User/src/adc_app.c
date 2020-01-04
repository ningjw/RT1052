#include "main.h"


#define NOTIFY_TMR1      (1<<1)
#define NOTIFY_ADC       (1<<2)
#define NOTIFY_ADS1271   (1<<3)
#define NOTIFY_FINISH    (1<<4)


#define ADC_LEN 40000
uint32_t ADC_SpdCnt = 0;
uint32_t ADC_ShakeCnt = 0;
AT_NONCACHEABLE_SECTION_INIT(float SpeedADC[ADC_LEN]);
AT_NONCACHEABLE_SECTION_INIT(float ShakeADC[ADC_LEN]);

#define ADC_MODE_LOW_POWER       GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1)  //低功耗模式
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 0)   //高速模式
#define ADC_MODE_HIGH_RESOLUTION //高精度模式(浮空)


TaskHandle_t ADC_TaskHandle = NULL;  /* ADC任务句柄 */

static volatile uint32_t ADC_ConvertedValue;
static uint32_t counterClock = 0;
static uint32_t timeCapt = 0;


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
    if( PIT_GetStatusFlags(PIT, kPIT_Chnl_2) == true){
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
    if(ADC_SpdCnt < ADC_LEN){
        SpeedADC[ADC_SpdCnt++] = ADC_ETC_GetADCConversionValue(ADC_ETC, 0U, 0U); /* Get trigger0 chain0 result. */
    }
    while (ADC_READY == 0);  //wait ads1271 ready
    if( ADC_ShakeCnt < ADC_LEN){
        ShakeADC[ADC_ShakeCnt++] = LPSPI4_ReadData();
    }
}


/***************************************************************************************
  * @brief   start adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStart(void)
{
    ADC_SpdCnt = 0;
    ADC_ShakeCnt = 0;
    g_sys_para.sampClk = 1000 * g_sys_para.sampFreq / 25;

    if(g_sys_para.inactiveCondition != 1){
        g_sys_para.inactiveCount = 0;
    }
    vTaskSuspend(BAT_TaskHandle);
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
    QTMR_StopTimer(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL);
    /* Stop channel 0. */
    PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
    /* Stop channel 1. */
    PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_1);
    vTaskResume(BAT_TaskHandle);
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
                
                /* 将转速信号转换*/
                for(uint32_t i = 0; i< ADC_SpdCnt; i++){
                    SpeedADC[i] = SpeedADC[i] * 3.3f / 4096.0f;
                }
                /* 将震动信号转换*/
                for(uint32_t i = 0; i< ADC_ShakeCnt; i++){
                    ShakeADC[i] = ShakeADC[i] * 2.43f * 2 / 8388607;
                }
                
                /* 计算震动信号周期 */
                g_sys_para.periodSpdSignal = (timeCapt * 1000) / counterClock;
                
                //将本次采样数据保存到文件
                eMMC_SaveSampleData();
            }
        }
    }
}



