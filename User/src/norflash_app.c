#include "main.h"



/* 读写测试使用的缓冲区 */
extern uint8_t s_nor_program_buffer[];
extern uint8_t s_nor_read_buffer[];

extern status_t FlexSPI_NorFlash_Enable_QE(FLEXSPI_Type *base);
extern uint8_t  FlexSPI_FlashUUID_Get_ISSI(uint8_t *buf);

AdcInfoTotal adcInfoTotal;
AdcInfo adcInfo;

/***************************************************************************************
  * @brief   保存固件升级参数,在接受到完整的固件包后,调用该函数
  * @input   
  * @return  
***************************************************************************************/
void NorFlash_SaveUpgradePara(void)
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
  * @brief   增加一条ADC数据
  * @input   
  * @return  
***************************************************************************************/
void NorFlash_AddAdcData(void)
{
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
	sprintf(adcInfo.AdcDataTime, "%d%02d%02d%02d%02d%02d", 
		                       sampTime.year%100, sampTime.month, sampTime.day, 
	                           sampTime.hour, sampTime.minute, sampTime.second);
	//初始化 adcInfo 结构体 数据长度
	adcInfo.AdcDataLen = sizeof(ADC_Set) + g_adc_set.shkCount*4 + g_adc_set.spdCount*4;
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
	
	//保存 ADC_Set 结构体
	FlexSPI_FlashWrite((uint8_t *)&g_adc_set.IDPath[0], adcInfo.AdcDataAddr, sizeof(ADC_Set));
	
	//保存 震动数据
	FlexSPI_FlashWrite((uint8_t *)&ShakeADC[0], adcInfo.AdcDataAddr+sizeof(ADC_Set), g_adc_set.shkCount*4);
	
	//保存 转速数据
	FlexSPI_FlashWrite((uint8_t *)&SpeedADC[0], adcInfo.AdcDataAddr+sizeof(ADC_Set)+g_adc_set.shkCount*4, g_adc_set.spdCount*4);
	
	//更新 adcInfoTotal 结构体中的总采样条数
	adcInfoTotal.totalAdcInfo++;//调用该函数,表示需要增加一条adc采样数据
	if(adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){//判断出已达到最大值
		adcInfoTotal.totalAdcInfo = MAX_ADC_INFO;
	}
	//更新 adcInfoTotal 结构体中的下次采样信息保存地址
	adcInfoTotal.addrOfNewInfo = adcInfoTotal.addrOfNewInfo + sizeof(adcInfoTotal);
	if(adcInfoTotal.addrOfNewInfo >= ((ADC_INFO_SECTOR+10) * SECTOR_SIZE)){
		adcInfoTotal.addrOfNewInfo = ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(AdcInfoTotal);
	}
	//更新 adcInfoTotal 结构体中的下次采样数据保存地址
	adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + adcInfo.AdcDataLen;
	if( adcInfoTotal.addrOfNewData % 4 != 0){//判断新地址不是4字节对齐的, 需要进行4字节对齐
		adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + (4-adcInfoTotal.addrOfNewData%4);
	}
	if (adcInfoTotal.addrOfNewData > FLASH_SIZE_BYTE){//判断出新地址超出flash范围,则从头开始记录
		adcInfoTotal.addrOfNewData = ADC_DATA_SECTOR * SECTOR_SIZE;
		adcInfoTotal.freeOfKb = 0;
	}else{
		adcInfoTotal.freeOfKb = (FLASH_SIZE_BYTE - adcInfoTotal.addrOfNewData)/1024;
	}

	//更新 adcInfoTotal 结构体
	FlexSPI_FlashWrite((uint8_t *)&adcInfoTotal.totalAdcInfo, ADC_INFO_SECTOR * SECTOR_SIZE, sizeof(AdcInfoTotal));
}



/***************************************************************************************
  * @brief   通过时间读取一条adc数据
  * @input   
  * @return  返回数据地址
***************************************************************************************/
char NorFlash_ReadAdcData(char *adcDataTime)
{
	uint32_t tempAddr;
	uint8_t  ret = false;
	
	//读取数据到 adcInfoTotal 结构体
	memcpy(&adcInfoTotal.totalAdcInfo, NORFLASH_AHB_POINTER(ADC_INFO_SECTOR * SECTOR_SIZE) ,sizeof(adcInfoTotal));
	
	//flash中还未保存有数据,直接返回
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){
		return ret;
	}
	
	for(uint32_t i = 0; i<adcInfoTotal.totalAdcInfo; i++){
		//往前查找文件
		tempAddr = adcInfoTotal.addrOfNewInfo - (i+1)*sizeof(AdcInfo);
		if(tempAddr < (ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(AdcInfoTotal))){
			tempAddr = (ADC_INFO_SECTOR + 10) * SECTOR_SIZE - sizeof(adcInfo);
		}
		//读取数据到 adcInfo 结构体
		memcpy(&adcInfo.AdcDataAddr, NORFLASH_AHB_POINTER(tempAddr), sizeof(adcInfo));
		
		//找到文件
		if( memcmp(adcDataTime, adcInfo.AdcDataTime, sizeof(adcInfo.AdcDataTime)) == 0){
			ret = true;
			break;
		}
	}
	if ( ret == true){
		//读取 ADC_Set 结构体数据
		memcpy((uint8_t *)&g_adc_set.IDPath[0], NORFLASH_AHB_POINTER(adcInfo.AdcDataAddr), sizeof(ADC_Set));
		
		//读取 震动数据
		memcpy(ShakeADC, NORFLASH_AHB_POINTER(adcInfo.AdcDataAddr+sizeof(ADC_Set)), g_adc_set.shkCount);
		
		//读取 转速数据
		if (g_adc_set.spdCount != 0){
			memcpy(SpeedADC, NORFLASH_AHB_POINTER(adcInfo.AdcDataAddr+sizeof(ADC_Set)+g_adc_set.shkCount), g_adc_set.spdCount);
		}
	}
	return ret;
}

/***************************************************************************************
  * @brief   获取最近几次数据采集的文件名
  * @input   
  * @return  返回数据地址
***************************************************************************************/
void NorFlash_ReadAdcInfo(int si, int num, char *buf)
{
	uint32_t tempAddr,ei;
	
	//读取数据到 adcInfoTotal 结构体
	memcpy(&adcInfoTotal.totalAdcInfo, NORFLASH_AHB_POINTER(ADC_INFO_SECTOR * SECTOR_SIZE) ,sizeof(adcInfoTotal));
	
	//flash中还未保存有数据,直接返回
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > MAX_ADC_INFO){
		adcInfoTotal.totalAdcInfo = 0;
		adcInfoTotal.freeOfKb = (FLASH_SIZE_BYTE - ADC_DATA_SECTOR*SECTOR_SIZE)/1024;
		return;
	}
	
	if(si > MAX_ADC_INFO)si= MAX_ADC_INFO;
	if(si <= 0 ) si = 1;
	
	ei = num+si;
	if(ei > adcInfoTotal.totalAdcInfo+1) ei=adcInfoTotal.totalAdcInfo+1;
	
	for(uint32_t i = si; i<ei; i++){
		//往前查找文件
		tempAddr = adcInfoTotal.addrOfNewInfo - i*sizeof(AdcInfo);
		if(tempAddr < (ADC_INFO_SECTOR * SECTOR_SIZE + sizeof(AdcInfoTotal))){
			tempAddr = (ADC_INFO_SECTOR + 10) * SECTOR_SIZE - sizeof(adcInfo);
		}
		//读取数据到 adcInfo 结构体
		memcpy(&adcInfo.AdcDataAddr, NORFLASH_AHB_POINTER(tempAddr), sizeof(adcInfo));
		strncat(buf, adcInfo.AdcDataTime, 12);
		strcat(buf, ",");//添加分隔符
	}
	if(strlen(buf) > 0){//去掉最后一个分隔符
		buf[strlen(buf)-1] = 0x00;
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

	FlexSPI_FlashWrite(s_nor_program_buffer, APP_START_SECTOR * SECTOR_SIZE + (FIRM_ONE_LEN), FIRM_ONE_LEN);

    /* 读取数据 */
    memcpy(s_nor_read_buffer, 
           NORFLASH_AHB_POINTER(APP_START_SECTOR * SECTOR_SIZE + FIRM_ONE_LEN),
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
