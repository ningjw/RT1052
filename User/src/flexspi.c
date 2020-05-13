#include "main.h"

flexspi_config_t flexspi_norflash_config;   //FlexSPI Nor Flash���ýṹ��

__align(64) uint8_t s_nor_program_buffer[256];
__align(64) uint8_t s_nor_read_buffer[256];


//ҳ��̣���ָ����ַ��ʼд�����256�ֽڵ����ݣ�ҳ��СΪ256B
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(���32bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!
//����ֵ��0 �ɹ�������ֵ ʧ�� 
uint8_t FlexSPI_FlashPage_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint8_t ret=0;
    flexspi_transfer_t FlashTransfer;
    
    FlexSPI_NorFlash_Write_Enable(FLEXSPI);                  //дʹ��
    FlexSPI_NorFlash_Wait_Bus_Busy(FLEXSPI);                               //�ȴ�Flash����
    
    FlashTransfer.deviceAddress=WriteAddr;                  //��ַ
    FlashTransfer.port=kFLEXSPI_PortA1;                     //�˿�
    FlashTransfer.cmdType=kFLEXSPI_Write;                   //�������ͣ�д����
    FlashTransfer.SeqNumber=1;                              //���
    FlashTransfer.seqIndex=NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD;    //LUT������������
    FlashTransfer.data=(uint32_t *)pBuffer;                      //Ҫд�������
    FlashTransfer.dataSize=NumByteToWrite;                  //���ݳ���
	
    if(FLEXSPI_TransferBlocking(FLEXSPI,&FlashTransfer)!=kStatus_Success) ret=1;

    FlexSPI_NorFlash_Wait_Bus_Busy(FLEXSPI);           //�ȴ�Flash����
	/* �ȴ�д����� */
    FlexSPI_NorFlash_Wait_Bus_Busy(FLEXSPI);
    return ret;  
}

//��FlexSPI Flash�ж�ȡ����
//pBuffer:���ݴ洢��
//ReadAddr:��ʼ��ȡ�ĵ�ַ(���32bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void FlexSPI_FlashRead(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)  
{
    uint32_t index=0;
    uint32_t startaddr=0;
    
    startaddr=FlexSPI_AMBA_BASE+ReadAddr;

    //��λAHB��������һ��Ҫ��AHB����������������������ȡ����
    FLEXSPI_SoftwareReset(FLEXSPI);     
    for(index=0;index<NumByteToRead;index++)
    {
        pBuffer[index]=*(uint8_t*)(startaddr+index);
    }
}


//�޼���дFlexSPI FLASH 
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(���32bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void FlexSPI_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 			 		 
	uint16_t pageremain;	   
	pageremain=256-WriteAddr%256; //��ҳʣ����ֽ���	
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//������256���ֽ�
	while(1)
	{	   
		FlexSPI_FlashPage_Write(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//д�������
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
			if(NumByteToWrite>256)pageremain=256; //һ�ο���д��256���ֽ�
			else pageremain=NumByteToWrite; 	  //����256���ֽ���
		}
	}   
} 

//дSPI FLASH  
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(���32bit)						
//NumByteToWrite:Ҫд����ֽ���(���65535)   
__align(64) uint8_t FLEXSPI_FLASH_BUFFER[4096];	
void FlexSPI_FlashWrite(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 
	uint32_t secpos=0;
	uint16_t secoff=0;
	uint16_t secremain=0;	   
 	uint16_t i=0;    
	uint8_t *FLEXSPI_BUF;
    
   	FLEXSPI_BUF=FLEXSPI_FLASH_BUFFER;	     
 	secpos=WriteAddr/4096;//������ַ  
	secoff=WriteAddr%4096;//�������ڵ�ƫ��
	secremain=4096-secoff;//����ʣ��ռ��С

 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//������4096���ֽ�
    while(1) 
	{	
		FlexSPI_FlashRead(FLEXSPI_BUF,secpos*4096,4096);//������������������
		FlexSPI_NorFlash_Erase_Sector(FLEXSPI,secpos*SECTOR_SIZE);
        for(i=0;i<secremain;i++)	                    //����
        {
            FLEXSPI_BUF[i+secoff]=pBuffer[i];	  
        }
        FlexSPI_Write_NoCheck(FLEXSPI_BUF,secpos*4096,4096);//д����������  
		if(NumByteToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0 	 

		   	pBuffer+=secremain;  //ָ��ƫ��
			WriteAddr+=secremain;//д��ַƫ��	   
		   	NumByteToWrite-=secremain;				//�ֽ����ݼ�
			if(NumByteToWrite>4096)secremain=4096;	//��һ����������д����
			else secremain=NumByteToWrite;			//��һ����������д����
		}	 
	};	 
}


