#include "main.h"



TaskHandle_t ADC_TaskHandle = NULL;  /* ADC任务句柄 */
SemaphoreHandle_t ADCRdySem = NULL;//ADC ready信号量，


void ADC_AppTask(void);

/***********************************************************************
  * @ 函数名  ： ADC_AppTask
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void ADC_AppTask(void)
{
    ADCRdySem = xSemaphoreCreateBinary();      //创建 二值 信号量 
    PRINTF("ADC Task Create and Running\r\n");
    while(1)
    {
        xSemaphoreTake(ADCRdySem, portMAX_DELAY);//获取信号量
        PRINTF("ADC Ready\r\n");
    }
}

/***************************************************************************************
  * @brief   ADC_READY 引脚中断，下降沿中断
  * @input   
  * @return  
***************************************************************************************/
void GPIO2_Combined_0_15_IRQHandler(void)
{
    /* 清除中断标志位 */
    GPIO_PortClearInterruptFlags(BOARD_ADC_RDY_GPIO, 1U << BOARD_ADC_RDY_PIN);
    xSemaphoreGive( ADCRdySem );
    __DSB();
}

