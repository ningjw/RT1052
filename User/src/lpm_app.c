#include "main.h"

#define LPM_GPC_IMR_NUM (sizeof(GPC->IMR) / sizeof(GPC->IMR[0]))
uint8_t s_wakeupTimeout = 2;            /* ���ѳ�ʱ�� ����λ�����룩*/
static uint32_t g_savedPrimask;

/**
 * @brief  LPM���û���Դ
 * @return ��
 *   @retval ��
 */
void LPM_EnableWakeupSource(uint32_t irq)
{
    GPC_EnableIRQ(GPC, irq);
}

/**
 * @brief  LPM�رջ���Դ
 * @return ��
 *   @retval ��
 */
void LPM_DisableWakeupSource(uint32_t irq)
{
    GPC_DisableIRQ(GPC, irq);
}


/**
 * @brief  ����LPM�ȴ�ģʽ
 * @return ��
 *   @retval ��
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
		���ģ�* ERR007265��CCM��ʹ�ò���ȷ�ĵ͹�������ʱ��
      * SoC��ARM�ں�ִ��WFI֮ǰ����͹���ģʽ��
     *
      *������������
      * 1�����Ӧ����IRQ��41��GPR_IRQ��ʼ�չ���
      *ͨ������IOMUXC_GPR_GPR1_GINT��
      * 2�����Ӧ������CCM֮ǰ��GPC��ȡ������IRQ��41
      *�͹���ģʽ��
      * 3�����Ӧ��CCM�͹���ģʽ����������IRQ��41
      *��λ������CCM_CLPCR��0-1λ����
	*/
    GPC_EnableIRQ(GPC, GPR_IRQ_IRQn);
    clpcr      = CCM->CLPCR & (~(CCM_CLPCR_LPM_MASK | CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK));
    CCM->CLPCR = clpcr | CCM_CLPCR_LPM(kCLOCK_ModeWait) | CCM_CLPCR_MASK_SCU_IDLE_MASK | CCM_CLPCR_MASK_L2CC_IDLE_MASK |
                 CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK | CCM_CLPCR_STBY_COUNT_MASK | CCM_CLPCR_BYPASS_LPM_HS0_MASK |
                 CCM_CLPCR_BYPASS_LPM_HS1_MASK;
    GPC_DisableIRQ(GPC, GPR_IRQ_IRQn);
}


/**
 * @brief  ����LPM����ģʽ
 * @return ��
 *   @retval ��
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

    /* ��ʼ��GPC����������IRQ */
    for (int i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        GPC->IMR[i] = 0xFFFFFFFFU;
    }
	
    gpt_config_t gptConfig;
    /* ��ʼGPTģ�����ڻ��� */
    GPT_GetDefaultConfig(&gptConfig);
    gptConfig.clockSource = kGPT_ClockSource_LowFreq; /* 32K RTC OSC */
    gptConfig.enableMode = true;                      /*ֹͣʱ��Ҫ���ּ����� */
    gptConfig.enableRunInDoze = true;
    /* ��ʼ��GPTģ�� */
    GPT_Init(GPT2, &gptConfig);
    GPT_SetClockDivider(GPT2, 1);
	
	GPT_StopTimer(GPT2);
	/* ���±Ƚ�channel1ֵ�����ü����� */
	GPT_SetOutputCompareValue(GPT2, kGPT_OutputCompare_Channel1,
							  (CLOCK_GetRtcFreq()/1000 * s_wakeupTimeout) - 1U);
	/*����GPT���Compare1�ж� */
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

	/* ������ʱ�� */
	GPT_StartTimer(GPT2);
	LPM_EnableWakeupSource(GPT2_IRQn);
	
	g_savedPrimask = DisableGlobalIRQ();
    __DSB();
    __ISB();
	/* ���õȴ�ģʽ���� */
    LPM_SetWaitModeConfig();

    /* DCDC �� 1.15V */
    DCDC_AdjustTargetVoltage(DCDC, 0xe, 0x1);

    /* �Ͽ����ص�����ڲ� */
    DCDC->REG1 &= ~DCDC_REG1_REG_RLOAD_SW_MASK;

    __DSB();
    __WFI();
    __ISB();
	
	/* �����ڲ����ص��� */
    DCDC->REG1 |= DCDC_REG1_REG_RLOAD_SW_MASK;
    /*��SOC��ѹ����Ϊ1.275V */
    DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);

    LPM_SetRunModeConfig();
	
	EnableGlobalIRQ(g_savedPrimask);
    __DSB();
    __ISB();
}


/**
 * @brief GPT �жϷ�����
 * @return ��
 *   @retval ��
 */
void GPT2_IRQHandler(void)
{
    /* ���GPT��־ */
    GPT_ClearStatusFlags(GPT2, kGPT_OutputCompare1Flag);
    /* ֹͣGPT��ʱ�� */
    GPT_StopTimer(GPT2);
    /* LPM�رջ���Դ */
    LPM_DisableWakeupSource(GPT2_IRQn);
}


