#include "main.h"

#define FILE_NAME_STR "220000000000"
#define ONE_LEN 10000   //�ļ�ϵͳһ��д�����byte��
#define FILE_NAME_LEN  12

/*�ļ�ϵͳ�����ṹ��*/
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
  * @brief  ��ȡeMMC�ļ��б�,ͨ���������
  * @input
  * @return
***************************************************************************************/
void eMMC_ScanFile(void)
{
    FRESULT res;
    DIR     dir;
    FILINFO fno;	//�ļ���Ϣ
    res = f_opendir(&dir, driverNumberBuffer); //��һ��Ŀ¼
    if (res == FR_OK)
    {
        while(1)
        {
            res = f_readdir(&dir, &fno);                   //��ȡĿ¼�µ�һ���ļ�
            if (res != FR_OK || fno.fname[0] == 0)
                break;  //��ĩβ��,�˳�
            PRINTF("\t%s -- %d\r\n", fno.fname, fno.fsize);
        }
    }
}

/***************************************************************************************
  * @brief  ��ȡʣ��ռ�
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
        PRINTF("eMMC�ܴ�С:%d Mb\r\n",g_sys_para.emmc_tot_size/1024);
        PRINTF("eMMCʣ���С:%d Mb\r\n",g_sys_para.emmc_fre_size/1024);
    } else {
        g_sys_para.emmcIsOk = false;
    }
}



/***************************************************************************************
  * @brief   ɾ�����紴�����ļ�
  * @input   
  * @return  
***************************************************************************************/
void eMMC_DelEarliestFile(void)
{
    FRESULT res;
    UINT    br,bw;
    char   *fileStr;
    
    /*�Կɶ���д��ʽ���ļ�*/
    res = f_open(&g_fileObject, _T("manage.txt"), (FA_READ | FA_OPEN_ALWAYS));
    if (res == FR_OK || res == FR_EXIST) {
        g_sys_para.emmcIsOk = true;
    } else {
        g_sys_para.emmcIsOk = false;
        return;
    }
    
    //�����ڴ����ڱ����ļ�����
    fileStr = malloc(g_fileObject.obj.objsize + 20);
    memset(fileStr, 0U, g_fileObject.obj.objsize + 20);
    
    /* �ƶ��ļ���дָ�뵽�ļ���ʼ�� */
    f_lseek(&g_fileObject, 0U);
    
    /*��ȡ�ļ������ݵ� fileStr ������*/
    for (int i=0;;i++) {
        res = f_read(&g_fileObject, fileStr+i*ONE_LEN, ONE_LEN, &br);
        if (res || br == 0) break; /* error or eof */
    }
    f_close(&g_fileObject);
    
    /*�༭�ļ�����*/
    if(g_fileObject.obj.objsize){//����ļ����ݲ�Ϊ��
        memset(g_sys_para.earliestFile, 0, sizeof(g_sys_para.earliestFile));
        strcat(g_sys_para.earliestFile, driverNumberBuffer);
        memcpy(g_sys_para.earliestFile+3, fileStr, FILE_NAME_LEN);
        f_unlink(g_sys_para.earliestFile);
        fileStr += (FILE_NAME_LEN+2);//2��������ַ�Ϊ"\r\n"
    }
    /*���༭�õ����ݴ�ӡ����*/
    PRINTF("%s\r\n",fileStr);
    
    /*���ַ�������д�뵽 manage.txt�ļ�*/
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
  * @brief   ��manage.txt�ļ��������
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
  * @brief   ��ӡmanage.txt�ļ�����
  * @input   
  * @return  
***************************************************************************************/
void eMMC_PrintfManageFile(void)
{
    FRESULT res;
    UINT    br;
    char   *fileStr;
    
    //�����ڴ����ڱ����ļ�����
    fileStr = malloc(g_fileObject.obj.objsize + 1);
    memset(fileStr, 0U, g_fileObject.obj.objsize + 1);
    
    /*�Կɶ���д��ʽ���ļ�*/
    f_open(&g_fileObject, _T("manage.txt"), (FA_READ | FA_OPEN_ALWAYS));

    /*��ȡ�ļ������ݵ� fileStr ������*/
    for (int i=0;;i++) {
        res = f_read(&g_fileObject, fileStr+i*ONE_LEN, ONE_LEN, &br);
        
        if (res || br == 0) break; /* error or eof */
    }
    PRINTF("%s\r\n",fileStr);
    f_close(&g_fileObject);
}
/***************************************************************************************
  * @brief   �����������,ͷ���������������ݵĳ���(�ɼ�����/�ٶȲ�������/�𶯲�������),
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
    
    //�ڳ��㹻�Ŀռ䱣�汾�β���.
    eMMC_GetFree();
    while(g_sys_para.emmc_fre_size < len + 20) {
        eMMC_DelEarliestFile();
        eMMC_GetFree();
    }
    
    /* ��ȡ���� */
    SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
    
    sprintf((char *)fileName, "%04d%02d%02d%02d%02d%02d", rtcDate.year, rtcDate.month, rtcDate.day, rtcDate.hour, rtcDate.minute, rtcDate.second);
    
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
            PRINTF("�ɹ���ADC����д��:%s �ļ�\r\n",fileName);
        }
    }
    
    /*���ļ���д��manage.txt�ļ�*/
    eMMC_AppendmanageFile(g_sys_para.fileName);
    
    return res;
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

    /*���ļ�*/
    error = f_open(&g_fileObject, _T("chk.txt"), (FA_WRITE | FA_READ | FA_CREATE_NEW));
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
        PRINTF("f_mountʧ��,����f_mount\r\n");
        ret = f_mount(&g_fileSystem, driverNumberBuffer, true);
        if(ret){
            PRINTF("f_mountʧ��,��ʼf_mkfs\r\n");
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
        }else{
            g_sys_para.emmcIsOk = true;
            PRINTF("f_mount�ɹ�\r\n");
        }
    } else {
        g_sys_para.emmcIsOk = true;
        PRINTF("f_mount�ɹ�\r\n");
    }
    /*����ʹ�����·��*/
    f_chdrive((char const *)&driverNumberBuffer[0U]);
    
    eMMC_CheckFatfs();
    
    eMMC_ScanFile();
}

