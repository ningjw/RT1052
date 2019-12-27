#include "main.h"

#define NOTIFY_START     (1<<0)
#define NOTIFY_TMR1      (1<<1)
#define NOTIFY_ADC       (1<<2)
#define NOTIFY_ADS1271   (1<<3)
#define NOTIFY_FINISH    (1<<4)


#define ADC_MODE_LOW_POWER       GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1)  //低功耗模式
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 0)   //高速模式
#define ADC_MODE_HIGH_RESOLUTION //高精度模式(浮空)
#define ADC_READY                GPIO_PinRead(BOARD_ADC_RDY_GPIO, BOARD_ADC_RDY_PIN)

TaskHandle_t ADC_TaskHandle = NULL;  /* ADC任务句柄 */

static volatile uint32_t ADC_ConvertedValue;
static uint32_t counterClock = 0;
static uint32_t timeCapt = 0;

extern volatile uint32_t g_eventTimeMilliseconds;
/***************************************************************************************
  * @brief   kPIT_Chnl_0用于触发ADC采样 ；kPIT_Chnl_1 配置为1ms中断
  * @input
  * @return
***************************************************************************************/
void PIT_IRQHandler(void)
{
    if( PIT_GetStatusFlags(PIT, kPIT_Chnl_1) == true ) {
        /* 清除中断标志位.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag);
        g_eventTimeMilliseconds++;
        if(g_sys_para2.sampStart && g_sys_para2.sampTimeCnt){
            g_sys_para2.sampTimeCnt --;
            if(g_sys_para2.sampTimeCnt == 0){
                ADC_SampleStop();
            }
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
    /* 触发一个事件  */
    xTaskNotify(ADC_TaskHandle, NOTIFY_TMR1, eSetBits);
}

float ADC_Voltage = 0;
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
    ADC_ConvertedValue = ADC_ETC_GetADCConversionValue(ADC_ETC, 0U, 0U); /* Get trigger0 chain0 result. */
    /* 触发一个事件  */
    xTaskNotify(ADC_TaskHandle, NOTIFY_ADC, eSetBits);
}


/***************************************************************************************
  * @brief   start adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStart(void)
{
    /* Setup the PWM mode of the timer channel */
    QTMR_SetupPwm(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, g_sys_para1.sampClk, 50U, false, QUADTIMER3_CHANNEL_0_CLOCK_SOURCE);
    /* Start the timer - select the timer counting mode */
    QTMR_StartTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
    /* Set channel 0 period to 1 s (66000000 ticks). */
    PIT_SetTimerPeriod(PIT1_PERIPHERAL, kPIT_Chnl_0, PIT1_CLK_FREQ/g_sys_para1.sampFreq - 1);
    /* Start channel 0. */
    PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
    
    g_sys_para2.sampStart = true;
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
    /* Stop channel 0. */
    PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
    
    g_sys_para2.sampStart = false;
    
    /* 触发ADC采样完成事件  */
    xTaskNotify(ADC_TaskHandle, NOTIFY_FINISH, eSetBits);
}

/***********************************************************************
  * @ 函数名  ： ADC_AppTask
  * @ 功能说明：
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void ADC_AppTask(void)
{
    uint32_t r_event;
    BaseType_t xReturn = pdTRUE;
    ADC_ETC_Config();
    XBARA_Configuration();
    LPSPI_Enable(LPSPI4, true);                //Enable LPSPI4
    counterClock = QUADTIMER1_CHANNEL_0_CLOCK_SOURCE / 1000;
    PRINTF("ADC Task Create and Running\r\n");
    while(1)
    {
        /*wait task notify*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &r_event, portMAX_DELAY);
        if ( pdTRUE == xReturn ) {
            if(r_event & NOTIFY_TMR1) {
                g_sys_para2.periodSpdSignal = (timeCapt * 1000) / counterClock;
            }
            if(r_event & NOTIFY_ADC) {
                g_sys_para2.voltageSpdSignal = ADC_ConvertedValue * 3.3 / 4096.0;

                if (ADC_READY == 0) { //check ads1271 ready
                    LPSPI4_ReadWriteByte();
                }
            }
            if(r_event & NOTIFY_FINISH){//完成采样
                
            }
        }
    }
}



