#include "main.h"

float SpeedADC[ADC_LEN];
float ShakeADC[ADC_LEN];
float Temperature[64];
char  SpeedStrADC[ADC_LEN * 4 + 1];
char  VibrateStrADC[ADC_LEN * 4 + 1];

#define ADC_MODE_LOW_POWER       GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1)  //低功耗模式
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 0)   //高速模式
#define ADC_MODE_HIGH_RESOLUTION //高精度模式(浮空)
#define ADC_SYNC_HIGH            GPIO_PinWrite(BOARD_ADC_SYNC_GPIO, BOARD_ADC_SYNC_PIN, 1)
#define ADC_SYNC_LOW             GPIO_PinWrite(BOARD_ADC_SYNC_GPIO, BOARD_ADC_SYNC_PIN, 0)

#define PWR_EN_LOW    GPIO_PinWrite(BOARD_PWR_EN_GPIO, BOARD_PWR_EN_PIN, 0)
#define PWR_EN_HIGH   GPIO_PinWrite(BOARD_PWR_EN_GPIO, BOARD_PWR_EN_PIN, 1)


TaskHandle_t ADC_TaskHandle = NULL;  /* ADC任务句柄 */

uint32_t timeCapt = 0;
char str[12];

uint32_t ADC_ShakeValue = 0;
uint32_t  ADC_InvalidCnt = 0;

extern lpspi_transfer_t spi_tranxfer;
extern void QuadTimer1_init(void);
/***************************************************************************************
  * @brief   kPIT_Chnl_0用于触发ADC采样 ；kPIT_Chnl_1 用于定时采样; kPIT_Chnl_2用于定时关机1分钟中断
  * @input
  * @return
***************************************************************************************/
void PIT_IRQHandler(void)
{
    if( PIT_GetStatusFlags(PIT, kPIT_Chnl_2) == true) {
        /* 清除中断标志位.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_2, kPIT_TimerFlag);
		
		//在采集数据时,每间隔1S获取一次温度数据
		if (g_sys_para.tempCount < sizeof(Temperature) && g_sys_para.WorkStatus){
			Temperature[g_sys_para.tempCount++] = MXL_ReadObjTemp();
		}
		
		SNVS_LP_SRTC_GetDatetime(SNVS_LP_PERIPHERAL, &SNVS_LP_dateTimeStruct);
		
        if(g_sys_para.inactiveCount++ >= (g_sys_para.inactiveTime + 1)*60-5) { //定时时间到
            GPIO_PinWrite(BOARD_SYS_PWR_OFF_GPIO, BOARD_SYS_PWR_OFF_PIN, 1);
            //SNVS->LPSR |= SNVS_LPCR_DP_EN(1);
            //SNVS->LPSR |= SNVS_LPCR_TOP(1);
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
	if(g_sys_para.spdCount < ADC_LEN){
		SpeedADC[g_sys_para.spdCount++] = QUADTIMER1_PERIPHERAL->CHANNEL[QUADTIMER1_CHANNEL_0_CHANNEL].CAPT;//读取寄存器值
	}
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
//    if(g_sys_para.shkCount < ADC_LEN) {
////        ADC_ETC_GetADCConversionValue(ADC_ETC, 0U, 0U); /* Get trigger0 chain0 result. */
//		ShakeADC[g_sys_para.shkCount++] = ADC_ShakeValue;
//	}
}

/***************************************************************************************
  * @brief   用于检测ADC_RDY引脚下降沿中断引脚
  * @input
  * @return
***************************************************************************************/
void GPIO2_Combined_0_15_IRQHandler(void)
{
	/* 清除中断标志位 */
	GPIO_PortClearInterruptFlags(BOARD_ADC_RDY_PORT,1U << BOARD_ADC_RDY_PIN);
}

/***************************************************************************************
  * @brief   start adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStart(void)
{
	g_sys_para.tempCount = 0;
    g_sys_para.spdCount = 0;
    g_sys_para.shkCount = 0;
	memset(ShakeADC,0,ADC_LEN);
	memset(SpeedADC,0,ADC_LEN);
	memset(VibrateStrADC,0,sizeof(VibrateStrADC));
	memset(SpeedStrADC,0,sizeof(SpeedStrADC));

		//判断自动关机条件
    if(g_sys_para.inactiveCondition != 1) {
        g_sys_para.inactiveCount = 0;
    }
	
	//判断点数是否超出数组界限
	if(g_sys_para.sampNumber > ADC_LEN){
		g_sys_para.sampNumber = ADC_LEN;
	}
	
	//挂起电池与LED灯的任务,并停止PendSV与SysTick中断
    vTaskSuspend(BAT_TaskHandle);
//    vTaskSuspend(LED_TaskHandle);
	NVIC_DisableIRQ(PendSV_IRQn);   
    NVIC_DisableIRQ(SysTick_IRQn);
	
	//根据采样率重新设置IPG_CLK的分频系数
	if(g_adc_set.SampleRate >= 256 && g_adc_set.SampleRate < 1000){
		CLOCK_SetDiv(kCLOCK_IpgDiv, 0x7);//设置分频系数
	}else if(g_adc_set.SampleRate >= 1000 && g_adc_set.SampleRate < 5000){
		CLOCK_SetDiv(kCLOCK_IpgDiv, 0x6);//设置分频系数
	}else if(g_adc_set.SampleRate >= 5000 && g_adc_set.SampleRate < 10000){
		CLOCK_SetDiv(kCLOCK_IpgDiv, 0x5);//设置分频系数
	}else{
		CLOCK_SetDiv(kCLOCK_IpgDiv, 0x3);//设置分频系数
	}
	
	//配置采样时钟,重新配置SPI波特率
	uint32_t *baud = (uint32_t *)&LPSPI4_config.baudRate;
	if(g_adc_set.SampleRate > 45000){
		ADC_MODE_HIGH_SPEED;//使用高速模式
		//使用PWM作为ADS1271的时钟, 其范围为37ns - 10000ns (10us)
		ADC_PwmClkConfig(g_adc_set.SampleRate * 256);
		*baud = 100000000;
	}else{
		ADC_MODE_LOW_POWER;//使用低速模式
		//使用PWM作为ADS1271的时钟, 其范围为37ns - 10000ns (10us)
		ADC_PwmClkConfig(g_adc_set.SampleRate * 512);
		*baud = 25000000;
	}
	LPSPI_Enable(LPSPI4, false);
//	LPSPI_Deinit(LPSPI4_PERIPHERAL);
	LPSPI_MasterInit(LPSPI4_PERIPHERAL, &LPSPI4_config, LPSPI4_CLOCK_FREQ);
	
    /* 输出PWM 用于LTC1063FA的时钟输入,控制采样带宽*/
	g_sys_para.Ltc1063Clk = 1000 * g_adc_set.SampleRate / 25;
    QTMR_SetupPwm(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, g_sys_para.Ltc1063Clk, 50U, false, CLOCK_GetFreq(kCLOCK_IpgClk));
    QTMR_StartTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
	
	//丢弃前500个数据
	ADC_InvalidCnt = 0;
	while (1) { //wait ads1271 ready
        while(ADC_READY == 1){};//等待ADC_READY为低电平
		ADC_ShakeValue = LPSPI4_ReadData();
		ADC_InvalidCnt++;
		if(ADC_InvalidCnt > 500) break;
    }
	
	//设置为true后,会在PIT中断中采集温度数据
	g_sys_para.WorkStatus = true;
	PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_2);
	PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_2);
	
	//开始采集数据前获取一次温度
	Temperature[g_sys_para.tempCount++] = MXL_ReadObjTemp();
	
	/* 输入捕获，计算转速信号周期 */
	QuadTimer1_init();
    QTMR_StartTimer(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
	
	/* Set channel 0 period (66000000 ticks). 用于触发内部ADC采样，采集转速信号*/
//    PIT_SetTimerPeriod(PIT1_PERIPHERAL, kPIT_Chnl_0, PIT1_CLK_FREQ / g_adc_set.SampleRate);
//    /* Start channel 0. 开启通道0,正式开始采样*/
//    PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
	
	while(1) { //wait ads1271 ready
        while(ADC_READY == 1){};//等待ADC_READY为低电平
		__disable_irq();//关闭中断
//		ADC_ShakeValue = LPSPI4_ReadData();
		LPSPI_MasterTransferBlocking(LPSPI4, &spi_tranxfer);	   //SPI阻塞发送
		ShakeADC[g_sys_para.shkCount++] = spi_tranxfer.rxData[2]<<16 | spi_tranxfer.rxData[1]<<8 | spi_tranxfer.rxData[0];
		__enable_irq();//开启中断
		if(g_sys_para.shkCount >= g_sys_para.sampNumber){
			g_sys_para.shkCount = g_sys_para.sampNumber;
			if(g_sys_para.sampNumber == 0){//Android发送中断采集命令后,该值为0
				g_sys_para.spdCount = 0;
			}
			break;
		}
    }

	ADC_SampleStop();
}


/***************************************************************************************
  * @brief   stop adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStop(void)
{
	/* Stop channel 0. */
//    PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
	/* Stop get temperature*/
	g_sys_para.WorkStatus = false;
	ADC_PwmClkStop();
    /* Stop the timer */
    QTMR_StopTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL);
    QTMR_StopTimer(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL);
	
    vTaskResume(BAT_TaskHandle);
//    vTaskResume(LED_TaskHandle);
	
	//结束采集后获取一次温度
	Temperature[g_sys_para.tempCount++] = MXL_ReadObjTemp();
	
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

	ADC_MODE_LOW_POWER;
	ADC_PwmClkConfig(1000000);

	
    /* 等待ADS1271 ready,并读取电压值,如果没有成功获取电压值, 则闪灯提示 */
    while (ADC_READY == 1){};  //wait ads1271 ready
    if(LPSPI4_ReadData() == 0) {
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
            if(r_event & NOTIFY_FINISH) {
                /* ---------------将震动信号转换-----------------------*/
				PRINTF("共采样到 %d 个震动信号\r\n", g_sys_para.shkCount);
                memset(VibrateStrADC, 0, sizeof(VibrateStrADC));
                int pos = 0;
				int tempValue = 0;
                for(uint32_t i = 0; i < g_sys_para.shkCount; i++) {
					if(ShakeADC[i] < 0x800000){
						ShakeADC[i] = ShakeADC[i] * g_sys_para.bias * 1.0f / 0x800000;
					}else{
						ShakeADC[i] = (0x1000000-ShakeADC[i]) * g_sys_para.bias * (-1.0f) / 0x800000;
					}
//					if(i < 2000)PRINTF("%01.5f,",ShakeADC[i]);
					tempValue = (ShakeADC[i]+2.5f) * 10000;//将浮点数转换为整数,并扩大10000倍
                    memset(str, 0, sizeof(str));
                    sprintf(str, "%04x", tempValue);
                    memcpy(VibrateStrADC + pos, str, 4);
                    pos += 4;
                }
				
				/* ------------------将转速信号转换--------------------*/
				PRINTF("\r\n共采样到 %d 个转速信号\r\n", g_sys_para.spdCount);
                memset(SpeedStrADC, 0, sizeof(SpeedStrADC));
                pos = 0;
				SpeedADC[0] = SpeedADC[1];//采集的第一个数据可能不是一个完整的周期,所以第一个数据丢弃.
                for(uint32_t i = 0; i < g_sys_para.spdCount; i++) {
                    SpeedADC[i] = SpeedADC[i] * 1000 / QUADTIMER1_CHANNEL_0_CLOCK_SOURCE;//单位ms
//					PRINTF("%f,",SpeedADC[i]);
					tempValue = SpeedADC[i] * 10;//扩大10倍
                    memset(str, 0, sizeof(str));
                    sprintf(str, "%04x", tempValue);
                    memcpy(SpeedStrADC + pos, str, 4);
                    pos += 4;
                }
#ifdef BLE_VERSION
				//计算发送震动信号需要多少个包,蓝牙数据一次发送160个Byte的数据, 而一个采样点需要4Byte表示, 则一次传送40个采样点
				g_sys_para.shkPacks = (g_sys_para.shkCount / 40) +  (g_sys_para.shkCount%40?1:0);
				//计算发送转速信号需要多少个包
				g_sys_para.spdPacks = (g_sys_para.spdCount / 40) +  (g_sys_para.spdCount%40?1:0);
				//计算将一次采集数据全部发送到Android需要多少个包
				g_sys_para.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 3;
#elif defined WIFI_VERSION
				g_sys_para.shkPacks = (g_sys_para.shkCount / 250) +  (g_sys_para.shkCount%250?1:0);
				g_sys_para.spdPacks = (g_sys_para.spdCount/250) +  (g_sys_para.spdCount%250?1:0);
				g_sys_para.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 1;
#endif
                /* ------------------统计平均温度,最小温度,最大温度--------------------*/
			    float sum = 0;
				int min_i = 0;
				int max_i = 0;
				for(int i=0;i<g_sys_para.tempCount;i++){
					sum += Temperature[g_sys_para.tempCount];
					min_i = Temperature[i] < Temperature[min_i] ? i : min_i;
					max_i = Temperature[i] > Temperature[max_i] ? i : max_i;
				}
				g_adc_set.Process = sum / g_sys_para.tempCount;
				g_adc_set.ProcessMax = Temperature[max_i];
				g_adc_set.ProcessMin = Temperature[min_i];
				
				SaveSampleData();
                /* 发送任务通知，并解锁阻塞在该任务通知下的任务 */
                xTaskNotifyGive( BLE_TaskHandle);
            }
        }
    }
}



