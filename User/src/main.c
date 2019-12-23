#include "main.h"

TimerHandle_t       InactiveTmr = NULL;//�����ʱ�����,����ATָ����ܳ�ʱ
static TaskHandle_t AppTaskCreate_Handle = NULL;      /* ���������� */
static void AppTaskCreate(void);                      /* ���ڴ������� */

SysPara1 g_sys_para1;
SysPara2 g_sys_para2;
/***************************************************************************************
  * @brief   ��ʱ������,�ò������ڼ�¼��������ˣ�ÿ1�����ж�һ��
  * @input   
  * @return  
***************************************************************************************/
static void InactiveTmr_Callback(void* parameter)
{
    PRINTF("1���Ӷ�ʱ����");
    if(g_sys_para2.inactiveCount++ >= g_sys_para1.inactiveTime + 1){//�Զ��ػ�
        
    }
}



/***********************************************************************
  * @ ������  �� AppTaskCreate
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
static void AppTaskCreate(void)
{
    taskENTER_CRITICAL();           //�����ٽ���
    
    /* ����LED_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )LED_AppTask,"LED_Task",128,NULL, 1,&LED_TaskHandle);
    
    /* ����Battery_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )BAT_AppTask,"BAT_Task",512,NULL, 2,&BAT_TaskHandle);

    /* ����BLE_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )BLE_AppTask,"BLE_Task",512,NULL, 3,&BLE_TaskHandle);
    
    /* ����ADC_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )ADC_AppTask, "ADC_Task",512,NULL, 4,&ADC_TaskHandle);
    
    //���������ʱ��������һ��Ϊ����ʱ�����ơ���ʱ���ڡ�����ģʽ��Ψһid���ص�����
    InactiveTmr = xTimerCreate("PwrOnTmr", 60*1000, pdTRUE, (void*)POWER_ON_TIMER_ID, (TimerCallbackFunction_t)InactiveTmr_Callback);
    
    vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}


/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
static void InitSysPara()
{
    g_sys_para1.inactiveTime = 15;//Ĭ��15����û�л���Զ��ػ���
    g_sys_para2.inactiveCount = 0;
    g_sys_para2.sampLedStatus = WORK_FINE;
    g_sys_para2.batLedStatus = BAT_FULL;
    g_sys_para2.bleLedStatus = BLE_CLOSE;
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
    InitSysPara();
    RTC_Config();//ʵʱʱ�ӳ�ʼ��
    FlexSPI_NorFlash_Init();
    SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);//1ms�жϣ�FreeRTOSʹ��
    
    EEPROM_Test();
    NorFlash_IPCommand_Test();
    
    /* ����AppTaskCreate���񡣲�������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate, "AppTaskCreate",512,NULL,1,&AppTaskCreate_Handle);
    /* ����������� */
    if(pdPASS == xReturn) {
        vTaskStartScheduler();   /* �������񣬿������� */
    }else PRINTF("���񴴽�ʧ��");

    while(1);
}
