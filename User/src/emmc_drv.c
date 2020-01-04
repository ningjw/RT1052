#include "main.h"

#define FILE_NAME_STR "220000000000"

/*�ļ�ϵͳ�����ṹ��*/
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
        PRINTF("eMMC�ܴ�С:%d Mb\r\n",g_sys_para.emmc_tot_size/1024);
        PRINTF("eMMCʣ���С:%d Mb\r\n",g_sys_para.emmc_fre_size/1024);
    } else {
        g_sys_para.emmcIsOk = false;
    }
}


/***************************************************************************************
  * @brief   ɨ���ļ�,�ҳ�����������ļ�,��ɾ��
  * @input
  * @return
***************************************************************************************/
void eMMC_ScanDelFile(void)
{
    FRESULT res;
    DIR     dir;
    FILINFO fno;	//�ļ���Ϣ
    TCHAR   fPath[20] = {"3:/"};
    TCHAR   temp[20] = {0};
    strcpy(temp, FILE_NAME_STR);
    res = f_opendir(&dir, driverNumberBuffer); //��һ��Ŀ¼
    if (res == FR_OK)
    {
        while(1)
        {
            res = f_readdir(&dir, &fno);                   //��ȡĿ¼�µ�һ���ļ�
            if (res != FR_OK || fno.fname[0] == 0)
                break;  //��ĩβ��,�˳�

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
  * @brief   �����������,ͷ���������������ݵĳ���(�ɼ�����/�ٶȲ�������/�𶯲�������),
  * @input
  * @return
***************************************************************************************/
uint32_t eMMC_SaveSampleData(void)
{
    FRESULT res;
    TCHAR   fileName[20] = {0};
    UINT    g_bytesWritten;
    int     samp_set_size = 0;
    //�ж��ڳ��Ŀռ��Ƿ񹻱��β���.
    while(g_sys_para.emmc_fre_size <= g_sys_para.sampFileSize) {
        eMMC_GetFree();
        eMMC_ScanDelFile();
    }

    /* ��ȡ���� */
    SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
    
    sprintf((char *)fileName, "%d%d%d%d%d%d", rtcDate.year, rtcDate.month, rtcDate.day, rtcDate.hour, rtcDate.minute, rtcDate.second);
    
    /*���������ļ�*/
    res = f_open(&g_fileObject, _T(fileName), FA_CREATE_NEW);
    if (res == FR_OK || res == FR_EXIST) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
        PRINTF("�����ļ�ʧ��:%d", res);
        return res;
    }
    
    
    res = f_open(&g_fileObject, _T(fileName), (FA_WRITE | FA_READ));
    if (res) { /* error or disk full */
        g_sys_para.emmcIsOk = false;
        PRINTF("���ļ�ʧ��:%d", res);
        return res;
    }

    /* ���ļ���д������ */
    samp_set_size = (int)&g_adc_set.end - (int)&g_adc_set.start;
    res = f_write(&g_fileObject, &g_adc_set.sampSpdSize, samp_set_size, &g_bytesWritten);
    if (res == FR_OK && g_bytesWritten == samp_set_size) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
        f_unlink(fileName);
        PRINTF("д���ļ�ʧ��:%d", res);
        return res;
    }

    /* Move to end of the file to append data */
    res = f_lseek(&g_fileObject, f_size(&g_fileObject));

    /* ���ļ���д������ */
    res = f_write(&g_fileObject, SpeedADC, g_adc_set.sampSpdSize, &g_bytesWritten);
    if (res == FR_OK && g_bytesWritten == g_adc_set.sampSpdSize) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
        f_unlink(fileName);
        PRINTF("д���ļ�ʧ��:%d", res);
        return res;
    }

    /* Move to end of the file to append data */
    res = f_lseek(&g_fileObject, f_size(&g_fileObject));

    /* ���ļ���д������ */
    res = f_write(&g_fileObject, ShakeADC, g_adc_set.sampShakeSize, &g_bytesWritten);
    if (res == FR_OK && g_bytesWritten == g_adc_set.sampShakeSize) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
        f_unlink(fileName);
        PRINTF("д���ļ�ʧ��:%d", res);
        return res;
    }
    
//    PRINTF("%s �ļ���С: %d\r\n",fileName, f_size(&g_fileObject));
    
    /*�ر��ļ�*/
    res = f_close(&g_fileObject);
    if (res) {
        g_sys_para.emmcIsOk = false;
        f_unlink(fileName);
        PRINTF("�ر��ļ�ʧ��:%d", res);
    } else {
        g_sys_para.emmcIsOk = true;
        PRINTF("�ɹ���ADC����д��:%s �ļ�\r\n",fileName);
    }
    return res;
}


/**
* ��������:��ʼ��USDHCʱ��
*/
void BOARD_USDHCClockConfiguration(void)
{
    /*����ϵͳPLL PFD0 ϵ��Ϊ 0x12*/
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 0x12U);
    /* ����USDHCʱ��Դ�ͷ�Ƶϵ�� */
    CLOCK_SetDiv(kCLOCK_Usdhc1Div, 4U);
    CLOCK_SetMux(kCLOCK_Usdhc1Mux, 1U);
}



/***************************************************************************************
  * @brief   �ļ�ϵͳ�Լ캯��
  * @input
  * @return
***************************************************************************************/
void eMMC_CheckFatfs(void)
{
    FRESULT error = FR_OK;
    UINT    g_bytesWritten;
    UINT    g_bytesRead;

    /*�����ļ�*/
    error = f_open(&g_fileObject, _T("chk.txt"), FA_CREATE_NEW);
    if (error == FR_OK || error == FR_EXIST) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
    }

    /*���ļ�*/
    error = f_open(&g_fileObject, _T("chk.txt"), (FA_WRITE | FA_READ ));
    if (error == FR_OK || error == FR_EXIST) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
    }

    /*��ʼ�����ݻ�������Ϊ�ļ��Ķ�д��׼��*/
    memset(g_bufferWrite, 'a', sizeof(g_bufferWrite));
    /* ���ļ���д������ */
    error = f_write(&g_fileObject, g_bufferWrite, sizeof(g_bufferWrite), &g_bytesWritten);
    if (error == FR_OK && g_bytesWritten == sizeof(g_bufferWrite)) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
    }

    /* �ƶ��ļ���дָ�뵽�ļ���ʼ�� */
    if (f_lseek(&g_fileObject, 0U)) {
        g_sys_para.emmcIsOk = false;
    } else {
        g_sys_para.emmcIsOk = true;
    }

    /*��ȡ�ļ������ݵ� g_data_read ������*/
    memset(g_bufferRead, 0U, sizeof(g_bufferRead));
    error = f_read(&g_fileObject, g_bufferRead, sizeof(g_bufferRead), &g_bytesRead);
    if ((error == FR_OK) && (g_bytesRead == sizeof(g_bufferRead))) {
        g_sys_para.emmcIsOk = false;
    } else {
        g_sys_para.emmcIsOk = true;
    }

    /*�Ƚ϶�д�����Ƿ�һ��*/
    if (memcmp(g_bufferWrite, g_bufferRead, sizeof(g_bufferWrite))) {
        g_sys_para.emmcIsOk = false;
    } else {
        g_sys_para.emmcIsOk = true;
    }

    /*�ر��ļ�*/
    error = f_close(&g_fileObject);
    if (error) {
        g_sys_para.emmcIsOk = false;
    } else {
        g_sys_para.emmcIsOk = true;
    }

    eMMC_GetFree();

    /*�ж��ļ�ϵͳ�Ƿ���Գɹ�*/
    if(g_sys_para.emmcIsOk == false) {
        g_sys_para.sampLedStatus = WORK_ERR;
    }else{
        PRINTF("�ļ�ϵͳУ��ɹ�\r\n");
    }
}


/***************************************************************************************
  * @brief  ��ʼ��eMMC�ļ�ϵͳ, ������ļ�ϵͳ.
  * @input
  * @return
***************************************************************************************/
void eMMC_Init(void)
{
    FRESULT ret = FR_OK;

    /* ��ʼ��SD����ʱ�� */
    BOARD_USDHCClockConfiguration();

    ret = f_mount(&g_fileSystem, driverNumberBuffer, true);
    if (ret)
    {
        g_sys_para.emmcIsOk = false;
        ret = f_mkfs(driverNumberBuffer, FM_FAT32, 0U, g_data_read, sizeof g_data_read);//�����ļ�ϵͳ
        if (ret) {
            g_sys_para.emmcIsOk = false;
        } else {
            g_sys_para.emmcIsOk = true;
            ret = f_mount(&g_fileSystem, driverNumberBuffer, true);//���¹���
            if(ret) {
                g_sys_para.emmcIsOk = false;
            } else {
                g_sys_para.emmcIsOk = true;
            }
        }
    } else {
        g_sys_para.emmcIsOk = true;
    }
    /*����ʹ�����·��*/
    f_chdrive((char const *)&driverNumberBuffer[0U]);

    eMMC_CheckFatfs();
}

//uint8_t mf_scan_files(uint8_t * path)
//{
//	FRESULT res;
//    res = f_opendir(&dir,(const TCHAR*)path); //��һ��Ŀ¼
//    if (res == FR_OK)
//	{
//		printf("\r\n");
//		while(1)
//		{
//	        res = f_readdir(&dir, &fileinfo);                   //��ȡĿ¼�µ�һ���ļ�
//	        if (res != FR_OK || fileinfo.fname[0] == 0) break;  //������/��ĩβ��,�˳�
//	        //if (fileinfo.fname[0] == '.') continue;             //�����ϼ�Ŀ¼
// 			printf("%s/", path);//��ӡ·��
//			printf("%s\r\n",fileinfo.fname);//��ӡ�ļ���
//		}
//    }
//    return res;
//}
