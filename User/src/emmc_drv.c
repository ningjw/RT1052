#include "main.h"

#define FILE_NAME_STR "220000000000"

/*文件系统描述结构体*/
FATFS g_fileSystem; /* File system object */
FIL   g_fileObject ;   /* File object */
extern mmc_card_t g_mmc;
extern float SpeedADC[];
extern float ShakeADC[];
const TCHAR driverNumberBuffer[3U] = {MMCDISK + '0', ':', '/'};

static DWORD fre_clust;

#define BUFFER_SIZE (100U)
SDK_ALIGN(uint8_t g_bufferWrite[SDK_SIZEALIGN(BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));
SDK_ALIGN(uint8_t g_bufferRead[SDK_SIZEALIGN(BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));



AT_NONCACHEABLE_SECTION_INIT(BYTE g_data_read[FF_MAX_SS]) = {0};

/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void eMMC_GetFree() {
    FRESULT res;
    FATFS   *fs;
    /* Get volume information and free clusters of drive 3 */
    res = f_getfree("3", &fre_clust, &fs);
    if (res == FR_OK) {
        g_sys_para.emmcIsOk = true;
        /* Get total sectors and free K */
        g_sys_para.emmc_tot_size = ((fs->n_fatent - 2) * fs->csize ) / 2;
        g_sys_para.emmc_fre_size = (fre_clust * fs->csize) / 2;
        PRINTF("eMMC总大小:%d Mb\r\n",g_sys_para.emmc_tot_size/1024);
        PRINTF("eMMC剩余大小:%d Mb\r\n",g_sys_para.emmc_fre_size/1024);
    } else {
        g_sys_para.emmcIsOk = false;
    }
}


/***************************************************************************************
  * @brief   扫描文件,找出创建最早的文件,并删除
  * @input
  * @return
***************************************************************************************/
void eMMC_ScanDelFile(void)
{
    FRESULT res;
    DIR     dir;
    FILINFO fno;	//文件信息
    TCHAR   fPath[20] = {"3:/"};
    TCHAR   temp[20] = {0};
    strcpy(temp, FILE_NAME_STR);
    res = f_opendir(&dir, driverNumberBuffer); //打开一个目录
    if (res == FR_OK)
    {
        while(1)
        {
            res = f_readdir(&dir, &fno);                   //读取目录下的一个文件
            if (res != FR_OK || fno.fname[0] == 0)
                break;  //到末尾了,退出

            if(strcmp(temp, fno.fname) >= 0) {
                strcpy(temp, fno.fname);
            }
        }
        if(strcmp(FILE_NAME_STR, temp) != 0) {
            strcat(fPath, temp);
            f_unlink(fPath);
        }
    }
}


/***************************************************************************************
  * @brief   保存采样数据,头部保存了三段数据的长度(采集设置/速度采样数据/震动采样数据),
  * @input
  * @return
***************************************************************************************/
uint32_t eMMC_SaveSampleData(void)
{
    FRESULT res;
    TCHAR   fileName[20] = {0};
    UINT    g_bytesWritten;
    int     samp_set_size = 0;
    //判断腾出的空间是否够本次采样.
    while(g_sys_para.emmc_fre_size <= g_sys_para.sampFileSize) {
        eMMC_GetFree();
        eMMC_ScanDelFile();
    }

    /* 获取日期 */
    SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
    
    sprintf((char *)fileName, "%d%d%d%d%d%d", rtcDate.year, rtcDate.month, rtcDate.day, rtcDate.hour, rtcDate.minute, rtcDate.second);
    
    /*创建并打开文件*/
    res = f_open(&g_fileObject, _T(fileName), FA_CREATE_NEW);
    if (res == FR_OK || res == FR_EXIST) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
        PRINTF("创建文件失败:%d", res);
        return res;
    }
    
    
    res = f_open(&g_fileObject, _T(fileName), (FA_WRITE | FA_READ));
    if (res) { /* error or disk full */
        g_sys_para.emmcIsOk = false;
        PRINTF("打开文件失败:%d", res);
        return res;
    }

    /* 向文件内写入内容 */
    samp_set_size = (int)&g_adc_set.end - (int)&g_adc_set.start;
    res = f_write(&g_fileObject, &g_adc_set.sampSpdSize, samp_set_size, &g_bytesWritten);
    if (res == FR_OK && g_bytesWritten == samp_set_size) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
        f_unlink(fileName);
        PRINTF("写入文件失败:%d", res);
        return res;
    }

    /* Move to end of the file to append data */
    res = f_lseek(&g_fileObject, f_size(&g_fileObject));

    /* 向文件内写入内容 */
    res = f_write(&g_fileObject, SpeedADC, g_adc_set.sampSpdSize, &g_bytesWritten);
    if (res == FR_OK && g_bytesWritten == g_adc_set.sampSpdSize) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
        f_unlink(fileName);
        PRINTF("写入文件失败:%d", res);
        return res;
    }

    /* Move to end of the file to append data */
    res = f_lseek(&g_fileObject, f_size(&g_fileObject));

    /* 向文件内写入内容 */
    res = f_write(&g_fileObject, ShakeADC, g_adc_set.sampShakeSize, &g_bytesWritten);
    if (res == FR_OK && g_bytesWritten == g_adc_set.sampShakeSize) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
        f_unlink(fileName);
        PRINTF("写入文件失败:%d", res);
        return res;
    }
    
//    PRINTF("%s 文件大小: %d\r\n",fileName, f_size(&g_fileObject));
    
    /*关闭文件*/
    res = f_close(&g_fileObject);
    if (res) {
        g_sys_para.emmcIsOk = false;
        f_unlink(fileName);
        PRINTF("关闭文件失败:%d", res);
    } else {
        g_sys_para.emmcIsOk = true;
        PRINTF("成功将ADC数据写入:%s 文件\r\n",fileName);
    }
    return res;
}


/**
* 函数功能:初始化USDHC时钟
*/
void BOARD_USDHCClockConfiguration(void)
{
    /*设置系统PLL PFD0 系数为 0x12*/
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 0x12U);
    /* 配置USDHC时钟源和分频系数 */
    CLOCK_SetDiv(kCLOCK_Usdhc1Div, 4U);
    CLOCK_SetMux(kCLOCK_Usdhc1Mux, 1U);
}



/***************************************************************************************
  * @brief   文件系统自检函数
  * @input
  * @return
***************************************************************************************/
void eMMC_CheckFatfs(void)
{
    FRESULT error = FR_OK;
    UINT    g_bytesWritten;
    UINT    g_bytesRead;

    /*创建文件*/
    error = f_open(&g_fileObject, _T("chk.txt"), FA_CREATE_NEW);
    if (error == FR_OK || error == FR_EXIST) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
    }

    /*打开文件*/
    error = f_open(&g_fileObject, _T("chk.txt"), (FA_WRITE | FA_READ ));
    if (error == FR_OK || error == FR_EXIST) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
    }

    /*初始化数据缓冲区，为文件的读写做准备*/
    memset(g_bufferWrite, 'a', sizeof(g_bufferWrite));
    /* 向文件内写入内容 */
    error = f_write(&g_fileObject, g_bufferWrite, sizeof(g_bufferWrite), &g_bytesWritten);
    if (error == FR_OK && g_bytesWritten == sizeof(g_bufferWrite)) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
    }

    /* 移动文件读写指针到文件开始处 */
    if (f_lseek(&g_fileObject, 0U)) {
        g_sys_para.emmcIsOk = false;
    } else {
        g_sys_para.emmcIsOk = true;
    }

    /*读取文件的内容到 g_data_read 缓冲区*/
    memset(g_bufferRead, 0U, sizeof(g_bufferRead));
    error = f_read(&g_fileObject, g_bufferRead, sizeof(g_bufferRead), &g_bytesRead);
    if ((error == FR_OK) && (g_bytesRead == sizeof(g_bufferRead))) {
        g_sys_para.emmcIsOk = false;
    } else {
        g_sys_para.emmcIsOk = true;
    }

    /*比较读写内容是否一致*/
    if (memcmp(g_bufferWrite, g_bufferRead, sizeof(g_bufferWrite))) {
        g_sys_para.emmcIsOk = false;
    } else {
        g_sys_para.emmcIsOk = true;
    }

    /*关闭文件*/
    error = f_close(&g_fileObject);
    if (error) {
        g_sys_para.emmcIsOk = false;
    } else {
        g_sys_para.emmcIsOk = true;
    }

    eMMC_GetFree();

    /*判断文件系统是否测试成功*/
    if(g_sys_para.emmcIsOk == false) {
        g_sys_para.sampLedStatus = WORK_ERR;
    }else{
        PRINTF("文件系统校验成功\r\n");
    }
}


/***************************************************************************************
  * @brief  初始化eMMC文件系统, 并检查文件系统.
  * @input
  * @return
***************************************************************************************/
void eMMC_Init(void)
{
    FRESULT ret = FR_OK;

    /* 初始化SD外设时钟 */
    BOARD_USDHCClockConfiguration();

    ret = f_mount(&g_fileSystem, driverNumberBuffer, true);
    if (ret)
    {
        g_sys_para.emmcIsOk = false;
        ret = f_mkfs(driverNumberBuffer, FM_FAT32, 0U, g_data_read, sizeof g_data_read);//制作文件系统
        if (ret) {
            g_sys_para.emmcIsOk = false;
        } else {
            g_sys_para.emmcIsOk = true;
            ret = f_mount(&g_fileSystem, driverNumberBuffer, true);//重新挂载
            if(ret) {
                g_sys_para.emmcIsOk = false;
            } else {
                g_sys_para.emmcIsOk = true;
            }
        }
    } else {
        g_sys_para.emmcIsOk = true;
    }
    /*允许使用相对路径*/
    f_chdrive((char const *)&driverNumberBuffer[0U]);

    eMMC_CheckFatfs();
}

//uint8_t mf_scan_files(uint8_t * path)
//{
//	FRESULT res;
//    res = f_opendir(&dir,(const TCHAR*)path); //打开一个目录
//    if (res == FR_OK)
//	{
//		printf("\r\n");
//		while(1)
//		{
//	        res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
//	        if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
//	        //if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
// 			printf("%s/", path);//打印路径
//			printf("%s\r\n",fileinfo.fname);//打印文件名
//		}
//    }
//    return res;
//}
