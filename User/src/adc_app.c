#include "main.h"

#define ADC_LEN      40000
#define ADC_STR_LEN  200000

AT_NONCACHEABLE_SECTION_INIT(float SpeedADC[ADC_LEN]);
AT_NONCACHEABLE_SECTION_INIT(float ShakeADC[ADC_LEN]);
AT_NONCACHEABLE_SECTION_INIT(char  StrSpeedADC[ADC_STR_LEN]);
AT_NONCACHEABLE_SECTION_INIT(char  StrShakeADC[ADC_STR_LEN]);

#define ADC_MODE_LOW_POWER       GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1)  //�͹���ģʽ
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 0)   //����ģʽ
#define ADC_MODE_HIGH_RESOLUTION //�߾���ģʽ(����)


TaskHandle_t ADC_TaskHandle = NULL;  /* ADC������ */


static uint32_t counterClock = 0;
uint32_t timeCapt = 0;
char str[12];

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
    else if( PIT_GetStatusFlags(PIT, kPIT_Chnl_2) == true){
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
    
    ADC_MODE_HIGH_SPEED;
    
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
                /* ����ת���ź����� */
                g_sys_para.periodSpdSignal = (timeCapt * 1000) / counterClock;
                
                PRINTF("���õĲ���Ƶ��Ϊ:%d Hz\r\n", g_sys_para.sampFreq);
                PRINTF("���õĲ���ʱ��:%d s\r\n", g_sys_para.sampTimeSet);
                PRINTF("�����ת���ź�����:%d us\r\n",g_sys_para.periodSpdSignal);
                PRINTF("�������� %d �����ź�\r\n", g_sys_para.ADC_ShakeCnt);
                PRINTF("�������� %d ��ת���ź�\r\n", g_sys_para.ADC_SpdCnt);
                
                /* �����ź�ת��*/
                memset(StrShakeADC, 0, sizeof(StrShakeADC));
                for(uint32_t i = 0; i< g_sys_para.ADC_ShakeCnt; i++){
                    ShakeADC[i] = ShakeADC[i] * g_sys_para.bias  / 0x400000;
                    memset(str, 0, sizeof(str));
                    sprintf(str,"%1.6f,",ShakeADC[i]);
                    strcat(StrShakeADC,str);
                }
                
                /* ��ת���ź�ת��*/
                memset(StrSpeedADC, 0, sizeof(StrSpeedADC));
                for(uint32_t i = 0; i< g_sys_para.ADC_SpdCnt; i++){
                    SpeedADC[i] = SpeedADC[i] * g_sys_para.refV / 4096;
                    memset(str, 0, sizeof(str));
                    sprintf(str,"%1.6f,",SpeedADC[i]);
                    strcat(StrSpeedADC,str);
                }
                
                /* ���������ݴ����json��ʽ,�����浽�ļ���*/
                ParseSampleData();
                
                /* ��������֪ͨ�������������ڸ�����֪ͨ�µ����� */
                vTaskNotifyGiveFromISR( BLE_TaskHandle, (void *)pdFALSE);
            }
        }
    }
}



