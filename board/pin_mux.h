/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

/*! @brief Direction type  */
typedef enum _pin_mux_direction
{
  kPIN_MUX_DirectionInput = 0U,         /* Input direction */
  kPIN_MUX_DirectionOutput = 1U,        /* Output direction */
  kPIN_MUX_DirectionInputOrOutput = 2U  /* Input or output direction */
} pin_mux_direction_t;

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

/* GPIO_AD_B0_09 (coord F14), LED */
#define BOARD_LED_GPIO                                                     GPIO1   /*!< GPIO device name: GPIO1 */
#define BOARD_LED_PORT                                                     GPIO1   /*!< PORT device name: GPIO1 */
#define BOARD_LED_PIN                                                         9U   /*!< GPIO1 pin index: 9 */

/* GPIO_B1_03 (coord D11), RST_4G */
#define BOARD_RST_4G_GPIO                                                  GPIO2   /*!< GPIO device name: GPIO2 */
#define BOARD_RST_4G_PORT                                                  GPIO2   /*!< PORT device name: GPIO2 */
#define BOARD_RST_4G_PIN                                                     19U   /*!< GPIO2 pin index: 19 */

/* GPIO_B1_02 (coord C11), PWR_4G */
#define BOARD_PWR_4G_GPIO                                                  GPIO2   /*!< GPIO device name: GPIO2 */
#define BOARD_PWR_4G_PORT                                                  GPIO2   /*!< PORT device name: GPIO2 */
#define BOARD_PWR_4G_PIN                                                     18U   /*!< GPIO2 pin index: 18 */

/* GPIO_B0_11 (coord A10), ADC_MODE */
#define BOARD_ADC_MODE_GPIO                                                GPIO2   /*!< GPIO device name: GPIO2 */
#define BOARD_ADC_MODE_PORT                                                GPIO2   /*!< PORT device name: GPIO2 */
#define BOARD_ADC_MODE_PIN                                                   11U   /*!< GPIO2 pin index: 11 */

/* GPIO_B0_15 (coord E11), ADC_FORMAT */
#define BOARD_ADC_FORMAT_GPIO                                              GPIO2   /*!< GPIO device name: GPIO2 */
#define BOARD_ADC_FORMAT_PORT                                              GPIO2   /*!< PORT device name: GPIO2 */
#define BOARD_ADC_FORMAT_PIN                                                 15U   /*!< GPIO2 pin index: 15 */

/* GPIO_B0_14 (coord E10), ADC_SYNC */
#define BOARD_ADC_SYNC_GPIO                                                GPIO2   /*!< GPIO device name: GPIO2 */
#define BOARD_ADC_SYNC_PORT                                                GPIO2   /*!< PORT device name: GPIO2 */
#define BOARD_ADC_SYNC_PIN                                                   14U   /*!< GPIO2 pin index: 14 */

/* GPIO_B0_04 (coord C8), ADC_RDY */
#define BOARD_ADC_RDY_GPIO                                                 GPIO2   /*!< GPIO device name: GPIO2 */
#define BOARD_ADC_RDY_PORT                                                 GPIO2   /*!< PORT device name: GPIO2 */
#define BOARD_ADC_RDY_PIN                                                     4U   /*!< GPIO2 pin index: 4 */

/* GPIO_B1_14 (coord C14), eMMC_EN */
#define BOARD_eMMC_EN_GPIO                                                 GPIO2   /*!< GPIO device name: GPIO2 */
#define BOARD_eMMC_EN_PORT                                                 GPIO2   /*!< PORT device name: GPIO2 */
#define BOARD_eMMC_EN_PIN                                                    30U   /*!< GPIO2 pin index: 30 */

/* GPIO_AD_B0_04 (coord F11), BOOT_MODE0 */
#define BOARD_BOOT_MODE0_PERIPHERAL                                          SRC   /*!< Device name: SRC */
#define BOARD_BOOT_MODE0_SIGNAL                                        BOOT_MODE   /*!< SRC signal: BOOT_MODE */
#define BOARD_BOOT_MODE0_CHANNEL                                              0U   /*!< SRC BOOT_MODE channel: 0 */

/* GPIO_AD_B0_05 (coord G14), CORE_BOARD_MODE_KEY */
#define BOARD_BOOT_MODE1_PERIPHERAL                                          SRC   /*!< Device name: SRC */
#define BOARD_BOOT_MODE1_SIGNAL                                        BOOT_MODE   /*!< SRC signal: BOOT_MODE */
#define BOARD_BOOT_MODE1_CHANNEL                                              1U   /*!< SRC BOOT_MODE channel: 1 */

/* GPIO_AD_B0_14 (coord H14), E103_RST */
#define BOARD_E103_RST_GPIO                                                GPIO1   /*!< GPIO device name: GPIO1 */
#define BOARD_E103_RST_PORT                                                GPIO1   /*!< PORT device name: GPIO1 */
#define BOARD_E103_RST_PIN                                                   14U   /*!< GPIO1 pin index: 14 */

/* GPIO_AD_B1_05 (coord K12), E103_PWR_EN */
#define BOARD_E103_PWR_EN_GPIO                                             GPIO1   /*!< GPIO device name: GPIO1 */
#define BOARD_E103_PWR_EN_PORT                                             GPIO1   /*!< PORT device name: GPIO1 */
#define BOARD_E103_PWR_EN_PIN                                                21U   /*!< GPIO1 pin index: 21 */

/* GPIO_AD_B0_11 (coord G10), E103_SETDF */
#define BOARD_E103_SETDF_GPIO                                              GPIO1   /*!< GPIO device name: GPIO1 */
#define BOARD_E103_SETDF_PORT                                              GPIO1   /*!< PORT device name: GPIO1 */
#define BOARD_E103_SETDF_PIN                                                 11U   /*!< GPIO1 pin index: 11 */

/* GPIO_AD_B1_04 (coord L12), BTM_EN */
#define BOARD_BTM_EN_GPIO                                                  GPIO1   /*!< GPIO device name: GPIO1 */
#define BOARD_BTM_EN_PORT                                                  GPIO1   /*!< PORT device name: GPIO1 */
#define BOARD_BTM_EN_PIN                                                     20U   /*!< GPIO1 pin index: 20 */

/* GPIO_AD_B0_10 (coord G13), BTM_MODE */
#define BOARD_BTM_MODE_GPIO                                                GPIO1   /*!< GPIO device name: GPIO1 */
#define BOARD_BTM_MODE_PORT                                                GPIO1   /*!< PORT device name: GPIO1 */
#define BOARD_BTM_MODE_PIN                                                   10U   /*!< GPIO1 pin index: 10 */


/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD(void);

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
