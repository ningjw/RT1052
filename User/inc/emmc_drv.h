#ifndef __EMMC_DRV_H
#define __EMMC_DRV_H

#include "clock_config.h"
#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_clock.h"

#define BOARD_USDHC1_BASEADDR USDHC1

#define BOARD_USDHC_CD_GPIO_BASE GPIO2
#define BOARD_USDHC_CD_GPIO_PIN 28
#define BOARD_USDHC_CD_PORT_IRQ GPIO2_Combined_16_31_IRQn
#define BOARD_USDHC_CD_PORT_IRQ_HANDLER GPIO2_Combined_16_31_IRQHandler

#define BOARD_USDHC_CD_STATUS() (GPIO_PinRead(BOARD_USDHC_CD_GPIO_BASE, BOARD_USDHC_CD_GPIO_PIN))

#define BOARD_USDHC_CD_INTERRUPT_STATUS() (GPIO_PortGetInterruptFlags(BOARD_USDHC_CD_GPIO_BASE))
#define BOARD_USDHC_CD_CLEAR_INTERRUPT(flag) (GPIO_PortClearInterruptFlags(BOARD_USDHC_CD_GPIO_BASE, flag))

#define BOARD_USDHC_CD_GPIO_INIT()                                                          \
    {                                                                                       \
        gpio_pin_config_t sw_config = {                                                     \
            kGPIO_DigitalInput,                                                             \
            0,                                                                              \
            kGPIO_IntRisingOrFallingEdge,                                                   \
        };                                                                                  \
        GPIO_PinInit(BOARD_USDHC_CD_GPIO_BASE, BOARD_USDHC_CD_GPIO_PIN, &sw_config);        \
        GPIO_PortEnableInterrupts(BOARD_USDHC_CD_GPIO_BASE, 1U << BOARD_USDHC_CD_GPIO_PIN); \
        GPIO_PortClearInterruptFlags(BOARD_USDHC_CD_GPIO_BASE, ~0);                         \
    }
#define BOARD_HAS_SDCARD (0U)
#define BOARD_SD_POWER_RESET_GPIO (GPIO1)
#define BOARD_SD_POWER_RESET_GPIO_PIN (5U)

#define BOARD_USDHC_CARD_INSERT_CD_LEVEL (0U)

#define BOARD_USDHC_MMCCARD_POWER_CONTROL(state)

#define BOARD_USDHC_MMCCARD_POWER_CONTROL_INIT()                                            \
    {                                                                                       \
        gpio_pin_config_t sw_config = {                                                     \
            kGPIO_DigitalOutput,                                                            \
            0,                                                                              \
            kGPIO_NoIntmode,                                                                \
        };                                                                                  \
        GPIO_PinInit(BOARD_SD_POWER_RESET_GPIO, BOARD_SD_POWER_RESET_GPIO_PIN, &sw_config); \
        GPIO_PinWrite(BOARD_SD_POWER_RESET_GPIO, BOARD_SD_POWER_RESET_GPIO_PIN, true);      \
    }

#define BOARD_USDHC_SDCARD_POWER_CONTROL_INIT()                                             \
    {                                                                                       \
        gpio_pin_config_t sw_config = {                                                     \
            kGPIO_DigitalOutput,                                                            \
            0,                                                                              \
            kGPIO_NoIntmode,                                                                \
        };                                                                                  \
        GPIO_PinInit(BOARD_SD_POWER_RESET_GPIO, BOARD_SD_POWER_RESET_GPIO_PIN, &sw_config); \
    }

#define BOARD_USDHC_SDCARD_POWER_CONTROL(state) \
    (GPIO_PinWrite(BOARD_SD_POWER_RESET_GPIO, BOARD_SD_POWER_RESET_GPIO_PIN, state))

#define BOARD_USDHC1_CLK_FREQ (CLOCK_GetSysPfdFreq(kCLOCK_Pfd0) / (CLOCK_GetDiv(kCLOCK_Usdhc1Div) + 1U))

#define BOARD_MMC_HOST_BASEADDR BOARD_USDHC1_BASEADDR
#define BOARD_MMC_HOST_CLK_FREQ BOARD_USDHC1_CLK_FREQ
#define BOARD_MMC_HOST_IRQ      USDHC1_IRQn
#define BOARD_MMC_VCCQ_SUPPLY   kMMC_VoltageWindow170to195
#define BOARD_MMC_VCC_SUPPLY    kMMC_VoltageWindows270to360

/* we are using the BB SD socket to DEMO the MMC example,but the
* SD socket provide 4bit bus only, so we define this macro to avoid
* 8bit data bus test
*/
#define BOARD_MMC_SUPPORT_8BIT_BUS (0U)
#define BOARD_SD_HOST_SUPPORT_SDR104_FREQ (200000000U)
#define BOARD_SD_HOST_SUPPORT_HS200_FREQ (180000000U)
    
void BOARD_MMC_Pin_Config(uint32_t speed, uint32_t strength);

#endif
