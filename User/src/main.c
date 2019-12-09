#include "main.h"


static TaskHandle_t AppTaskCreate_Handle = NULL;      /* ���������� */
static void AppTaskCreate(void);                      /* ���ڴ������� */


/***********************************************************************
  * @ ������  �� AppTaskCreate
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
static void AppTaskCreate(void)
{
    BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    
    taskENTER_CRITICAL();           //�����ٽ���
    
    /* ����BLE_Task���� */
    xReturn = xTaskCreate((TaskFunction_t )AppBLE_Task,  /* ������ں��� */
                          (const char*    )"BLE_Task",/* �������� */
                          (uint16_t       )512,  /* ����ջ��С */
                          (void*          )NULL, /* ������ں������� */
                          (UBaseType_t    )1,    /* ��������ȼ� */
                          (TaskHandle_t*  )&AppBLE_TaskHandle);/* ������ƿ�ָ�� */
    
    
    /* ����ADC_Task���� */
    xReturn = xTaskCreate((TaskFunction_t )AppADC_Task,  /* ������ں��� */
                          (const char*    )"ADC_Task",/* �������� */
                          (uint16_t       )512,  /* ����ջ��С */
                          (void*          )NULL, /* ������ں������� */
                          (UBaseType_t    )2,    /* ��������ȼ� */
                          (TaskHandle_t*  )&AppADC_TaskHandle);/* ������ƿ�ָ�� */
                          
    /* ����eMMC_Task���� */
    xReturn = xTaskCreate((TaskFunction_t )AppEMMC_Task,  /* ������ں��� */
                          (const char*    )"eMMC_Task",/* �������� */
                          (uint16_t       )512,  /* ����ջ��С */
                          (void*          )NULL, /* ������ں������� */
                          (UBaseType_t    )3,    /* ��������ȼ� */
                          (TaskHandle_t*  )&AppEMMC_TaskHandle);/* ������ƿ�ָ�� */

    if(pdPASS == xReturn) {//���񴴽��ɹ�
        
        }else PRINTF("���񴴽�ʧ��:%d",xReturn);
    vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����

    taskEXIT_CRITICAL();            //�˳��ٽ���
}



int main(void)
{
    BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */

    BOARD_BootClockRUN();
    BOARD_InitBootPins();
    BOARD_InitPeripherals();

    BOARD_InitDebugConsole();
    PRINTF("***** Welcome *****\r\n");

    RTC_Config();
    
    SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);//1ms�жϣ�FreeRTOSʹ��

    /* ����AppTaskCreate���� */
    xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                          (const char*    )"AppTaskCreate",/* �������� */
                          (uint16_t       )512,  /* ����ջ��С */
                          (void*          )NULL,/* ������ں������� */
                          (UBaseType_t    )1, /* ��������ȼ� */
                          (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */
    /* ����������� */
    if(pdPASS == xReturn) {
        vTaskStartScheduler();   /* �������񣬿������� */
    }


    while(1);
}
