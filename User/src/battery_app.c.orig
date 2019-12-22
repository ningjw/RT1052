#include "main.h"


TaskHandle_t BAT_TaskHandle = NULL;  /* 电池管理任务句柄 */

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
uint8_t getChargeSta(void)
{
    if (GPIO_PinRead(BOARD_CHARGE_COMPELETE_GPIO, BOARD_CHARGE_COMPELETE_PIN) == 0 ) {//充电完成
        return 0;
    } else if( GPIO_PinRead(BOARD_CHARGING_GPIO, BOARD_CHARGING_PIN) == 0 ){//充电中
        return 1;
    }else{//未检测到电池
        return 0xFF;
    }
}

/***********************************************************************
  * @ 函数名  ： BAT_AppTask
  * @ 功能说明： 
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void BAT_AppTask(void)
{
    uint32_t i,j,k;
    // Enable auto measurement of battery voltage and temperature
	LTC2942_SetADCMode(LTC2942_ADC_AUTO);

	// Enable analog section of the chip (in case if it disabled)
	LTC2942_SetAnalog(LTC2942_AN_ENABLED);

	// Set prescaler M value
	// M=16 for 600mAh battery, the 1LSB of AC value is 0,010625mAh
	LTC2942_SetPrescaler(LTC2942_PSCM_16);

	// Disable AL/CC pin
	LTC2942_SetALCCMode(LTC2942_ALCC_DISABLED);
//    
    PRINTF("Battery Task Create and Running\r\n");
    
    while(1)
    {
        // Battery voltage
		i = LTC2942_GetVoltage();

		// Chip temperature
		j = LTC2942_GetTemperature();
        
		// Accumulated charge
		k = LTC2942_GetAC();
//        
//		// Dump values to log
//		PRINTF("Vbat: %.3uV  Temp: %.2iC  Charge: %.2u%% ~%umAh [0x%04X %u]\r\n",
//				i, /* Battery voltage */
//				j, /* Chip temperature */
//				(k * 10000) / 65535, /* Accumulated charge in percent */
//				(k * 85 * 16) / 128000, /* Very rough current charge capacity (16 - prescaler M value) */
//				k, /* Raw accumulated charge (HEX) */
//				k  /* Raw accumulated charge (DEC) */
//			);
        
        vTaskDelay(10000);
    }
}







