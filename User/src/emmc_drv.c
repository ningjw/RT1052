#include "emmc_drv.h"
#include "fsl_iomuxc.h"
#include "fsl_mmc.h"
#include "fsl_debug_console.h"

/*! @brief Card结构描述符. */
mmc_card_t g_emmc;


/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void BOARD_MMC_Pin_Config(uint32_t speed, uint32_t strength)
{
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_00_USDHC1_CMD,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                        IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_01_USDHC1_CLK,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(0) |
                        IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_02_USDHC1_DATA0,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                        IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_03_USDHC1_DATA1,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                        IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_04_USDHC1_DATA2,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                        IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_05_USDHC1_DATA3,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                        IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
}

/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
static void BOARD_USDHCClockConfiguration(void)
{
    /*设置系统PLL PFD2 系数为 18*/
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 0x12U);
    /* 配置USDHC时钟源和分频系数 */
    CLOCK_SetDiv(kCLOCK_Usdhc1Div, 0U);
    CLOCK_SetMux(kCLOCK_Usdhc1Mux, 1U);
}


/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
int emmc_init(void)
{
    mmc_card_t *emmc = &g_emmc;
    /* 初始化SD外设时钟 */
    BOARD_USDHCClockConfiguration();

    emmc->host.base = MMC_HOST_BASEADDR;
    emmc->host.sourceClock_Hz = MMC_HOST_CLK_FREQ;

    /* SD主机初始化函数 */
    if (MMC_HostInit(emmc) != kStatus_Success)
    {
        PRINTF("\r\nSD主机初始化失败\r\n");
        return -1;
    }
    /* 初始化SD卡 */
    if (MMC_CardInit(emmc))
    {
        PRINTF("\r\nSD初始化失败\r\n");
        return -1;
    }
    return 0;
}


