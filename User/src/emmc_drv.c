#include "main.h"

#define FILE_NAME_STR "220000000000"
#define ONE_LEN 10000   //文件系统一次写入最大byte数
#define FILE_NAME_LEN  12

/*文件系统描述结构体*/
FATFS g_fileSystem; /* File system object */
FIL   g_fileObject ;   /* File object */
extern mmc_card_t g_mmc;
extern float SpeedADC[];
extern float ShakeADC[];
const TCHAR driverNumberBuffer[3U] = {MMCDISK + '0', ':', '/'};

static DWORD fre_clust;

#define BUFFER_SIZE (120U)
SDK_ALIGN(uint8_t g_bufferWrite[SDK_SIZEALIGN(BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));
SDK_ALIGN(uint8_t g_bufferRead[SDK_SIZEALIGN(BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));



AT_NONCACHEABLE_SECTION_INIT(BYTE g_data_read[FF_MAX_SS]) = {0};
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

/***************************************************************************************
  * @brief  获取eMMC文件列表,通过串口输出
  * @input
  * @return
***************************************************************************************/
void eMMC_ScanFile(void)
{
    FRESULT res;
    DIR     dir;
    FILINFO fno;	//文件信息
    res = f_opendir(&dir, driverNumberBuffer); //打开一个目录
    if (res == FR_OK)
    {
        while(1)
        {
            res = f_readdir(&dir, &fno);                   //读取目录下的一个文件
            if (res != FR_OK || fno.fname[0] == 0)
                break;  //到末尾了,退出
            PRINTF("\t%s -- %d\r\n", fno.fname, fno.fsize);
        }
    }
}

/***************************************************************************************
  * @brief  获取剩余空间
  * @input
  * @return
***************************************************************************************/
void eMMC_GetFree(void) 
{
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
  * @brief   删除最早创建的文件
  * @input   
  * @return  
***************************************************************************************/
void eMMC_DelEarliestFile(void)
{
    FRESULT res;
    UINT    br,bw;
    char   *fileStr;
    
    /*以可读可写方式打开文件*/
    res = f_open(&g_fileObject, _T("manage.txt"), (FA_READ | FA_OPEN_ALWAYS));
    if (res == FR_OK || res == FR_EXIST) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
        return;
    }
    
    //申请内存用于保存文件内容
    fileStr = malloc(g_fileObject.obj.objsize + 20);
    memset(fileStr, 0U, g_fileObject.obj.objsize + 20);
    
    /* 移动文件读写指针到文件开始处 */
    f_lseek(&g_fileObject, 0U);
    
    /*读取文件的内容到 fileStr 缓冲区*/
    for (int i=0;;i++) {
        res = f_read(&g_fileObject, fileStr+i*ONE_LEN, ONE_LEN, &br);
        if (res || br == 0) break; /* error or eof */
    }
    f_close(&g_fileObject);
    
    /*编辑文件内容*/
    if(g_fileObject.obj.objsize){//如果文件内容不为空
        memset(g_sys_para.earliestFile, 0, sizeof(g_sys_para.earliestFile));
        strcat(g_sys_para.earliestFile, driverNumberBuffer);
        memcpy(g_sys_para.earliestFile+3, fileStr, FILE_NAME_LEN);
        f_unlink(g_sys_para.earliestFile);
        fileStr += (FILE_NAME_LEN+2);//2个额外的字符为"\r\n"
    }
    /*将编辑好的内容打印出来*/
    PRINTF("%s\r\n",fileStr);
    
    /*将字符串重新写入到 manage.txt文件*/
    f_open(&g_fileObject, _T("manage.txt"), (FA_WRITE | FA_CREATE_ALWAYS));
    uint32_t fileSize = strlen(fileStr);
    uint32_t w_len = 0;
    uint8_t  w_times = fileSize / ONE_LEN + ((fileSize%ONE_LEN) ? 1 : 0);
    for (int i=0; i < w_times; i++) {
        
        if(i == (w_times -1)){
            w_len = fileSize%ONE_LEN;
        }else{
            w_len = ONE_LEN;
        }
        
        f_write(&g_fileObject, fileStr+i*ONE_LEN, w_len, &bw);
    }
    f_close(&g_fileObject);
}



/***************************************************************************************
  * @brief   在manage.txt文件添加内容
  * @input   
  * @return  
***************************************************************************************/
void eMMC_AppendmanageFile(char *str)
{
    UINT    bw;
    TCHAR   fileName[20] = {0};
    strcat(fileName,str);
    strcat(fileName,"\r\n");
    f_open(&g_fileObject, _T("manage.txt"), (FA_WRITE | FA_OPEN_ALWAYS | FA_OPEN_APPEND));
    f_write(&g_fileObject, fileName, strlen(fileName), &bw);
    f_close(&g_fileObject);
}
/***************************************************************************************
  * @brief   打印manage.txt文件内容
  * @input   
  * @return  
***************************************************************************************/
void eMMC_PrintfManageFile(void)
{
    FRESULT res;
    UINT    br;
    char   *fileStr;
    
    //申请内存用于保存文件内容
    fileStr = malloc(g_fileObject.obj.objsize + 1);
    memset(fileStr, 0U, g_fileObject.obj.objsize + 1);
    
    /*以可读可写方式打开文件*/
    f_open(&g_fileObject, _T("manage.txt"), (FA_READ | FA_OPEN_ALWAYS));

    /*读取文件的内容到 fileStr 缓冲区*/
    for (int i=0;;i++) {
        res = f_read(&g_fileObject, fileStr+i*ONE_LEN, ONE_LEN, &br);
        
        if (res || br == 0) break; /* error or eof */
    }
    PRINTF("%s\r\n",fileStr);
    f_close(&g_fileObject);
}
/***************************************************************************************
  * @brief   保存采样数据,头部保存了三段数据的长度(采集设置/速度采样数据/震动采样数据),
  * @input
  * @return
***************************************************************************************/
uint32_t eMMC_SaveSampleData(char *buff, uint32_t len)
{
    FRESULT res;
    TCHAR   fileName[20] = {0};
    UINT    g_bytesWritten;
    uint8_t  w_times = len / ONE_LEN + ((len%ONE_LEN) ? 1 : 0);
    uint32_t w_len = ONE_LEN;
    
    //腾出足够的空间保存本次采样.
    eMMC_GetFree();
    while(g_sys_para.emmc_fre_size < len + 20) {
        eMMC_DelEarliestFile();
        eMMC_GetFree();
    }
    
    /* 获取日期 */
    SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
    
    sprintf((char *)fileName, "%04d%02d%02d%02d%02d%02d", rtcDate.year, rtcDate.month, rtcDate.day, rtcDate.hour, rtcDate.minute, rtcDate.second);
    
    /*创建并打开文件*/
    res = f_open(&g_fileObject, _T(fileName), FA_CREATE_NEW);
    if (res == FR_OK || res == FR_EXIST) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
        PRINTF("创建文件失败:%d", res);
        return res;
    }
    
    PRINTF("共分%d次写入文件\r\n",w_times);
    for(uint8_t i=0; i < w_times; i++){
        
        res = f_open(&g_fileObject, _T(fileName), (FA_WRITE));
        if (res) { /* error or disk full */
            g_sys_para.emmcIsOk = false;
            f_unlink(fileName);
            PRINTF("打开文件失败:%d\r\n", res);
            return res;
        }
        
        /* 定位写入的位置*/
        f_lseek(&g_fileObject, i*ONE_LEN);
        
        if(i == (w_times -1)){
            w_len = len%ONE_LEN;
        }else{
            w_len = ONE_LEN;
        }
        PRINTF("第%d次写入文件,大小为%d\r\n",i,w_len);
        /* 向文件内写入内容 */
        res = f_write(&g_fileObject, buff + i*ONE_LEN, w_len, &g_bytesWritten);
        if (res == FR_OK && g_bytesWritten == w_len) {
            g_sys_para.emmcIsOk = true;
        } else {
            g_sys_para.emmcIsOk = false;
            f_unlink(fileName);
            PRINTF("写入文件失败:%d\r\n", res);
            return res;
        }
        
        PRINTF("\r\n%s 文件大小: %d\r\n",fileName, f_size(&g_fileObject));
        
        /*关闭文件*/
        res = f_close(&g_fileObject);
        if (res) {
            g_sys_para.emmcIsOk = false;
            f_unlink(fileName);
            PRINTF("关闭文件失败:%d\r\n", res);
        } else {
            memset(g_sys_para.fileName, 0, sizeof(g_sys_para.fileName));
            strcpy(g_sys_para.fileName, fileName);
            g_sys_para.emmcIsOk = true;
            PRINTF("成功将ADC数据写入:%s 文件\r\n",fileName);
        }
    }
    
    /*将文件名写入manage.txt文件*/
    eMMC_AppendmanageFile(g_sys_para.fileName);
    
    return res;
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

    /*打开文件*/
    error = f_open(&g_fileObject, _T("chk.txt"), (FA_WRITE | FA_READ | FA_CREATE_NEW));
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

    BOARD_USDHCClockConfiguration();/* 初始化SD外设时钟 */
    ret = f_mount(&g_fileSystem, driverNumberBuffer, true);
    if (ret)
    {
        PRINTF("f_mount失败,重试f_mount\r\n");
        ret = f_mount(&g_fileSystem, driverNumberBuffer, true);
        if(ret){
            PRINTF("f_mount失败,开始f_mkfs\r\n");
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
        }else{
            g_sys_para.emmcIsOk = true;
            PRINTF("f_mount成功\r\n");
        }
    } else {
        g_sys_para.emmcIsOk = true;
        PRINTF("f_mount成功\r\n");
    }
    /*允许使用相对路径*/
    f_chdrive((char const *)&driverNumberBuffer[0U]);
    
    eMMC_CheckFatfs();
    
    eMMC_ScanFile();
}

