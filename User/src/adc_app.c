#include "main.h"

TaskHandle_t ADC_TaskHandle = NULL;  /* ADC������ */
SemaphoreHandle_t ADCRdySem = NULL;//ADC ready�ź�����

volatile uint32_t ADC_ConvertedValue;
uint32_t counterClock = 0;
uint32_t timeCapt = 0;

/***************************************************************************************
  * @brief   ADC_READY �����жϣ��½����ж�
  * @input
  * @return
***************************************************************************************/
void GPIO2_Combined_0_15_IRQHandler(void)
{
    /* ����жϱ�־λ */
    GPIO_PortClearInterruptFlags(BOARD_ADC_RDY_GPIO, 1U << BOARD_ADC_RDY_PIN);
//    xSemaphoreGive( ADCRdySem );
    __DSB();
}

/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void TMR1_IRQHandler(void)
{
    /* ����жϱ�־ */
    QTMR_ClearStatusFlags(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_EdgeFlag);
    timeCapt = QUADTIMER1_PERIPHERAL->CHANNEL[QUADTIMER1_CHANNEL_0_CHANNEL].CAPT;//��ȡ�Ĵ���ֵ
//    xSemaphoreGive(ADCRdySem);//�����ź���
    __DSB();
}


/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void ADC_ETC_IRQ0_IRQHandler(void)
{
  /*���ת������жϱ�־λ*/
  ADC_ETC_ClearInterruptStatusFlags(ADC_ETC, (adc_etc_external_trigger_source_t)0U, kADC_ETC_Done0StatusFlagMask);
  /*��ȡת�����*/
  ADC_ConvertedValue = ADC_ETC_GetADCConversionValue(ADC_ETC, 0U, 0U); /* Get trigger0 chain0 result. */
  //�����ź���
  xSemaphoreGive(ADCRdySem);
}


/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************//***********************************************************************
  * @ ������  �� ADC_AppTask
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void ADC_AppTask(void)
{
    ADCRdySem = xSemaphoreCreateBinary();      //���� ��ֵ �ź���
    ADC_ETC_Config();
    XBARA_Configuration();
    counterClock = QUADTIMER1_CHANNEL_0_CLOCK_SOURCE / 1000;
    PRINTF("ADC Task Create and Running\r\n");
    while(1)
    {
        xSemaphoreTake(ADCRdySem, portMAX_DELAY);//��ȡ�ź���
//        PRINTF("\r\nCaptured Period time=%d us\n", (timeCapt * 1000) / counterClock);
        PRINTF("\r\nADC Value = %d \n", ADC_ConvertedValue);
    }
}



