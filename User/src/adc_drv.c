/* --------------------------------------------------------------------------
 * QTIMER3_TIMER0 输出PWM波，用于控制采样频率，通过底板A-21引脚输出。
 * QTIMER1_TIMER0 配置为输入捕获，通底底板A-37引脚输出。
 * ADC_MODE: 0:High-Speed mode; float:High-Resolution mode; 1:Low-Power mode
 *
 *
 *
 *---------------------------------------------------------------------------*/
#include "main.h"

#define SPI_CLK_LOW   GPIO_PinWrite(BOARD_ADC_SCK_GPIO, BOARD_ADC_SCK_PIN, 0)
#define SPI_CLK_HIGH  GPIO_PinWrite(BOARD_ADC_SCK_GPIO, BOARD_ADC_SCK_PIN, 1)
#define SPI_READ_DATA GPIO_PinRead(BOARD_ADC_SDI_GPIO, BOARD_ADC_SDI_PIN)
/***************************************************************************************
  * @brief   Configure XBARA to work with ADC_ETC
  * @input
  * @return
***************************************************************************************/
void XBARA_Configuration(void)
{
    /* 初始化XBARA1模块*/
    XBARA_Init(XBARA1);

    /*建立输入与输出信号之间的联系*/
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputPitTrigger0, kXBARA1_OutputAdcEtcXbar0Trig0);
}


/***************************************************************************************
  * @brief   配置为允许外部触发
  * @input
  * @return
***************************************************************************************/
void ADC_ETC_Config(void)
{
    adc_etc_config_t adcEtcConfig;//配置外部触发全局控制器。
    adc_etc_trigger_config_t adcEtcTriggerConfig; //配置外部触发转换通道组。主要包括优先级、触发方式、触发通道数量
    adc_etc_trigger_chain_config_t adcEtcTriggerChainConfig; // 配置外部触发转换通道组具体的ADC转换通道


    /*配置外部触发控制器*/
    ADC_ETC_GetDefaultConfig(&adcEtcConfig);

    /*如果要使用ADC2则必须要设置该属性*/
    adcEtcConfig.enableTSCBypass = false;//设置该位允许使用ADC2

    adcEtcConfig.XBARtriggerMask = 1; /* 并且选择了触发通道为触发通道0*/
    ADC_ETC_Init(ADC_ETC, &adcEtcConfig);

    /*  配置外部触发转换通道组。*/
    adcEtcTriggerConfig.enableSyncMode = false;                      //异步模式
    adcEtcTriggerConfig.enableSWTriggerMode = false;                 //禁止软件触发
    adcEtcTriggerConfig.triggerChainLength = 0; //设置转换通道数量，实际转换通道数量为DEMO_ADC_ETC_CHAIN_LENGTH+1
    adcEtcTriggerConfig.triggerPriority = 0U;                         //外部触发优先级
    adcEtcTriggerConfig.sampleIntervalDelay = 0U;
    adcEtcTriggerConfig.initialDelay = 0U;
    ADC_ETC_SetTriggerConfig(ADC_ETC, 0U, &adcEtcTriggerConfig);//设置外部XBAR触发器配置

    /*配置外部触发转换通道组具体的ADC转换通道*/
    /*****************************************************************************************************************************/
    adcEtcTriggerChainConfig.enableB2BMode = true;                               //使用B2B模式
    adcEtcTriggerChainConfig.ADCHCRegisterSelect = 1; //选择要参与转换的ADC转换通道
    adcEtcTriggerChainConfig.ADCChannelSelect = 0;                               // 选择外部输入通道
    adcEtcTriggerChainConfig.InterruptEnable = kADC_ETC_Done0InterruptEnable;    // 使能该通道的转换完成中断0
    ADC_ETC_SetTriggerChainConfig(ADC_ETC, 0U, 0U, &adcEtcTriggerChainConfig); //
    /*****************************************************************************************************************************/

    /*使能中断 NVIC. */
    EnableIRQ(ADC_ETC_IRQ0_IRQn);
}



/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
uint32_t LPSPI4_ReadData(void)
{
    status_t sta;
    g_sys_para2.ads1271IsOk = false;
//    GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1);
    uint32_t spiData = 0;
    uint8_t spiTxData[3] = {0xFF};
    uint8_t spiRxData[3] = {0x00};
    memset(spiRxData, 0, 3);
    lpspi_transfer_t spi_tranxfer;
    
    spi_tranxfer.configFlags = kLPSPI_MasterPcs1 | kLPSPI_MasterPcsContinuous;
    spi_tranxfer.txData = spiTxData;                //要发送的数据
    spi_tranxfer.rxData = spiRxData;                 //接收到的数据
    spi_tranxfer.dataSize = 3;                        //数据长度
    sta = LPSPI_MasterTransferBlocking(LPSPI4, &spi_tranxfer);	   //SPI阻塞发送
    if(sta == kStatus_Success){
        g_sys_para2.ads1271IsOk = true;
        spiData = spiRxData[2]<<16 | spiRxData[1]<<8 | spiRxData[0];
    }
    return spiData;
}






