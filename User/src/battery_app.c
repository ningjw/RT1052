#include "main.h"

#define BAT_CHG_COMPLETE   GPIO_PinWrite(BOARD_PWR_CHG_COMPLETE_GPIO, BOARD_PWR_CHG_COMPLETE_PIN, 1)
#define BAT_CHG_UNCOMPLETE GPIO_PinWrite(BOARD_PWR_CHG_COMPLETE_GPIO, BOARD_PWR_CHG_COMPLETE_PIN, 0)

#define READ_CHARGE_STA    GPIO_PinRead(BOARD_BAT_CHRG_GPIO, BOARD_BAT_CHRG_PIN)
#define READ_STDBY_STA     GPIO_PinRead(BOARD_BAT_STDBY_GPIO,BOARD_BAT_STDBY_PIN)

TaskHandle_t BAT_TaskHandle = NULL;  /* 电池管理任务句柄 */
static uint8_t set_battery_full = false;
/***********************************************************************
  * @ 函数名  ： BAT_AppTask
  * @ 功能说明： 
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void BAT_AppTask(void)
{
    float remain = 0;
   
    // Enable auto measurement of battery voltage and temperature
	LTC2942_SetADCMode(LTC2942_ADC_AUTO);
    
	// Enable analog section of the chip (in case if it disabled)
	LTC2942_SetAnalog(LTC2942_AN_ENABLED);

	// Set prescaler M value
	// M=32 for 2000mAh battery, 
	LTC2942_SetPrescaler(LTC2942_PSCM_32);

	//Charge Mode
	LTC2942_SetALCCMode(LTC2942_ALCC_DISABLED);
    
    g_sys_para2.batVoltage = LTC2942_GetVoltage() / 1000.0;//获取电池电压值
    
    g_sys_para2.batChargePercent = LTC2942_GetAC() * 100.0 / 65535;//获取电池剩余容量
    
    //根据电压值计算电池容量
    if(g_sys_para2.batVoltage >= 3.73f){//(3.73 - 4.2)
        remain = -308.19f * g_sys_para2.batVoltage * g_sys_para2.batVoltage + 2607.7f * g_sys_para2.batVoltage - 5417.9f;
    }else if(g_sys_para2.batVoltage >= 3.68f){//(3.68 - 3.73)
        remain = -1666.7f * g_sys_para2.batVoltage*g_sys_para2.batVoltage + 12550 * g_sys_para2.batVoltage - 23603;
    }else{// (3.5 - 3.68)
        remain = 55.556f * g_sys_para2.batVoltage - 194.44f;
    }
    
    //偏差%5,重新设定电池容量
    if(abs((int)(remain - g_sys_para2.batVoltage)) / remain * 100 >= 5.0f){
        LTC2942_SetAC(remain / 100 * 0xFFFF);
    }
    
    PRINTF("Battery Task Create and Running\r\n");
    
    while(1)
    {
        // Battery voltage
		g_sys_para2.batVoltage = LTC2942_GetVoltage() / 1000.0;
        
		// Chip temperature
		g_sys_para2.batTemp = LTC2942_GetTemperature() / 100.0;
        
		// Accumulated charge
        g_sys_para2.batChargePercent = LTC2942_GetAC() * 100.0 / 65535;
        
        if(READ_CHARGE_STA == 0 && READ_STDBY_STA == 1){//充电中
            set_battery_full = false;
            g_sys_para2.batLedStatus = BAT_CHARGING;
            BAT_CHG_UNCOMPLETE;
        }else if(READ_CHARGE_STA == 1 && READ_STDBY_STA == 0){//充电完成
            g_sys_para2.batLedStatus = BAT_FULL;
            if(set_battery_full == false){
                set_battery_full = true;
                LTC2942_SetAC(0xFFFF);
//                LTC2942_SetALCCMode(LTC2942_ALCC_CHG);
//                BAT_CHG_COMPLETE;
//                vTaskDelay(1);
//                LTC2942_SetALCCMode(LTC2942_ALCC_DISABLED);
            }
        }else if(g_sys_para2.batChargePercent <= g_sys_para1.batAlarmValue){//低于报警值
            set_battery_full = false;
            g_sys_para2.batLedStatus = BAT_ALARM;
        }else if(g_sys_para2.batChargePercent <= 20){//电池电量小于百分之20
            set_battery_full = false;
            g_sys_para2.batLedStatus = BAT_LOW20;
        }else{
            set_battery_full = false;
            g_sys_para2.batLedStatus = BAT_NORMAL;
        }
        vTaskDelay(2000);
    }
}



