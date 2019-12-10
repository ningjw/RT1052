/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "fsl_edma.h"
#include "fsl_dmamux.h"
#include "fsl_common.h"
#include "fsl_lpuart.h"
#include "fsl_clock.h"
#include "fsl_pit.h"
#include "fsl_lpi2c.h"
#include "fsl_qtmr.h"
#include "fsl_lpuart_edma.h"
#include "fsl_lpspi.h"
#include "fsl_gpio.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
/* Definitions for BOARD_InitPeripherals functional group */
/* Used DMA device. */
#define EDMA_DMA_BASEADDR DMA0
/* Associated DMAMUX device that is used for muxing of requests. */
#define EDMA_DMAMUX_BASEADDR DMAMUX
/* Definition of peripheral ID */
#define LPUART1_PERIPHERAL LPUART1
/* Definition of the clock source frequency */
#define LPUART1_CLOCK_SOURCE 80000000UL
/* LPUART1 interrupt vector ID (number). */
#define LPUART1_SERIAL_RX_TX_IRQN LPUART1_IRQn
/* LPUART1 interrupt handler identifier. */
#define LPUART1_SERIAL_RX_TX_IRQHANDLER LPUART1_IRQHandler
/* BOARD_InitPeripherals defines for PIT */
/* Definition of peripheral ID. */
#define PIT1_PERIPHERAL PIT
/* Definition of clock source frequency. */
#define PIT1_CLK_FREQ 66000000UL
/* Definition of ticks count for channel 0. */
#define PIT1_0_TICKS 65999U
/* Definition of ticks count for channel 1. */
#define PIT1_1_TICKS 65999999U
/* PIT1 interrupt vector ID (number). */
#define PIT1_IRQN PIT_IRQn
/* PIT1 interrupt vector priority. */
#define PIT1_IRQ_PRIORITY 5
/* PIT1 interrupt handler identifier. */
#define PIT1_IRQHANDLER PIT_IRQHandler
/* BOARD_InitPeripherals defines for LPI2C1 */
/* Definition of peripheral ID */
#define LPI2C1_PERIPHERAL LPI2C1
/* Definition of clock source */
#define LPI2C1_CLOCK_FREQ 3000000UL
/* Definition of peripheral ID */
#define QUADTIMER3_PERIPHERAL TMR3
/* Definition of the timer channel Channel_0. */
#define QUADTIMER3_CHANNEL_0_CHANNEL kQTMR_Channel_0
/* Definition of the timer channel Channel_0 clock source frequency. */
#define QUADTIMER3_CHANNEL_0_CLOCK_SOURCE 33000000UL
/* Definition of peripheral ID */
#define LPUART2_PERIPHERAL LPUART2
/* Definition of the clock source frequency */
#define LPUART2_CLOCK_SOURCE 80000000UL
/* LPUART2 eDMA source request. */
#define LPUART2_RX_DMA_REQUEST kDmaRequestMuxLPUART2Rx
/* Selected eDMA channel number. */
#define LPUART2_RX_DMA_CHANNEL 0
/* DMAMUX device that is used for muxing of the request. */
#define LPUART2_RX_DMAMUX_BASEADDR DMAMUX
/* Used DMA device. */
#define LPUART2_RX_DMA_BASEADDR DMA0
/* Definition of peripheral ID */
#define LPUART4_PERIPHERAL LPUART4
/* Definition of the clock source frequency */
#define LPUART4_CLOCK_SOURCE 80000000UL
/* LPUART4 interrupt vector ID (number). */
#define LPUART4_SERIAL_RX_TX_IRQN LPUART4_IRQn
/* LPUART4 interrupt handler identifier. */
#define LPUART4_SERIAL_RX_TX_IRQHANDLER LPUART4_IRQHandler
/* Definition of peripheral ID */
#define LPUART3_PERIPHERAL LPUART3
/* Definition of the clock source frequency */
#define LPUART3_CLOCK_SOURCE 80000000UL
/* LPUART3 interrupt vector ID (number). */
#define LPUART3_SERIAL_RX_TX_IRQN LPUART3_IRQn
/* LPUART3 interrupt handler identifier. */
#define LPUART3_SERIAL_RX_TX_IRQHANDLER LPUART3_IRQHandler
/* Definition of peripheral ID */
#define LPUART5_PERIPHERAL LPUART5
/* Definition of the clock source frequency */
#define LPUART5_CLOCK_SOURCE 80000000UL
/* LPUART5 interrupt vector ID (number). */
#define LPUART5_SERIAL_RX_TX_IRQN LPUART5_IRQn
/* LPUART5 interrupt handler identifier. */
#define LPUART5_SERIAL_RX_TX_IRQHANDLER LPUART5_IRQHandler
/* BOARD_InitPeripherals defines for LPSPI4 */
/* Definition of peripheral ID */
#define LPSPI4_PERIPHERAL LPSPI4
/* Definition of clock source */
#define LPSPI4_CLOCK_FREQ 24000000UL
/* GPIO2 interrupt vector ID (number). */
#define GPIO2_GPIO_COMB_0_15_IRQN GPIO2_Combined_0_15_IRQn
/* GPIO2 interrupt handler identifier. */
#define GPIO2_GPIO_COMB_0_15_IRQHANDLER GPIO2_Combined_0_15_IRQHandler
/* GPIO2 interrupt vector ID (number). */
#define GPIO2_GPIO_COMB_16_31_IRQN GPIO2_Combined_16_31_IRQn
/* GPIO2 interrupt handler identifier. */
#define GPIO2_GPIO_COMB_16_31_IRQHANDLER GPIO2_Combined_16_31_IRQHandler
/* BOARD_InitPeripherals defines for LPI2C3 */
/* Definition of peripheral ID */
#define LPI2C3_PERIPHERAL LPI2C3
/* Definition of clock source */
#define LPI2C3_CLOCK_FREQ 3000000UL
/* Definition of peripheral ID */
#define QUADTIMER1_PERIPHERAL TMR1
/* Definition of the timer channel Channel_0. */
#define QUADTIMER1_CHANNEL_0_CHANNEL kQTMR_Channel_0
/* Definition of the timer channel Channel_0 clock source frequency. */
#define QUADTIMER1_CHANNEL_0_CLOCK_SOURCE 66000000UL

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
extern const edma_config_t eDMA_config;
extern const lpuart_config_t LPUART1_config;
extern const pit_config_t PIT1_config;
extern const lpi2c_master_config_t LPI2C1_masterConfig;
extern const qtmr_config_t QuadTimer3_Channel_0_config;
extern const lpuart_config_t LPUART2_config;
extern edma_handle_t LPUART2_RX_Handle;
extern lpuart_edma_handle_t LPUART2_eDMA_Handle;
extern const lpuart_config_t LPUART4_config;
extern const lpuart_config_t LPUART3_config;
extern const lpuart_config_t LPUART5_config;
extern const lpspi_master_config_t LPSPI4_config;
extern const lpi2c_master_config_t LPI2C3_masterConfig;
extern const qtmr_config_t QuadTimer1_Channel_0_config;

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void);

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void);

#if defined(__cplusplus)
}
#endif

#endif /* _PERIPHERALS_H_ */
