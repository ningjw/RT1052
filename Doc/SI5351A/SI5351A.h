/********************************************
主控芯片：STM32F405RGT6主频168Mhz
晶体频率：HSE=8Mhz  SYSCLK=168Mhz
模块型号：STM32串口初始化
通讯方式：
函数功能：Si5351时钟芯片
作者：苏夏雨
授权：未经作者允许，禁止转载
********************************************/
#ifndef _si5351a_h
#define _si5351a_h
#include "system.h"
//Si5351寄存器声明
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
//IIC总线引脚配置
#define SDA(n) {n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);}
#define CLK(n) {n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);}
//相关函数声明
void Si5351Init(void);//初始化Si5351的GPIO
void SetPLLClk(uint8_t pll, uint8_t mult, uint32_t num, uint32_t denom);//设置PPL时钟
void SetFrequency(uint32_t frequency);//时钟Si5351时钟频率
void SetMultisynth(uint8_t synth,uint32_t divider,uint8_t rDiv);//设置多synth
#endif
