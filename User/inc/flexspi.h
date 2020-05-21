#ifndef _FLEXSPI_H
#define _FLEXSPI_H

#define FLEXSPI_FLASH_SIZE      0x2000              //Flash大小为8MB
#define FLASH_PAGE_SIZE         256                 //编程页大小：256字节
//#define SECTOR_SIZE             0x1000              //扇区大小4096
#define FLEXSPI_FLASHID         0X16                //Flash ID

//命令序列
#define NOR_CMD_LUT_SEQ_IDX_READ_NORMAL 0
#define NOR_CMD_LUT_SEQ_IDX_READ_FAST 1
#define NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD 2
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS 3
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE 4
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR 5
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE 6
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD 7
#define NOR_CMD_LUT_SEQ_IDX_READID 8
#define NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG1 9
#define NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG2 10
#define NOR_CMD_LUT_SEQ_IDX_EXITQPI 11
#define NOR_CMD_LUT_SEQ_IDX_READSTATUSREG1 12
#define NOR_CMD_LUT_SEQ_IDX_READMANUID 13
#define NOR_CMD_LUT_SEQ_IDX_READSTATUSREG2 14

//#define CUSTOM_LUT_LENGTH 60

void FlexSPI_FlashInit(void);
void FlexSPI_FlashWrite_Enable(void);
uint8_t FlexSPI_FlashRead_SR(uint8_t regno);
void FlexSPI_FlashWrite_SR(uint8_t regno,uint8_t regdata);
void FlexSPI_FlashWait_Busy(void);
void FlexSPI_QPI_Enable(void);
uint8_t FlexSPI_FlashID_Get(void);
uint8_t FlexSPI_FlashErase_Sector(uint32_t sectorno);
uint8_t FlexSPI_FlashPage_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite);
void FlexSPI_FlashRead(uint8_t* pBuffer,uint32_t ReadAddr,uint32_t NumByteToRead);
void FlexSPI_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite);
void FlexSPI_FlashWrite(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite);
void FlexSPI_Sector_Write(uint8_t* pBuffer,uint32_t setcorno);

#endif
