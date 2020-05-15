/********************************************
����оƬ��STM32F405RGT6��Ƶ168Mhz
����Ƶ�ʣ�HSE=8Mhz  SYSCLK=168Mhz
ģ���ͺţ�STM32���ڳ�ʼ��
ͨѶ��ʽ��
�������ܣ�Si5351ʱ��оƬ
���ߣ�������
��Ȩ��δ������������ֹת��
********************************************/
#ifndef _si5351a_h
#define _si5351a_h
#include "system.h"
//Si5351�Ĵ�������
#define SI_CLK0_CONTROL	16			// Register definitions
#define SI_CLK1_CONTROL	17
#define SI_CLK2_CONTROL	18
#define SI_SYNTH_PLL_A	26
#define SI_SYNTH_PLL_B	34
#define SI_SYNTH_MS_0		42
#define SI_SYNTH_MS_1		50
#define SI_SYNTH_MS_2		58
#define SI_PLL_RESET		177

#define SI_R_DIV_1		0x00			// R-division ratio definitions
#define SI_R_DIV_2		0b00010000
#define SI_R_DIV_4		0b00100000
#define SI_R_DIV_8		0b00110000
#define SI_R_DIV_16		0b01000000
#define SI_R_DIV_32		0b01010000
#define SI_R_DIV_64		0b01100000
#define SI_R_DIV_128		0b01110000

#define SI_CLK_SRC_PLL_A	0x00
#define SI_CLK_SRC_PLL_B	0b00100000
#define XTAL_FREQ	25000000			// Crystal frequency
//IIC������������
#define SDA(n) {n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);}
#define CLK(n) {n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);}
//��غ�������
void Si5351Init(void);//��ʼ��Si5351��GPIO
void SetPLLClk(uint8_t pll, uint8_t mult, uint32_t num, uint32_t denom);//����PPLʱ��
void SetFrequency(uint32_t frequency);//ʱ��Si5351ʱ��Ƶ��
void SetMultisynth(uint8_t synth,uint32_t divider,uint8_t rDiv);//���ö�synth
#endif
