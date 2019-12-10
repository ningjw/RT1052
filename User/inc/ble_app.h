#ifndef __BLE_APP_H
#define __BLE_APP_H



#define BT_AT       "AT"
#define BT_NAME		"+NAME"
#define BT_PIN		"+PIN"
#define BT_BAUD		"+BAUD"
#define BT_LPM		"+LPM"
#define BT_VER		"+VER"
#define BT_ADDR		"+ADDR"

#define BT_FLOWCTL	"+FLOWCTL"
#define BT_REBOOT	"+REBOOT"
#define BT_RESTORE	"+RESTORE"

#define RESP_OK		"OK"
#define RESP_ERROR	"ERROR"

typedef struct
{
    uint16_t resp_time100ms;  //发送后查询返回信息的延时，100ms为单位。可设为指令最大响应时间。
    uint8_t  try_delay1ms;    //发送失败后再次发送时的延时，1ms为单位
    uint8_t  max_try_times;   //最大重试次数
    uint8_t  max_reset_times; //最大重启次数
}stcATConfig;


typedef void (*SendMsgFunc_t)(uint8_t * buf, uint32_t len);





extern TaskHandle_t AppBLE_TaskHandle ;  /* 蓝牙任务句柄 */


void AppBLE_Task(void);



#endif
