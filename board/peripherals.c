/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Peripherals v6.0
processor: MIMXRT1052xxxxB
package_id: MIMXRT1052CVL5B
mcu_data: ksdk2_0
processor_version: 6.0.1
functionalGroups:
- name: BOARD_InitPeripherals
  called_from_default_init: true
  selectedCore: core0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'system'
- type_id: 'system_54b53072540eeeb8f8e9343e71f28176'
- global_system_definitions: []
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'msg'
- type_id: 'msg_6e2baaf3b97dbeef01c0043275f9a0e7'
- global_messages: []
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "peripherals.h"

/***********************************************************************************************************************
 * BOARD_InitPeripherals functional group
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * eDMA initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'eDMA'
- type: 'edma'
- mode: 'basic'
- type_id: 'edma_a23fca76a894e1bcdf9d01a687505ff9'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'DMA0'
- config_sets:
  - fsl_edma:
    - common_settings:
      - enableContinuousLinkMode: 'false'
      - enableHaltOnError: 'true'
      - enableRoundRobinArbitration: 'false'
      - enableDebugMode: 'false'
    - dma_table: []
    - edma_channels: []
    - quick_selection: 'default'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const edma_config_t eDMA_config = {
  .enableContinuousLinkMode = false,
  .enableHaltOnError = true,
  .enableRoundRobinArbitration = false,
  .enableDebugMode = false
};

void eDMA_init(void) {
}

/***********************************************************************************************************************
 * LPUART1 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'LPUART1'
- type: 'lpuart'
- mode: 'interrupts'
- type_id: 'lpuart_bebe3e12b6ec22bbd14199038f2bf459'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'LPUART1'
- config_sets:
  - lpuartConfig_t:
    - lpuartConfig:
      - clockSource: 'LpuartClock'
      - lpuartSrcClkFreq: 'custom:80000000'
      - baudRate_Bps: '115200'
      - parityMode: 'kLPUART_ParityDisabled'
      - dataBitsCount: 'kLPUART_EightDataBits'
      - isMsb: 'false'
      - stopBitCount: 'kLPUART_OneStopBit'
      - txFifoWatermark: '0'
      - rxFifoWatermark: '0'
      - enableRxRTS: 'false'
      - enableTxCTS: 'false'
      - txCtsSource: 'kLPUART_CtsSourcePin'
      - txCtsConfig: 'kLPUART_CtsSampleAtStart'
      - rxIdleType: 'kLPUART_IdleTypeStartBit'
      - rxIdleConfig: 'kLPUART_IdleCharacter1'
      - enableTx: 'true'
      - enableRx: 'true'
  - interruptsCfg:
    - interrupts: 'kLPUART_RxDataRegFullInterruptEnable kLPUART_RxOverrunInterruptEnable'
    - interrupt_vectors:
      - enable_rx_tx_irq: 'true'
      - interrupt_rx_tx:
        - IRQn: 'LPUART1_IRQn'
        - enable_priority: 'false'
        - priority: '0'
        - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const lpuart_config_t LPUART1_config = {
  .baudRate_Bps = 115200,
  .parityMode = kLPUART_ParityDisabled,
  .dataBitsCount = kLPUART_EightDataBits,
  .isMsb = false,
  .stopBitCount = kLPUART_OneStopBit,
  .txFifoWatermark = 0,
  .rxFifoWatermark = 0,
  .enableRxRTS = false,
  .enableTxCTS = false,
  .txCtsSource = kLPUART_CtsSourcePin,
  .txCtsConfig = kLPUART_CtsSampleAtStart,
  .rxIdleType = kLPUART_IdleTypeStartBit,
  .rxIdleConfig = kLPUART_IdleCharacter1,
  .enableTx = true,
  .enableRx = true
};

void LPUART1_init(void) {
  LPUART_Init(LPUART1_PERIPHERAL, &LPUART1_config, LPUART1_CLOCK_SOURCE);
  LPUART_EnableInterrupts(LPUART1_PERIPHERAL, kLPUART_RxDataRegFullInterruptEnable | kLPUART_RxOverrunInterruptEnable);
  /* Enable interrupt LPUART1_IRQn request in the NVIC */
  EnableIRQ(LPUART1_SERIAL_RX_TX_IRQN);
}

/***********************************************************************************************************************
 * PIT1 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'PIT1'
- type: 'pit'
- mode: 'LPTMR_GENERAL'
- type_id: 'pit_a4782ba5223c8a2527ba91aeb2bc4159'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'PIT'
- config_sets:
  - fsl_pit:
    - enableRunInDebug: 'true'
    - enableSharedInterrupt: 'true'
    - sharedInterrupt:
      - IRQn: 'PIT_IRQn'
      - enable_priority: 'true'
      - priority: '5'
      - enable_custom_name: 'false'
    - timingConfig:
      - clockSource: 'BusInterfaceClock'
      - clockSourceFreq: 'BOARD_BootClockRUN'
    - channels:
      - 0:
        - channelNumber: '0'
        - enableChain: 'false'
        - timerPeriod: '1000ms'
        - startTimer: 'true'
        - enableInterrupt: 'true'
      - 1:
        - channelNumber: '1'
        - enableChain: 'false'
        - timerPeriod: '1ms'
        - startTimer: 'true'
        - enableInterrupt: 'true'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const pit_config_t PIT1_config = {
  .enableRunInDebug = true
};

void PIT1_init(void) {
  /* Initialize the PIT. */
  PIT_Init(PIT1_PERIPHERAL, &PIT1_config);
  /* Set channel 0 period to 1 s (66000000 ticks). */
  PIT_SetTimerPeriod(PIT1_PERIPHERAL, kPIT_Chnl_0, PIT1_0_TICKS);
  /* Set channel 1 period to 1 ms (66000 ticks). */
  PIT_SetTimerPeriod(PIT1_PERIPHERAL, kPIT_Chnl_1, PIT1_1_TICKS);
  /* Enable interrupts from channel 0. */
  PIT_EnableInterrupts(PIT1_PERIPHERAL, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
  /* Enable interrupts from channel 1. */
  PIT_EnableInterrupts(PIT1_PERIPHERAL, kPIT_Chnl_1, kPIT_TimerInterruptEnable);
  /* Interrupt vector PIT1_IRQN priority settings in the NVIC */
  NVIC_SetPriority(PIT1_IRQN, PIT1_IRQ_PRIORITY);
  /* Enable interrupt PIT1_IRQN request in the NVIC */
  EnableIRQ(PIT1_IRQN);
  /* Start channel 0. */
  PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
  /* Start channel 1. */
  PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_1);
}

/***********************************************************************************************************************
 * LPI2C1 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'LPI2C1'
- type: 'lpi2c'
- mode: 'master'
- type_id: 'lpi2c_db68d4f4f06a22e25ab51fe9bd6db4d2'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'LPI2C1'
- config_sets:
  - main:
    - clockSource: 'Lpi2cClock'
    - clockSourceFreq: 'BOARD_BootClockRUN'
    - interrupt:
      - IRQn: 'LPI2C1_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
    - quick_selection: 'qs_interrupt'
  - master:
    - mode: 'polling'
    - config:
      - enableMaster: 'true'
      - enableDoze: 'true'
      - debugEnable: 'false'
      - ignoreAck: 'false'
      - pinConfig: 'kLPI2C_2PinOpenDrain'
      - baudRate_Hz: '10000'
      - busIdleTimeout_ns: '0'
      - pinLowTimeout_ns: '0'
      - sdaGlitchFilterWidth_ns: '0'
      - sclGlitchFilterWidth_ns: '0'
      - hostRequest:
        - enable: 'false'
        - source: 'kLPI2C_HostRequestExternalPin'
        - polarity: 'kLPI2C_HostRequestPinActiveHigh'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const lpi2c_master_config_t LPI2C1_masterConfig = {
  .enableMaster = true,
  .enableDoze = true,
  .debugEnable = false,
  .ignoreAck = false,
  .pinConfig = kLPI2C_2PinOpenDrain,
  .baudRate_Hz = 10000,
  .busIdleTimeout_ns = 0,
  .pinLowTimeout_ns = 0,
  .sdaGlitchFilterWidth_ns = 0,
  .sclGlitchFilterWidth_ns = 0,
  .hostRequest = {
    .enable = false,
    .source = kLPI2C_HostRequestExternalPin,
    .polarity = kLPI2C_HostRequestPinActiveHigh
  }
};

void LPI2C1_init(void) {
  LPI2C_MasterInit(LPI2C1_PERIPHERAL, &LPI2C1_masterConfig, LPI2C1_CLOCK_FREQ);
}

/***********************************************************************************************************************
 * QuadTimer3 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'QuadTimer3'
- type: 'qtmr'
- mode: 'general'
- type_id: 'qtmr_460dd7aa3f3371843c2548acd54252b0'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'TMR3'
- config_sets:
  - fsl_qtmr:
    - clockSource: 'BusInterfaceClock'
    - clockSourceFreq: 'BOARD_BootClockRUN'
    - qtmr_channels:
      - 0:
        - channel_prefix_id: 'Channel_0'
        - channel: 'kQTMR_Channel_0'
        - primarySource: 'kQTMR_ClockDivide_1'
        - secondarySource: 'kQTMR_Counter0InputPin'
        - countingMode: 'kQTMR_PriSrcRiseEdge'
        - enableMasterMode: 'false'
        - enableExternalForce: 'false'
        - faultFilterCount: '3'
        - faultFilterPeriod: '0'
        - debugMode: 'kQTMR_RunNormalInDebug'
        - timerModeInit: 'pwmOutput'
        - pwmMode:
          - freq_value_str: '11'
          - dutyCyclePercent: '50'
          - outputPolarity: 'false'
        - dmaIntMode: 'polling'
    - interruptVector:
      - enable_irq: 'false'
      - interrupt:
        - IRQn: 'TMR3_IRQn'
        - enable_priority: 'false'
        - priority: '0'
        - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const qtmr_config_t QuadTimer3_Channel_0_config = {
  .primarySource = kQTMR_ClockDivide_1,
  .secondarySource = kQTMR_Counter0InputPin,
  .enableMasterMode = false,
  .enableExternalForce = false,
  .faultFilterCount = 0,
  .faultFilterPeriod = 0,
  .debugMode = kQTMR_RunNormalInDebug
};

void QuadTimer3_init(void) {
  /* Quad timer channel Channel_0 peripheral initialization */
  QTMR_Init(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, &QuadTimer3_Channel_0_config);
  /* Setup the PWM mode of the timer channel */
  QTMR_SetupPwm(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, 12000000UL, 50U, false, QUADTIMER3_CHANNEL_0_CLOCK_SOURCE);
  /* Start the timer - select the timer counting mode */
  QTMR_StartTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
}

/***********************************************************************************************************************
 * LPUART2 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'LPUART2'
- type: 'lpuart'
- mode: 'interrupts'
- type_id: 'lpuart_bebe3e12b6ec22bbd14199038f2bf459'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'LPUART2'
- config_sets:
  - lpuartConfig_t:
    - lpuartConfig:
      - clockSource: 'LpuartClock'
      - lpuartSrcClkFreq: 'custom:80000000'
      - baudRate_Bps: '115200'
      - parityMode: 'kLPUART_ParityDisabled'
      - dataBitsCount: 'kLPUART_EightDataBits'
      - isMsb: 'false'
      - stopBitCount: 'kLPUART_OneStopBit'
      - txFifoWatermark: '0'
      - rxFifoWatermark: '0'
      - enableRxRTS: 'false'
      - enableTxCTS: 'false'
      - txCtsSource: 'kLPUART_CtsSourcePin'
      - txCtsConfig: 'kLPUART_CtsSampleAtStart'
      - rxIdleType: 'kLPUART_IdleTypeStartBit'
      - rxIdleConfig: 'kLPUART_IdleCharacter1'
      - enableTx: 'true'
      - enableRx: 'true'
  - interruptsCfg:
    - interrupts: 'kLPUART_RxDataRegFullInterruptEnable kLPUART_RxOverrunInterruptEnable'
    - interrupt_vectors:
      - enable_rx_tx_irq: 'true'
      - interrupt_rx_tx:
        - IRQn: 'LPUART2_IRQn'
        - enable_priority: 'false'
        - priority: '0'
        - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const lpuart_config_t LPUART2_config = {
  .baudRate_Bps = 115200,
  .parityMode = kLPUART_ParityDisabled,
  .dataBitsCount = kLPUART_EightDataBits,
  .isMsb = false,
  .stopBitCount = kLPUART_OneStopBit,
  .txFifoWatermark = 0,
  .rxFifoWatermark = 0,
  .enableRxRTS = false,
  .enableTxCTS = false,
  .txCtsSource = kLPUART_CtsSourcePin,
  .txCtsConfig = kLPUART_CtsSampleAtStart,
  .rxIdleType = kLPUART_IdleTypeStartBit,
  .rxIdleConfig = kLPUART_IdleCharacter1,
  .enableTx = true,
  .enableRx = true
};

void LPUART2_init(void) {
  LPUART_Init(LPUART2_PERIPHERAL, &LPUART2_config, LPUART2_CLOCK_SOURCE);
  LPUART_EnableInterrupts(LPUART2_PERIPHERAL, kLPUART_RxDataRegFullInterruptEnable | kLPUART_RxOverrunInterruptEnable);
  /* Enable interrupt LPUART2_IRQn request in the NVIC */
  EnableIRQ(LPUART2_SERIAL_RX_TX_IRQN);
}

/***********************************************************************************************************************
 * LPUART4 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'LPUART4'
- type: 'lpuart'
- mode: 'interrupts'
- type_id: 'lpuart_bebe3e12b6ec22bbd14199038f2bf459'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'LPUART4'
- config_sets:
  - lpuartConfig_t:
    - lpuartConfig:
      - clockSource: 'LpuartClock'
      - lpuartSrcClkFreq: 'custom:80000000'
      - baudRate_Bps: '115200'
      - parityMode: 'kLPUART_ParityDisabled'
      - dataBitsCount: 'kLPUART_EightDataBits'
      - isMsb: 'false'
      - stopBitCount: 'kLPUART_OneStopBit'
      - txFifoWatermark: '0'
      - rxFifoWatermark: '0'
      - enableRxRTS: 'false'
      - enableTxCTS: 'false'
      - txCtsSource: 'kLPUART_CtsSourcePin'
      - txCtsConfig: 'kLPUART_CtsSampleAtStart'
      - rxIdleType: 'kLPUART_IdleTypeStartBit'
      - rxIdleConfig: 'kLPUART_IdleCharacter1'
      - enableTx: 'true'
      - enableRx: 'true'
  - interruptsCfg:
    - interrupts: 'kLPUART_RxDataRegFullInterruptEnable kLPUART_RxOverrunInterruptEnable'
    - interrupt_vectors:
      - enable_rx_tx_irq: 'true'
      - interrupt_rx_tx:
        - IRQn: 'LPUART4_IRQn'
        - enable_priority: 'false'
        - priority: '0'
        - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const lpuart_config_t LPUART4_config = {
  .baudRate_Bps = 115200,
  .parityMode = kLPUART_ParityDisabled,
  .dataBitsCount = kLPUART_EightDataBits,
  .isMsb = false,
  .stopBitCount = kLPUART_OneStopBit,
  .txFifoWatermark = 0,
  .rxFifoWatermark = 0,
  .enableRxRTS = false,
  .enableTxCTS = false,
  .txCtsSource = kLPUART_CtsSourcePin,
  .txCtsConfig = kLPUART_CtsSampleAtStart,
  .rxIdleType = kLPUART_IdleTypeStartBit,
  .rxIdleConfig = kLPUART_IdleCharacter1,
  .enableTx = true,
  .enableRx = true
};

void LPUART4_init(void) {
  LPUART_Init(LPUART4_PERIPHERAL, &LPUART4_config, LPUART4_CLOCK_SOURCE);
  LPUART_EnableInterrupts(LPUART4_PERIPHERAL, kLPUART_RxDataRegFullInterruptEnable | kLPUART_RxOverrunInterruptEnable);
  /* Enable interrupt LPUART4_IRQn request in the NVIC */
  EnableIRQ(LPUART4_SERIAL_RX_TX_IRQN);
}

/***********************************************************************************************************************
 * LPUART3 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'LPUART3'
- type: 'lpuart'
- mode: 'interrupts'
- type_id: 'lpuart_bebe3e12b6ec22bbd14199038f2bf459'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'LPUART3'
- config_sets:
  - lpuartConfig_t:
    - lpuartConfig:
      - clockSource: 'LpuartClock'
      - lpuartSrcClkFreq: 'custom:80000000'
      - baudRate_Bps: '115200'
      - parityMode: 'kLPUART_ParityDisabled'
      - dataBitsCount: 'kLPUART_EightDataBits'
      - isMsb: 'false'
      - stopBitCount: 'kLPUART_OneStopBit'
      - txFifoWatermark: '0'
      - rxFifoWatermark: '0'
      - enableRxRTS: 'false'
      - enableTxCTS: 'false'
      - txCtsSource: 'kLPUART_CtsSourcePin'
      - txCtsConfig: 'kLPUART_CtsSampleAtStart'
      - rxIdleType: 'kLPUART_IdleTypeStartBit'
      - rxIdleConfig: 'kLPUART_IdleCharacter1'
      - enableTx: 'true'
      - enableRx: 'true'
  - interruptsCfg:
    - interrupts: 'kLPUART_RxDataRegFullInterruptEnable kLPUART_RxOverrunInterruptEnable'
    - interrupt_vectors:
      - enable_rx_tx_irq: 'true'
      - interrupt_rx_tx:
        - IRQn: 'LPUART3_IRQn'
        - enable_priority: 'false'
        - priority: '0'
        - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const lpuart_config_t LPUART3_config = {
  .baudRate_Bps = 115200,
  .parityMode = kLPUART_ParityDisabled,
  .dataBitsCount = kLPUART_EightDataBits,
  .isMsb = false,
  .stopBitCount = kLPUART_OneStopBit,
  .txFifoWatermark = 0,
  .rxFifoWatermark = 0,
  .enableRxRTS = false,
  .enableTxCTS = false,
  .txCtsSource = kLPUART_CtsSourcePin,
  .txCtsConfig = kLPUART_CtsSampleAtStart,
  .rxIdleType = kLPUART_IdleTypeStartBit,
  .rxIdleConfig = kLPUART_IdleCharacter1,
  .enableTx = true,
  .enableRx = true
};

void LPUART3_init(void) {
  LPUART_Init(LPUART3_PERIPHERAL, &LPUART3_config, LPUART3_CLOCK_SOURCE);
  LPUART_EnableInterrupts(LPUART3_PERIPHERAL, kLPUART_RxDataRegFullInterruptEnable | kLPUART_RxOverrunInterruptEnable);
  /* Enable interrupt LPUART3_IRQn request in the NVIC */
  EnableIRQ(LPUART3_SERIAL_RX_TX_IRQN);
}

/***********************************************************************************************************************
 * LPUART5 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'LPUART5'
- type: 'lpuart'
- mode: 'interrupts'
- type_id: 'lpuart_bebe3e12b6ec22bbd14199038f2bf459'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'LPUART5'
- config_sets:
  - lpuartConfig_t:
    - lpuartConfig:
      - clockSource: 'LpuartClock'
      - lpuartSrcClkFreq: 'custom:80000000'
      - baudRate_Bps: '115200'
      - parityMode: 'kLPUART_ParityDisabled'
      - dataBitsCount: 'kLPUART_EightDataBits'
      - isMsb: 'false'
      - stopBitCount: 'kLPUART_OneStopBit'
      - txFifoWatermark: '0'
      - rxFifoWatermark: '0'
      - enableRxRTS: 'false'
      - enableTxCTS: 'false'
      - txCtsSource: 'kLPUART_CtsSourcePin'
      - txCtsConfig: 'kLPUART_CtsSampleAtStart'
      - rxIdleType: 'kLPUART_IdleTypeStartBit'
      - rxIdleConfig: 'kLPUART_IdleCharacter1'
      - enableTx: 'true'
      - enableRx: 'true'
  - interruptsCfg:
    - interrupts: 'kLPUART_RxDataRegFullInterruptEnable kLPUART_RxOverrunInterruptEnable'
    - interrupt_vectors:
      - enable_rx_tx_irq: 'true'
      - interrupt_rx_tx:
        - IRQn: 'LPUART5_IRQn'
        - enable_priority: 'false'
        - priority: '0'
        - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const lpuart_config_t LPUART5_config = {
  .baudRate_Bps = 115200,
  .parityMode = kLPUART_ParityDisabled,
  .dataBitsCount = kLPUART_EightDataBits,
  .isMsb = false,
  .stopBitCount = kLPUART_OneStopBit,
  .txFifoWatermark = 0,
  .rxFifoWatermark = 0,
  .enableRxRTS = false,
  .enableTxCTS = false,
  .txCtsSource = kLPUART_CtsSourcePin,
  .txCtsConfig = kLPUART_CtsSampleAtStart,
  .rxIdleType = kLPUART_IdleTypeStartBit,
  .rxIdleConfig = kLPUART_IdleCharacter1,
  .enableTx = true,
  .enableRx = true
};

void LPUART5_init(void) {
  LPUART_Init(LPUART5_PERIPHERAL, &LPUART5_config, LPUART5_CLOCK_SOURCE);
  LPUART_EnableInterrupts(LPUART5_PERIPHERAL, kLPUART_RxDataRegFullInterruptEnable | kLPUART_RxOverrunInterruptEnable);
  /* Enable interrupt LPUART5_IRQn request in the NVIC */
  EnableIRQ(LPUART5_SERIAL_RX_TX_IRQN);
}

/***********************************************************************************************************************
 * LPSPI4 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'LPSPI4'
- type: 'lpspi'
- mode: 'polling'
- type_id: 'lpspi_6e21a1e0a09f0a012d683c4f91752db8'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'LPSPI4'
- config_sets:
  - main:
    - mode: 'kLPSPI_Master'
    - clockSource: 'LpspiClock'
    - clockSourceFreq: 'BOARD_BootClockRUN'
    - master:
      - baudRate: '400000'
      - bitsPerFrame: '24'
      - cpol: 'kLPSPI_ClockPolarityActiveHigh'
      - cpha: 'kLPSPI_ClockPhaseFirstEdge'
      - direction: 'kLPSPI_MsbFirst'
      - pcsToSckDelayInNanoSec: '0'
      - lastSckToPcsDelayInNanoSec: '0'
      - betweenTransferDelayInNanoSec: '0'
      - whichPcs: 'kLPSPI_Pcs0'
      - pcsActiveHighOrLow: 'kLPSPI_PcsActiveLow'
      - pinCfg: 'kLPSPI_SdiInSdoOut'
      - dataOutConfig: 'kLpspiDataOutRetained'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const lpspi_master_config_t LPSPI4_config = {
  .baudRate = 400000,
  .bitsPerFrame = 24,
  .cpol = kLPSPI_ClockPolarityActiveHigh,
  .cpha = kLPSPI_ClockPhaseFirstEdge,
  .direction = kLPSPI_MsbFirst,
  .pcsToSckDelayInNanoSec = 0,
  .lastSckToPcsDelayInNanoSec = 0,
  .betweenTransferDelayInNanoSec = 0,
  .whichPcs = kLPSPI_Pcs0,
  .pcsActiveHighOrLow = kLPSPI_PcsActiveLow,
  .pinCfg = kLPSPI_SdiInSdoOut,
  .dataOutConfig = kLpspiDataOutRetained
};

void LPSPI4_init(void) {
  LPSPI_MasterInit(LPSPI4_PERIPHERAL, &LPSPI4_config, LPSPI4_CLOCK_FREQ);
}

/***********************************************************************************************************************
 * GPIO2 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'GPIO2'
- type: 'igpio'
- mode: 'GPIO'
- type_id: 'igpio_b1c1fa279aa7069dca167502b8589cb7'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'GPIO2'
- config_sets:
  - fsl_gpio:
    - enable_irq_comb_0_15: 'true'
    - gpio_interrupt_comb_0_15:
      - IRQn: 'GPIO2_Combined_0_15_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'true'
      - handler_custom_name: 'GPIO2_COMB_0_15_IRQHANDLER'
    - enable_irq_comb_16_31: 'false'
    - gpio_interrupt_comb_16_31:
      - IRQn: 'GPIO2_Combined_16_31_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

void GPIO2_init(void) {
  /* Make sure, the clock gate for GPIO2 is enabled (e. g. in pin_mux.c) */
  /* Enable interrupt GPIO2_Combined_0_15_IRQn request in the NVIC */
  EnableIRQ(GPIO2_Combined_0_15_IRQn);
}

/***********************************************************************************************************************
 * QuadTimer1 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'QuadTimer1'
- type: 'qtmr'
- mode: 'general'
- type_id: 'qtmr_460dd7aa3f3371843c2548acd54252b0'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'TMR1'
- config_sets:
  - fsl_qtmr:
    - clockSource: 'BusInterfaceClock'
    - clockSourceFreq: 'BOARD_BootClockRUN'
    - qtmr_channels:
      - 0:
        - channel_prefix_id: 'Channel_0'
        - channel: 'kQTMR_Channel_0'
        - primarySource: 'kQTMR_ClockDivide_1'
        - secondarySource: 'kQTMR_Counter0InputPin'
        - countingMode: 'kQTMR_PriSrcRiseEdge'
        - enableMasterMode: 'false'
        - enableExternalForce: 'false'
        - faultFilterCount: '3'
        - faultFilterPeriod: '0'
        - debugMode: 'kQTMR_RunNormalInDebug'
        - timerModeInit: 'inputCapture'
        - inputCaptureMode:
          - inputPolarity: 'false'
          - reloadOnCapture: 'true'
          - captureMode: 'kQTMR_RisingEdge'
        - dmaIntMode: 'interrupt'
        - interrupts: 'kQTMR_EdgeInterruptEnable'
    - interruptVector:
      - enable_irq: 'true'
      - interrupt:
        - IRQn: 'TMR1_IRQn'
        - enable_priority: 'false'
        - priority: '0'
        - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const qtmr_config_t QuadTimer1_Channel_0_config = {
  .primarySource = kQTMR_ClockDivide_1,
  .secondarySource = kQTMR_Counter0InputPin,
  .enableMasterMode = false,
  .enableExternalForce = false,
  .faultFilterCount = 0,
  .faultFilterPeriod = 0,
  .debugMode = kQTMR_RunNormalInDebug
};

void QuadTimer1_init(void) {
  /* Quad timer channel Channel_0 peripheral initialization */
  QTMR_Init(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, &QuadTimer1_Channel_0_config);
  /* Setup the Input capture mode of the timer channel */
  QTMR_SetupInputCapture(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_Counter0InputPin, false, true, kQTMR_RisingEdge);
  /* Enable interrupt requests of the timer channel */
  QTMR_EnableInterrupts(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_EdgeInterruptEnable);
  /* Enable interrupt TMR1_IRQn request in the NVIC */
  EnableIRQ(QUADTIMER1_IRQN);
  /* Start the timer - select the timer counting mode */
  QTMR_StartTimer(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
}

/***********************************************************************************************************************
 * ADC1 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'ADC1'
- type: 'adc_12b1msps_sar'
- mode: 'ADC_GENERAL'
- type_id: 'adc_12b1msps_sar_6a490e886349a7b2b07bed10ce7b299b'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'ADC1'
- config_sets:
  - fsl_adc:
    - clockConfig:
      - clockSource: 'kADC_ClockSourceAD'
      - clockSourceFreq: 'custom:10 MHz'
      - clockDriver: 'kADC_ClockDriver2'
      - samplePeriodMode: 'kADC_SamplePeriodShort2Clocks'
      - enableAsynchronousClockOutput: 'true'
    - conversionConfig:
      - resolution: 'kADC_Resolution12Bit'
      - hardwareAverageMode: 'kADC_HardwareAverageCount32'
      - enableHardwareTrigger: 'hardware'
      - enableHighSpeed: 'false'
      - enableLowPower: 'false'
      - enableContinuousConversion: 'false'
      - enableOverWrite: 'true'
      - enableDma: 'false'
    - resultingTime: []
    - resultCorrection:
      - doAutoCalibration: 'true'
      - offset: '0'
    - hardwareCompareConfiguration:
      - hardwareCompareMode: 'disabled'
      - value1: '0'
      - value2: '0'
    - enableInterrupt: 'false'
    - adc_interrupt:
      - IRQn: 'ADC1_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
    - adc_channels_config:
      - 0:
        - channelNumber: 'CHANNEL_DRIVEN_BY_ADC_ETC'
        - channelName: ''
        - channelGroup: '0'
        - initializeChannel: 'true'
        - enableInterruptOnConversionCompleted: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const adc_config_t ADC1_config = {
  .enableOverWrite = true,
  .enableContinuousConversion = false,
  .enableHighSpeed = false,
  .enableLowPower = false,
  .enableLongSample = false,
  .enableAsynchronousClockOutput = true,
  .referenceVoltageSource = kADC_ReferenceVoltageSourceAlt0,
  .samplePeriodMode = kADC_SamplePeriodShort2Clocks,
  .clockSource = kADC_ClockSourceAD,
  .clockDriver = kADC_ClockDriver2,
  .resolution = kADC_Resolution12Bit
};
const adc_channel_config_t ADC1_channels_config[1] = {
  {
    .channelNumber = ADC1_CHANNEL_DRIVEN_BY_ADC_ETC,
    .enableInterruptOnConversionCompleted = false
  }
};
void ADC1_init(void) {
  /* Initialize ADC1 peripheral. */
  ADC_Init(ADC1_PERIPHERAL, &ADC1_config);
  /* Configure ADC1 peripheral to average 32 conversions in one measurement. */
  ADC_SetHardwareAverageConfig(ADC1_PERIPHERAL, kADC_HardwareAverageCount32);
  /* Perform ADC1 auto calibration. */
  ADC_DoAutoCalibration(ADC1_PERIPHERAL);
  /* Enable ADC1 hardware trigger. */
  ADC_EnableHardwareTrigger(ADC1_PERIPHERAL, true);
  /* Initialize ADC1 channel ADC1_CHANNEL_DRIVEN_BY_ADC_ETC. */
  ADC_SetChannelConfig(ADC1_PERIPHERAL, 0U, &ADC1_channels_config[0]);
}

/***********************************************************************************************************************
 * ADC_ETC initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'ADC_ETC'
- type: 'adc_etc'
- mode: 'ADC_ETC_SW'
- type_id: 'adc_etc_f54f0f22e1814a87101dbc3d3da0f030'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'ADC_ETC'
- config_sets:
  - fsl_adc_etc:
    - config:
      - clockConfig_t:
        - clockSource: 'BusInterfaceClock'
        - clockSourceFreq: 'BOARD_BootClockRUN'
        - clockPreDivider: '1'
      - dmaMode: 'kADC_ETC_TrigDMAWithPulsedSignal'
    - tscConfig:
      - enableTSCBypass: 'false'
      - enableTSC0Trigger: 'false'
      - TSC0triggerPriority: '0'
      - enableTSC1Trigger: 'false'
      - TSC1triggerPriority: '0'
    - triggers_config: []
    - enableInterrupt: 'true'
    - adc_etc_interrupt_err:
      - IRQn: 'ADC_ETC_ERROR_IRQ_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'true'
      - handler_custom_name: 'ADC_ETC_ERROR_IRQHANDLER'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
/* ADC_ETC configuration */
const adc_etc_config_t ADC_ETC_config = {
  .clockPreDivider = 1,
  .dmaMode = kADC_ETC_TrigDMAWithPulsedSignal,
  .enableTSCBypass = false,
  .enableTSC0Trigger = false,
  .enableTSC1Trigger = false,
  .TSC0triggerPriority = 0,
  .TSC1triggerPriority = 0,
  .XBARtriggerMask = 0
};

void ADC_ETC_init(void) {
  /* ADC_ETC initialization */
  ADC_ETC_Init(ADC_ETC_PERIPHERAL, &ADC_ETC_config);
  /* Enable interrupt ADC_ETC_ERROR_IRQ_IRQn request in the NVIC */
  EnableIRQ(ADC_ETC_ERROR_IRQ_IRQn);
}

/***********************************************************************************************************************
 * QuadTimer2 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'QuadTimer2'
- type: 'qtmr'
- mode: 'general'
- type_id: 'qtmr_460dd7aa3f3371843c2548acd54252b0'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'TMR2'
- config_sets:
  - fsl_qtmr:
    - clockSource: 'BusInterfaceClock'
    - clockSourceFreq: 'BOARD_BootClockRUN'
    - qtmr_channels:
      - 0:
        - channel_prefix_id: 'Channel_0'
        - channel: 'kQTMR_Channel_0'
        - primarySource: 'kQTMR_ClockDivide_8'
        - secondarySource: 'kQTMR_Counter0InputPin'
        - countingMode: 'kQTMR_PriSrcRiseEdge'
        - enableMasterMode: 'false'
        - enableExternalForce: 'false'
        - faultFilterCount: '3'
        - faultFilterPeriod: '0'
        - debugMode: 'kQTMR_RunNormalInDebug'
        - timerModeInit: 'timer'
        - timerMode:
          - freq_value_str: '3300'
        - dmaIntMode: 'interrupt'
        - interrupts: 'kQTMR_CompareInterruptEnable'
    - interruptVector:
      - enable_irq: 'true'
      - interrupt:
        - IRQn: 'TMR2_IRQn'
        - enable_priority: 'false'
        - priority: '0'
        - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const qtmr_config_t QuadTimer2_Channel_0_config = {
  .primarySource = kQTMR_ClockDivide_8,
  .secondarySource = kQTMR_Counter0InputPin,
  .enableMasterMode = false,
  .enableExternalForce = false,
  .faultFilterCount = 0,
  .faultFilterPeriod = 0,
  .debugMode = kQTMR_RunNormalInDebug
};

void QuadTimer2_init(void) {
  /* Quad timer channel Channel_0 peripheral initialization */
  QTMR_Init(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL, &QuadTimer2_Channel_0_config);
  /* Setup the timer period of the channel */
  QTMR_SetTimerPeriod(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL, 3300U);
  /* Enable interrupt requests of the timer channel */
  QTMR_EnableInterrupts(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL, kQTMR_CompareInterruptEnable);
  /* Enable interrupt TMR2_IRQn request in the NVIC */
  EnableIRQ(QUADTIMER2_IRQN);
  /* Start the timer - select the timer counting mode */
//  QTMR_StartTimer(QUADTIMER2_PERIPHERAL, QUADTIMER2_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
}

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
  /* Global initialization */
  DMAMUX_Init(EDMA_DMAMUX_BASEADDR);
  EDMA_Init(EDMA_DMA_BASEADDR, &eDMA_config);

  /* Initialize components */
  eDMA_init();
  LPUART1_init();
  PIT1_init();
  LPI2C1_init();
  QuadTimer3_init();
  LPUART2_init();
  LPUART4_init();
  LPUART3_init();
  LPUART5_init();
  LPSPI4_init();
  GPIO2_init();
  QuadTimer1_init();
  ADC1_init();
  ADC_ETC_init();
  QuadTimer2_init();
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
  BOARD_InitPeripherals();
}
