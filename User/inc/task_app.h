#ifndef __TASK_APP_H
#define __TASK_APP_H



extern TaskHandle_t AppADC_TaskHandle ;  /* ADC任务句柄 */
extern TaskHandle_t AppEMMC_TaskHandle; /* eMMC任务句柄 */


void AppADC_Task(void);
void AppEMMC_Task(void);

#endif







