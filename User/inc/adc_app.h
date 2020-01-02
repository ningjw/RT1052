#ifndef __ADC_APP_H
#define __ADC_APP_H

#define ADC_READY                GPIO_PinRead(BOARD_ADC_RDY_GPIO, BOARD_ADC_RDY_PIN)



extern TaskHandle_t ADC_TaskHandle ;  /* ADCÈÎÎñ¾ä±ú */

void ADC_SampleStart(void);
void ADC_AppTask(void);
void ADC_SampleStop(void);



#endif
