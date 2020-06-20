/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _POWER_MODE_SWITCH_H_
#define _POWER_MODE_SWITCH_H_

#include "fsl_common.h"

#define APP_WAKEUP_BUTTON_GPIO     BOARD_KEY_OFF_GPIO
#define APP_WAKEUP_BUTTON_GPIO_PIN BOARD_KEY_OFF_PIN
#define APP_WAKEUP_BUTTON_IRQ      GPIO2_Combined_16_31_IRQn

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum _app_wakeup_source
{
    kAPP_WakeupSourceGPT, /*!< Wakeup by PIT.        */
    kAPP_WakeupSourcePin, /*!< Wakeup by external pin. */
} app_wakeup_source_t;

void APP_PowerModeChange(char  powerMode);
	
#endif /* _POWER_MODE_SWITCH_H_ */
