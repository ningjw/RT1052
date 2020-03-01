#include "main.h"

#define ADC_LEN      50000
float SpeedADC[ADC_LEN];
float ShakeADC[ADC_LEN];
char  SpeedStrADC[ADC_LEN * 8 + 1];
char  ShakeStrADC[ADC_LEN * 8 + 1];

#define ADC_MODE_LOW_POWER       GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1)  //低功耗模式
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 0)   //高速模式
#define ADC_MODE_HIGH_RESOLUTION //高精度模式(浮空)
#define ADC_SYNC_HIGH            GPIO_PinWrite(BOARD_ADC_SYNC_GPIO, BOARD_ADC_SYNC_PIN, 1)
#define ADC_SYNC_LOW             GPIO_PinWrite(BOARD_ADC_SYNC_GPIO, BOARD_ADC_SYNC_PIN, 0)
TaskHandle_t ADC_TaskHandle = NULL;  /* ADC任务句柄 */

static uint32_t counterClock = 0;
uint32_t timeCapt = 0;
char str[12];
extern volatile uint32_t g_eventTimeMilliseconds;
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
    else if( PIT_GetStatusFlags(PIT, kPIT_Chnl_2) == true) {
        /* 清除中断标志位.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_2, kPIT_TimerFlag);
		g_eventTimeMilliseconds++;
        if(g_sys_para.inactiveCount++ >= (g_sys_para.inactiveTime + 1)*60-5) { //定时时间到
            GPIO_PinWrite(BOARD_SYS_PWR_OFF_GPIO, BOARD_SYS_PWR_OFF_PIN, 1);
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
    if(g_sys_para.spdCount < ADC_LEN) {
        SpeedADC[g_sys_para.spdCount++] = ADC_ETC_GetADCConversionValue(ADC_ETC, 0U, 0U); /* Get trigger0 chain0 result. */
    }

    uint32_t wait_time = 0;
    while (1) { //wait ads1271 ready
        if(ADC_READY == 0) {
            if( g_sys_para.shkCount < ADC_LEN) {
                ShakeADC[g_sys_para.shkCount++] = LPSPI4_ReadData();
            }
            break;
        }
        if(wait_time++ >= 5) break;
    }
	
	if(g_sys_para.spdCount >= g_sys_para.sampNumber){
		ADC_SampleStop();
	}
}


/***************************************************************************************
  * @brief   start adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStart(void)
{
    g_sys_para.spdCount = 0;
    g_sys_para.shkCount = 0;
    g_sys_para.Ltc1063Clk = 1000 * g_adc_set.SampleRate / 25;

	//判断自动关机条件
    if(g_sys_para.inactiveCondition != 1) {
        g_sys_para.inactiveCount = 0;
    }
	//判断点数是否超出数组界限
	if(g_sys_para.sampNumber > ADC_LEN){
		g_sys_para.sampNumber = ADC_LEN;
	}
    vTaskSuspend(BAT_TaskHandle);
    vTaskSuspend(LED_TaskHandle);
    /* Setup the PWM mode of the timer channel 用于LTC1063FA的时钟输入,控制采样带宽*/
    QTMR_SetupPwm(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, g_sys_para.Ltc1063Clk, 50U, false, QUADTIMER3_CHANNEL_0_CLOCK_SOURCE);
    /* Set channel 0 period (66000000 ticks). 用于触发内部ADC采样*/
    PIT_SetTimerPeriod(PIT1_PERIPHERAL, kPIT_Chnl_0, PIT1_CLK_FREQ / g_adc_set.SampleRate);
    /* Set channel 1 period (66000000 ticks). 用于控制采样时间*/
//    PIT_SetTimerPeriod(PIT1_PERIPHERAL, kPIT_Chnl_1, PIT1_CLK_FREQ/1000 * g_sys_para.sampNumber);
    /* Start the timer - select the timer counting mode. */
    QTMR_StartTimer(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
    /* Start the timer - select the timer counting mode */
    QTMR_StartTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
    /* Start channel 0. */
    PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
    /* Start channel 1. */
//    PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_1);
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
//    PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_1);
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
    if(LPSPI4_ReadData() == 0) {
        g_sys_para.sampLedStatus = WORK_FATAL_ERR;
    }
    
//    while(1) {
//        
//        while(ADC_READY == 0);
//        if( g_sys_para.shkCount < 5000) {
//            ShakeADC[g_sys_para.shkCount++] = LPSPI4_ReadData();
//        } else {
//            /* 将震动信号转换*/
//            memset(ShakeStrADC, 0, sizeof(ShakeStrADC));
//            int pos = 0;
//            g_sys_para.voltageADS1271 = 0;
//            for(uint32_t i = 0; i < g_sys_para.shkCount; i++) {
//                ShakeADC[i] = ShakeADC[i] * g_sys_para.bias * 1.0f / 0x800000;
//                memset(str, 0, sizeof(str));
//                sprintf(str, "%01.5f,", ShakeADC[i]);
//                memcpy(ShakeStrADC + pos, str, 8);
//                pos += 8;
//            }
//            PRINTF("共采样到 %d 个震动信号\r\n", g_sys_para.shkCount);
//            PRINTF("%s",ShakeStrADC);
//            break;
//        }
//    }


    PRINTF("ADC Task Create and Running\r\n");
    while(1)
    {
        /*等待ADC完成采样事件*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &r_event, portMAX_DELAY);

        /* 判断是否成功等待到事件 */
        if ( pdTRUE == xReturn ) {
			
            /* 完成采样事件*/
            if(r_event & NOTIFY_FINISH) {
                /* 计算转速信号周期 */
                g_sys_para.periodSpdSignal = (timeCapt * 1000) / counterClock;

                PRINTF("设置的采样频率为:%d Hz\r\n", g_adc_set.SampleRate);
                PRINTF("设置的采样点数:%d ms\r\n", g_sys_para.sampNumber);
                PRINTF("计算出转速信号周期:%d us\r\n", g_sys_para.periodSpdSignal);
                PRINTF("共采样到 %d 个震动信号\r\n", g_sys_para.shkCount);
                PRINTF("共采样到 %d 个转速信号\r\n", g_sys_para.spdCount);

                /* ---------------将震动信号转换-----------------------*/
                memset(ShakeStrADC, 0, sizeof(ShakeStrADC));
                int pos = 0;
                g_sys_para.voltageADS1271 = 0;
                for(uint32_t i = 0; i < g_sys_para.shkCount; i++) {
                    ShakeADC[i] = ShakeADC[i] * g_sys_para.bias * 1.0f / 0x800000;
                    memset(str, 0, sizeof(str));
                    sprintf(str, "%01.5f,", ShakeADC[i]);
                    memcpy(ShakeStrADC + pos, str, 8);
                    pos += 8;
//                    g_sys_para.voltageADS1271 += ShakeADC[i];
                }
				//计算发送震动信号需要多少个包
				g_sys_para.shkPacks = (g_sys_para.shkCount / 13) +  (g_sys_para.shkCount%13?1:0);
//                g_sys_para.voltageADS1271 /= g_sys_para.shkCount;
                g_sys_para.voltageADS1271 = ShakeADC[g_sys_para.shkCount - 1];
				
                /* ------------------将转速信号转换--------------------*/
                memset(SpeedStrADC, 0, sizeof(SpeedStrADC));
                pos = 0;
                g_sys_para.voltageSpd = 0;
                for(uint32_t i = 0; i < g_sys_para.spdCount; i++) {
                    SpeedADC[i] = SpeedADC[i] * g_sys_para.refV / 4096.0f;
                    memset(str, 0, sizeof(str));
                    sprintf(str, "%01.5f,", SpeedADC[i]);
                    memcpy(SpeedStrADC + pos, str, 8);
                    pos += 8;
//                  g_sys_para.voltageSpd += SpeedADC[i];
                }
//                g_sys_para.voltageSpd /= g_sys_para.spdCount;
                g_sys_para.voltageSpd = SpeedADC[g_sys_para.spdCount - 1];
				//计算发送震动信号需要多少个包
				g_sys_para.spdPacks = (g_sys_para.spdCount / 13) +  (g_sys_para.spdCount%13?1:0);
				
				//计算将一次采集数据全部发送到Android需要多少个包
				g_sys_para.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 5;
				
                /* -----------将采用数据打包成json格式,并保存到文件中-----*/
				
//                PacketSampleData();

                /* 发送任务通知，并解锁阻塞在该任务通知下的任务 */
                xTaskNotifyGive( BLE_TaskHandle);
            }
        }
    }
}



