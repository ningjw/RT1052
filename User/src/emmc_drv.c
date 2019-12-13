#include "emmc_drv.h"
#include "fsl_iomuxc.h"
#include "fsl_mmc.h"
#include "fsl_debug_console.h"
#include "ff.h"
#include "fsl_mmc_disk.h"
/*文件系统描述结构体*/
FATFS g_fileSystem; /* File system object */

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
void emmc_init(void)
{
    FRESULT res = FR_OK;
    const TCHAR driverNumberBuffer[3U] = {MMCDISK + '0', ':', '/'};
    BYTE work[FF_MAX_SS];
    
    /* 初始化SD外设时钟 */
    BOARD_USDHCClockConfiguration();
    
    res = f_mount(&g_fileSystem, driverNumberBuffer, 0U);
    if (res == FR_NO_FILESYSTEM){
        PRINTF("开始制作文件系统");
        if (f_mkfs(driverNumberBuffer, FM_ANY, 0U, work, sizeof work) == FR_OK){
            //重新挂载系统
            if(f_mount(&g_fileSystem, driverNumberBuffer, 0U) != FR_OK){
                PRINTF("eMMC错误");
            }
        }
    }
    else if(res == FR_OK){
        PRINTF("文件挂载成功");
    }else {
        PRINTF("eMMC错误");
    }
}


