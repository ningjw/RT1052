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
#define LPI2C1_CLOCK_FREQ 60000000UL

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
extern const edma_config_t eDMA_config;
extern const lpuart_config_t LPUART1_config;
extern const pit_config_t PIT1_config;
extern const lpi2c_master_config_t LPI2C1_masterConfig;

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
