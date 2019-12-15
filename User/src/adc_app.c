#include "main.h"

TaskHandle_t ADC_TaskHandle = NULL;  /* ADC任务句柄 */
SemaphoreHandle_t ADCRdySem = NULL;//ADC ready信号量，

volatile uint32_t ADC_ConvertedValue;
uint32_t counterClock = 0;
uint32_t timeCapt = 0;

/***************************************************************************************
  * @brief   ADC_READY 引脚中断，下降沿中断
  * @input
  * @return
***************************************************************************************/
void GPIO2_Combined_0_15_IRQHandler(void)
{
    /* 清除中断标志位 */
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
    uint32_t timerCount = 0;
    /* 清除中断标志 */
    QTMR_ClearStatusFlags(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_EdgeFlag);
    timeCapt = QUADTIMER1_PERIPHERAL->CHANNEL[QUADTIMER1_CHANNEL_0_CHANNEL].CAPT;//读取寄存器值
    xSemaphoreGive(ADCRdySem);//获取信号量
}


/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void ADC1_IRQHandler(void)
{
    /*读取转换结果，读取之后硬件自动清除转换完成中断标志位*/
    ADC_ConvertedValue = ADC_GetChannelConversionValue(ADC1, 0U);

}

/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************//***********************************************************************
  * @ 函数名  ： ADC_AppTask
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void ADC_AppTask(void)
{
    ADCRdySem = xSemaphoreCreateBinary();      //创建 二值 信号量
    counterClock = QUADTIMER1_CHANNEL_0_CLOCK_SOURCE / 1000;
    PRINTF("ADC Task Create and Running\r\n");
    while(1)
    {
        xSemaphoreTake(ADCRdySem, portMAX_DELAY);//获取信号量
        PRINTF("\r\nCaptured Period time=%d us\n", (timeCapt * 1000) / counterClock);
    }
}



