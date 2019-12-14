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

typedef void (*SendMsgFunc_t)(uint8_t * buf, uint32_t len);

typedef struct
{
    uint16_t resp_time;       //���ͺ��ѯ������Ϣ����ʱ��msΪ��λ������Ϊָ�������Ӧʱ�䡣
    uint8_t  try_times;       //���Դ���
    uint8_t  try_cnt;         //���ڼ�¼��ǰ���Դ���
}ATCfg_t;


extern TaskHandle_t BLE_TaskHandle ;  /* ���������� */

void BLE_AppTask(void);



#endif
