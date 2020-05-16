#include "fsl_debug_console.h"

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "./norflash/bsp_norflash.h"

#define FIRM_APP_ADDR     0x60010000		//	APP代码起始地址
#define FIRM_APP_SECTOR    16
#define APP_INFO_SECTOR    128 
#define APP_START_SECTOR   129 
/*******************************************************************
 * Prototypes
 *******************************************************************/

volatile bool test = false;
uint32_t appSize = 0x32000;
typedef void (*iapFun)(void);
iapFun appMain; 
__asm void MSR_MSP(uint32_t address)
{
	MSR MSP, r0
	BX r14
}
__align(64) uint8_t s_nor_read_buffer[SECTOR_SIZE];
//该结构体定义了需要保存到EEPROM中的参数
typedef struct{
    uint8_t  firmUpdate;    //固件更新
    uint32_t firmSizeTotal; //固件总大小
    uint32_t firmCrc16;     //固件校验码
    uint32_t firmPacksTotal;//固件总包数
}UpdatePara_t;
UpdatePara_t UpdatePara;
/**
  * @brief  跳转至APP运行
  *	
  * @param	 
  * @retval 
  */
void jumpToApp(void)
{
	/*	判断地址是否合法	*/
	if((FIRM_APP_ADDR&0xFF000000) == 0x60000000){
		//读取升级参数
		memcpy(&UpdatePara.firmUpdate, NORFLASH_AHB_POINTER(APP_INFO_SECTOR * SECTOR_SIZE), 16);
		if (UpdatePara.firmUpdate == true){//需要更新系统
			
			for(int i=0; i<=UpdatePara.firmSizeTotal/SECTOR_SIZE; i++){
				memcpy(s_nor_read_buffer,NORFLASH_AHB_POINTER((APP_START_SECTOR+i) * SECTOR_SIZE), SECTOR_SIZE);
				
				FlexSPI_FlashWrite(s_nor_read_buffer,(FIRM_APP_SECTOR+i)*SECTOR_SIZE ,SECTOR_SIZE);
			}
			
			/*最新的app复制到运行处	*/
//			memcpy((void*)FIRM_APP_ADDR, (uint8_t *)(FlexSPI_AMBA_BASE + APP_START_SECTOR * SECTOR_SIZE), UpdatePara.firmSizeTotal);
		
			//将标识位写入flash
//			UpdatePara.firmUpdate = false;
//			FlexSPI_NorFlash_Erase_Sector(FLEXSPI, APP_INFO_SECTOR * SECTOR_SIZE);
//			FlexSPI_NorFlash_Buffer_Program(FLEXSPI, APP_INFO_SECTOR * SECTOR_SIZE, &UpdatePara.firmUpdate, 13);
		}
		
		PRINTF("Jump to app\n");
		//设置中断向量表
		SCB->VTOR = FIRM_APP_ADDR;
        
		appMain = (iapFun)*(volatile uint32_t*)(FIRM_APP_ADDR+4);
		MSR_MSP(*(volatile uint32_t*)FIRM_APP_ADDR);
		appMain();
	}
	else{
		PRINTF("jump add err\n");
	}
}

/*******************************************************************
 * Code
 *******************************************************************/

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
    /* 初始化内存保护单元 */
    BOARD_ConfigMPU();
    /* 初始化开发板引脚 */
    BOARD_InitPins();
    /* 初始化开发板时钟 */
    BOARD_BootClockRUN();
    /* 初始化调试串口 */
    BOARD_InitDebugConsole();
    
    /* 初始化FlexSPI外设 */
    FlexSPI_NorFlash_Init();
    
    /* 判断是否有新的固件需要更新 */
    
    while(1)
    {
        jumpToApp();
    }
}



/****************************END OF FILE**********************/
