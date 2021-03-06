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
#include "fsl_snvs_lp.h"
#include "fsl_gpio.h"
#include "fsl_clock.h"
#include "fsl_lpi2c.h"
#include "fsl_lpspi.h"
#include "fsl_lpuart.h"
#include "fsl_pit.h"
#include "fsl_qtmr.h"

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
#define SNVS_LP_PERIPHERAL SNVS
/* GPIO interrupt vector ID (number). */
#define GPIO_GPIO_COMB_16_31_IRQN GPIO2_Combined_16_31_IRQn
/* GPIO interrupt handler identifier. */
#define GPIO_GPIO_COMB_16_31_IRQHANDLER GPIO2_Combined_16_31_IRQHandler
/* BOARD_InitPeripherals defines for LPI2C1 */
/* Definition of peripheral ID */
#define LPI2C1_PERIPHERAL LPI2C1
/* Definition of clock source */
#define LPI2C1_CLOCK_FREQ 24000000UL
/* BOARD_InitPeripherals defines for LPI2C3 */
/* Definition of peripheral ID */
#define LPI2C3_PERIPHERAL LPI2C3
/* Definition of clock source */
#define LPI2C3_CLOCK_FREQ 24000000UL
/* BOARD_InitPeripherals defines for LPSPI4 */
/* Definition of peripheral ID */
#define LPSPI4_PERIPHERAL LPSPI4
/* Definition of clock source */
#define LPSPI4_CLOCK_FREQ 120000000UL
/* Definition of peripheral ID */
#define LPUART2_PERIPHERAL LPUART2
/* Definition of the clock source frequency */
#define LPUART2_CLOCK_SOURCE 24000000UL
/* LPUART2 interrupt vector ID (number). */
#define LPUART2_SERIAL_RX_TX_IRQN LPUART2_IRQn
/* LPUART2 interrupt handler identifier. */
#define LPUART2_SERIAL_RX_TX_IRQHANDLER LPUART2_IRQHandler
/* BOARD_InitPeripherals defines for PIT */
/* Definition of peripheral ID. */
#define PIT1_PERIPHERAL PIT
/* Definition of clock source frequency. */
#define PIT1_CLK_FREQ 24000000UL
/* Definition of ticks count for channel 0 - deprecated. */
#define PIT1_0_TICKS 2399U
/* Definition of ticks count for channel 2 - deprecated. */
#define PIT1_2_TICKS 23999999U
/* Definition of channel number for channel 0. */
#define PIT1_0 kPIT_Chnl_0
/* Definition of channel number for channel 2. */
#define PIT1_2 kPIT_Chnl_2
/* PIT1 interrupt vector ID (number). */
#define PIT1_IRQN PIT_IRQn
/* PIT1 interrupt vector priority. */
#define PIT1_IRQ_PRIORITY 0
/* PIT1 interrupt handler identifier. */
#define PIT1_IRQHANDLER PIT_IRQHandler
/* Definition of peripheral ID */
#define QUADTIMER1_PERIPHERAL TMR1
/* Definition of the timer channel Channel_0. */
#define QUADTIMER1_CHANNEL_0_CHANNEL kQTMR_Channel_0
/* Definition of the timer channel Channel_0 clock source frequency. */
#define QUADTIMER1_CHANNEL_0_CLOCK_SOURCE 234375UL
/* QuadTimer1 interrupt vector ID (number). */
#define QUADTIMER1_IRQN TMR1_IRQn
/* QuadTimer1 interrupt handler identifier. */
#define QUADTIMER1_IRQHANDLER TMR1_IRQHandler
/* Definition of peripheral ID */
#define QUADTIMER2_PERIPHERAL TMR2
/* Definition of the timer channel Channel_0. */
#define QUADTIMER2_CHANNEL_0_CHANNEL kQTMR_Channel_0
/* Definition of the timer channel Channel_0 clock source frequency. */
#define QUADTIMER2_CHANNEL_0_CLOCK_SOURCE 3750000UL
/* QuadTimer2 interrupt vector ID (number). */
#define QUADTIMER2_IRQN TMR2_IRQn
/* QuadTimer2 interrupt handler identifier. */
#define QUADTIMER2_IRQHANDLER TMR2_IRQHandler
/* Definition of peripheral ID */
#define QUADTIMER3_PERIPHERAL TMR3
/* Definition of the timer channel Channel_0. */
#define QUADTIMER3_CHANNEL_0_CHANNEL kQTMR_Channel_0
/* Definition of the timer channel Channel_0 clock source frequency. */
#define QUADTIMER3_CHANNEL_0_CLOCK_SOURCE 30000000UL

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
extern const edma_config_t eDMA_config;
extern const snvs_lp_srtc_config_t SNVS_LP_config;
extern const lpi2c_master_config_t LPI2C1_masterConfig;
extern const lpi2c_master_config_t LPI2C3_masterConfig;
extern const lpspi_master_config_t LPSPI4_config;
extern const lpuart_config_t LPUART2_config;
extern const pit_config_t PIT1_config;
extern const qtmr_config_t QuadTimer1_Channel_0_config;
extern const qtmr_config_t QuadTimer2_Channel_0_config;
extern const qtmr_config_t QuadTimer3_Channel_0_config;

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
