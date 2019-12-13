#ifndef __MAIN_H


#define __MAIN_H

#include "stdint.h"
#include "string.h"

#include "clock_config.h"
#include "peripherals.h"
#include "pin_mux.h"

#include "fsl_gpio.h"
#include "fsl_lpuart.h"
#include "fsl_debug_console.h"
#include "fsl_snvs_hp.h"
#include "fsl_iomuxc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "task_app.h"

#include "cJSON.h"
#include "interrupt.h"
#include "lpuart1.h"
#include "i2c_eeprom_drv.h"
#include "rtc_drv.h"
#include "emmc_drv.h"
#include "adc_drv.h"
#include "battery_drv.h"

#include "ble_app.h"
#include "adc_app.h"
#include "battery_app.h"

#define VERSION "V0.01"

#endif
