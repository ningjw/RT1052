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
 * @brief  LPM进入等待模式
 * @return 无
 *   @retval 无
 */
void LPM_PreEnterWaitMode(void)
{
    g_savedPrimask = DisableGlobalIRQ();
    __DSB();
    __ISB();
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
 * @brief 设置低功耗时钟门
 * @return 无
 *   @retval 无
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
    /* 我们可以在需要配置时启用DCDC，并在配置后关闭它*/
    CCM->CCGR6 = CCM_CCGR6_CG3(1) | CCM_CCGR6_CG4(1) | CCM_CCGR6_CG5(1) | CCM_CCGR6_CG9(1) | CCM_CCGR6_CG10(1) |
                 CCM_CCGR6_CG11(1);
}



void ClockSelectRcOsc(void)
{
    /* 启用内部RC. */
    XTALOSC24M->LOWPWR_CTRL |= XTALOSC24M_LOWPWR_CTRL_RC_OSC_EN_MASK;
    /* 等待CCM操作完成 */
    CLOCK_CCM_HANDSHAKE_WAIT();
    /* 延时 */
    SDK_DelayAtLeastUs(4000);
    /* 将时钟源切换到内部RC. */
    XTALOSC24M->LOWPWR_CTRL_SET = XTALOSC24M_LOWPWR_CTRL_SET_OSC_SEL_MASK;
    /* 禁用XTAL 24MHz时钟源. */
    CCM_ANALOG->MISC0_SET = CCM_ANALOG_MISC0_XTAL_24M_PWD_MASK;
}
/*!
 * @brief将CCM DIV节点设置为特定值。
 *
 * @param divider 要设置哪个div节点，请参阅\ ref clock_div_t。
 * @param value   时钟div值设置，不同的分频器具有不同的值范围。
 */
void CLOCK_SET_DIV(clock_div_t divider, uint32_t value)
{
    uint32_t busyShift;

    busyShift                   = CCM_TUPLE_BUSY_SHIFT(divider);
    CCM_TUPLE_REG(CCM, divider) = (CCM_TUPLE_REG(CCM, divider) & (~CCM_TUPLE_MASK(divider))) |
                                  (((uint32_t)((value) << CCM_TUPLE_SHIFT(divider))) & CCM_TUPLE_MASK(divider));

    assert(busyShift <= CCM_NO_BUSY_WAIT);

    /* 时钟切换需要握手吗？ */
    if (CCM_NO_BUSY_WAIT != busyShift)
    {
        /*等到CCM内部握手完成。 */
        while (CCM->CDHIPR & (1U << busyShift))
        {
        }
    }
}

/*!
 * @brief 将CCM MUX节点设置为特定值。
 *
 * @param mux   要设置哪个mux节点，请参阅\ ref clock_mux_t。
 * @param value 时钟复用值设置，不同的复用器具有不同的值范围。
 */
void CLOCK_SET_MUX(clock_mux_t mux, uint32_t value)
{
    uint32_t busyShift;

    busyShift               = CCM_TUPLE_BUSY_SHIFT(mux);
    CCM_TUPLE_REG(CCM, mux) = (CCM_TUPLE_REG(CCM, mux) & (~CCM_TUPLE_MASK(mux))) |
                              (((uint32_t)((value) << CCM_TUPLE_SHIFT(mux))) & CCM_TUPLE_MASK(mux));

    assert(busyShift <= CCM_NO_BUSY_WAIT);

    /* 时钟切换需要握手吗？ */
    if (CCM_NO_BUSY_WAIT != busyShift)
    {
        /* 等到CCM内部握手完成。 */
        while (CCM->CDHIPR & (1U << busyShift))
        {
        }
    }
}




/**
 * @brief 选择系统时钟
 * @param power_mode 电源模式
 * @return 无
 *   @retval 无
 */
void SwitchSystemClocks(void)
{
	CLOCK_SET_DIV(kCLOCK_PeriphClk2Div, 0);
	CLOCK_SET_MUX(kCLOCK_PeriphClk2Mux, 1); // PERIPH_CLK2多路复用到OSC
	CLOCK_SET_MUX(kCLOCK_PeriphMux, 1);     // PERIPH_CLK mux到PERIPH_CLK2
	CLOCK_SET_DIV(kCLOCK_SemcDiv, 0);
	CLOCK_SET_MUX(kCLOCK_SemcMux, 0);    // SEMC复用到PERIPH_CLK
//        CLOCK_SET_DIV(kCLOCK_FlexspiDiv, 0); // DDR模式下的FLEXSPI
//        CLOCK_SET_MUX(kCLOCK_FlexspiMux, 0); // FLEXSPI mux到semc_clk_root_pre
	/* CORE CLK至24MHz，AHB，IPG，PERCLK至12MHz */
	CLOCK_SET_DIV(kCLOCK_PerclkDiv, 0);
	CLOCK_SET_DIV(kCLOCK_IpgDiv, 1);
	CLOCK_SET_DIV(kCLOCK_AhbDiv, 0);
	CLOCK_SET_MUX(kCLOCK_PerclkMux, 0); // PERCLK mux到IPG CLK
}
/**
 * @brief 时钟设置为低功耗空闲模式
 * @return 无
 *   @retval 无
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
 * @brief 断电 USB PHY
 * @return 无
 *   @retval 无
 */
void PowerDownUSBPHY(void)
{
    USBPHY1->CTRL = 0xFFFFFFFF;
    USBPHY2->CTRL = 0xFFFFFFFF;
}

/**
 * @brief 使能弱LDO
 * @return 无
 *   @retval 无
 */
void EnableWeakLDO(void)
{
    /*  Enable Weak LDO 2P5 and 1P1 */
    PMU->REG_2P5_SET = PMU_REG_2P5_ENABLE_WEAK_LINREG_MASK;
    PMU->REG_1P1_SET = PMU_REG_1P1_ENABLE_WEAK_LINREG_MASK;

    SDK_DelayAtLeastUs(40);
}


/**
 * @brief 失能常规LDO
 * @return 无
 *   @retval 无
 */
void DisableRegularLDO(void)
{
    /* Disable Regular LDO 2P5 and 1P1 */
    PMU->REG_2P5_CLR = PMU_REG_2P5_ENABLE_LINREG_MASK;
    PMU->REG_1P1_CLR = PMU_REG_1P1_ENABLE_LINREG_MASK;
}


/**
 * @brief 带隙失能
 * @return 无
 *   @retval 无
 */
void BandgapOff(void)
{
    XTALOSC24M->LOWPWR_CTRL_SET = XTALOSC24M_LOWPWR_CTRL_LPBG_SEL_MASK;
    PMU->MISC0_SET              = PMU_MISC0_REFTOP_PWD_MASK;
}

/**
 * @brief 外设进入打盹模式
 * @return 无
 *   @retval 无
 */
void PeripheralEnterDozeMode(void)
{
    IOMUXC_GPR->GPR8 = GPR8_DOZE_BITS;
    IOMUXC_GPR->GPR12 = GPR12_DOZE_BITS;
}


/**
 * @brief  LPM 初始化
 * @return 无
 *   @retval 无
 */
void LPM_Init(void)
{
    uint32_t i;
    uint32_t tmp_reg = 0;

    CLOCK_SetMode(kCLOCK_ModeRun);

    CCM->CGPR |= CCM_CGPR_INT_MEM_CLK_LPM_MASK;

    /* 启用RC OSC。 它需要至少4ms才能稳定，因此需要启用自调整. */
    XTALOSC24M->LOWPWR_CTRL |= XTALOSC24M_LOWPWR_CTRL_RC_OSC_EN_MASK;
    /* 配置RC OSC */
    XTALOSC24M->OSC_CONFIG0 = XTALOSC24M_OSC_CONFIG0_RC_OSC_PROG_CUR(0x4) | XTALOSC24M_OSC_CONFIG0_SET_HYST_MINUS(0x2) |
                              XTALOSC24M_OSC_CONFIG0_RC_OSC_PROG(0xA7) | XTALOSC24M_OSC_CONFIG0_START_MASK |
                              XTALOSC24M_OSC_CONFIG0_ENABLE_MASK;
    XTALOSC24M->OSC_CONFIG1 = XTALOSC24M_OSC_CONFIG1_COUNT_RC_CUR(0x40) | XTALOSC24M_OSC_CONFIG1_COUNT_RC_TRG(0x2DC);
    /* 延时 */
    SDK_DelayAtLeastUs(4000);
    /* 添加一些滞后 */
    tmp_reg = XTALOSC24M->OSC_CONFIG0;
    tmp_reg &= ~(XTALOSC24M_OSC_CONFIG0_HYST_PLUS_MASK | XTALOSC24M_OSC_CONFIG0_HYST_MINUS_MASK);
    tmp_reg |= XTALOSC24M_OSC_CONFIG0_HYST_PLUS(3) | XTALOSC24M_OSC_CONFIG0_HYST_MINUS(3);
    XTALOSC24M->OSC_CONFIG0 = tmp_reg;
    /* 设置COUNT_1M_TRG */
    tmp_reg = XTALOSC24M->OSC_CONFIG2;
    tmp_reg &= ~XTALOSC24M_OSC_CONFIG2_COUNT_1M_TRG_MASK;
    tmp_reg |= XTALOSC24M_OSC_CONFIG2_COUNT_1M_TRG(0x2d7);
    XTALOSC24M->OSC_CONFIG2 = tmp_reg;
    /* 硬件需要读取OSC_CONFIG0或OSC_CONFIG1以使OSC_CONFIG2写入工作 */
    tmp_reg                 = XTALOSC24M->OSC_CONFIG1;
    XTALOSC24M->OSC_CONFIG1 = tmp_reg;

    /* ERR007265 */
    IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_GINT_MASK;

    /* 初始化GPC以屏蔽所有IRQ */
    for (i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        GPC->IMR[i] = 0xFFFFFFFFU;
    }
}

/**
 * @brief 外设退出打盹模式
 * @return 无
 *   @retval 无
 */
void PeripheralExitDozeMode(void)
{
    IOMUXC_GPR->GPR8 = 0x00000000;
    IOMUXC_GPR->GPR12 = 0x00000000;
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

/**
 * @brief  LPM退出低功耗空闲
 * @return 无
 *   @retval 无
 */
void LPM_ExitLowPowerIdle(void)
{
    /* 外设退出打盹模式 */
    PeripheralExitDozeMode();
    /* 设置运行模式 */
    LPM_SetRunModeConfig();
}

/**
 * @brief  LPM退出等待模式
 * @return 无
 *   @retval 无
 */
void LPM_PostExitWaitMode(void)
{
    EnableGlobalIRQ(g_savedPrimask);
    __DSB();
    __ISB();
}

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
 * @brief 设置唤醒配置
 * @param targetMode 当前模式
 * @return 无
 *   @retval 无
 */
void APP_SetWakeupConfig(void)
{
	GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
	/* 使能GPIO引脚中断 */
	GPIO_EnableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
	NVIC_EnableIRQ(APP_WAKEUP_BUTTON_IRQ);
	/* 启用中断*/
	EnableIRQ(APP_WAKEUP_BUTTON_IRQ);
	/* 启用GPC中断*/
//	LPM_EnableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
	LPM_EnableWakeupSource(LPUART2_IRQn);
}

/**
 * @brief  LPM进入低功耗空闲
 * @return 无
 *   @retval 无
 */
void LPM_EnterLowPowerIdle(void)
{
	//设置唤醒源
	APP_SetWakeupConfig();
	
	LPM_PreEnterWaitMode();
     /*************************第一部分*********************/
     /* 设置等待模式配置 */
    LPM_SetWaitModeConfig();
     /* 设置低功耗时钟门 */
    SetLowPowerClockGate();
     /* 将时钟设置成低功耗空闲 */
    ClockSetToLowPowerIdle();
    /*断电 USBPHY */
    PowerDownUSBPHY();
     /*************************第二部分*********************/
    /* 将SOC电压调整为0.95V */
    DCDC_AdjustTargetVoltage(DCDC, 0x6, 0x1);
    /* DCM 模式 */
    DCDC_BootIntoDCM(DCDC);
    /* 断开负载电阻的内部 */
    DCDC->REG1 &= ~DCDC_REG1_REG_RLOAD_SW_MASK;
    /* 掉电输出范围比较器*/
    DCDC->REG0 |= DCDC_REG0_PWD_CMP_OFFSET_MASK;
    /* 使能 FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* 连接vdd_high_in并连接vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;
     /*************************第三部分*********************/
    /* 使能弱LDO */
    EnableWeakLDO();
    /* 失能常规LDO */
    DisableRegularLDO();
    /* 带隙失能 */
    BandgapOff();
    /* 外围设备进入打盹模式 */
    PeripheralEnterDozeMode();
    __DSB();
    __WFI();
    __ISB();
	LPM_ExitLowPowerIdle();
    LPM_PostExitWaitMode();
}


/**
 * @brief  LPM进入停止模式
 * @return 无
 *   @retval 无
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
 * @brief  LPM关闭唤醒源
 * @return 无
 *   @retval 无
 */
void LPM_DisableWakeupSource(uint32_t irq)
{
    GPC_DisableIRQ(GPC, irq);
}

/**
 * @brief  配置LPM停止模式
 * @return 无
 *   @retval 无
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
	
	译文：* ERR007265：CCM：使用不正确的低功率序列时，
      * SoC在ARM内核执行WFI之前进入低功耗模式。
     *
      *软件解决方法：
      * 1）软件应触发IRQ＃41（GPR_IRQ）始终挂起
      *通过设置IOMUXC_GPR_GPR1_GINT。
      * 2）软件应在设置CCM之前在GPC中取消屏蔽IRQ＃41
      *低功耗模式。
      * 3）软件应在CCM低功耗模式后立即屏蔽IRQ＃41
      *置位（设置CCM_CLPCR的0-1位）。*/
    GPC_EnableIRQ(GPC, GPR_IRQ_IRQn);
    clpcr      = CCM->CLPCR & (~(CCM_CLPCR_LPM_MASK | CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK));
    CCM->CLPCR = clpcr | CCM_CLPCR_LPM(kCLOCK_ModeStop) | CCM_CLPCR_MASK_L2CC_IDLE_MASK | CCM_CLPCR_MASK_SCU_IDLE_MASK |
                 CCM_CLPCR_VSTBY_MASK | CCM_CLPCR_STBY_COUNT_MASK | CCM_CLPCR_SBYOS_MASK |
                 CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK | CCM_CLPCR_BYPASS_LPM_HS0_MASK | CCM_CLPCR_BYPASS_LPM_HS1_MASK;
    GPC_DisableIRQ(GPC, GPR_IRQ_IRQn);
}

/**
 * @brief 外设退出停止模式
 * @return 无
 *   @retval 无
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
 * @brief  LPM进入暂停模式
 * @return 无
 *   @retval 无
 */
void LPM_EnterSuspend(void)
{
	  /*************************第一部分*********************/
    uint32_t i;
    uint32_t gpcIMR[LPM_GPC_IMR_NUM];
	LPM_PreEnterStopMode();
	//设置唤醒源
	APP_SetWakeupConfig();
    /*设置LPM停止模式*/
    LPM_SetStopModeConfig();
    /* 设置低功耗时钟门 */
    SetLowPowerClockGate();
		/*************************第二部分*********************/
    /* 断开负载电阻的内部 */
    DCDC->REG1 &= ~DCDC_REG1_REG_RLOAD_SW_MASK;
    /* 关掉 FlexRAM0 */
    GPC->CNTR |= GPC_CNTR_PDRAM0_PGE_MASK;
    /* 关掉 FlexRAM1 */
    PGC->MEGA_CTRL |= PGC_MEGA_CTRL_PCR_MASK;
    /*清理并禁用数据高速缓存以确保将上下文保存到RAM中 */
    SCB_CleanDCache();
    SCB_DisableDCache();
	/*************************第三部分*********************/
    /* 将LP电压调整为0.925V */
    DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);
    /* 切换DCDC以使用DCDC内部OSC */
    DCDC_SetClockSource(DCDC, kDCDC_ClockInternalOsc);
    /* 断电 USBPHY */
    PowerDownUSBPHY();
    /* 请求时关闭CPU */
    PGC->CPU_CTRL = PGC_CPU_CTRL_PCR_MASK;
    /* 使能 FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* 连接vdd_high_in并连接vdd_snvs_in*/
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;
    /* STOP_MODE配置，在停止模式下关闭RTC以外的所有模拟 */
    PMU->MISC0_CLR = PMU_MISC0_STOP_MODE_CONFIG_MASK;
	/*************************第四部分*********************/
		 /*在启用RBC计数器之前屏蔽所有GPC中断
			*如果已经中断，请避免计数器启动太早
			*等待。*/
    /* 循环清楚  gpcIMR寄存器*/
    for (i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        gpcIMR[i]   = GPC->IMR[i];
        GPC->IMR[i] = 0xFFFFFFFFU;
    }
		/*CCM：无法通过电源门控从等待/停止模式恢复
      *将REG_BYPASS_COUNTER配置为2
      *在此启用RBC旁路计数器以阻止中断。 RBC柜台
      *需要不少于2*/
    CCM->CCR = (CCM->CCR & ~CCM_CCR_REG_BYPASS_COUNT_MASK) | CCM_CCR_REG_BYPASS_COUNT(2);
    CCM->CCR |= (CCM_CCR_OSCNT(0xAF) | CCM_CCR_COSC_EN_MASK | CCM_CCR_RBC_EN_MASK);
		 /*现在延迟一会儿（3usec）
			*所以短循环就足够了。 需要这种延迟来确保这一点
			*如果中断已经挂起，RBC计数器可以开始计数
			*或者在ARM即将断言DSM_request时中断到达。*/
    SDK_DelayAtLeastUs(3);
		/*************************第五部分*********************/
    /* 恢复所有GPC中断。 */
    for (i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        GPC->IMR[i] = gpcIMR[i];
    }
    /*外设退出停止模式*/
    PeripheralEnterStopMode();
    __DSB();
    __WFI();
    __ISB();
	LPM_PostExitStopMode();
}

/**
 * @brief 配置串口引脚
 * @return 无
 *   @retval 无
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
 * @brief 再次配置串口引脚
 * @return 无
 *   @retval 无
 */
void ReConfigUartRxPin(void)
{
    /* GPIO_AD_B1_03 is configured as LPUART2_RX */
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_03_LPUART2_RX,0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_03_LPUART2_RX, IOMUXC_SW_PAD_CTL_PAD_SPEED(2));
}

/**
 * @brief 按键唤醒服务函数
 * @return 无
 *   @retval 无
 */
void APP_WAKEUP_BUTTON_IRQ_HANDLER(void)
{
    if ((1U << APP_WAKEUP_BUTTON_GPIO_PIN) & GPIO_GetPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO))
    {
        /* 禁用中断. */
        GPIO_DisableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
         /* LPM关闭唤醒源 */
        GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
         /* LPM关闭唤醒源 */
        LPM_DisableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
    }
}
