#include "main.h"

#define LPM_GPC_IMR_NUM (sizeof(GPC->IMR) / sizeof(GPC->IMR[0]))
uint8_t s_wakeupTimeout = 2;            /* 唤醒超时。 （单位：毫秒）*/
static uint32_t g_savedPrimask;

/**
 * @brief  LPM启用唤醒源
 * @return 无
 *   @retval 无
 */
void LPM_EnableWakeupSource(uint32_t irq)
{
    GPC_EnableIRQ(GPC, irq);
}

/**
 * @brief  LPM关闭唤醒源
 * @return 无
 *   @retval 无
 */
void LPM_DisableWakeupSource(uint32_t irq)
{
    GPC_DisableIRQ(GPC, irq);
}


/**
 * @brief  配置LPM等待模式
 * @return 无
 *   @retval 无
 */
void LPM_SetWaitModeConfig(void)
{
    uint32_t clpcr;

    /*
     * ERR007265: CCM: When improper low-power sequence is used,
     * the SoC enters low power mode before the ARM core executes WFI.
     *
     * Software workaround:
     * 1) Software should trigger IRQ #41 (GPR_IRQ) to be always pending
     *      by setting IOMUXC_GPR_GPR1_GINT.
     * 2) Software should then unmask IRQ #41 in GPC before setting CCM
     *      Low-Power mode.
     * 3) Software should mask IRQ #41 right after CCM Low-Power mode
     *      is set (set bits 0-1 of CCM_CLPCR).
     */
	/*
		译文：* ERR007265：CCM：使用不正确的低功率序列时，
      * SoC在ARM内核执行WFI之前进入低功耗模式。
     *
      *软件解决方法：
      * 1）软件应触发IRQ＃41（GPR_IRQ）始终挂起
      *通过设置IOMUXC_GPR_GPR1_GINT。
      * 2）软件应在设置CCM之前在GPC中取消屏蔽IRQ＃41
      *低功耗模式。
      * 3）软件应在CCM低功耗模式后立即屏蔽IRQ＃41
      *置位（设置CCM_CLPCR的0-1位）。
	*/
    GPC_EnableIRQ(GPC, GPR_IRQ_IRQn);
    clpcr      = CCM->CLPCR & (~(CCM_CLPCR_LPM_MASK | CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK));
    CCM->CLPCR = clpcr | CCM_CLPCR_LPM(kCLOCK_ModeWait) | CCM_CLPCR_MASK_SCU_IDLE_MASK | CCM_CLPCR_MASK_L2CC_IDLE_MASK |
                 CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK | CCM_CLPCR_STBY_COUNT_MASK | CCM_CLPCR_BYPASS_LPM_HS0_MASK |
                 CCM_CLPCR_BYPASS_LPM_HS1_MASK;
    GPC_DisableIRQ(GPC, GPR_IRQ_IRQn);
}


/**
 * @brief  配置LPM运行模式
 * @return 无
 *   @retval 无
 */
void LPM_SetRunModeConfig(void)
{
    CCM->CLPCR &= ~(CCM_CLPCR_LPM_MASK | CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK);
}

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void LPM_Init(void)
{
	/* ERR007265 */
    IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_GINT_MASK;

    /* 初始化GPC以屏蔽所有IRQ */
    for (int i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        GPC->IMR[i] = 0xFFFFFFFFU;
    }
	
    gpt_config_t gptConfig;
    /* 初始GPT模块用于唤醒 */
    GPT_GetDefaultConfig(&gptConfig);
    gptConfig.clockSource = kGPT_ClockSource_LowFreq; /* 32K RTC OSC */
    gptConfig.enableMode = true;                      /*停止时不要保持计数器 */
    gptConfig.enableRunInDoze = true;
    /* 初始化GPT模块 */
    GPT_Init(GPT2, &gptConfig);
    GPT_SetClockDivider(GPT2, 1);
	
	GPT_StopTimer(GPT2);
	/* 更新比较channel1值将重置计数器 */
	GPT_SetOutputCompareValue(GPT2, kGPT_OutputCompare_Channel1,
							  (CLOCK_GetRtcFreq()/1000 * s_wakeupTimeout) - 1U);
	/*启用GPT输出Compare1中断 */
	GPT_EnableInterrupts(GPT2, kGPT_OutputCompare1InterruptEnable);
	NVIC_EnableIRQ(GPT2_IRQn);
}


/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void LPM_EnterSystemIdle(void)
{
	EnableIRQ(GPT2_IRQn);

	/* 重启计时器 */
	GPT_StartTimer(GPT2);
	LPM_EnableWakeupSource(GPT2_IRQn);
	
	g_savedPrimask = DisableGlobalIRQ();
    __DSB();
    __ISB();
	/* 设置等待模式配置 */
    LPM_SetWaitModeConfig();

    /* DCDC 到 1.15V */
    DCDC_AdjustTargetVoltage(DCDC, 0xe, 0x1);

    /* 断开负载电阻的内部 */
    DCDC->REG1 &= ~DCDC_REG1_REG_RLOAD_SW_MASK;

    __DSB();
    __WFI();
    __ISB();
	
	/* 连接内部负载电阻 */
    DCDC->REG1 |= DCDC_REG1_REG_RLOAD_SW_MASK;
    /*将SOC电压调整为1.275V */
    DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);

    LPM_SetRunModeConfig();
	
	EnableGlobalIRQ(g_savedPrimask);
    __DSB();
    __ISB();
}


/**
 * @brief GPT 中断服务函数
 * @return 无
 *   @retval 无
 */
void GPT2_IRQHandler(void)
{
    /* 清除GPT标志 */
    GPT_ClearStatusFlags(GPT2, kGPT_OutputCompare1Flag);
    /* 停止GPT计时器 */
    GPT_StopTimer(GPT2);
    /* LPM关闭唤醒源 */
    LPM_DisableWakeupSource(GPT2_IRQn);
}


