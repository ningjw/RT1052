#include "main.h"

#define CLOCK_CCM_HANDSHAKE_WAIT() \
                                   \
    do                             \
    {                              \
        while (CCM->CDHIPR != 0)   \
        {                          \
        }                          \
                                   \
    } while (0)
	
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
void SwitchSystemClocks(lpm_power_mode_t power_mode)
{
    switch (power_mode)
    {
    case LPM_PowerModeOverRun:
//        CLOCK_SET_DIV(kCLOCK_SemcDiv, 3);    // SEMC CLK不应超过166MHz
        /* CORE CLK至600MHz，AHB，IPG至150MHz，PERCLK至75MHz */
        CLOCK_SET_DIV(kCLOCK_PerclkDiv, 1);
        CLOCK_SET_DIV(kCLOCK_IpgDiv, 3);
        CLOCK_SET_DIV(kCLOCK_AhbDiv, 0);
        CLOCK_SET_MUX(kCLOCK_PerclkMux, 0);    //PERCLK mux到IPG CLK
        CLOCK_SET_MUX(kCLOCK_PrePeriphMux, 3); //PRE_PERIPH_CLK mux到ARM PLL
        CLOCK_SET_MUX(kCLOCK_PeriphMux, 0);    //PERIPH_CLK mux到PRE_PERIPH_CLK
        break;
    case LPM_PowerModeFullRun:
//        CLOCK_SET_DIV(kCLOCK_SemcDiv, 3);    // SEMC CLK不应超过166MHz
        /* CORE CLK为528MHz，AHB，IPG为132MHz，PERCLK为66MHz */
        CLOCK_SET_DIV(kCLOCK_PerclkDiv, 1);
        CLOCK_SET_DIV(kCLOCK_IpgDiv, 3);
        CLOCK_SET_DIV(kCLOCK_AhbDiv, 0);
        CLOCK_SET_MUX(kCLOCK_PerclkMux, 0);    // PERCLK mux到IPG CLK
        CLOCK_SET_MUX(kCLOCK_PrePeriphMux, 0); // PRE_PERIPH_CLK mux到SYS PLL
        CLOCK_SET_MUX(kCLOCK_PeriphMux, 0);    // PERIPH_CLK mux到PRE_PERIPH_CLK
        break;
    case LPM_PowerModeLowSpeedRun:
    case LPM_PowerModeSysIdle:   
//        CLOCK_SET_DIV(kCLOCK_SemcDiv, 3);    // SEMC CLK不应超过166MHz
        CLOCK_SET_DIV(kCLOCK_PerclkDiv, 0);
        CLOCK_SET_DIV(kCLOCK_IpgDiv, 3);
        CLOCK_SET_DIV(kCLOCK_AhbDiv, 3);
        CLOCK_SET_MUX(kCLOCK_PerclkMux, 0);    // PERCLK mux到IPG CLK
        CLOCK_SET_MUX(kCLOCK_PrePeriphMux, 0); // 将PRE_PERIPH_CLK切换到SYS PLL
        CLOCK_SET_MUX(kCLOCK_PeriphMux, 0);    // 将PERIPH_CLK切换为PRE_PERIPH_CLK
        break;
    case LPM_PowerModeLowPowerRun://LPM_PowerModeLowPowerRun
			
    case LPM_PowerModeLPIdle:

        CLOCK_SET_DIV(kCLOCK_PeriphClk2Div, 0);
        CLOCK_SET_MUX(kCLOCK_PeriphClk2Mux, 1); // PERIPH_CLK2多路复用到OSC
        CLOCK_SET_MUX(kCLOCK_PeriphMux, 1);     // PERIPH_CLK mux到PERIPH_CLK2
//        CLOCK_SET_DIV(kCLOCK_SemcDiv, 0);
//        CLOCK_SET_MUX(kCLOCK_SemcMux, 0);    // SEMC复用到PERIPH_CLK
        /* CORE CLK至24MHz，AHB，IPG，PERCLK至12MHz */
        CLOCK_SET_DIV(kCLOCK_PerclkDiv, 0);
        CLOCK_SET_DIV(kCLOCK_IpgDiv, 1);
        CLOCK_SET_DIV(kCLOCK_AhbDiv, 0);
        CLOCK_SET_MUX(kCLOCK_PerclkMux, 0); // PERCLK mux到IPG CLK
        break;
    default:
        break;
    }
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

/**
 * @brief 时钟设置为低功耗运行模式
 * @return 无
 *   @retval 无
 */
void ClockSetToLowPowerRun(void)
{
    // CORE CLK mux to 24M before reconfigure PLLs                             	
    SwitchSystemClocks(LPM_PowerModeLowPowerRun);                               
    ClockSelectRcOsc();                                                         
    /* Deinit ARM PLL */                                                        
//    CLOCK_DeinitArmPll();                                                       
    /* Deinit SYS PLL */                                                        
//    CLOCK_DeinitSysPll();                                                       
//    /* Deinit SYS PLL PFD 0 1 2 3 */                                            
//    CLOCK_DeinitSysPfd(kCLOCK_Pfd0);                                            
//    CLOCK_DeinitSysPfd(kCLOCK_Pfd1);                                            
//    CLOCK_DeinitSysPfd(kCLOCK_Pfd2);                                            
//    CLOCK_DeinitSysPfd(kCLOCK_Pfd3);                                            
    /* Deinit USB1 PLL PFD 0 1 2 3 */                                           
    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd0);///////		                                
		CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd1);                                           
    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd2);                                           
    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd3);                                           
    /* Deinit USB2 PLL */                                                       
    CLOCK_DeinitUsb2Pll();                                                      
    /* Deinit AUDIO PLL */                                                      
    CLOCK_DeinitAudioPll();                                                     
    /* Deinit VIDEO PLL */                                                      
    CLOCK_DeinitVideoPll();                                                     
    /* Deinit ENET PLL */                                                       
    CLOCK_DeinitEnetPll();
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
 * @brief 失能弱LDO
 * @return 无
 *   @retval 无
 */
void DisableWeakLDO(void)
{
    /* Disable Weak LDO 2P5 and 1P1 */
    PMU->REG_2P5_CLR = PMU_REG_2P5_ENABLE_WEAK_LINREG_MASK;
    PMU->REG_1P1_CLR = PMU_REG_1P1_ENABLE_WEAK_LINREG_MASK;
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
 * @brief 使能常规LDO
 * @return 无
 *   @retval 无
 */
void EnableRegularLDO(void)
{
    /*  Enable Regular LDO 2P5 and 1P1 */
    PMU->REG_2P5_SET = PMU_REG_2P5_ENABLE_LINREG_MASK;
    PMU->REG_1P1_SET = PMU_REG_1P1_ENABLE_LINREG_MASK;
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
 * @brief 带隙使能
 * @return 无
 *   @retval 无
 */
void BandgapOn(void)
{
    /* 打开常规带隙并等待稳定 */
    PMU->MISC0_CLR = PMU_MISC0_REFTOP_PWD_MASK;
    while ((PMU->MISC0 & PMU_MISC0_REFTOP_VBGUP_MASK) == 0)
    {
    }
    /* 低功耗带隙禁用 */
    XTALOSC24M->LOWPWR_CTRL_CLR = XTALOSC24M_LOWPWR_CTRL_LPBG_SEL_MASK;
}

void ClockSelectXtalOsc(void)
{
    /*启用XTAL 24MHz时钟源。 */
    CLOCK_InitExternalClk(0);
    /* 等待CCM操作完成 */
    CLOCK_CCM_HANDSHAKE_WAIT();
    /* 延时 */
    SDK_DelayAtLeastUs(40);
    /* 将时钟源切换到外部OSC。 */
    CLOCK_SwitchOsc(kCLOCK_XtalOsc);
    /* 关闭XTAL-OSC探测器 */
    CCM_ANALOG->MISC0_CLR = CCM_ANALOG_MISC0_OSC_XTALOK_EN_MASK;
    /* 关闭内部RC。 */
    CLOCK_DeinitRcOsc24M();
}

/**
 * @brief 时钟设置为满载运行模式
 * @return 无
 *   @retval 无
 */
void ClockSetToFullSpeedRun(void)
{
    // CORE CLK mux to 24M before reconfigure PLLs
    SwitchSystemClocks(LPM_PowerModeLowPowerRun);
    ClockSelectXtalOsc();

    SwitchSystemClocks(LPM_PowerModeFullRun);
}

/**
 * @brief  LPM低功耗运行模式
 * @return 无
 *   @retval 无
 */
void LPM_LowPowerRun(void)
{
    ClockSetToLowPowerRun();

    /* 断电 USBPHY */
    PowerDownUSBPHY();

    /* 将SOC电压调整为0.95V */
    DCDC_AdjustTargetVoltage(DCDC, 0x6, 0x1);
    /* DCM 模式 */
    DCDC_BootIntoDCM(DCDC);
    /* 断开负载电阻的内部 */
    DCDC->REG1 &= ~DCDC_REG1_REG_RLOAD_SW_MASK;
    /* 掉电输出范围比较器 */
    DCDC->REG0 |= DCDC_REG0_PWD_CMP_OFFSET_MASK;

    /* 使能 FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* 连接vdd_high_in并连接vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;

    EnableWeakLDO();
    DisableRegularLDO();
    BandgapOff();
}

/**
 * @brief  LPM满载运行模式
 * @return 无
 *   @retval 无
 */
void LPM_FullSpeedRun(void)
{
	
    /* CCM 模式 */
    DCDC_BootIntoCCM(DCDC);
    /* 连接内部负载电阻 */
    DCDC->REG1 |= DCDC_REG1_REG_RLOAD_SW_MASK;
    /*将SOC电压调整为1.275V */
    DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);

    /* 启用FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* 启用FET ODRIVEConnect vdd_high_in并连接vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;

    BandgapOn();
     /* 使能常规LDO */
    EnableRegularLDO();
    /* 失能 弱LDO */
    DisableWeakLDO();
    /* 设置满载运行的时钟 */
    ClockSetToFullSpeedRun();
    /* 将SOC电压调整为1.15V */
    DCDC_AdjustTargetVoltage(DCDC, 0xe, 0x1);
}

