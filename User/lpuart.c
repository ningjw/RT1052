#include "main.h"

/*
  Ring buffer for data input and output, in this example, input data are saved
  to ring buffer in IRQ handler. The main function polls the ring buffer status,
  if there are new data, then send them out.
  Ring buffer full: (((rxIndex + 1) % DEMO_RING_BUFFER_SIZE) == txIndex)
  Ring buffer empty: (rxIndex == txIndex)
*/
#define RX_BUF_SIZE 16
uint8_t LpuartRxBuffer[16];
volatile uint16_t txIndex; /* Index of the data to send out. */
volatile uint16_t rxIndex; /* Index of the memory to save new arrived data. */



/* Get debug console frequency. */
uint32_t BOARD_DebugConsoleSrcFreq(void)
{
    uint32_t freq;
    /* To make it simple, we assume default PLL and divider settings, and the only variable
       from application is use PLL3 source or OSC source */
    if (CLOCK_GetMux(kCLOCK_UartMux) == 0) /* PLL3 div6 80M */
        freq = (CLOCK_GetPllFreq(kCLOCK_PllUsb1) / 6U) / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U);
    else
        freq = CLOCK_GetOscFreq() / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U);

    return freq;
}

//uint8_t g_tipString[] ="Lpuart functional API interrupt example";
void LPUART1_Init(void)
{
    lpuart_config_t config;
    uint32_t freq;
    //LPUART1所使用的IO功能配置，即：从ALT0~ALT7选择合适的功能。
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_12_LPUART1_TX,0U);	//GPIO_AD_B0_12设置为LPUART1_TX
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_13_LPUART1_RX,0U);	//GPIO_AD_B0_13设置为LPUART1_RX

	//配置IO引脚GPIO_AD_B0_12和GPIO_AD_B0_13的功能
	//低转换速度,驱动能力为R0/6,速度为100Mhz，关闭开路功能，使能pull/keepr
	//选择keeper功能，下拉100K Ohm，关闭Hyst
	IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_12_LPUART1_TX,0x10B0u); 
	IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_13_LPUART1_RX,0x10B0u); 
    
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = 115200U;
    config.enableTx = true;
    config.enableRx = true;
    freq = BOARD_DebugConsoleSrcFreq();
    LPUART_Init(LPUART1, &config, freq);
    DbgConsole_Init((uint32_t)LPUART1, config.baudRate_Bps, DEBUG_CONSOLE_DEVICE_TYPE_LPUART,freq);
    /* Send g_tipString out. */
//    LPUART_WriteBlocking(LPUART1, g_tipString, sizeof(g_tipString) / sizeof(g_tipString[0]));
    
    /* Enable RX interrupt. */
    LPUART_EnableInterrupts(LPUART1, kLPUART_RxDataRegFullInterruptEnable);
    EnableIRQ(LPUART1_IRQn);
}


void LPUART1_IRQHandler(void)
{
    uint8_t data;

    /* If new data arrived. */
    if ((kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(LPUART1))
    {
        data = LPUART_ReadByte(LPUART1);

        /* If rx buffer is not full, add data to ring buffer. */
        if (((rxIndex + 1) % RX_BUF_SIZE) != txIndex)
        {
            LpuartRxBuffer[rxIndex] = data;
            rxIndex++;
            rxIndex %= RX_BUF_SIZE;
        }
    }
}

