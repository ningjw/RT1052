#include "main.h"

TaskHandle_t AppBLE_TaskHandle = NULL;  /* ���������� */
TaskHandle_t AppADC_TaskHandle = NULL;  /* ADC������ */
TaskHandle_t AppEMMC_TaskHandle = NULL; /* eMMC������ */




/***********************************************************************
  * @ ������  �� AppBLE_Task
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void AppBLE_Task(void)
{
    
    while(1)
    {
        /* ��ȡ���� */
        SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
        /* ��ӡ����&ʱ�� */ 
        PRINTF("BLE TASK:%02d-%02d-%02d  %02d:%02d:%02d \r\n", rtcDate.year,rtcDate.month, rtcDate.day,rtcDate.hour, rtcDate.minute, rtcDate.second);
        vTaskDelay(1000);
    }
}

/***********************************************************************
  * @ ������  �� AppADC_Task
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void AppADC_Task(void)
{
    while(1)
    {
        PRINTF("ADC TASK\r\n");
        vTaskDelay(1000);
    }
}

/***********************************************************************
  * @ ������  �� AppADC_Task
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void AppEMMC_Task(void)
{
    while(1)
    {
        PRINTF("eMMC TASK\r\n");
        vTaskDelay(1000);
    }
}



