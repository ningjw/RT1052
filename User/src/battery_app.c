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
    LTC2942_SetPrescaler(LTC2942_PSCM_64);

    //Charge Mode
    LTC2942_SetALCCMode(LTC2942_ALCC_CHG);

    // Battery voltage
    g_sys_para.batVoltage = LTC2942_GetVoltage() / 1000.0;

    // Accumulated charge
    g_sys_para.batRemainPercent = LTC2942_GetAC() * 100.0 / 65535;

    //根据电压计算电池容量
    if(g_sys_para.batVoltage >= 3.73f) { //(3.73 - 4.2)
        remain = -308.19f * g_sys_para.batVoltage * g_sys_para.batVoltage + 2607.7f * g_sys_para.batVoltage - 5417.9f;
    } else if(g_sys_para.batVoltage >= 3.68f) { //(3.68 - 3.73)
        remain = -1666.7f * g_sys_para.batVoltage * g_sys_para.batVoltage + 12550 * g_sys_para.batVoltage - 23603;
    } else { // (3.5 - 3.68)
        remain = 55.556f * g_sys_para.batVoltage - 194.44f;
    }

    //偏差较大,重新设置电池容量
    if(abs((int)(remain - g_sys_para.batVoltage)) / remain * 100 >= 10.0f) {
        LTC2942_SetAC(remain / 100 * 0xFFFF);
    }

    PRINTF("Battery Task Create and Running\r\n");

    while(1)
    {
        // Battery voltage
        g_sys_para.batVoltage = LTC2942_GetVoltage() / 1000.0;

        // Chip temperature
        g_sys_para.batTemp = LTC2942_GetTemperature() / 100.0;

        //根据电压计算电池容量
        if(g_sys_para.batVoltage >= 3.73f) { //(3.73 - 4.2)
            remain = -308.19f * g_sys_para.batVoltage * g_sys_para.batVoltage + 2607.7f * g_sys_para.batVoltage - 5417.9f;
        } else if(g_sys_para.batVoltage >= 3.68f) { //(3.68 - 3.73)
            remain = -1666.7f * g_sys_para.batVoltage * g_sys_para.batVoltage + 12550 * g_sys_para.batVoltage - 23603;
        } else { // (3.5 - 3.68)
            remain = 55.556f * g_sys_para.batVoltage - 194.44f;
        }

        // Accumulated charge
        g_sys_para.batRemainPercent = remain;
//        g_sys_para.batRemainPercent = LTC2942_GetAC() * 100.0 / 65535;

        //battery is in charging
        if(READ_CHARGE_STA == 0 && READ_STDBY_STA == 1) {
            set_battery_full = false;
            g_sys_para.batLedStatus = BAT_CHARGING;
            BAT_CHG_UNCOMPLETE;
        } else if(READ_CHARGE_STA == 1 && READ_STDBY_STA == 0) { //charge compelete
            g_sys_para.batLedStatus = BAT_FULL;
            if(set_battery_full == false) {
                set_battery_full = true;
                LTC2942_SetAC(0xFFFF);
            }
        } else if(g_sys_para.batRemainPercent <= g_sys_para.batAlarmValue) { //battery is less than alarm value
            g_sys_para.batLedStatus = BAT_ALARM;
            set_battery_full = false;
        } else if(g_sys_para.batRemainPercent <= 20) { //battery is less than 20
            set_battery_full = false;
            g_sys_para.batLedStatus = BAT_LOW20;
        } else if(g_sys_para.batRemainPercent <= 1) {
            GPIO_PinWrite(BOARD_SYS_PWR_OFF_GPIO, BOARD_SYS_PWR_OFF_PIN, 1);
        } else {
            set_battery_full = false;
            g_sys_para.batLedStatus = BAT_NORMAL;
        }
        vTaskDelay(2000);
    }
}



