#include "main.h"

#define BAT_CHG_COMPLETE   GPIO_PinWrite(BOARD_PWR_CHG_COMPLETE_GPIO, BOARD_PWR_CHG_COMPLETE_PIN, 1)
#define BAT_CHG_UNCOMPLETE GPIO_PinWrite(BOARD_PWR_CHG_COMPLETE_GPIO, BOARD_PWR_CHG_COMPLETE_PIN, 0)

#define READ_CHARGE_STA    GPIO_PinRead(BOARD_BAT_CHRG_GPIO, BOARD_BAT_CHRG_PIN)
#define READ_STDBY_STA     GPIO_PinRead(BOARD_BAT_STDBY_GPIO,BOARD_BAT_STDBY_PIN)

TaskHandle_t BAT_TaskHandle = NULL;  /* ��ع��������� */

float remain;
/***********************************************************************
  * @ ������  �� BAT_AppTask
  * @ ����˵����
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void BAT_AppTask(void)
{
    // Enable auto measurement of battery voltage and temperature
    LTC2942_SetADCMode(LTC2942_ADC_AUTO);
	
    // Enable analog section of the chip (in case if it disabled)
    LTC2942_SetAnalog(LTC2942_AN_ENABLED);

    // Set prescaler M value
    // M=128 for 2000mAh battery,
    LTC2942_SetPrescaler(LTC2942_PSCM_64);

	LTC2942_SetALCCMode(LTC2942_ALCC_DISABLED);
	
    PRINTF("Battery Task Create and Running\r\n");

    while(1)
    {
        // ��ȡ��ص�ѹ
        g_sys_para.batVoltage = LTC2942_GetVoltage() / 1000.0;

        // ��ȡ�¶ȴ������¶�
//        g_sys_para.batTemp = LTC2942_GetTemperature() / 100.0;

        // ��ȡ�����ٷֱ�
        g_sys_para.batRemainPercent = LTC2942_GetAC() * 100.0 / 0xFFFF;
		
        if(READ_CHARGE_STA == 0 && READ_STDBY_STA == 1) {//��統��
            g_sys_para.batLedStatus = BAT_CHARGING;
            BAT_CHG_UNCOMPLETE;
			if (g_sys_para.batRemainPercent == 100){//��統�м�⵽����Ϊ100%, ��Ϊ99%
				g_sys_para.batRemainPercent = 99;
			}
        } else if(READ_CHARGE_STA == 1 && READ_STDBY_STA == 0) { //������
            g_sys_para.batLedStatus = BAT_FULL;
            LTC2942_SetAC(0xFFFF);
        } else if(g_sys_para.batRemainPercent <= g_sys_para.batAlarmValue) { //�������ڱ���ֵ
            g_sys_para.batLedStatus = BAT_ALARM;
			if(g_sys_para.batRemainPercent == 0){//�ŵ統��,����Ϊ0,�ֶ���Ϊ1
				g_sys_para.batRemainPercent = 1;
			}
        } else if(g_sys_para.batRemainPercent <= 20) { //��������20%
            g_sys_para.batLedStatus = BAT_LOW20;
			if(g_sys_para.batRemainPercent == 0){//�ŵ統��,����Ϊ0,�ֶ���Ϊ1
				g_sys_para.batRemainPercent = 1;
			}
        } else {
            g_sys_para.batLedStatus = BAT_NORMAL;
        }
		
        vTaskDelay(2000);
    }
}



