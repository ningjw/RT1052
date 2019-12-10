#ifndef __TASK_APP_H
#define __TASK_APP_H



extern TaskHandle_t AppADC_TaskHandle ;  /* ADC������ */
extern TaskHandle_t AppEMMC_TaskHandle; /* eMMC������ */


void AppADC_Task(void);
void AppEMMC_Task(void);

#endif







