#include "main.h"

#define ON  0
#define OFF 1

static uint32_t sys_led_cnt = 0;
static uint32_t bat_led_cnt = 0;
static uint32_t ble_led_cnt = 0;

TaskHandle_t LED_TaskHandle = NULL;  /* µç³Ø¹ÜÀíÈÎÎñ¾ä±ú */

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
        //ÏµÍ³×´Ì¬Ö¸Ê¾µÆ
        switch(g_sys_para2.sampLedStatus)
        {
            case WORK_FINE://ºìµÆÃð,ÂÌµÆÁÁ
                GPIO_PinWrite(BOARD_LED_SYS_RED_GPIO,  BOARD_LED_SYS_RED_PIN, OFF);
                GPIO_PinWrite(BOARD_LED_SYS_GREEN_GPIO, BOARD_LED_SYS_GREEN_PIN, ON);
                break;
            case WORK_ERR://ÂÌµÆÃð,ºìµÆ0.6ÃëÉÁË¸
                GPIO_PinWrite(BOARD_LED_SYS_GREEN_GPIO, BOARD_LED_SYS_GREEN_PIN, OFF);
                if(sys_led_cnt++ % 3 == 0){
                    BOARD_LED_SYS_RED_GPIO->DR ^= (1 << BOARD_LED_SYS_RED_PIN);
                }
                break;
            case WORK_FATAL_ERR://ÂÌµÆÃð,ºìµÆ0.2ÃëÉÁË¸
                GPIO_PinWrite(BOARD_LED_SYS_GREEN_GPIO, BOARD_LED_SYS_GREEN_PIN, OFF);
                BOARD_LED_SYS_RED_GPIO->DR ^= (1 << BOARD_LED_SYS_RED_PIN);
                break;
            default:
                break;
        }
        
        //µç³Ø×´Ì¬Ö¸Ê¾µÆ
        switch(g_sys_para2.batLedStatus)
        {
            case BAT_FULL://ºìµÆÃð,ÂÌµÆÁÁ
                GPIO_PinWrite(BOARD_LED_BAT_RED_GPIO,  BOARD_LED_BAT_RED_PIN, OFF);
                GPIO_PinWrite(BOARD_LED_BAT_GREEN_GPIO,BOARD_LED_BAT_GREEN_PIN, ON);
                break;
            case BAT_CHARGING://ºìµÆÁÁ,ÂÌµÆÃð
                GPIO_PinWrite(BOARD_LED_BAT_RED_GPIO,   BOARD_LED_BAT_RED_PIN,  ON);
                GPIO_PinWrite(BOARD_LED_BAT_GREEN_GPIO, BOARD_LED_BAT_GREEN_PIN,OFF);
                break;
            case BAT_LOW://ÂÌµÆÃð,ºìµÆ0.6ÃëÉÁË¸
                GPIO_PinWrite(BOARD_LED_BAT_GREEN_GPIO, BOARD_LED_BAT_GREEN_PIN,OFF);
                if(bat_led_cnt++ % 3 == 0){
                    BOARD_LED_BAT_RED_GPIO->DR ^= (1 << BOARD_LED_BAT_RED_PIN);
                }
                break;
            case BAT_ALARM://ÂÌµÆÃð,ºìµÆ0.2ÃëÉÁË¸
                GPIO_PinWrite(BOARD_LED_BAT_GREEN_GPIO, BOARD_LED_BAT_GREEN_PIN,OFF);
                BOARD_LED_BAT_RED_GPIO->DR ^= (1 << BOARD_LED_BAT_RED_PIN);
                break;
            default:
                break;
        }
        
        //À¶ÑÀ×´Ì¬Ö¸Ê¾µÆ
        switch(g_sys_para2.bleLedStatus)
        {
            case BLE_CLOSE://ºìµÆÃð,ÂÌµÆÃð
                GPIO_PinWrite(BOARD_LED_BLE_RED_GPIO,  BOARD_LED_BLE_RED_PIN, OFF);
                GPIO_PinWrite(BOARD_LED_BLE_GREEN_GPIO,BOARD_LED_BLE_GREEN_PIN,OFF);
                break;
            case BLE_READY://ºìµÆÃð,ÂÌµÆ0.6ÃëÉÁ
                GPIO_PinWrite(BOARD_LED_BLE_RED_GPIO,  BOARD_LED_BLE_RED_PIN, OFF);
                if(ble_led_cnt++ % 3 == 0){
                    BOARD_LED_BLE_GREEN_GPIO->DR ^= (1<<BOARD_LED_BLE_GREEN_PIN);
                }
                 break;
            case BLE_CONNECT:  //ºìµÆÃð,ÂÌµÆÁÁ
                GPIO_PinWrite(BOARD_LED_BLE_RED_GPIO,  BOARD_LED_BLE_RED_PIN, OFF);
                GPIO_PinWrite(BOARD_LED_BLE_GREEN_GPIO,BOARD_LED_BLE_GREEN_PIN,ON);
                 break;
            case BLE_UPDATE://ºìÂÌµÆ½»ÌæÉÁË¸
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


