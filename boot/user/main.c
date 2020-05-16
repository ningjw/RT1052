#include "fsl_debug_console.h"

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "./norflash/bsp_norflash.h"

#define FIRM_APP_ADDR     0x60010000		//	APP������ʼ��ַ
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
//�ýṹ�嶨������Ҫ���浽EEPROM�еĲ���
typedef struct{
    uint8_t  firmUpdate;    //�̼�����
    uint32_t firmSizeTotal; //�̼��ܴ�С
    uint32_t firmCrc16;     //�̼�У����
    uint32_t firmPacksTotal;//�̼��ܰ���
}UpdatePara_t;
UpdatePara_t UpdatePara;
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
		//��ȡ��������
		memcpy(&UpdatePara.firmUpdate, NORFLASH_AHB_POINTER(APP_INFO_SECTOR * SECTOR_SIZE), 16);
		if (UpdatePara.firmUpdate == true){//��Ҫ����ϵͳ
			
			for(int i=0; i<=UpdatePara.firmSizeTotal/SECTOR_SIZE; i++){
				memcpy(s_nor_read_buffer,NORFLASH_AHB_POINTER((APP_START_SECTOR+i) * SECTOR_SIZE), SECTOR_SIZE);
				
				FlexSPI_FlashWrite(s_nor_read_buffer,(FIRM_APP_SECTOR+i)*SECTOR_SIZE ,SECTOR_SIZE);
			}
			
			/*���µ�app���Ƶ����д�	*/
//			memcpy((void*)FIRM_APP_ADDR, (uint8_t *)(FlexSPI_AMBA_BASE + APP_START_SECTOR * SECTOR_SIZE), UpdatePara.firmSizeTotal);
		
			//����ʶλд��flash
//			UpdatePara.firmUpdate = false;
//			FlexSPI_NorFlash_Erase_Sector(FLEXSPI, APP_INFO_SECTOR * SECTOR_SIZE);
//			FlexSPI_NorFlash_Buffer_Program(FLEXSPI, APP_INFO_SECTOR * SECTOR_SIZE, &UpdatePara.firmUpdate, 13);
		}
		
		PRINTF("Jump to app\n");
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
