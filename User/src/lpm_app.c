#include "main.h"

#define APP_WAKEUP_BUTTON_GPIO        GPIO1
#define APP_WAKEUP_BUTTON_GPIO_PIN    19U
#define APP_WAKEUP_BUTTON_IRQ         GPIO1_Combined_16_31_IRQn
#define APP_WAKEUP_BUTTON_IRQ_HANDLER GPIO1_Combined_16_31_IRQnHandler

#define GPR4_STOP_REQ_BITS                                                                                          \
    (IOMUXC_GPR_GPR4_ENET_STOP_REQ_MASK | IOMUXC_GPR_GPR4_SAI1_STOP_REQ_MASK | IOMUXC_GPR_GPR4_SAI2_STOP_REQ_MASK | \
     IOMUXC_GPR_GPR4_SAI3_STOP_REQ_MASK | IOMUXC_GPR_GPR4_SEMC_STOP_REQ_MASK | IOMUXC_GPR_GPR4_PIT_STOP_REQ_MASK |  \
     IOMUXC_GPR_GPR4_FLEXIO1_STOP_REQ_MASK | IOMUXC_GPR_GPR4_FLEXIO2_STOP_REQ_MASK)

#define GPR4_STOP_ACK_BITS                                                                                          \
    (IOMUXC_GPR_GPR4_ENET_STOP_ACK_MASK | IOMUXC_GPR_GPR4_SAI1_STOP_ACK_MASK | IOMUXC_GPR_GPR4_SAI2_STOP_ACK_MASK | \
     IOMUXC_GPR_GPR4_SAI3_STOP_ACK_MASK | IOMUXC_GPR_GPR4_SEMC_STOP_ACK_MASK | IOMUXC_GPR_GPR4_PIT_STOP_ACK_MASK |  \
     IOMUXC_GPR_GPR4_FLEXIO1_STOP_ACK_MASK | IOMUXC_GPR_GPR4_FLEXIO2_STOP_ACK_MASK)

#define GPR7_STOP_ACK_BITS                                                           \
    (IOMUXC_GPR_GPR7_LPI2C1_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPI2C2_STOP_ACK_MASK |   \
     IOMUXC_GPR_GPR7_LPI2C3_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPI2C4_STOP_ACK_MASK |   \
     IOMUXC_GPR_GPR7_LPSPI1_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPSPI2_STOP_ACK_MASK |   \
     IOMUXC_GPR_GPR7_LPSPI3_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPSPI4_STOP_ACK_MASK |   \
     IOMUXC_GPR_GPR7_LPUART1_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPUART2_STOP_ACK_MASK | \
     IOMUXC_GPR_GPR7_LPUART3_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPUART4_STOP_ACK_MASK | \
     IOMUXC_GPR_GPR7_LPUART5_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPUART6_STOP_ACK_MASK | \
     IOMUXC_GPR_GPR7_LPUART7_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPUART8_STOP_ACK_MASK)

#define GPR7_STOP_REQ_BITS                                                           \
    (IOMUXC_GPR_GPR7_LPI2C1_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPI2C2_STOP_REQ_MASK |   \
     IOMUXC_GPR_GPR7_LPI2C3_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPI2C4_STOP_REQ_MASK |   \
     IOMUXC_GPR_GPR7_LPSPI1_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPSPI2_STOP_REQ_MASK |   \
     IOMUXC_GPR_GPR7_LPSPI3_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPSPI4_STOP_REQ_MASK |   \
     IOMUXC_GPR_GPR7_LPUART1_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPUART2_STOP_REQ_MASK | \
     IOMUXC_GPR_GPR7_LPUART3_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPUART4_STOP_REQ_MASK | \
     IOMUXC_GPR_GPR7_LPUART5_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPUART6_STOP_REQ_MASK | \
     IOMUXC_GPR_GPR7_LPUART7_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPUART8_STOP_REQ_MASK)

#define GPR8_DOZE_BITS                                                               \
    (IOMUXC_GPR_GPR8_LPI2C1_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPI2C2_IPG_DOZE_MASK |   \
     IOMUXC_GPR_GPR8_LPI2C3_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPI2C4_IPG_DOZE_MASK |   \
     IOMUXC_GPR_GPR8_LPSPI1_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPSPI2_IPG_DOZE_MASK |   \
     IOMUXC_GPR_GPR8_LPSPI3_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPSPI4_IPG_DOZE_MASK |   \
     IOMUXC_GPR_GPR8_LPUART1_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPUART2_IPG_DOZE_MASK | \
     IOMUXC_GPR_GPR8_LPUART3_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPUART4_IPG_DOZE_MASK | \
     IOMUXC_GPR_GPR8_LPUART5_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPUART6_IPG_DOZE_MASK | \
     IOMUXC_GPR_GPR8_LPUART7_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPUART8_IPG_DOZE_MASK)
	 
#define GPR8_STOP_MODE_BITS                                                                    \
    (IOMUXC_GPR_GPR8_LPI2C1_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPI2C2_IPG_STOP_MODE_MASK |   \
     IOMUXC_GPR_GPR8_LPI2C3_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPI2C4_IPG_STOP_MODE_MASK |   \
     IOMUXC_GPR_GPR8_LPSPI1_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPSPI2_IPG_STOP_MODE_MASK |   \
     IOMUXC_GPR_GPR8_LPSPI3_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPSPI4_IPG_STOP_MODE_MASK |   \
     IOMUXC_GPR_GPR8_LPUART2_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPUART3_IPG_STOP_MODE_MASK | \
     IOMUXC_GPR_GPR8_LPUART4_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPUART5_IPG_STOP_MODE_MASK | \
     IOMUXC_GPR_GPR8_LPUART6_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPUART7_IPG_STOP_MODE_MASK | \
     IOMUXC_GPR_GPR8_LPUART8_IPG_STOP_MODE_MASK)


#define GPR12_DOZE_BITS (IOMUXC_GPR_GPR12_FLEXIO1_IPG_DOZE_MASK | IOMUXC_GPR_GPR12_FLEXIO2_IPG_DOZE_MASK)

#define GPR12_STOP_MODE_BITS (IOMUXC_GPR_GPR12_FLEXIO1_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR12_FLEXIO2_IPG_STOP_MODE_MASK)

#define LPM_GPC_IMR_NUM (sizeof(GPC->IMR) / sizeof(GPC->IMR[0]))

#define CLOCK_CCM_HANDSHAKE_WAIT() \
                                   \
    do                             \
    {                              \
        while (CCM->CDHIPR != 0)   \
        {                          \
        }                          \
                                   \
    } while (0)
	
	
static uint32_t g_savedPrimask;

/**
 * @brief  LPM����ȴ�ģʽ
 * @return ��
 *   @retval ��
 */
void LPM_PreEnterWaitMode(void)
{
    g_savedPrimask = DisableGlobalIRQ();
    __DSB();
    __ISB();
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
 * @brief ���õ͹���ʱ����
 * @return ��
 *   @retval ��
 */
void SetLowPowerClockGate(void)
{
    CCM->CCGR0 = CCM_CCGR0_CG0(1) | CCM_CCGR0_CG1(1) | CCM_CCGR0_CG3(3) | CCM_CCGR0_CG11(1) | CCM_CCGR0_CG12(1);
    CCM->CCGR1 = CCM_CCGR1_CG9(3) | CCM_CCGR1_CG10(1) | CCM_CCGR1_CG13(1) | CCM_CCGR1_CG14(1) | CCM_CCGR1_CG15(1);
    CCM->CCGR2 = CCM_CCGR2_CG2(1) | CCM_CCGR2_CG8(1) | CCM_CCGR2_CG9(1) | CCM_CCGR2_CG10(1);
    CCM->CCGR3 = CCM_CCGR3_CG2(1) | CCM_CCGR3_CG4(1) | CCM_CCGR3_CG9(1) | CCM_CCGR3_CG14(1) | CCM_CCGR3_CG15(1);
    CCM->CCGR4 =
        CCM_CCGR4_CG1(1) | CCM_CCGR4_CG2(1) | CCM_CCGR4_CG4(1) | CCM_CCGR4_CG5(1) | CCM_CCGR4_CG6(1) | CCM_CCGR4_CG7(1);
    CCM->CCGR5 = CCM_CCGR5_CG0(1) | CCM_CCGR5_CG1(1) | CCM_CCGR5_CG4(1) | CCM_CCGR5_CG6(1) | CCM_CCGR5_CG12(1) |
                 CCM_CCGR5_CG14(1) | CCM_CCGR5_CG15(1);
    /* ���ǿ�������Ҫ����ʱ����DCDC���������ú�ر���*/
    CCM->CCGR6 = CCM_CCGR6_CG3(1) | CCM_CCGR6_CG4(1) | CCM_CCGR6_CG5(1) | CCM_CCGR6_CG9(1) | CCM_CCGR6_CG10(1) |
                 CCM_CCGR6_CG11(1);
}



void ClockSelectRcOsc(void)
{
    /* �����ڲ�RC. */
    XTALOSC24M->LOWPWR_CTRL |= XTALOSC24M_LOWPWR_CTRL_RC_OSC_EN_MASK;
    /* �ȴ�CCM������� */
    CLOCK_CCM_HANDSHAKE_WAIT();
    /* ��ʱ */
    SDK_DelayAtLeastUs(4000);
    /* ��ʱ��Դ�л����ڲ�RC. */
    XTALOSC24M->LOWPWR_CTRL_SET = XTALOSC24M_LOWPWR_CTRL_SET_OSC_SEL_MASK;
    /* ����XTAL 24MHzʱ��Դ. */
    CCM_ANALOG->MISC0_SET = CCM_ANALOG_MISC0_XTAL_24M_PWD_MASK;
}
/*!
 * @brief��CCM DIV�ڵ�����Ϊ�ض�ֵ��
 *
 * @param divider Ҫ�����ĸ�div�ڵ㣬�����\ ref clock_div_t��
 * @param value   ʱ��divֵ���ã���ͬ�ķ�Ƶ�����в�ͬ��ֵ��Χ��
 */
void CLOCK_SET_DIV(clock_div_t divider, uint32_t value)
{
    uint32_t busyShift;

    busyShift                   = CCM_TUPLE_BUSY_SHIFT(divider);
    CCM_TUPLE_REG(CCM, divider) = (CCM_TUPLE_REG(CCM, divider) & (~CCM_TUPLE_MASK(divider))) |
                                  (((uint32_t)((value) << CCM_TUPLE_SHIFT(divider))) & CCM_TUPLE_MASK(divider));

    assert(busyShift <= CCM_NO_BUSY_WAIT);

    /* ʱ���л���Ҫ������ */
    if (CCM_NO_BUSY_WAIT != busyShift)
    {
        /*�ȵ�CCM�ڲ�������ɡ� */
        while (CCM->CDHIPR & (1U << busyShift))
        {
        }
    }
}

/*!
 * @brief ��CCM MUX�ڵ�����Ϊ�ض�ֵ��
 *
 * @param mux   Ҫ�����ĸ�mux�ڵ㣬�����\ ref clock_mux_t��
 * @param value ʱ�Ӹ���ֵ���ã���ͬ�ĸ��������в�ͬ��ֵ��Χ��
 */
void CLOCK_SET_MUX(clock_mux_t mux, uint32_t value)
{
    uint32_t busyShift;

    busyShift               = CCM_TUPLE_BUSY_SHIFT(mux);
    CCM_TUPLE_REG(CCM, mux) = (CCM_TUPLE_REG(CCM, mux) & (~CCM_TUPLE_MASK(mux))) |
                              (((uint32_t)((value) << CCM_TUPLE_SHIFT(mux))) & CCM_TUPLE_MASK(mux));

    assert(busyShift <= CCM_NO_BUSY_WAIT);

    /* ʱ���л���Ҫ������ */
    if (CCM_NO_BUSY_WAIT != busyShift)
    {
        /* �ȵ�CCM�ڲ�������ɡ� */
        while (CCM->CDHIPR & (1U << busyShift))
        {
        }
    }
}




/**
 * @brief ѡ��ϵͳʱ��
 * @param power_mode ��Դģʽ
 * @return ��
 *   @retval ��
 */
void SwitchSystemClocks(void)
{
	CLOCK_SET_DIV(kCLOCK_PeriphClk2Div, 0);
	CLOCK_SET_MUX(kCLOCK_PeriphClk2Mux, 1); // PERIPH_CLK2��·���õ�OSC
	CLOCK_SET_MUX(kCLOCK_PeriphMux, 1);     // PERIPH_CLK mux��PERIPH_CLK2
	CLOCK_SET_DIV(kCLOCK_SemcDiv, 0);
	CLOCK_SET_MUX(kCLOCK_SemcMux, 0);    // SEMC���õ�PERIPH_CLK
//        CLOCK_SET_DIV(kCLOCK_FlexspiDiv, 0); // DDRģʽ�µ�FLEXSPI
//        CLOCK_SET_MUX(kCLOCK_FlexspiMux, 0); // FLEXSPI mux��semc_clk_root_pre
	/* CORE CLK��24MHz��AHB��IPG��PERCLK��12MHz */
	CLOCK_SET_DIV(kCLOCK_PerclkDiv, 0);
	CLOCK_SET_DIV(kCLOCK_IpgDiv, 1);
	CLOCK_SET_DIV(kCLOCK_AhbDiv, 0);
	CLOCK_SET_MUX(kCLOCK_PerclkMux, 0); // PERCLK mux��IPG CLK
}
/**
 * @brief ʱ������Ϊ�͹��Ŀ���ģʽ
 * @return ��
 *   @retval ��
 */
void ClockSetToLowPowerIdle(void)
{
    // CORE CLK mux to 24M before reconfigure PLLs
    SwitchSystemClocks();
    ClockSelectRcOsc();

    /* Deinit ARM PLL */
    CLOCK_DeinitArmPll();

    /* Deinit SYS PLL */
    CLOCK_DeinitSysPll();

    /* Deinit SYS PLL PFD 0 1 2 3 */
    CLOCK_DeinitSysPfd(kCLOCK_Pfd0);
    CLOCK_DeinitSysPfd(kCLOCK_Pfd1);
    CLOCK_DeinitSysPfd(kCLOCK_Pfd2);
    CLOCK_DeinitSysPfd(kCLOCK_Pfd3);

//    /* Deinit USB1 PLL */
//    CLOCK_DeinitUsb1Pll();

//    /* Deinit USB1 PLL PFD 0 1 2 3 */
//    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd0);
//    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd1);
//    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd2);
//    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd3);

    /* Deinit USB2 PLL */
    CLOCK_DeinitUsb2Pll();

    /* Deinit AUDIO PLL */
    CLOCK_DeinitAudioPll();

    /* Deinit VIDEO PLL */
    CLOCK_DeinitVideoPll();

    /* Deinit ENET PLL */
    CLOCK_DeinitEnetPll();

    SwitchSystemClocks();
}


/**
 * @brief �ϵ� USB PHY
 * @return ��
 *   @retval ��
 */
void PowerDownUSBPHY(void)
{
    USBPHY1->CTRL = 0xFFFFFFFF;
    USBPHY2->CTRL = 0xFFFFFFFF;
}

/**
 * @brief ʹ����LDO
 * @return ��
 *   @retval ��
 */
void EnableWeakLDO(void)
{
    /*  Enable Weak LDO 2P5 and 1P1 */
    PMU->REG_2P5_SET = PMU_REG_2P5_ENABLE_WEAK_LINREG_MASK;
    PMU->REG_1P1_SET = PMU_REG_1P1_ENABLE_WEAK_LINREG_MASK;

    SDK_DelayAtLeastUs(40);
}


/**
 * @brief ʧ�ܳ���LDO
 * @return ��
 *   @retval ��
 */
void DisableRegularLDO(void)
{
    /* Disable Regular LDO 2P5 and 1P1 */
    PMU->REG_2P5_CLR = PMU_REG_2P5_ENABLE_LINREG_MASK;
    PMU->REG_1P1_CLR = PMU_REG_1P1_ENABLE_LINREG_MASK;
}


/**
 * @brief ��϶ʧ��
 * @return ��
 *   @retval ��
 */
void BandgapOff(void)
{
    XTALOSC24M->LOWPWR_CTRL_SET = XTALOSC24M_LOWPWR_CTRL_LPBG_SEL_MASK;
    PMU->MISC0_SET              = PMU_MISC0_REFTOP_PWD_MASK;
}

/**
 * @brief ����������ģʽ
 * @return ��
 *   @retval ��
 */
void PeripheralEnterDozeMode(void)
{
    IOMUXC_GPR->GPR8 = GPR8_DOZE_BITS;
    IOMUXC_GPR->GPR12 = GPR12_DOZE_BITS;
}


/**
 * @brief  LPM ��ʼ��
 * @return ��
 *   @retval ��
 */
void LPM_Init(void)
{
    uint32_t i;
    uint32_t tmp_reg = 0;

    CLOCK_SetMode(kCLOCK_ModeRun);

    CCM->CGPR |= CCM_CGPR_INT_MEM_CLK_LPM_MASK;

    /* ����RC OSC�� ����Ҫ����4ms�����ȶ��������Ҫ�����Ե���. */
    XTALOSC24M->LOWPWR_CTRL |= XTALOSC24M_LOWPWR_CTRL_RC_OSC_EN_MASK;
    /* ����RC OSC */
    XTALOSC24M->OSC_CONFIG0 = XTALOSC24M_OSC_CONFIG0_RC_OSC_PROG_CUR(0x4) | XTALOSC24M_OSC_CONFIG0_SET_HYST_MINUS(0x2) |
                              XTALOSC24M_OSC_CONFIG0_RC_OSC_PROG(0xA7) | XTALOSC24M_OSC_CONFIG0_START_MASK |
                              XTALOSC24M_OSC_CONFIG0_ENABLE_MASK;
    XTALOSC24M->OSC_CONFIG1 = XTALOSC24M_OSC_CONFIG1_COUNT_RC_CUR(0x40) | XTALOSC24M_OSC_CONFIG1_COUNT_RC_TRG(0x2DC);
    /* ��ʱ */
    SDK_DelayAtLeastUs(4000);
    /* ���һЩ�ͺ� */
    tmp_reg = XTALOSC24M->OSC_CONFIG0;
    tmp_reg &= ~(XTALOSC24M_OSC_CONFIG0_HYST_PLUS_MASK | XTALOSC24M_OSC_CONFIG0_HYST_MINUS_MASK);
    tmp_reg |= XTALOSC24M_OSC_CONFIG0_HYST_PLUS(3) | XTALOSC24M_OSC_CONFIG0_HYST_MINUS(3);
    XTALOSC24M->OSC_CONFIG0 = tmp_reg;
    /* ����COUNT_1M_TRG */
    tmp_reg = XTALOSC24M->OSC_CONFIG2;
    tmp_reg &= ~XTALOSC24M_OSC_CONFIG2_COUNT_1M_TRG_MASK;
    tmp_reg |= XTALOSC24M_OSC_CONFIG2_COUNT_1M_TRG(0x2d7);
    XTALOSC24M->OSC_CONFIG2 = tmp_reg;
    /* Ӳ����Ҫ��ȡOSC_CONFIG0��OSC_CONFIG1��ʹOSC_CONFIG2д�빤�� */
    tmp_reg                 = XTALOSC24M->OSC_CONFIG1;
    XTALOSC24M->OSC_CONFIG1 = tmp_reg;

    /* ERR007265 */
    IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_GINT_MASK;

    /* ��ʼ��GPC����������IRQ */
    for (i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        GPC->IMR[i] = 0xFFFFFFFFU;
    }
}

/**
 * @brief �����˳�����ģʽ
 * @return ��
 *   @retval ��
 */
void PeripheralExitDozeMode(void)
{
    IOMUXC_GPR->GPR8 = 0x00000000;
    IOMUXC_GPR->GPR12 = 0x00000000;
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

/**
 * @brief  LPM�˳��͹��Ŀ���
 * @return ��
 *   @retval ��
 */
void LPM_ExitLowPowerIdle(void)
{
    /* �����˳�����ģʽ */
    PeripheralExitDozeMode();
    /* ��������ģʽ */
    LPM_SetRunModeConfig();
}

/**
 * @brief  LPM�˳��ȴ�ģʽ
 * @return ��
 *   @retval ��
 */
void LPM_PostExitWaitMode(void)
{
    EnableGlobalIRQ(g_savedPrimask);
    __DSB();
    __ISB();
}

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
 * @brief ���û�������
 * @param targetMode ��ǰģʽ
 * @return ��
 *   @retval ��
 */
void APP_SetWakeupConfig(void)
{
	GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
	/* ʹ��GPIO�����ж� */
	GPIO_EnableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
	NVIC_EnableIRQ(APP_WAKEUP_BUTTON_IRQ);
	/* �����ж�*/
	EnableIRQ(APP_WAKEUP_BUTTON_IRQ);
	/* ����GPC�ж�*/
//	LPM_EnableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
	LPM_EnableWakeupSource(LPUART2_IRQn);
}

/**
 * @brief  LPM����͹��Ŀ���
 * @return ��
 *   @retval ��
 */
void LPM_EnterLowPowerIdle(void)
{
	//���û���Դ
	APP_SetWakeupConfig();
	
	LPM_PreEnterWaitMode();
     /*************************��һ����*********************/
     /* ���õȴ�ģʽ���� */
    LPM_SetWaitModeConfig();
     /* ���õ͹���ʱ���� */
    SetLowPowerClockGate();
     /* ��ʱ�����óɵ͹��Ŀ��� */
    ClockSetToLowPowerIdle();
    /*�ϵ� USBPHY */
    PowerDownUSBPHY();
     /*************************�ڶ�����*********************/
    /* ��SOC��ѹ����Ϊ0.95V */
    DCDC_AdjustTargetVoltage(DCDC, 0x6, 0x1);
    /* DCM ģʽ */
    DCDC_BootIntoDCM(DCDC);
    /* �Ͽ����ص�����ڲ� */
    DCDC->REG1 &= ~DCDC_REG1_REG_RLOAD_SW_MASK;
    /* ���������Χ�Ƚ���*/
    DCDC->REG0 |= DCDC_REG0_PWD_CMP_OFFSET_MASK;
    /* ʹ�� FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* ����vdd_high_in������vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;
     /*************************��������*********************/
    /* ʹ����LDO */
    EnableWeakLDO();
    /* ʧ�ܳ���LDO */
    DisableRegularLDO();
    /* ��϶ʧ�� */
    BandgapOff();
    /* ��Χ�豸�������ģʽ */
    PeripheralEnterDozeMode();
    __DSB();
    __WFI();
    __ISB();
	LPM_ExitLowPowerIdle();
    LPM_PostExitWaitMode();
}


/**
 * @brief  LPM����ֹͣģʽ
 * @return ��
 *   @retval ��
 */
void LPM_PreEnterStopMode(void)
{
    g_savedPrimask = DisableGlobalIRQ();
    __DSB();
    __ISB();
}

void LPM_PostExitStopMode(void)
{
    EnableGlobalIRQ(g_savedPrimask);
    __DSB();
    __ISB();
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
 * @brief  ����LPMֹͣģʽ
 * @return ��
 *   @retval ��
 */
void LPM_SetStopModeConfig(void)
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
      *��λ������CCM_CLPCR��0-1λ����*/
    GPC_EnableIRQ(GPC, GPR_IRQ_IRQn);
    clpcr      = CCM->CLPCR & (~(CCM_CLPCR_LPM_MASK | CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK));
    CCM->CLPCR = clpcr | CCM_CLPCR_LPM(kCLOCK_ModeStop) | CCM_CLPCR_MASK_L2CC_IDLE_MASK | CCM_CLPCR_MASK_SCU_IDLE_MASK |
                 CCM_CLPCR_VSTBY_MASK | CCM_CLPCR_STBY_COUNT_MASK | CCM_CLPCR_SBYOS_MASK |
                 CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK | CCM_CLPCR_BYPASS_LPM_HS0_MASK | CCM_CLPCR_BYPASS_LPM_HS1_MASK;
    GPC_DisableIRQ(GPC, GPR_IRQ_IRQn);
}

/**
 * @brief �����˳�ֹͣģʽ
 * @return ��
 *   @retval ��
 */
void PeripheralEnterStopMode(void)
{
    IOMUXC_GPR->GPR4 = IOMUXC_GPR_GPR4_ENET_STOP_REQ_MASK;
    while ((IOMUXC_GPR->GPR4 & IOMUXC_GPR_GPR4_ENET_STOP_ACK_MASK) != IOMUXC_GPR_GPR4_ENET_STOP_ACK_MASK)
    {
    }
    IOMUXC_GPR->GPR4 = GPR4_STOP_REQ_BITS;
    IOMUXC_GPR->GPR7 = GPR7_STOP_REQ_BITS;
    IOMUXC_GPR->GPR8 = GPR8_DOZE_BITS | GPR8_STOP_MODE_BITS;
    IOMUXC_GPR->GPR12 = GPR12_DOZE_BITS | GPR12_STOP_MODE_BITS;
    while ((IOMUXC_GPR->GPR4 & GPR4_STOP_ACK_BITS) != GPR4_STOP_ACK_BITS)
    {
    }
    while ((IOMUXC_GPR->GPR7 & GPR7_STOP_ACK_BITS) != GPR7_STOP_ACK_BITS)
    {
    }
}

/**
 * @brief  LPM������ͣģʽ
 * @return ��
 *   @retval ��
 */
void LPM_EnterSuspend(void)
{
	  /*************************��һ����*********************/
    uint32_t i;
    uint32_t gpcIMR[LPM_GPC_IMR_NUM];
	LPM_PreEnterStopMode();
	//���û���Դ
	APP_SetWakeupConfig();
    /*����LPMֹͣģʽ*/
    LPM_SetStopModeConfig();
    /* ���õ͹���ʱ���� */
    SetLowPowerClockGate();
		/*************************�ڶ�����*********************/
    /* �Ͽ����ص�����ڲ� */
    DCDC->REG1 &= ~DCDC_REG1_REG_RLOAD_SW_MASK;
    /* �ص� FlexRAM0 */
    GPC->CNTR |= GPC_CNTR_PDRAM0_PGE_MASK;
    /* �ص� FlexRAM1 */
    PGC->MEGA_CTRL |= PGC_MEGA_CTRL_PCR_MASK;
    /*�����������ݸ��ٻ�����ȷ���������ı��浽RAM�� */
    SCB_CleanDCache();
    SCB_DisableDCache();
	/*************************��������*********************/
    /* ��LP��ѹ����Ϊ0.925V */
    DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);
    /* �л�DCDC��ʹ��DCDC�ڲ�OSC */
    DCDC_SetClockSource(DCDC, kDCDC_ClockInternalOsc);
    /* �ϵ� USBPHY */
    PowerDownUSBPHY();
    /* ����ʱ�ر�CPU */
    PGC->CPU_CTRL = PGC_CPU_CTRL_PCR_MASK;
    /* ʹ�� FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* ����vdd_high_in������vdd_snvs_in*/
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;
    /* STOP_MODE���ã���ֹͣģʽ�¹ر�RTC���������ģ�� */
    PMU->MISC0_CLR = PMU_MISC0_STOP_MODE_CONFIG_MASK;
	/*************************���Ĳ���*********************/
		 /*������RBC������֮ǰ��������GPC�ж�
			*����Ѿ��жϣ���������������̫��
			*�ȴ���*/
    /* ѭ�����  gpcIMR�Ĵ���*/
    for (i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        gpcIMR[i]   = GPC->IMR[i];
        GPC->IMR[i] = 0xFFFFFFFFU;
    }
		/*CCM���޷�ͨ����Դ�ſشӵȴ�/ֹͣģʽ�ָ�
      *��REG_BYPASS_COUNTER����Ϊ2
      *�ڴ�����RBC��·����������ֹ�жϡ� RBC��̨
      *��Ҫ������2*/
    CCM->CCR = (CCM->CCR & ~CCM_CCR_REG_BYPASS_COUNT_MASK) | CCM_CCR_REG_BYPASS_COUNT(2);
    CCM->CCR |= (CCM_CCR_OSCNT(0xAF) | CCM_CCR_COSC_EN_MASK | CCM_CCR_RBC_EN_MASK);
		 /*�����ӳ�һ�����3usec��
			*���Զ�ѭ�����㹻�ˡ� ��Ҫ�����ӳ���ȷ����һ��
			*����ж��Ѿ�����RBC���������Կ�ʼ����
			*������ARM��������DSM_requestʱ�жϵ��*/
    SDK_DelayAtLeastUs(3);
		/*************************���岿��*********************/
    /* �ָ�����GPC�жϡ� */
    for (i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        GPC->IMR[i] = gpcIMR[i];
    }
    /*�����˳�ֹͣģʽ*/
    PeripheralEnterStopMode();
    __DSB();
    __WFI();
    __ISB();
	LPM_PostExitStopMode();
}

/**
 * @brief ���ô�������
 * @return ��
 *   @retval ��
 */
void ConfigUartRxPinToGpio(void)
{
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_03_GPIO1_IO19, 0);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_03_GPIO1_IO19,
                        IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | 
	                    IOMUXC_SW_PAD_CTL_PAD_PUS(2) | 
	                    IOMUXC_SW_PAD_CTL_PAD_PUE_MASK);
}

/**
 * @brief �ٴ����ô�������
 * @return ��
 *   @retval ��
 */
void ReConfigUartRxPin(void)
{
    /* GPIO_AD_B1_03 is configured as LPUART2_RX */
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_03_LPUART2_RX,0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_03_LPUART2_RX, IOMUXC_SW_PAD_CTL_PAD_SPEED(2));
}

/**
 * @brief �������ѷ�����
 * @return ��
 *   @retval ��
 */
void APP_WAKEUP_BUTTON_IRQ_HANDLER(void)
{
    if ((1U << APP_WAKEUP_BUTTON_GPIO_PIN) & GPIO_GetPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO))
    {
        /* �����ж�. */
        GPIO_DisableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
         /* LPM�رջ���Դ */
        GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
         /* LPM�رջ���Դ */
        LPM_DisableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
    }
}
