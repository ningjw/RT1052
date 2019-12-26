#include "main.h"

//��Ӧ���İ��A-26��
#define ADC_MODE_LOW_POWER       GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1)  //�͹���ģʽ
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 0)   //����ģʽ
#define ADC_MODE_HIGH_RESOLUTION   //�߾���ģʽ(����)
#define ADC_READY                GPIO_PinRead(BOARD_ADC_RDY_GPIO, BOARD_ADC_RDY_PIN)

TaskHandle_t ADC_TaskHandle = NULL;  /* ADC������ */
SemaphoreHandle_t ADCRdySem = NULL;//ADC ready�ź�����

volatile uint32_t ADC_ConvertedValue;
uint32_t counterClock = 0;
uint32_t timeCapt = 0;

uint32_t speedPeriod = 0;
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
    speedPeriod = (timeCapt * 1000) / counterClock;
//    xSemaphoreGive(ADCRdySem);//�����ź���
    __DSB();
}

float ADC_Voltage = 0;
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
    ADC_Voltage = ADC_ConvertedValue * 3.3 / 4096.0;
    //�����ź���
//    xSemaphoreGive(ADCRdySem);
}
uint8_t rxData[3] = {0};
uint32_t ADC1271_Value = 0;
/***********************************************************************
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
//    LPSPI_Enable(LPSPI4, true);                         //ʹ��LPSPI3
    counterClock = QUADTIMER1_CHANNEL_0_CLOCK_SOURCE / 1000;
    PRINTF("ADC Task Create and Running\r\n");
    while(1)
    {
//        xSemaphoreTake(ADCRdySem, portMAX_DELAY);//��ȡ�ź���
//        PRINTF("\r\nCaptured Period time=%d us\n", (timeCapt * 1000) / counterClock);
//        PRINTF("\r\nADC Value = %d \n", ADC_ConvertedValue);
        if (ADC_READY == 0) { //�͵�ƽ��Ч
            LPSPI4_ReadWriteByte();
        }
        vTaskDelay(1);
    }
}



