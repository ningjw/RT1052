#ifndef __LPM_APP_H
#define __LPM_APP_H

/* 低功耗管理的功耗模式定义。
 * 唤醒持续时间关闭> Dsm>空闲>等待>运行。
 */
typedef enum _lpm_power_mode
{
    LPM_PowerModeOverRun = 0, /* 超载运行模式 */
    LPM_PowerModeFullRun, /* 满载运行模式*/
    LPM_PowerModeLowSpeedRun,/* 低速运行模式 */
    LPM_PowerModeLowPowerRun,/* 低功耗运行模式 */
    LPM_PowerModeRunEnd = LPM_PowerModeLowPowerRun,
		/*
			译文：在系统等待模式下，cpu时钟被门控。
      *所有外设都可以保持活动状态，时钟门控由CCGR设置决定。
      *当没有访问权限时，DRAM进入自动刷新模式。
		*/
    LPM_PowerModeSysIdle, /* 系统空闲模式 */
		 /*
			译文：在低功耗空闲模式下，所有PLL / PFD都关闭，cpu电源关闭。
      *模拟模块在低功耗模式下运行。
      *所有高速外设都是电源门控
      *低速外设可以保持低频运行
      * DRAM自刷新。
		 */
    LPM_PowerModeLPIdle, /* 低功耗空闲模式 */
		 /*
		 译文：在深度睡眠模式下，所有PLL / PFD都关闭，XTAL关闭，cpu电源关闭。
      *除32K RTC时钟外，所有时钟均关闭
      *所有高速外设都是电源门控
      *低速外设是时钟门控的
      * DRAM自刷新。
      *如果使用RTOS，将在DSM中禁用systick
		 */
    LPM_PowerModeSuspend, /* 暂停模式 */
    LPM_PowerModeSNVS, /*电源关闭模式或关机模式 */
    LPM_PowerModeEnd = LPM_PowerModeSNVS/*枚举结尾 */
} lpm_power_mode_t;

void LPM_LowPowerRun(void);
void LPM_FullSpeedRun(void);


#endif
