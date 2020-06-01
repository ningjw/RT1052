#include "main.h"

float SpeedADC[ADC_LEN];
float ShakeADC[ADC_LEN];
float Temperature[64];
char  SpeedStrADC[ADC_LEN * 4 + 1];
char  VibrateStrADC[ADC_LEN * 4 + 1];

#define ADC_MODE_LOW_POWER       GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1)  //�͹���ģʽ
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 0)   //����ģʽ
#define ADC_MODE_HIGH_RESOLUTION //�߾���ģʽ(����)
#define ADC_SYNC_HIGH            GPIO_PinWrite(BOARD_ADC_SYNC_GPIO, BOARD_ADC_SYNC_PIN, 1)
#define ADC_SYNC_LOW             GPIO_PinWrite(BOARD_ADC_SYNC_GPIO, BOARD_ADC_SYNC_PIN, 0)

#define PWR_EN_LOW    GPIO_PinWrite(BOARD_PWR_EN_GPIO, BOARD_PWR_EN_PIN, 0)
#define PWR_EN_HIGH   GPIO_PinWrite(BOARD_PWR_EN_GPIO, BOARD_PWR_EN_PIN, 1)


TaskHandle_t ADC_TaskHandle = NULL;  /* ADC������ */

uint32_t timeCapt = 0;
char str[12];

uint32_t ADC_ShakeValue = 0;
uint32_t  ADC_InvalidCnt = 0;

extern lpspi_transfer_t spi_tranxfer;
extern void QuadTimer1_init(void);
/***************************************************************************************
  * @brief   kPIT_Chnl_0���ڴ���ADC���� ��kPIT_Chnl_1 ���ڶ�ʱ����; kPIT_Chnl_2���ڶ�ʱ�ػ�1�����ж�
  * @input
  * @return
***************************************************************************************/
void PIT_IRQHandler(void)
{
    if( PIT_GetStatusFlags(PIT, kPIT_Chnl_2) == true) {
        /* ����жϱ�־λ.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_2, kPIT_TimerFlag);
		
		//�ڲɼ�����ʱ,ÿ���1S��ȡһ���¶�����
		if (g_sys_para.tempCount < sizeof(Temperature) && g_sys_para.WorkStatus){
			Temperature[g_sys_para.tempCount++] = MXL_ReadObjTemp();
		}
		
		SNVS_LP_SRTC_GetDatetime(SNVS_LP_PERIPHERAL, &SNVS_LP_dateTimeStruct);
		
        if(g_sys_para.inactiveCount++ >= (g_sys_para.inactiveTime + 1)*60-5) { //��ʱʱ�䵽
            GPIO_PinWrite(BOARD_SYS_PWR_OFF_GPIO, BOARD_SYS_PWR_OFF_PIN, 1);
            //SNVS->LPSR |= SNVS_LPCR_DP_EN(1);
            //SNVS->LPSR |= SNVS_LPCR_TOP(1);
        }
    }

    __DSB();
}

/***************************************************************************************
  * @brief  ��ʱ�����ڻ�ȡת���źŵ�����(Ƶ��)
  * @input
  * @return
***************************************************************************************/
void TMR1_IRQHandler(void)
{
    /* ����жϱ�־ */
    QTMR_ClearStatusFlags(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_EdgeFlag);
	if(g_sys_para.spdCount < ADC_LEN){
		SpeedADC[g_sys_para.spdCount++] = QUADTIMER1_PERIPHERAL->CHANNEL[QUADTIMER1_CHANNEL_0_CHANNEL].CAPT;//��ȡ�Ĵ���ֵ
	}
}


/***************************************************************************************
  * @brief   ���ڻ�ȡת���źŵĵ�ѹֵ,�òɼ�ͨ��PIT1��Channel0����
  * @input
  * @return
***************************************************************************************/
void ADC_ETC_IRQ0_IRQHandler(void)
{
    /*���ת������жϱ�־λ*/
    ADC_ETC_ClearInterruptStatusFlags(ADC_ETC, (adc_etc_external_trigger_source_t)0U, kADC_ETC_Done0StatusFlagMask);
    /*��ȡת�����*/
//    if(g_sys_para.shkCount < ADC_LEN) {
////        ADC_ETC_GetADCConversionValue(ADC_ETC, 0U, 0U); /* Get trigger0 chain0 result. */
//		ShakeADC[g_sys_para.shkCount++] = ADC_ShakeValue;
//	}
}

/***************************************************************************************
  * @brief   ���ڼ��ADC_RDY�����½����ж�����
  * @input
  * @return
***************************************************************************************/
void GPIO2_Combined_0_15_IRQHandler(void)
{
	/* ����жϱ�־λ */
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

		//�ж��Զ��ػ�����
    if(g_sys_para.inactiveCondition != 1) {
        g_sys_para.inactiveCount = 0;
    }
	
	//�жϵ����Ƿ񳬳��������
	if(g_sys_para.sampNumber > ADC_LEN){
		g_sys_para.sampNumber = ADC_LEN;
	}
	
	//��������LED�Ƶ�����,��ֹͣPendSV��SysTick�ж�
    vTaskSuspend(BAT_TaskHandle);
//    vTaskSuspend(LED_TaskHandle);
	NVIC_DisableIRQ(PendSV_IRQn);   
    NVIC_DisableIRQ(SysTick_IRQn);
	
	//���ݲ�������������IPG_CLK�ķ�Ƶϵ��
	if(g_adc_set.SampleRate >= 256 && g_adc_set.SampleRate < 1000){
		CLOCK_SetDiv(kCLOCK_IpgDiv, 0x7);//���÷�Ƶϵ��
	}else if(g_adc_set.SampleRate >= 1000 && g_adc_set.SampleRate < 5000){
		CLOCK_SetDiv(kCLOCK_IpgDiv, 0x6);//���÷�Ƶϵ��
	}else if(g_adc_set.SampleRate >= 5000 && g_adc_set.SampleRate < 10000){
		CLOCK_SetDiv(kCLOCK_IpgDiv, 0x5);//���÷�Ƶϵ��
	}else{
		CLOCK_SetDiv(kCLOCK_IpgDiv, 0x3);//���÷�Ƶϵ��
	}
	
	//���ò���ʱ��,��������SPI������
	uint32_t *baud = (uint32_t *)&LPSPI4_config.baudRate;
	if(g_adc_set.SampleRate > 45000){
		ADC_MODE_HIGH_SPEED;//ʹ�ø���ģʽ
		//ʹ��PWM��ΪADS1271��ʱ��, �䷶ΧΪ37ns - 10000ns (10us)
		ADC_PwmClkConfig(g_adc_set.SampleRate * 256);
		*baud = 100000000;
	}else{
		ADC_MODE_LOW_POWER;//ʹ�õ���ģʽ
		//ʹ��PWM��ΪADS1271��ʱ��, �䷶ΧΪ37ns - 10000ns (10us)
		ADC_PwmClkConfig(g_adc_set.SampleRate * 512);
		*baud = 25000000;
	}
	LPSPI_Enable(LPSPI4, false);
//	LPSPI_Deinit(LPSPI4_PERIPHERAL);
	LPSPI_MasterInit(LPSPI4_PERIPHERAL, &LPSPI4_config, LPSPI4_CLOCK_FREQ);
	
    /* ���PWM ����LTC1063FA��ʱ������,���Ʋ�������*/
	g_sys_para.Ltc1063Clk = 1000 * g_adc_set.SampleRate / 25;
    QTMR_SetupPwm(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, g_sys_para.Ltc1063Clk, 50U, false, CLOCK_GetFreq(kCLOCK_IpgClk));
    QTMR_StartTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
	
	//����ǰ500������
	ADC_InvalidCnt = 0;
	while (1) { //wait ads1271 ready
        while(ADC_READY == 1){};//�ȴ�ADC_READYΪ�͵�ƽ
		ADC_ShakeValue = LPSPI4_ReadData();
		ADC_InvalidCnt++;
		if(ADC_InvalidCnt > 500) break;
    }
	
	//����Ϊtrue��,����PIT�ж��вɼ��¶�����
	g_sys_para.WorkStatus = true;
	PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_2);
	PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_2);
	
	//��ʼ�ɼ�����ǰ��ȡһ���¶�
	Temperature[g_sys_para.tempCount++] = MXL_ReadObjTemp();
	
	/* ���벶�񣬼���ת���ź����� */
	QuadTimer1_init();
    QTMR_StartTimer(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
	
	/* Set channel 0 period (66000000 ticks). ���ڴ����ڲ�ADC�������ɼ�ת���ź�*/
//    PIT_SetTimerPeriod(PIT1_PERIPHERAL, kPIT_Chnl_0, PIT1_CLK_FREQ / g_adc_set.SampleRate);
//    /* Start channel 0. ����ͨ��0,��ʽ��ʼ����*/
//    PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
	
	while(1) { //wait ads1271 ready
        while(ADC_READY == 1){};//�ȴ�ADC_READYΪ�͵�ƽ
		__disable_irq();//�ر��ж�
//		ADC_ShakeValue = LPSPI4_ReadData();
		LPSPI_MasterTransferBlocking(LPSPI4, &spi_tranxfer);	   //SPI��������
		ShakeADC[g_sys_para.shkCount++] = spi_tranxfer.rxData[2]<<16 | spi_tranxfer.rxData[1]<<8 | spi_tranxfer.rxData[0];
		__enable_irq();//�����ж�
		if(g_sys_para.shkCount >= g_sys_para.sampNumber){
			g_sys_para.shkCount = g_sys_para.sampNumber;
			if(g_sys_para.sampNumber == 0){//Android�����жϲɼ������,��ֵΪ0
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
	
	//�����ɼ����ȡһ���¶�
	Temperature[g_sys_para.tempCount++] = MXL_ReadObjTemp();
	
    /* ����ADC��������¼�  */
    xTaskNotify(ADC_TaskHandle, NOTIFY_FINISH, eSetBits);
}


/***********************************************************************
  * @ ������  �� ADC�ɼ�����
  * @ ����˵����
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void ADC_AppTask(void)
{
    uint32_t r_event;
    BaseType_t xReturn = pdTRUE;
    /* ����ADC���ⲿ����ģʽ */
    ADC_ETC_Config();
    XBARA_Configuration();

	ADC_MODE_LOW_POWER;
	ADC_PwmClkConfig(1000000);

	
    /* �ȴ�ADS1271 ready,����ȡ��ѹֵ,���û�гɹ���ȡ��ѹֵ, ��������ʾ */
    while (ADC_READY == 1){};  //wait ads1271 ready
    if(LPSPI4_ReadData() == 0) {
        g_sys_para.sampLedStatus = WORK_FATAL_ERR;
    }
    PRINTF("ADC Task Create and Running\r\n");
    while(1)
    {
        /*�ȴ�ADC��ɲ����¼�*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &r_event, portMAX_DELAY);

        /* �ж��Ƿ�ɹ��ȴ����¼� */
        if ( pdTRUE == xReturn ) {
			
            /* ��ɲ����¼�*/
            if(r_event & NOTIFY_FINISH) {
                /* ---------------�����ź�ת��-----------------------*/
				PRINTF("�������� %d �����ź�\r\n", g_sys_para.shkCount);
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
					tempValue = (ShakeADC[i]+2.5f) * 10000;//��������ת��Ϊ����,������10000��
                    memset(str, 0, sizeof(str));
                    sprintf(str, "%04x", tempValue);
                    memcpy(VibrateStrADC + pos, str, 4);
                    pos += 4;
                }
				
				/* ------------------��ת���ź�ת��--------------------*/
				PRINTF("\r\n�������� %d ��ת���ź�\r\n", g_sys_para.spdCount);
                memset(SpeedStrADC, 0, sizeof(SpeedStrADC));
                pos = 0;
				SpeedADC[0] = SpeedADC[1];//�ɼ��ĵ�һ�����ݿ��ܲ���һ������������,���Ե�һ�����ݶ���.
                for(uint32_t i = 0; i < g_sys_para.spdCount; i++) {
                    SpeedADC[i] = SpeedADC[i] * 1000 / QUADTIMER1_CHANNEL_0_CLOCK_SOURCE;//��λms
//					PRINTF("%f,",SpeedADC[i]);
					tempValue = SpeedADC[i] * 10;//����10��
                    memset(str, 0, sizeof(str));
                    sprintf(str, "%04x", tempValue);
                    memcpy(SpeedStrADC + pos, str, 4);
                    pos += 4;
                }
#ifdef BLE_VERSION
				//���㷢�����ź���Ҫ���ٸ���,��������һ�η���160��Byte������, ��һ����������Ҫ4Byte��ʾ, ��һ�δ���40��������
				g_sys_para.shkPacks = (g_sys_para.shkCount / 40) +  (g_sys_para.shkCount%40?1:0);
				//���㷢��ת���ź���Ҫ���ٸ���
				g_sys_para.spdPacks = (g_sys_para.spdCount / 40) +  (g_sys_para.spdCount%40?1:0);
				//���㽫һ�βɼ�����ȫ�����͵�Android��Ҫ���ٸ���
				g_sys_para.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 3;
#elif defined WIFI_VERSION
				g_sys_para.shkPacks = (g_sys_para.shkCount / 250) +  (g_sys_para.shkCount%250?1:0);
				g_sys_para.spdPacks = (g_sys_para.spdCount/250) +  (g_sys_para.spdCount%250?1:0);
				g_sys_para.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 1;
#endif
                /* ------------------ͳ��ƽ���¶�,��С�¶�,����¶�--------------------*/
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
                /* ��������֪ͨ�������������ڸ�����֪ͨ�µ����� */
                xTaskNotifyGive( BLE_TaskHandle);
            }
        }
    }
}



