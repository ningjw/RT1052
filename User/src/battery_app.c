#include "main.h"

#define BAT_CHG_COMPLETE   GPIO_PinWrite(BOARD_PWR_CHG_COMPLETE_GPIO, BOARD_PWR_CHG_COMPLETE_PIN, 1)
#define BAT_CHG_UNCOMPLETE GPIO_PinWrite(BOARD_PWR_CHG_COMPLETE_GPIO, BOARD_PWR_CHG_COMPLETE_PIN, 0)

#define READ_CHARGE_STA    GPIO_PinRead(BOARD_BAT_CHRG_GPIO, BOARD_BAT_CHRG_PIN)
#define READ_STDBY_STA     GPIO_PinRead(BOARD_BAT_STDBY_GPIO,BOARD_BAT_STDBY_PIN)

TaskHandle_t BAT_TaskHandle = NULL;  /* 电池管理任务句柄 */
static uint8_t set_battery_full = false;
float remain;
/***********************************************************************
  * @ 函数名  ： BAT_AppTask
  * @ 功能说明：
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void BAT_AppTask(void)
{
    // Enable auto measurement of battery voltage and temperature
    LTC2942_SetADCMode(LTC2942_ADC_AUTO);
	
    // Enable analog section of the chip (in case if it disabled)
    LTC2942_SetAnalog(LTC2942_AN_ENABLED);

    // Set prescaler M value
    // M=64 for 2000mAh battery,
    LTC2942_SetPrescaler(LTC2942_PSCM_128);

    //Charge Mode
    LTC2942_SetALCCMode(LTC2942_ALCC_CHG);

    PRINTF("Battery Task Create and Running\r\n");

    while(1)
    {
        // Battery voltage
        g_sys_para.batVoltage = LTC2942_GetVoltage() / 1000.0;

        // Chip temperature
        g_sys_para.batTemp = LTC2942_GetTemperature() / 100.0;

        // Accumulated charge
        g_sys_para.batRemainPercent = LTC2942_GetAC() * 100.0 / 65535;
        
        //battery is in charging
        if(READ_CHARGE_STA == 0 && READ_STDBY_STA == 1) {//充电当中
            set_battery_full = false;
            g_sys_para.batLedStatus = BAT_CHARGING;
            BAT_CHG_UNCOMPLETE;
        } else if(READ_CHARGE_STA == 1 && READ_STDBY_STA == 0) { //充电完成
            g_sys_para.batLedStatus = BAT_FULL;
            if(set_battery_full == false) {
                set_battery_full = true;
                LTC2942_SetAC(0xFFFF);
            }
        } else if(g_sys_para.batRemainPercent <= g_sys_para.batAlarmValue) { //电量低于报警值
            g_sys_para.batLedStatus = BAT_ALARM;
            set_battery_full = false;
        } else if(g_sys_para.batRemainPercent <= 20) { //电量低于20%
            set_battery_full = false;
            g_sys_para.batLedStatus = BAT_LOW20;
        } else {
            set_battery_full = false;
            g_sys_para.batLedStatus = BAT_NORMAL;
        }
        vTaskDelay(2000);
    }
}



