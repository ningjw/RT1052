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
uint32_t eMMC_SaveSampleData(char *buff, uint32_t len)
{
    #define ONE_LEN 10000
    FRESULT res;
    TCHAR   fileName[20] = {0};
    UINT    g_bytesWritten;
    uint8_t  w_times = len / ONE_LEN + ((len%ONE_LEN) ? 1 : 0);
    uint32_t w_len = ONE_LEN;
    
    g_sys_para.saveOk = false;
    
    //�ж��ڳ��Ŀռ��Ƿ񹻱��β���.
    while(g_sys_para.emmc_fre_size <= len) {
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
    
    PRINTF("����%d��д���ļ�\r\n",w_times);
    for(uint8_t i=0; i < w_times; i++){
        
        res = f_open(&g_fileObject, _T(fileName), (FA_WRITE));
        if (res) { /* error or disk full */
            g_sys_para.emmcIsOk = false;
            f_unlink(fileName);
            PRINTF("���ļ�ʧ��:%d\r\n", res);
            return res;
        }
        
        /* ��λд���λ��*/
        f_lseek(&g_fileObject, i*ONE_LEN);
        
        if(i == (w_times -1)){
            w_len = len%ONE_LEN;
        }else{
            w_len = ONE_LEN;
        }
        PRINTF("��%d��д���ļ�,��СΪ%d\r\n",i,w_len);
        /* ���ļ���д������ */
        res = f_write(&g_fileObject, buff + i*ONE_LEN, w_len, &g_bytesWritten);
        if (res == FR_OK && g_bytesWritten == w_len) {
            g_sys_para.emmcIsOk = true;
        } else {
            g_sys_para.emmcIsOk = false;
            f_unlink(fileName);
            PRINTF("д���ļ�ʧ��:%d\r\n", res);
            return res;
        }
        
        PRINTF("\r\n%s �ļ���С: %d\r\n",fileName, f_size(&g_fileObject));
        
        /*�ر��ļ�*/
        res = f_close(&g_fileObject);
        if (res) {
            g_sys_para.emmcIsOk = false;
            f_unlink(fileName);
            PRINTF("�ر��ļ�ʧ��:%d\r\n", res);
        } else {
            memset(g_sys_para.fileName, 0, sizeof(g_sys_para.fileName));
            strcpy(g_sys_para.fileName, fileName);
            g_sys_para.emmcIsOk = true;
            g_sys_para.saveOk = true;
            PRINTF("�ɹ���ADC����д��:%s �ļ�\r\n",fileName);
        }
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
    CLOCK_SetDiv(kCLOCK_Usdhc1Div, 0U);
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

    BOARD_USDHCClockConfiguration();/* ��ʼ��SD����ʱ�� */
    
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

//    eMMC_CheckFatfs();
}

