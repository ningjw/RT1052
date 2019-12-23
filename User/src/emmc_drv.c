#include "emmc_drv.h"
#include "fsl_debug_console.h"
#include "ff.h"
#include "fsl_sd.h"
#include "pin_mux.h"
#include "fsl_gpio.h"


/*文件系统描述结构体*/
FATFS g_fileSystem; /* File system object */
/*Card结构描述符*/
extern sd_card_t g_sd;

/*定义发送缓冲区和接收发送缓冲区，并进行数据对齐
 *说明：
  1.宏SDK_SIZEALIGN(N(数据大小), x)该宏定义的作用是增加N的值直到能被x整除，
 例如 N=6,x=4.则宏定义的结果是8。N=7,x=2宏定义的结果为8.
  2.宏SDK_ALIGN用于实现数据对齐
*/
SDK_ALIGN(uint8_t g_dataWrite[SDK_SIZEALIGN(DATA_BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));
/* 读取数据缓存 */
SDK_ALIGN(uint8_t g_dataRead[SDK_SIZEALIGN(DATA_BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));	


/*
*函数功能：初始化SD卡外部引脚、设置SD卡供电电压
*/
void USDHC1_gpio_init(void)
{
  /*定义GPIO引脚配置结构体*/
  gpio_pin_config_t gpt_config;
  
    /*配置初始化结构体*/
  gpt_config.direction = kGPIO_DigitalOutput; //输出模式
  gpt_config.outputLogic =  1;                //默认高电平
  gpt_config.interruptMode = kGPIO_NoIntmode; //不使用中断
  
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
  gpt_config.outputLogic =  0;                //默认低电平
  IOMUXC_SetPinMux(SD_POWER_IOMUXC, 0U);
  IOMUXC_SetPinConfig(SD_POWER_IOMUXC, SD_POWER_PAD_CONFIG_DATA);
  GPIO_PinInit(SD_POWER_GPIO, SD_POWER_GPIO_PIN, &gpt_config);
 /*选择 usdhc 输出电压
 *当使用UHS-I协议通讯时，需要把SD总线信号电压降为1.8V，默认为3.0V。本实验不使用UHS-I协议通讯，电压保持默认
 */
  //UDSHC_SelectVoltage(SD_HOST_BASEADDR, SelectVoltage_for_UHS_I_1V8);
}


/**
* 函数功能:初始化USDHC时钟
*/
void BOARD_USDHCClockConfiguration(void)
{
  /*设置系统PLL PFD0 系数为 0x12*/
  CLOCK_InitSysPfd(kCLOCK_Pfd0, 0x12U);
  /* 配置USDHC时钟源和分频系数 */
  CLOCK_SetDiv(kCLOCK_Usdhc1Div, 0U);
  CLOCK_SetMux(kCLOCK_Usdhc1Mux, 1U);
}

/**
* 函数功能:初始化USDHC_Host
* 函数参数: sd_struct,SD卡结构体指针；
* 返回值 ：0，成功；-1：失败；
*/
int USDHC_Host_Init(sd_card_t* sd_struct)
{
  sd_card_t *card = sd_struct;
  
  /* 初始化SD外设时钟 */
  BOARD_USDHCClockConfiguration();

  card->host.base = SD_HOST_BASEADDR;
  card->host.sourceClock_Hz = SD_HOST_CLK_FREQ;
  
  /* SD主机初始化函数 */
  if (SD_HostInit(card) != kStatus_Success)
  {
    PRINTF("\r\nSD主机初始化失败\r\n");
    return -1;
  } 
  
  return 0;		
}


/**
* 函数功能:初始化SD卡
* 函数参数: sd_struct,SD卡结构体指针；
* 返回值 ：0，成功；-1：失败；
*/
int SD_Card_Init(sd_card_t* sd_struct)
{
  sd_card_t *card = sd_struct;

  /* Init card. */
  if (SD_CardInit(card))//重新初始化SD卡
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


