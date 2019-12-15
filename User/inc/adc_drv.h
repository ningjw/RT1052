#ifndef __ADC_DRV_H
#define __ADC_DRV_H

//对应核心板的A-26脚
#define SET_ADC_HIGH_SPEED_MODE()  GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 0U);
#define SET_ADC_LOW_POWER_MODE()   GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1U);

//对应核心板的A-19脚
#define ADC_READY() GPIO_PinRead(BOARD_ADC_RDY_GPIO, BOARD_ADC_RDY_PIN)
                   

void XBARA_Configuration(void);
void ADC_ETC_Config(void);

#endif
