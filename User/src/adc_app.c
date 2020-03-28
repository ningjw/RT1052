#include "main.h"


float SpeedADC[ADC_LEN];
float ShakeADC[ADC_LEN];
char  SpeedStrADC[ADC_LEN * 4 + 1];
char  VibrateStrADC[ADC_LEN * 4 + 1];

#define ADC_MODE_LOW_POWER       GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1)  //�͹���ģʽ
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 0)   //����ģʽ
#define ADC_MODE_HIGH_RESOLUTION //�߾���ģʽ(����)
#define ADC_SYNC_HIGH            GPIO_PinWrite(BOARD_ADC_SYNC_GPIO, BOARD_ADC_SYNC_PIN, 1)
#define ADC_SYNC_LOW             GPIO_PinWrite(BOARD_ADC_SYNC_GPIO, BOARD_ADC_SYNC_PIN, 0)
TaskHandle_t ADC_TaskHandle = NULL;  /* ADC������ */

uint32_t timeCapt = 0;
char str[12];
extern volatile uint32_t g_eventTimeMilliseconds;
uint32_t ADC_ShakeValue = 0;
uint8_t  ADC_InvalidCnt = 0;
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
		g_eventTimeMilliseconds++;
        if(g_sys_para.inactiveCount++ >= (g_sys_para.inactiveTime + 1)*60-5) { //��ʱʱ�䵽
            GPIO_PinWrite(BOARD_SYS_PWR_OFF_GPIO, BOARD_SYS_PWR_OFF_PIN, 1);
            //        SNVS->LPSR |= SNVS_LPCR_DP_EN(1);
            //        SNVS->LPSR |= SNVS_LPCR_TOP(1);
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
    if(g_sys_para.shkCount < ADC_LEN) {
//        ADC_ETC_GetADCConversionValue(ADC_ETC, 0U, 0U); /* Get trigger0 chain0 result. */
		ShakeADC[g_sys_para.shkCount++] = ADC_ShakeValue;
	}
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
    g_sys_para.spdCount = 0;
    g_sys_para.shkCount = 0;
	memset(ShakeADC,0,ADC_LEN);
	memset(SpeedADC,0,ADC_LEN);
    g_sys_para.Ltc1063Clk = 1000 * g_adc_set.SampleRate / 25;

	//�ж��Զ��ػ�����
    if(g_sys_para.inactiveCondition != 1) {
        g_sys_para.inactiveCount = 0;
    }
	//�жϵ����Ƿ񳬳��������
	if(g_sys_para.sampNumber > ADC_LEN){
		g_sys_para.sampNumber = ADC_LEN;
	}
    vTaskSuspend(BAT_TaskHandle);
    vTaskSuspend(LED_TaskHandle);
    /* Setup the PWM mode of the timer channel ����LTC1063FA��ʱ������,���Ʋ�������*/
    QTMR_SetupPwm(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, g_sys_para.Ltc1063Clk, 50U, false, QUADTIMER3_CHANNEL_0_CLOCK_SOURCE);
    QTMR_StartTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
	/* Set channel 0 period (66000000 ticks). ���ڴ����ڲ�ADC�������ɼ�ת���ź�*/
    PIT_SetTimerPeriod(PIT1_PERIPHERAL, kPIT_Chnl_0, PIT1_CLK_FREQ / g_adc_set.SampleRate);
    /* Set channel 1 period (66000000 ticks). ���ڿ��Ʋ���Ƶ��*/
//    PIT_SetTimerPeriod(PIT1_PERIPHERAL, kPIT_Chnl_1, PIT1_CLK_FREQ / g_adc_set.SampleRate);
    
	NVIC_DisableIRQ(PendSV_IRQn);   
    NVIC_DisableIRQ(SysTick_IRQn);
	/* stop channel 2. */
	PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_2);
	ADC_InvalidCnt = 0;
	while (1) { //wait ads1271 ready
        while(ADC_READY == 0);
		ADC_ShakeValue = LPSPI4_ReadData();
		ADC_InvalidCnt++;
		if(ADC_InvalidCnt > 20) break;//���ݵ���ʱ�����ݹ۲�,����ǰ��20������
    }
	/* ���벶�񣬼���ת���ź����� */
    QTMR_StartTimer(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
    /* Start channel 0. ����ͨ��0,��ʽ��ʼ����*/
    PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
	while(1) { //wait ads1271 ready
        while(ADC_READY == 0);
		ADC_ShakeValue = LPSPI4_ReadData();
//		ShakeADC[g_sys_para.shkCount++] = LPSPI4_ReadData();
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
    PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
	
	/* Start channel 2. */
	PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_2);
	
    /* Stop the timer */
    QTMR_StopTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL);
    QTMR_StopTimer(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL);

    vTaskResume(BAT_TaskHandle);
    vTaskResume(LED_TaskHandle);
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

    /* ʹ��LPSPI4���ڶ�ȡADS1271��ֵ*/
    LPSPI_Enable(LPSPI4, true);

    ADC_MODE_HIGH_SPEED;

    /* �ȴ�ADS1271 ready,����ȡ��ѹֵ,���û�гɹ���ȡ��ѹֵ, ��������ʾ */
    while (ADC_READY == 0);  //wait ads1271 ready
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
                    ShakeADC[i] = ShakeADC[i] * g_sys_para.bias * 1.0f / 0x800000;
					PRINTF("%01.5f,",ShakeADC[i]);
					tempValue = ShakeADC[i] * 10000;//��������ת��Ϊ����,������10000��
                    memset(str, 0, sizeof(str));
                    sprintf(str, "%04x", tempValue);
                    memcpy(VibrateStrADC + pos, str, 4);
                    pos += 4;
                }
				//���㷢�����ź���Ҫ���ٸ���
				g_sys_para.shkPacks = (g_sys_para.shkCount / 38) +  (g_sys_para.shkCount%38?1:0);

				/* ------------------��ת���ź�ת��--------------------*/
				PRINTF("\r\n�������� %d ��ת���ź�\r\n", g_sys_para.spdCount);
                memset(SpeedStrADC, 0, sizeof(SpeedStrADC));
                pos = 0;
				SpeedADC[0] = SpeedADC[1];//�ɼ��ĵ�һ�����ݿ��ܲ���һ������������,���Ե�һ�����ݶ���.
                for(uint32_t i = 0; i < g_sys_para.spdCount; i++) {
                    SpeedADC[i] = SpeedADC[i] * 1000 / QUADTIMER1_CHANNEL_0_CLOCK_SOURCE;//��λms
					PRINTF("%f,",SpeedADC[i]);
					tempValue = SpeedADC[i] * 10;//����10��
                    memset(str, 0, sizeof(str));
                    sprintf(str, "%04x", tempValue);
                    memcpy(SpeedStrADC + pos, str, 4);
                    pos += 4;
                }
				//���㷢�����ź���Ҫ���ٸ���
				g_sys_para.spdPacks = (g_sys_para.spdCount / 38) +  (g_sys_para.spdCount%38?1:0);
				
				//���㽫һ�βɼ�����ȫ�����͵�Android��Ҫ���ٸ���
				g_sys_para.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 3;

                /* -----------���������ݴ����json��ʽ,�����浽�ļ���-----*/
                PacketSampleData();

                /* ��������֪ͨ�������������ڸ�����֪ͨ�µ����� */
                xTaskNotifyGive( BLE_TaskHandle);
            }
        }
    }
}



