#ifndef WIFI_APP_H
#define WIFI_APP_H


extern TaskHandle_t WIFI_TaskHandle ;  /* ���������� */

void LPUART3_SendString(const char *str);
void WIFI_AppTask(void);


#endif

