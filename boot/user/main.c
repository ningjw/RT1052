#include "fsl_debug_console.h"

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "./norflash/bsp_norflash.h"

#define FIRM_APP_ADDR     0x60010000		//	APP代码起始地址

#define APP_INFO_SECTOR    63 /* 升级信息保存在NorFlash的第63个扇区*/
#define APP_START_SECTOR   64 /* App数据从第64个扇区开始保存 */
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
		
		PRINTF("Jump to app\n");
		
		/*	将APP的代码从FLASH 中拷贝至SDRAM中运行	*/
//		memcpy((void*)SDRAM_APP_CODE_ADD, (void*)FLASH_APP_CODE_ADD, appSize);
		
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
