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
    taskENTER_CRITICAL();           //�����ٽ���
    
    /* ����BLE_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )AppBLE_Task,"BLE_Task",512,NULL,1,&AppBLE_TaskHandle);
    
    /* ����ADC_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )AppADC_Task, "ADC_Task",512,NULL, 2,&AppADC_TaskHandle);
                          
    /* ����eMMC_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )AppEMMC_Task,"eMMC_Task",512,NULL,3,&AppEMMC_TaskHandle);

    vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����

    taskEXIT_CRITICAL();            //�˳��ٽ���
}





/***************************************************************************************
  * @brief   ��ں���
  * @input   
  * @return  
***************************************************************************************/
int main(void)
{
    BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */

    BOARD_BootClockRUN();
    BOARD_InitBootPins();
    BOARD_InitPeripherals();

    BOARD_InitDebugConsole();
    PRINTF("***** Welcome *****\r\n");
//    EEPROM_Test();
    RTC_Config();//ʵʱʱ�ӳ�ʼ��
    
    SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);//1ms�жϣ�FreeRTOSʹ��
    
    /* ����AppTaskCreate���񡣲�������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate, "AppTaskCreate",512,NULL,1,&AppTaskCreate_Handle);
    /* ����������� */
    if(pdPASS == xReturn) {
        vTaskStartScheduler();   /* �������񣬿������� */
    }else PRINTF("���񴴽�ʧ��");

    while(1);
}
