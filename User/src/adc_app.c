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

#define ADC_MODE_LOW_POWER       GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1)  //�͹���ģʽ
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 0)   //����ģʽ
#define ADC_MODE_HIGH_RESOLUTION //�߾���ģʽ(����)


TaskHandle_t ADC_TaskHandle = NULL;  /* ADC������ */

static volatile uint32_t ADC_ConvertedValue;
static uint32_t counterClock = 0;
static uint32_t timeCapt = 0;


/***************************************************************************************
  * @brief   kPIT_Chnl_0���ڴ���ADC���� ��kPIT_Chnl_1 ���ڶ�ʱ����; kPIT_Chnl_2���ڶ�ʱ�ػ�1�����ж�
  * @input
  * @return
***************************************************************************************/
void PIT_IRQHandler(void)
{
    if( PIT_GetStatusFlags(PIT, kPIT_Chnl_1) == true ) {
        /* ����жϱ�־λ.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag);
        ADC_SampleStop();
    }
    if( PIT_GetStatusFlags(PIT, kPIT_Chnl_2) == true){
        /* ����жϱ�־λ.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_2, kPIT_TimerFlag);
         if(g_sys_para.inactiveCount++ >= g_sys_para.inactiveTime + 1){//��ʱʱ�䵽
            GPIO_PinWrite(BOARD_SYS_PWR_OFF_GPIO,BOARD_SYS_PWR_OFF_PIN,1);
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
    timeCapt = QUADTIMER1_PERIPHERAL->CHANNEL[QUADTIMER1_CHANNEL_0_CHANNEL].CAPT;//��ȡ�Ĵ���ֵ
    
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
    
    /* ��ʼ��counterClock,���ڼ��㲶������ */
    counterClock = QUADTIMER1_CHANNEL_0_CLOCK_SOURCE / 1000;
    
    /* �ȴ�ADS1271 ready,����ȡ��ѹֵ,���û�гɹ���ȡ��ѹֵ, ��������ʾ */
    while (ADC_READY == 0);  //wait ads1271 ready
    if(LPSPI4_ReadData() == 0){
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
            if(r_event & NOTIFY_FINISH){
                
                /* ��ת���ź�ת��*/
                for(uint32_t i = 0; i< ADC_SpdCnt; i++){
                    SpeedADC[i] = SpeedADC[i] * 3.3f / 4096.0f;
                }
                /* �����ź�ת��*/
                for(uint32_t i = 0; i< ADC_ShakeCnt; i++){
                    ShakeADC[i] = ShakeADC[i] * 2.43f * 2 / 8388607;
                }
                
                /* �������ź����� */
                g_sys_para.periodSpdSignal = (timeCapt * 1000) / counterClock;
                
                //�����β������ݱ��浽�ļ�
                eMMC_SaveSampleData();
            }
        }
    }
}



