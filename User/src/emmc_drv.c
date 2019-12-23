#include "emmc_drv.h"
#include "fsl_debug_console.h"
#include "ff.h"
#include "fsl_sd.h"
#include "pin_mux.h"
#include "fsl_gpio.h"


/*�ļ�ϵͳ�����ṹ��*/
FATFS g_fileSystem; /* File system object */
/*Card�ṹ������*/
extern sd_card_t g_sd;

/*���巢�ͻ������ͽ��շ��ͻ����������������ݶ���
 *˵����
  1.��SDK_SIZEALIGN(N(���ݴ�С), x)�ú궨�������������N��ֱֵ���ܱ�x������
 ���� N=6,x=4.��궨��Ľ����8��N=7,x=2�궨��Ľ��Ϊ8.
  2.��SDK_ALIGN����ʵ�����ݶ���
*/
SDK_ALIGN(uint8_t g_dataWrite[SDK_SIZEALIGN(DATA_BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));
/* ��ȡ���ݻ��� */
SDK_ALIGN(uint8_t g_dataRead[SDK_SIZEALIGN(DATA_BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));	


/*
*�������ܣ���ʼ��SD���ⲿ���š�����SD�������ѹ
*/
void USDHC1_gpio_init(void)
{
  /*����GPIO�������ýṹ��*/
  gpio_pin_config_t gpt_config;
  
    /*���ó�ʼ���ṹ��*/
  gpt_config.direction = kGPIO_DigitalOutput; //���ģʽ
  gpt_config.outputLogic =  1;                //Ĭ�ϸߵ�ƽ
  gpt_config.interruptMode = kGPIO_NoIntmode; //��ʹ���ж�
  
  /*SD1_D0*/
  IOMUXC_SetPinMux(USDHC1_DATA0_IOMUXC, 0U);
  IOMUXC_SetPinConfig(USDHC1_DATA0_IOMUXC, USDHC1_DATA_PAD_CONFIG_DATA);
  GPIO_PinInit(USDHC1_DATA0_GPIO, USDHC1_DATA0_GPIO_PIN, &gpt_config);
  
  /*SD1_D1*/
  IOMUXC_SetPinMux(USDHC1_DATA1_IOMUXC, 0U);
  IOMUXC_SetPinConfig(USDHC1_DATA1_IOMUXC, USDHC1_DATA_PAD_CONFIG_DATA);
  GPIO_PinInit(USDHC1_DATA1_GPIO, USDHC1_DATA1_GPIO_PIN, &gpt_config);

  /*SD1_D2*/
  IOMUXC_SetPinMux(USDHC1_DATA2_IOMUXC, 0U);
  IOMUXC_SetPinConfig(USDHC1_DATA2_IOMUXC, USDHC1_DATA_PAD_CONFIG_DATA);
  GPIO_PinInit(USDHC1_DATA2_GPIO, USDHC1_DATA2_GPIO_PIN, &gpt_config);
  
  /*SD1_D3*/
  IOMUXC_SetPinMux(USDHC1_DATA3_IOMUXC, 0U);
  IOMUXC_SetPinConfig(USDHC1_DATA3_IOMUXC, USDHC1_DATA_PAD_CONFIG_DATA);
  GPIO_PinInit(USDHC1_DATA3_GPIO, USDHC1_DATA3_GPIO_PIN, &gpt_config);  
  
  /*SD1_CMD*/
  IOMUXC_SetPinMux(USDHC1_CMD_IOMUXC, 0U);
  IOMUXC_SetPinConfig(USDHC1_CMD_IOMUXC, USDHC1_DATA_PAD_CONFIG_DATA);
  GPIO_PinInit(USDHC1_CMD_GPIO, USDHC1_CMD_GPIO_PIN, &gpt_config);
  
  /*SD1_CLK*/
  IOMUXC_SetPinMux(USDHC1_CLK_IOMUXC, 0U);
  IOMUXC_SetPinConfig(USDHC1_CLK_IOMUXC, USDHC1_CLK_PAD_CONFIG_DATA);
  GPIO_PinInit(USDHC1_CLK_GPIO, USDHC1_CLK_GPIO_PIN, &gpt_config);
  
  /*SD1_POWER*/
  gpt_config.outputLogic =  0;                //Ĭ�ϵ͵�ƽ
  IOMUXC_SetPinMux(SD_POWER_IOMUXC, 0U);
  IOMUXC_SetPinConfig(SD_POWER_IOMUXC, SD_POWER_PAD_CONFIG_DATA);
  GPIO_PinInit(SD_POWER_GPIO, SD_POWER_GPIO_PIN, &gpt_config);
 /*ѡ�� usdhc �����ѹ
 *��ʹ��UHS-IЭ��ͨѶʱ����Ҫ��SD�����źŵ�ѹ��Ϊ1.8V��Ĭ��Ϊ3.0V����ʵ�鲻ʹ��UHS-IЭ��ͨѶ����ѹ����Ĭ��
 */
  //UDSHC_SelectVoltage(SD_HOST_BASEADDR, SelectVoltage_for_UHS_I_1V8);
}


/**
* ��������:��ʼ��USDHCʱ��
*/
void BOARD_USDHCClockConfiguration(void)
{
  /*����ϵͳPLL PFD0 ϵ��Ϊ 0x12*/
  CLOCK_InitSysPfd(kCLOCK_Pfd0, 0x12U);
  /* ����USDHCʱ��Դ�ͷ�Ƶϵ�� */
  CLOCK_SetDiv(kCLOCK_Usdhc1Div, 0U);
  CLOCK_SetMux(kCLOCK_Usdhc1Mux, 1U);
}

/**
* ��������:��ʼ��USDHC_Host
* ��������: sd_struct,SD���ṹ��ָ�룻
* ����ֵ ��0���ɹ���-1��ʧ�ܣ�
*/
int USDHC_Host_Init(sd_card_t* sd_struct)
{
  sd_card_t *card = sd_struct;
  
  /* ��ʼ��SD����ʱ�� */
  BOARD_USDHCClockConfiguration();

  card->host.base = SD_HOST_BASEADDR;
  card->host.sourceClock_Hz = SD_HOST_CLK_FREQ;
  
  /* SD������ʼ������ */
  if (SD_HostInit(card) != kStatus_Success)
  {
    PRINTF("\r\nSD������ʼ��ʧ��\r\n");
    return -1;
  } 
  
  return 0;		
}


/**
* ��������:��ʼ��SD��
* ��������: sd_struct,SD���ṹ��ָ�룻
* ����ֵ ��0���ɹ���-1��ʧ�ܣ�
*/
int SD_Card_Init(sd_card_t* sd_struct)
{
  sd_card_t *card = sd_struct;

  /* Init card. */
  if (SD_CardInit(card))//���³�ʼ��SD��
  {
    PRINTF("\r\nSD card init failed.\r\n");
    return -1;
  }
  
  return 0;
}



/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void emmc_init(void)
{

}


