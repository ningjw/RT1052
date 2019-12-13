#include "main.h"



TaskHandle_t ADC_TaskHandle = NULL;  /* ADC������ */
SemaphoreHandle_t ADCRdySem = NULL;//ADC ready�ź�����


void ADC_AppTask(void);

/***********************************************************************
  * @ ������  �� ADC_AppTask
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void ADC_AppTask(void)
{
    ADCRdySem = xSemaphoreCreateBinary();      //���� ��ֵ �ź��� 
    PRINTF("ADC Task Create and Running\r\n");
    while(1)
    {
        xSemaphoreTake(ADCRdySem, portMAX_DELAY);//��ȡ�ź���
        PRINTF("ADC Ready\r\n");
    }
}

/***************************************************************************************
  * @brief   ADC_READY �����жϣ��½����ж�
  * @input   
  * @return  
***************************************************************************************/
void GPIO2_Combined_0_15_IRQHandler(void)
{
    /* ����жϱ�־λ */
    GPIO_PortClearInterruptFlags(BOARD_ADC_RDY_GPIO, 1U << BOARD_ADC_RDY_PIN);
    xSemaphoreGive( ADCRdySem );
    __DSB();
}

