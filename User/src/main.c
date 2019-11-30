#include "main.h"


static TaskHandle_t AppTaskCreate_Handle = NULL;      /* ���������� */
static void AppTaskCreate(void);               /* ���ڴ������� */




/***********************************************************************
  * @ ������  �� AppTaskCreate
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
static void AppTaskCreate(void)
{
//    taskENTER_CRITICAL();           //�����ٽ���
    while(1){
        vTaskDelay(1000);
        PRINTF("AppTaskCreate\r\n");
    }
//    vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����

//    taskEXIT_CRITICAL();            //�˳��ٽ���
}



int main(void)
{
    BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */

    BOARD_BootClockRUN();
    BOARD_InitBootPins();
    BOARD_InitPeripherals();

    BOARD_InitDebugConsole();
    PRINTF("***** Welcome *****\r\n");

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
