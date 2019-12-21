#include "main.h"

#define ON  0
#define OFF 1

static uint32_t sys_led_cnt = 0;
static uint32_t bat_led_cnt = 0;
static uint32_t ble_led_cnt = 0;

TaskHandle_t LED_TaskHandle = NULL;  /* ��ع��������� */

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void LED_AppTask(void)
{
    PRINTF("LED Task Create and Running\r\n");
    GPIO_PinWrite(BOARD_LED_PWR_RED_GPIO, BOARD_LED_PWR_RED_PIN, OFF);
    GPIO_PinWrite(BOARD_LED_PWR_GREEN_GPIO, BOARD_LED_PWR_GREEN_PIN, ON);
    while(1)
    {
        //ϵͳ״ָ̬ʾ��
        switch(g_sys_para2.sampLedStatus)
        {
            case WORK_FINE://�����,�̵���
                GPIO_PinWrite(BOARD_LED_SYS_RED_GPIO,  BOARD_LED_SYS_RED_PIN, OFF);
                GPIO_PinWrite(BOARD_LED_SYS_GREEN_GPIO, BOARD_LED_SYS_GREEN_PIN, ON);
                break;
            case WORK_ERR://�̵���,���0.6����˸
                GPIO_PinWrite(BOARD_LED_SYS_GREEN_GPIO, BOARD_LED_SYS_GREEN_PIN, OFF);
                if(sys_led_cnt++ % 3 == 0){
                    BOARD_LED_SYS_RED_GPIO->DR ^= (1 << BOARD_LED_SYS_RED_PIN);
                }
                break;
            case WORK_FATAL_ERR://�̵���,���0.2����˸
                GPIO_PinWrite(BOARD_LED_SYS_GREEN_GPIO, BOARD_LED_SYS_GREEN_PIN, OFF);
                BOARD_LED_SYS_RED_GPIO->DR ^= (1 << BOARD_LED_SYS_RED_PIN);
                break;
            default:
                break;
        }
        
        //���״ָ̬ʾ��
        switch(g_sys_para2.batLedStatus)
        {
            case BAT_FULL://�����,�̵���
                GPIO_PinWrite(BOARD_LED_BAT_RED_GPIO,  BOARD_LED_BAT_RED_PIN, OFF);
                GPIO_PinWrite(BOARD_LED_BAT_GREEN_GPIO,BOARD_LED_BAT_GREEN_PIN, ON);
                break;
            case BAT_CHARGING://�����,�̵���
                GPIO_PinWrite(BOARD_LED_BAT_RED_GPIO,   BOARD_LED_BAT_RED_PIN,  ON);
                GPIO_PinWrite(BOARD_LED_BAT_GREEN_GPIO, BOARD_LED_BAT_GREEN_PIN,OFF);
                break;
            case BAT_LOW://�̵���,���0.6����˸
                GPIO_PinWrite(BOARD_LED_BAT_GREEN_GPIO, BOARD_LED_BAT_GREEN_PIN,OFF);
                if(bat_led_cnt++ % 3 == 0){
                    BOARD_LED_BAT_RED_GPIO->DR ^= (1 << BOARD_LED_BAT_RED_PIN);
                }
                break;
            case BAT_ALARM://�̵���,���0.2����˸
                GPIO_PinWrite(BOARD_LED_BAT_GREEN_GPIO, BOARD_LED_BAT_GREEN_PIN,OFF);
                BOARD_LED_BAT_RED_GPIO->DR ^= (1 << BOARD_LED_BAT_RED_PIN);
                break;
            default:
                break;
        }
        
        //����״ָ̬ʾ��
        switch(g_sys_para2.bleLedStatus)
        {
            case BLE_CLOSE://�����,�̵���
                GPIO_PinWrite(BOARD_LED_BLE_RED_GPIO,  BOARD_LED_BLE_RED_PIN, OFF);
                GPIO_PinWrite(BOARD_LED_BLE_GREEN_GPIO,BOARD_LED_BLE_GREEN_PIN,OFF);
                break;
            case BLE_READY://�����,�̵�0.6����
                GPIO_PinWrite(BOARD_LED_BLE_RED_GPIO,  BOARD_LED_BLE_RED_PIN, OFF);
                if(ble_led_cnt++ % 3 == 0){
                    BOARD_LED_BLE_GREEN_GPIO->DR ^= (1<<BOARD_LED_BLE_GREEN_PIN);
                }
                 break;
            case BLE_CONNECT:  //�����,�̵���
                GPIO_PinWrite(BOARD_LED_BLE_RED_GPIO,  BOARD_LED_BLE_RED_PIN, OFF);
                GPIO_PinWrite(BOARD_LED_BLE_GREEN_GPIO,BOARD_LED_BLE_GREEN_PIN,ON);
                 break;
            case BLE_UPDATE://���̵ƽ�����˸
                if(ble_led_cnt++ % 2 == 0){
                    GPIO_PinWrite(BOARD_LED_BLE_RED_GPIO,  BOARD_LED_BLE_RED_PIN, OFF);
                    GPIO_PinWrite(BOARD_LED_BLE_GREEN_GPIO,BOARD_LED_BLE_GREEN_PIN,ON);
                }else{
                    GPIO_PinWrite(BOARD_LED_BLE_RED_GPIO,  BOARD_LED_BLE_RED_PIN, ON);
                    GPIO_PinWrite(BOARD_LED_BLE_GREEN_GPIO,BOARD_LED_BLE_GREEN_PIN,OFF);
                }
                 break;
            default:
                break;
        }
        
        vTaskDelay(200);
    }
}


