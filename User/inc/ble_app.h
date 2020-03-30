#ifndef __BLE_APP_H
#define __BLE_APP_H



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




#define LPUART2_BUFF_LEN 250

typedef void (*SendMsgFunc_t)(uint8_t * buf, uint32_t len);

typedef struct
{
    uint16_t resp_time;       //发送后查询返回信息的延时，ms为单位。可设为指令最大响应时间。
    uint8_t  try_times;       //重试次数
    uint8_t  try_cnt;         //用于记录当前重试次数
}ATCfg_t;

extern uint8_t g_puart2TxCnt;
extern uint8_t g_puart2RxCnt;
extern TaskHandle_t BLE_TaskHandle ;  /* 蓝牙任务句柄 */
extern uint8_t g_lpuart2TxBuf[];
extern uint8_t g_puart2StartRx;
extern uint32_t  g_puart2RxTimeCnt;

void BLE_AppTask(void);
void LPUART2_SendString(const char *str);


#endif
