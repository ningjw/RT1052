#include "main.h"

/*文件系统描述结构体*/
FATFS g_fileSystem; /* File system object */
extern mmc_card_t g_mmc;
const TCHAR driverNumberBuffer[3U] = {MMCDISK + '0', ':', '/'};
FATFS g_fileSystem;


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


uint8_t txBuf[10] = "ningjw";
uint8_t rxBuf[10] = {0};
/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void emmc_init(void)
{
    FRESULT ret = FR_OK;
    BYTE work[FF_MAX_SS];
    
    /* 初始化SD外设时钟 */
    BOARD_USDHCClockConfiguration();

    ret = f_mount(&g_fileSystem, driverNumberBuffer, true);
    if (ret)
    {
        g_sys_para2.emmcIsOk = false;
        ret = f_mkfs(driverNumberBuffer, FM_FAT32, 0U, work, sizeof work);//制作文件系统
        if (ret){
            g_sys_para2.emmcIsOk = false;
        } else {
             g_sys_para2.emmcIsOk = true;
            ret = f_mount(&g_fileSystem, driverNumberBuffer, true);//重新挂载
            if(ret) {
                g_sys_para2.emmcIsOk = false;
                
            } else{
                 g_sys_para2.emmcIsOk = true;
            }
        }
    } else {
        g_sys_para2.emmcIsOk = true;
    }
    
//    g_mmc.host.base = USDHC1;
//    g_mmc.host.sourceClock_Hz = MMC_HOST_CLK_FREQ;
//    g_mmc.hostVoltageWindowVCC = kMMC_VoltageWindows270to360;

    /* SD主机初始化函数 */
//    if (MMC_Init(&g_mmc) != kStatus_Success)
//    {
//        PRINTF("\r\nSD主机初始化失败\r\n");
//        return;
//    }else{
//        PRINTF("\r\nSD主机初始化成功\r\n");
//        for (uint32_t i =0 ;i < 10000;i++){
//            MMC_WriteBlocks(&g_mmc, txBuf, i, 10);
//            MMC_ReadBlocks(&g_mmc, rxBuf, i, 10);
//            if ( memcmp(txBuf,rxBuf,6) != 0){
//                PRINTF("\r\n读写测试失败\r\n");
//                break;
//            }
//        }
//    }

}


