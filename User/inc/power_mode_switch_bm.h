/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _POWER_MODE_SWITCH_H_
#define _POWER_MODE_SWITCH_H_


#include "lpm.h"

extern uint8_t s_wakeupTimeout;            /* Wakeup timeout. (Unit: Second) */

extern const char *s_modeNames[];

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum _app_wakeup_source
{
    kAPP_WakeupSourceGPT, /*!< Wakeup by PIT.        */
    kAPP_WakeupSourcePin, /*!< Wakeup by external pin. */
} app_wakeup_source_t;


uint8_t APP_GetWakeupTimeout(void);
app_wakeup_source_t APP_GetWakeupSource(lpm_power_mode_t targetMode);
void APP_GetWakeupConfig(lpm_power_mode_t targetMode);
void APP_SetWakeupConfig(lpm_power_mode_t targetMode);
lpm_power_mode_t APP_GetRunMode(void);
void APP_SetRunMode(lpm_power_mode_t powerMode);
void APP_ShowPowerMode(lpm_power_mode_t powerMode);
bool APP_CheckPowerMode(lpm_power_mode_t originPowerMode, lpm_power_mode_t targetPowerMode);
void APP_PowerPreSwitchHook(lpm_power_mode_t targetMode);
void APP_PowerPostSwitchHook(lpm_power_mode_t targetMode);
void APP_PowerModeSwitch(lpm_power_mode_t targetPowerMode);



#endif /* _POWER_MODE_SWITCH_H_ */
