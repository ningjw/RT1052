#include "main.h"

flexspi_config_t flexspi_norflash_config;   //FlexSPI Nor Flash配置结构体

__align(64) uint8_t s_nor_program_buffer[256];
__align(64) uint8_t s_nor_read_buffer[256];


//页编程，在指定地址开始写入最大256字节的数据，页大小为256B
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(最大32bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!
//返回值：0 成功，其他值 失败 
uint8_t FlexSPI_FlashPage_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint8_t ret=0;
    flexspi_transfer_t FlashTransfer;
    
    FlexSPI_NorFlash_Write_Enable(FLEXSPI);                  //写使能
    FlexSPI_NorFlash_Wait_Bus_Busy(FLEXSPI);                               //等待Flash空闲
    
    FlashTransfer.deviceAddress=WriteAddr;                  //地址
    FlashTransfer.port=kFLEXSPI_PortA1;                     //端口
    FlashTransfer.cmdType=kFLEXSPI_Write;                   //操作类型：写数据
    FlashTransfer.SeqNumber=1;                              //序号
    FlashTransfer.seqIndex=NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD;    //LUT表中命令索引
    FlashTransfer.data=(uint32_t *)pBuffer;                      //要写入的数据
    FlashTransfer.dataSize=NumByteToWrite;                  //数据长度
	
    if(FLEXSPI_TransferBlocking(FLEXSPI,&FlashTransfer)!=kStatus_Success) ret=1;

    FlexSPI_NorFlash_Wait_Bus_Busy(FLEXSPI);           //等待Flash空闲
	/* 等待写入完成 */
    FlexSPI_NorFlash_Wait_Bus_Busy(FLEXSPI);
    return ret;  
}

//从FlexSPI Flash中读取数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(最大32bit)
//NumByteToRead:要读取的字节数(最大65535)
void FlexSPI_FlashRead(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)  
{
    uint32_t index=0;
    uint32_t startaddr=0;
    
    startaddr=FlexSPI_AMBA_BASE+ReadAddr;

    //复位AHB缓冲区，一定要复AHB缓冲区，否则数据连续读取出错！
    FLEXSPI_SoftwareReset(FLEXSPI);     
    for(index=0;index<NumByteToRead;index++)
    {
        pBuffer[index]=*(uint8_t*)(startaddr+index);
    }
}


//无检验写FlexSPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(最大32bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void FlexSPI_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 			 		 
	uint16_t pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数	
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		FlexSPI_FlashPage_Write(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	}   
} 

//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(最大32bit)						
//NumByteToWrite:要写入的字节数(最大65535)   
__align(64) uint8_t FLEXSPI_FLASH_BUFFER[4096];	
void FlexSPI_FlashWrite(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 
	uint32_t secpos=0;
	uint16_t secoff=0;
	uint16_t secremain=0;	   
 	uint16_t i=0;    
	uint8_t *FLEXSPI_BUF;
    
   	FLEXSPI_BUF=FLEXSPI_FLASH_BUFFER;	     
 	secpos=WriteAddr/4096;//扇区地址  
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小

 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
    while(1) 
	{	
		FlexSPI_FlashRead(FLEXSPI_BUF,secpos*4096,4096);//读出整个扇区的内容
		FlexSPI_NorFlash_Erase_Sector(FLEXSPI,secpos*SECTOR_SIZE);
        for(i=0;i<secremain;i++)	                    //复制
        {
            FLEXSPI_BUF[i+secoff]=pBuffer[i];	  
        }
        FlexSPI_Write_NoCheck(FLEXSPI_BUF,secpos*4096,4096);//写入整个扇区  
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
		}	 
	};	 
}


