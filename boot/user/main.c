#include "fsl_debug_console.h"

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "./norflash/bsp_norflash.h"

#define FIRM_APP_ADDR     0x60010000		//	APP������ʼ��ַ

#define APP_INFO_SECTOR    63 /* ������Ϣ������NorFlash�ĵ�63������*/
#define APP_START_SECTOR   64 /* App���ݴӵ�64��������ʼ���� */
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
  * @brief  ��ת��APP����
  *	
  * @param	 
  * @retval 
  */
void jumpToApp(void)
{
	/*	�жϵ�ַ�Ƿ�Ϸ�	*/
	if((FIRM_APP_ADDR&0xFF000000) == 0x60000000){
		
		PRINTF("Jump to app\n");
		
		/*	��APP�Ĵ����FLASH �п�����SDRAM������	*/
//		memcpy((void*)SDRAM_APP_CODE_ADD, (void*)FLASH_APP_CODE_ADD, appSize);
		
		//�����ж�������
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
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{
    /* ��ʼ���ڴ汣����Ԫ */
    BOARD_ConfigMPU();
    /* ��ʼ������������ */
    BOARD_InitPins();
    /* ��ʼ��������ʱ�� */
    BOARD_BootClockRUN();
    /* ��ʼ�����Դ��� */
    BOARD_InitDebugConsole();
    
    /* ��ʼ��FlexSPI���� */
    FlexSPI_NorFlash_Init();
    
    /* �ж��Ƿ����µĹ̼���Ҫ���� */
    
    while(1)
    {
        jumpToApp();
    }
}


/****************************END OF FILE**********************/
