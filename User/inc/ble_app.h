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
    uint16_t resp_time100ms;  //���ͺ��ѯ������Ϣ����ʱ��100msΪ��λ������Ϊָ�������Ӧʱ�䡣
    uint8_t  try_delay1ms;    //����ʧ�ܺ��ٴη���ʱ����ʱ��1msΪ��λ
    uint8_t  max_try_times;   //������Դ���
    uint8_t  max_reset_times; //�����������
}stcATConfig;


typedef void (*SendMsgFunc_t)(uint8_t * buf, uint32_t len);





extern TaskHandle_t AppBLE_TaskHandle ;  /* ���������� */


void AppBLE_Task(void);



#endif
