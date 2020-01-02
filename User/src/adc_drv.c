/* --------------------------------------------------------------------------
 * QTIMER3_TIMER0 ���PWM�������ڿ��Ʋ���Ƶ�ʣ�ͨ���װ�A-21���������
 * QTIMER1_TIMER0 ����Ϊ���벶��ͨ�׵װ�A-37���������
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
    /* ��ʼ��XBARA1ģ��*/
    XBARA_Init(XBARA1);

    /*��������������ź�֮�����ϵ*/
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputPitTrigger0, kXBARA1_OutputAdcEtcXbar0Trig0);
}


/***************************************************************************************
  * @brief   ����Ϊ�����ⲿ����
  * @input
  * @return
***************************************************************************************/
void ADC_ETC_Config(void)
{
    adc_etc_config_t adcEtcConfig;//�����ⲿ����ȫ�ֿ�������
    adc_etc_trigger_config_t adcEtcTriggerConfig; //�����ⲿ����ת��ͨ���顣��Ҫ�������ȼ���������ʽ������ͨ������
    adc_etc_trigger_chain_config_t adcEtcTriggerChainConfig; // �����ⲿ����ת��ͨ��������ADCת��ͨ��


    /*�����ⲿ����������*/
    ADC_ETC_GetDefaultConfig(&adcEtcConfig);

    /*���Ҫʹ��ADC2�����Ҫ���ø�����*/
    adcEtcConfig.enableTSCBypass = false;//���ø�λ����ʹ��ADC2

    adcEtcConfig.XBARtriggerMask = 1; /* ����ѡ���˴���ͨ��Ϊ����ͨ��0*/
    ADC_ETC_Init(ADC_ETC, &adcEtcConfig);

    /*  �����ⲿ����ת��ͨ���顣*/
    adcEtcTriggerConfig.enableSyncMode = false;                      //�첽ģʽ
    adcEtcTriggerConfig.enableSWTriggerMode = false;                 //��ֹ�������
    adcEtcTriggerConfig.triggerChainLength = 0; //����ת��ͨ��������ʵ��ת��ͨ������ΪDEMO_ADC_ETC_CHAIN_LENGTH+1
    adcEtcTriggerConfig.triggerPriority = 0U;                         //�ⲿ�������ȼ�
    adcEtcTriggerConfig.sampleIntervalDelay = 0U;
    adcEtcTriggerConfig.initialDelay = 0U;
    ADC_ETC_SetTriggerConfig(ADC_ETC, 0U, &adcEtcTriggerConfig);//�����ⲿXBAR����������

    /*�����ⲿ����ת��ͨ��������ADCת��ͨ��*/
    /*****************************************************************************************************************************/
    adcEtcTriggerChainConfig.enableB2BMode = true;                               //ʹ��B2Bģʽ
    adcEtcTriggerChainConfig.ADCHCRegisterSelect = 1; //ѡ��Ҫ����ת����ADCת��ͨ��
    adcEtcTriggerChainConfig.ADCChannelSelect = 0;                               // ѡ���ⲿ����ͨ��
    adcEtcTriggerChainConfig.InterruptEnable = kADC_ETC_Done0InterruptEnable;    // ʹ�ܸ�ͨ����ת������ж�0
    ADC_ETC_SetTriggerChainConfig(ADC_ETC, 0U, 0U, &adcEtcTriggerChainConfig); //
    /*****************************************************************************************************************************/

    /*ʹ���ж� NVIC. */
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
    spi_tranxfer.txData = spiTxData;                //Ҫ���͵�����
    spi_tranxfer.rxData = spiRxData;                 //���յ�������
    spi_tranxfer.dataSize = 3;                        //���ݳ���
    sta = LPSPI_MasterTransferBlocking(LPSPI4, &spi_tranxfer);	   //SPI��������
    if(sta == kStatus_Success){
        g_sys_para2.ads1271IsOk = true;
        spiData = spiRxData[2]<<16 | spiRxData[1]<<8 | spiRxData[0];
    }
    return spiData;
}






