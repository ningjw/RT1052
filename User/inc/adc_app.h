#ifndef __ADC_APP_H
#define __ADC_APP_H





extern TaskHandle_t ADC_TaskHandle ;  /* ADCÈÎÎñ¾ä±ú */

void ADC_SampleStart(void);
void ADC_AppTask(void);
void ADC_SampleStop(void);



#endif
