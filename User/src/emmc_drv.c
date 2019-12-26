#include "main.h"

/*�ļ�ϵͳ�����ṹ��*/
FATFS g_fileSystem; /* File system object */
extern mmc_card_t g_mmc;
const TCHAR driverNumberBuffer[3U] = {MMCDISK + '0', ':', '/'};
FATFS g_fileSystem;


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
    
    /* ��ʼ��SD����ʱ�� */
    BOARD_USDHCClockConfiguration();

    ret = f_mount(&g_fileSystem, driverNumberBuffer, true);
    if (ret)
    {
        g_sys_para2.emmcIsOk = false;
        ret = f_mkfs(driverNumberBuffer, FM_FAT32, 0U, work, sizeof work);//�����ļ�ϵͳ
        if (ret){
            g_sys_para2.emmcIsOk = false;
        } else {
             g_sys_para2.emmcIsOk = true;
            ret = f_mount(&g_fileSystem, driverNumberBuffer, true);//���¹���
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

    /* SD������ʼ������ */
//    if (MMC_Init(&g_mmc) != kStatus_Success)
//    {
//        PRINTF("\r\nSD������ʼ��ʧ��\r\n");
//        return;
//    }else{
//        PRINTF("\r\nSD������ʼ���ɹ�\r\n");
//        for (uint32_t i =0 ;i < 10000;i++){
//            MMC_WriteBlocks(&g_mmc, txBuf, i, 10);
//            MMC_ReadBlocks(&g_mmc, rxBuf, i, 10);
//            if ( memcmp(txBuf,rxBuf,6) != 0){
//                PRINTF("\r\n��д����ʧ��\r\n");
//                break;
//            }
//        }
//    }

}


