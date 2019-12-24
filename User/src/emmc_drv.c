#include "emmc_drv.h"
#include "fsl_debug_console.h"
#include "ff.h"
#include "fsl_sd.h"
#include "pin_mux.h"
#include "fsl_gpio.h"
#include "diskio.h"
#include "fsl_mmc.h"
/*文件系统描述结构体*/
FATFS g_fileSystem; /* File system object */


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



mmc_card_t g_mmc;
char testBuf[] = "ningjw";
char readBuf[10] = {0};
/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void emmc_init(void)
{
    USDHC1_gpio_init();
    /* 初始化SD外设时钟 */
    BOARD_USDHCClockConfiguration();

    g_mmc.host.base = USDHC1;
    g_mmc.host.sourceClock_Hz = MMC_HOST_CLK_FREQ;
    g_mmc.hostVoltageWindowVCC = kMMC_VoltageWindows270to360;
    
    /* SD主机初始化函数 */
    if (MMC_Init(&g_mmc) != kStatus_Success)
    {
        PRINTF("\r\nSD主机初始化失败\r\n");
        return;
    }else{
        PRINTF("\r\nSD主机初始化成功\r\n");
        MMC_WriteBlocks(&g_mmc, testBuf, 10, 10);
        MMC_ReadBlocks(&g_mmc, readBuf, 10, 10);
    }
    
}


