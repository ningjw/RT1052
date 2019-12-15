/* -------------------------------------------------------------------------- 
 * QTIMER3_TIMER0 ���PWM�������ڿ��Ʋ���Ƶ�ʣ�ͨ���װ�A-21���������
 * QTIMER1_TIMER0 ����Ϊ���벶��ͨ�׵װ�A-37���������
 * ADC_MODE: 0:High-Speed mode; float:High-Resolution mode; 1:Low-Power mode
 * 
 * 
 *
 *---------------------------------------------------------------------------*/
#include "main.h"


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







