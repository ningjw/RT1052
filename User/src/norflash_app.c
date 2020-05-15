/**
  ******************************************************************
  * @file    norflash_app.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  */
#include "main.h"

#define MAX_ADC_INFO 200
#define EXAMPLE_SECTOR      64//4096      /* 要进行读写测试的扇区号 */

/* 读写测试使用的缓冲区 */
extern uint8_t s_nor_program_buffer[];
extern uint8_t s_nor_read_buffer[];

extern status_t FlexSPI_NorFlash_Enable_QE(FLEXSPI_Type *base);
extern uint8_t  FlexSPI_FlashUUID_Get_ISSI(uint8_t *buf);



/***************************************************************************************
  * @brief   保存固件升级参数,在接受到完整的固件包后,调用该函数
  * @input   
  * @return  
***************************************************************************************/
void NorFlash_SaveFirmPara(void)
{
    FlexSPI_NorFlash_Erase_Sector(FLEXSPI, APP_INFO_SECTOR * SECTOR_SIZE);
    FlexSPI_NorFlash_Buffer_Program(FLEXSPI, APP_INFO_SECTOR * SECTOR_SIZE, &g_sys_para.firmUpdate, 20);
	
	/* 使用软件复位来重置 AHB 缓冲区. */
    FLEXSPI_SoftwareReset(FLEXSPI);
	PRINTF("升级参数:\r\n");
	for (int i=0;i<20;i++){
		PRINTF("%02x ", NORFLASH_AHB_READ_BYTE(APP_INFO_SECTOR * SECTOR_SIZE + i));
	}
}

/***************************************************************************************
  * @brief   增加一条json格式的ADC数据
  * @input   
  * @return  
***************************************************************************************/
void NorFlash_AddAdcInfo(char *adcJsonData)
{
	
	AdcInfoTotal adcInfoTotal;
	AdcInfo adcInfo;
	
	//前12字节保存的是 adcInfoTotal 结构体
	memcpy(&adcInfoTotal.totalAdcInfo, NORFLASH_AHB_POINTER(ADC_INFO_SECTOR * SECTOR_SIZE) ,sizeof(adcInfoTotal));
	
	//判断为首次上电运行
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){
		//总数为初始化为0
		adcInfoTotal.totalAdcInfo = 0;
		//本次 AdcInfo 结构体保存地址
		adcInfoTotal.addrOfNewInfo = ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(adcInfoTotal);
		//本次数据的开始地址
		adcInfoTotal.addrOfNewData = ADC_DATA_SECTOR * SECTOR_SIZE;
	}
	
	//初始化 adcInfo 结构体 数据时间
    SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
	sprintf(adcInfo.AdcDataTime, "%d%02d%02d%02d%02d%02d", 
		                       rtcDate.year%100, rtcDate.month, rtcDate.day, 
	                           rtcDate.hour, rtcDate.minute, rtcDate.second);
	//初始化 adcInfo 结构体 数据长度
	adcInfo.AdcDataLen = strlen(adcJsonData) + 1;//将'\0'结束符也要保存到flash当中
	//初始化 adcInfo 结构体 数据地址
	adcInfo.AdcDataAddr = adcInfoTotal.addrOfNewData;
	if((adcInfo.AdcDataAddr % 4) != 0){//判断地址是否四字节对齐
		adcInfo.AdcDataAddr = adcInfo.AdcDataAddr + (4 - (adcInfo.AdcDataAddr % 4));
	}
	if((adcInfo.AdcDataAddr + adcInfo.AdcDataLen) > FLASH_SIZE_BYTE){//判断地址是否超过flash范围
		adcInfo.AdcDataAddr = ADC_DATA_SECTOR * SECTOR_SIZE;
	}
	
	//保存 adcInfo 结构体
	FlexSPI_FlashWrite((uint8_t *)&adcInfo.AdcDataAddr, adcInfoTotal.addrOfNewInfo, sizeof(adcInfo));
	
	//保存 采样数据
	FlexSPI_FlashWrite((uint8_t *)adcJsonData, adcInfo.AdcDataAddr, adcInfo.AdcDataLen);
	
	//初始化 adcInfoTotal 结构体中的总采样条数
	adcInfoTotal.totalAdcInfo++;//调用该函数,表示需要增加一条adc采样数据
	if(adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){//判断出已达到最大值
		adcInfoTotal.totalAdcInfo = MAX_ADC_INFO;
	}
	//初始化 adcInfoTotal 结构体中的下次采样信息保存地址
	adcInfoTotal.addrOfNewInfo = adcInfoTotal.addrOfNewInfo + sizeof(adcInfoTotal);
	if(adcInfoTotal.addrOfNewInfo >= ((ADC_INFO_SECTOR+1) * SECTOR_SIZE)){
		adcInfoTotal.addrOfNewInfo = ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(adcInfo);
	}
	//初始化 adcInfoTotal 结构体中的下次采样数据保存地址
	adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + adcInfo.AdcDataLen;
	if( adcInfoTotal.addrOfNewData % 4 != 0){//判断新地址不是4字节对齐的, 需要进行4字节对齐
		adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + (4-adcInfoTotal.addrOfNewData%4);
	}
	if (adcInfoTotal.addrOfNewData > FLASH_SIZE_BYTE){//判断出新地址超出flash范围,则从头开始记录
		adcInfoTotal.addrOfNewData = ADC_DATA_SECTOR * SECTOR_SIZE;
	}
	
	//保存 adcInfoTotal 结构体
	FlexSPI_FlashWrite((uint8_t *)adcInfoTotal.totalAdcInfo, ADC_INFO_SECTOR * SECTOR_SIZE, sizeof(AdcInfoTotal));
}



/***************************************************************************************
  * @brief   通过时间读取一条adc数据
  * @input   
  * @return  返回数据地址
***************************************************************************************/
uint32_t NorFlash_ReadAdcData(char *adcDataTime)
{
	char *jsonAdcData;
	uint32_t tempAddr;
	AdcInfoTotal adcInfoTotal;
	AdcInfo adcInfo;
	
	//读取数据到 adcInfoTotal 结构体
	memcpy(&adcInfoTotal.totalAdcInfo, NORFLASH_AHB_POINTER(ADC_INFO_SECTOR * SECTOR_SIZE) ,sizeof(adcInfoTotal));
	
	//flash中还未保存有数据,直接返回
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){
		return NULL;
	}
	
	for(uint32_t i = 0; i<adcInfoTotal.totalAdcInfo; i++){
		//往前查找文件
		tempAddr = adcInfoTotal.addrOfNewInfo - (i+1)*sizeof(AdcInfo);
		if(tempAddr < (ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(AdcInfoTotal))){
			tempAddr = (ADC_INFO_SECTOR + 1) * SECTOR_SIZE - sizeof(adcInfo);
		}
		//读取数据到 adcInfo 结构体
		memcpy(&adcInfo.AdcDataAddr, NORFLASH_AHB_POINTER(tempAddr), sizeof(adcInfo));
		
		//找到文件
		if( memcmp(adcDataTime, adcInfo.AdcDataTime, sizeof(adcInfo.AdcDataTime)) == 0){
			return adcInfo.AdcDataAddr;
		}
	}
	
	return NULL;
}

/***************************************************************************************
  * @brief   获取最近几次数据采集的文件名
  * @input   
  * @return  返回数据地址
***************************************************************************************/
void NorFlash_ReadAdcInfo(int si, int num, char *buf)
{
	char *jsonAdcData;
	uint32_t tempAddr;
	AdcInfoTotal adcInfoTotal;
	AdcInfo adcInfo;
	
	//读取数据到 adcInfoTotal 结构体
	memcpy(&adcInfoTotal.totalAdcInfo, NORFLASH_AHB_POINTER(ADC_INFO_SECTOR * SECTOR_SIZE) ,sizeof(adcInfoTotal));
	
	//flash中还未保存有数据,直接返回
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){
		return;
	}
	
	if(si > MAX_ADC_INFO)si= MAX_ADC_INFO;
	if(si <= 0 ) si = 1;
	
	for(uint32_t i = si; i<num+si; i++){
		//往前查找文件
		tempAddr = adcInfoTotal.addrOfNewInfo - i*sizeof(AdcInfo);
		if(tempAddr < (ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(AdcInfoTotal))){
			tempAddr = (ADC_INFO_SECTOR + 1) * SECTOR_SIZE - sizeof(adcInfo);
		}
		//读取数据到 adcInfo 结构体
		memcpy(&adcInfo.AdcDataAddr, NORFLASH_AHB_POINTER(tempAddr), sizeof(adcInfo));
		
		strcat(buf, adcInfo.AdcDataTime);
		strcat(buf,";");//使用逗号分隔开
	}
}



/**
* @brief  使用AHB命令的方式进行读写测试
* @param  无
* @retval 测试结果，正常为0
*/
int NorFlash_ChkSelf(void)
{
    uint32_t i = 0;
    uint32_t JedecDeviceID = 0;

    /***************************读ID测试****************************/
    /* 获取JedecDevice ID. */
    FlexSPI_NorFlash_Get_JedecDevice_ID(FLEXSPI, &JedecDeviceID);
    
    if(JedecDeviceID != FLASH_WINBOND_JEDECDEVICE_ID && 
			 JedecDeviceID != FLASH_ISSI_JEDECDEVICE_ID)
    {
      PRINTF("FLASH检测错误，读取到的JedecDeviceID值为: 0x%x\r\n", JedecDeviceID);
      return -1;
    }
    
    /***************************一次写入一个扇区数据测试****************************/
    for (i = 0; i < FIRM_ONE_LEN; i++){
        s_nor_program_buffer[i] = (uint8_t)i;
    }

	FlexSPI_FlashWrite(s_nor_program_buffer, EXAMPLE_SECTOR * SECTOR_SIZE + FIRM_ONE_LEN, FIRM_ONE_LEN);

    /* 读取数据 */
    memcpy(s_nor_read_buffer, 
           NORFLASH_AHB_POINTER(EXAMPLE_SECTOR * SECTOR_SIZE + FIRM_ONE_LEN),
           FIRM_ONE_LEN);
    
    /* 把读出的数据与写入的比较 */
    if (memcmp(s_nor_program_buffer, s_nor_read_buffer,FIRM_ONE_LEN))
    {
        PRINTF("写入数据，读出数据不正确 !\r\n ");
        return -1;
    } else {
        PRINTF("大量数据写入和读取测试成功. \r\n");
    }

    return 0;  
}

/****************************END OF FILE**********************/
