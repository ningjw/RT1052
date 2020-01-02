#include "fsl_debug_console.h"

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "./norflash/bsp_norflash.h"


#define FLASH_APP_CODE_ADD							0x60020000		//	APP������ʼ��ַ
#define SDRAM_APP_CODE_ADD							0x80010000		//	sdram APP������ʼ��ַ

/*******************************************************************
 * Prototypes
 *******************************************************************/

extern int NorFlash_AHBCommand_Test(void);
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
	if((SDRAM_APP_CODE_ADD&0xFF000000) == 0x80000000){
		
		PRINTF("Jump to app\n");
		
		/*	��APP�Ĵ����FLASH �п�����SDRAM������	*/
		memcpy((void*)SDRAM_APP_CODE_ADD, (void*)FLASH_APP_CODE_ADD, appSize);
		
		//�����ж�������
		SCB->VTOR = SDRAM_APP_CODE_ADD;	
		appMain = (iapFun)*(volatile uint32_t*)(SDRAM_APP_CODE_ADD+4);
		MSR_MSP(*(volatile uint32_t*)SDRAM_APP_CODE_ADD);
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
    
    while(1)
    {
        jumpToApp();
    }
}


/****************************END OF FILE**********************/
