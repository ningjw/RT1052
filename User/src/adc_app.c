#include "main.h"

//对应核心板的A-26脚
#define ADC_MODE_LOW_POWER       GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1)  //低功耗模式
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 0)   //高速模式
#define ADC_MODE_HIGH_RESOLUTION   //高精度模式(浮空)
#define ADC_READY                GPIO_PinRead(BOARD_ADC_RDY_GPIO, BOARD_ADC_RDY_PIN)

TaskHandle_t ADC_TaskHandle = NULL;  /* ADC任务句柄 */
SemaphoreHandle_t ADCRdySem = NULL;//ADC ready信号量，

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
    /* 清除中断标志 */
    QTMR_ClearStatusFlags(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_EdgeFlag);
    timeCapt = QUADTIMER1_PERIPHERAL->CHANNEL[QUADTIMER1_CHANNEL_0_CHANNEL].CAPT;//读取寄存器值
    speedPeriod = (timeCapt * 1000) / counterClock;
//    xSemaphoreGive(ADCRdySem);//发送信号量
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
    /*清除转换完成中断标志位*/
    ADC_ETC_ClearInterruptStatusFlags(ADC_ETC, (adc_etc_external_trigger_source_t)0U, kADC_ETC_Done0StatusFlagMask);
    /*读取转换结果*/
    ADC_ConvertedValue = ADC_ETC_GetADCConversionValue(ADC_ETC, 0U, 0U); /* Get trigger0 chain0 result. */
    ADC_Voltage = ADC_ConvertedValue * 3.3 / 4096.0;
    //发送信号量
//    xSemaphoreGive(ADCRdySem);
}
uint8_t rxData[3] = {0};
uint32_t ADC1271_Value = 0;
/***********************************************************************
  * @ 函数名  ： ADC_AppTask
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void ADC_AppTask(void)
{
    ADCRdySem = xSemaphoreCreateBinary();      //创建 二值 信号量
    ADC_ETC_Config();
    XBARA_Configuration();
//    LPSPI_Enable(LPSPI4, true);                         //使能LPSPI3
    counterClock = QUADTIMER1_CHANNEL_0_CLOCK_SOURCE / 1000;
    PRINTF("ADC Task Create and Running\r\n");
    while(1)
    {
//        xSemaphoreTake(ADCRdySem, portMAX_DELAY);//获取信号量
//        PRINTF("\r\nCaptured Period time=%d us\n", (timeCapt * 1000) / counterClock);
//        PRINTF("\r\nADC Value = %d \n", ADC_ConvertedValue);
        if (ADC_READY == 0) { //低电平有效
            LPSPI4_ReadWriteByte();
        }
        vTaskDelay(1);
    }
}



