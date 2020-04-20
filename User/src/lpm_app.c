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
void SwitchSystemClocks(lpm_power_mode_t power_mode)
{
    switch (power_mode)
    {
    case LPM_PowerModeOverRun:
//        CLOCK_SET_DIV(kCLOCK_SemcDiv, 3);    // SEMC CLK��Ӧ����166MHz
        /* CORE CLK��600MHz��AHB��IPG��150MHz��PERCLK��75MHz */
        CLOCK_SET_DIV(kCLOCK_PerclkDiv, 1);
        CLOCK_SET_DIV(kCLOCK_IpgDiv, 3);
        CLOCK_SET_DIV(kCLOCK_AhbDiv, 0);
        CLOCK_SET_MUX(kCLOCK_PerclkMux, 0);    //PERCLK mux��IPG CLK
        CLOCK_SET_MUX(kCLOCK_PrePeriphMux, 3); //PRE_PERIPH_CLK mux��ARM PLL
        CLOCK_SET_MUX(kCLOCK_PeriphMux, 0);    //PERIPH_CLK mux��PRE_PERIPH_CLK
        break;
    case LPM_PowerModeFullRun:
//        CLOCK_SET_DIV(kCLOCK_SemcDiv, 3);    // SEMC CLK��Ӧ����166MHz
        /* CORE CLKΪ528MHz��AHB��IPGΪ132MHz��PERCLKΪ66MHz */
        CLOCK_SET_DIV(kCLOCK_PerclkDiv, 1);
        CLOCK_SET_DIV(kCLOCK_IpgDiv, 3);
        CLOCK_SET_DIV(kCLOCK_AhbDiv, 0);
        CLOCK_SET_MUX(kCLOCK_PerclkMux, 0);    // PERCLK mux��IPG CLK
        CLOCK_SET_MUX(kCLOCK_PrePeriphMux, 0); // PRE_PERIPH_CLK mux��SYS PLL
        CLOCK_SET_MUX(kCLOCK_PeriphMux, 0);    // PERIPH_CLK mux��PRE_PERIPH_CLK
        break;
    case LPM_PowerModeLowSpeedRun:
    case LPM_PowerModeSysIdle:   
//        CLOCK_SET_DIV(kCLOCK_SemcDiv, 3);    // SEMC CLK��Ӧ����166MHz
        CLOCK_SET_DIV(kCLOCK_PerclkDiv, 0);
        CLOCK_SET_DIV(kCLOCK_IpgDiv, 3);
        CLOCK_SET_DIV(kCLOCK_AhbDiv, 3);
        CLOCK_SET_MUX(kCLOCK_PerclkMux, 0);    // PERCLK mux��IPG CLK
        CLOCK_SET_MUX(kCLOCK_PrePeriphMux, 0); // ��PRE_PERIPH_CLK�л���SYS PLL
        CLOCK_SET_MUX(kCLOCK_PeriphMux, 0);    // ��PERIPH_CLK�л�ΪPRE_PERIPH_CLK
        break;
    case LPM_PowerModeLowPowerRun://LPM_PowerModeLowPowerRun
			
    case LPM_PowerModeLPIdle:

        CLOCK_SET_DIV(kCLOCK_PeriphClk2Div, 0);
        CLOCK_SET_MUX(kCLOCK_PeriphClk2Mux, 1); // PERIPH_CLK2��·���õ�OSC
        CLOCK_SET_MUX(kCLOCK_PeriphMux, 1);     // PERIPH_CLK mux��PERIPH_CLK2
//        CLOCK_SET_DIV(kCLOCK_SemcDiv, 0);
//        CLOCK_SET_MUX(kCLOCK_SemcMux, 0);    // SEMC���õ�PERIPH_CLK
        /* CORE CLK��24MHz��AHB��IPG��PERCLK��12MHz */
        CLOCK_SET_DIV(kCLOCK_PerclkDiv, 0);
        CLOCK_SET_DIV(kCLOCK_IpgDiv, 1);
        CLOCK_SET_DIV(kCLOCK_AhbDiv, 0);
        CLOCK_SET_MUX(kCLOCK_PerclkMux, 0); // PERCLK mux��IPG CLK
        break;
    default:
        break;
    }
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

/**
 * @brief ʱ������Ϊ�͹�������ģʽ
 * @return ��
 *   @retval ��
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
 * @brief ʧ����LDO
 * @return ��
 *   @retval ��
 */
void DisableWeakLDO(void)
{
    /* Disable Weak LDO 2P5 and 1P1 */
    PMU->REG_2P5_CLR = PMU_REG_2P5_ENABLE_WEAK_LINREG_MASK;
    PMU->REG_1P1_CLR = PMU_REG_1P1_ENABLE_WEAK_LINREG_MASK;
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
 * @brief ʹ�ܳ���LDO
 * @return ��
 *   @retval ��
 */
void EnableRegularLDO(void)
{
    /*  Enable Regular LDO 2P5 and 1P1 */
    PMU->REG_2P5_SET = PMU_REG_2P5_ENABLE_LINREG_MASK;
    PMU->REG_1P1_SET = PMU_REG_1P1_ENABLE_LINREG_MASK;
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
 * @brief ��϶ʹ��
 * @return ��
 *   @retval ��
 */
void BandgapOn(void)
{
    /* �򿪳����϶���ȴ��ȶ� */
    PMU->MISC0_CLR = PMU_MISC0_REFTOP_PWD_MASK;
    while ((PMU->MISC0 & PMU_MISC0_REFTOP_VBGUP_MASK) == 0)
    {
    }
    /* �͹��Ĵ�϶���� */
    XTALOSC24M->LOWPWR_CTRL_CLR = XTALOSC24M_LOWPWR_CTRL_LPBG_SEL_MASK;
}

void ClockSelectXtalOsc(void)
{
    /*����XTAL 24MHzʱ��Դ�� */
    CLOCK_InitExternalClk(0);
    /* �ȴ�CCM������� */
    CLOCK_CCM_HANDSHAKE_WAIT();
    /* ��ʱ */
    SDK_DelayAtLeastUs(40);
    /* ��ʱ��Դ�л����ⲿOSC�� */
    CLOCK_SwitchOsc(kCLOCK_XtalOsc);
    /* �ر�XTAL-OSC̽���� */
    CCM_ANALOG->MISC0_CLR = CCM_ANALOG_MISC0_OSC_XTALOK_EN_MASK;
    /* �ر��ڲ�RC�� */
    CLOCK_DeinitRcOsc24M();
}

/**
 * @brief ʱ������Ϊ��������ģʽ
 * @return ��
 *   @retval ��
 */
void ClockSetToFullSpeedRun(void)
{
    // CORE CLK mux to 24M before reconfigure PLLs
    SwitchSystemClocks(LPM_PowerModeLowPowerRun);
    ClockSelectXtalOsc();

    SwitchSystemClocks(LPM_PowerModeFullRun);
}

/**
 * @brief  LPM�͹�������ģʽ
 * @return ��
 *   @retval ��
 */
void LPM_LowPowerRun(void)
{
    ClockSetToLowPowerRun();

    /* �ϵ� USBPHY */
    PowerDownUSBPHY();

    /* ��SOC��ѹ����Ϊ0.95V */
    DCDC_AdjustTargetVoltage(DCDC, 0x6, 0x1);
    /* DCM ģʽ */
    DCDC_BootIntoDCM(DCDC);
    /* �Ͽ����ص�����ڲ� */
    DCDC->REG1 &= ~DCDC_REG1_REG_RLOAD_SW_MASK;
    /* ���������Χ�Ƚ��� */
    DCDC->REG0 |= DCDC_REG0_PWD_CMP_OFFSET_MASK;

    /* ʹ�� FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* ����vdd_high_in������vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;

    EnableWeakLDO();
    DisableRegularLDO();
    BandgapOff();
}

/**
 * @brief  LPM��������ģʽ
 * @return ��
 *   @retval ��
 */
void LPM_FullSpeedRun(void)
{
	
    /* CCM ģʽ */
    DCDC_BootIntoCCM(DCDC);
    /* �����ڲ����ص��� */
    DCDC->REG1 |= DCDC_REG1_REG_RLOAD_SW_MASK;
    /*��SOC��ѹ����Ϊ1.275V */
    DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);

    /* ����FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* ����FET ODRIVEConnect vdd_high_in������vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;

    BandgapOn();
     /* ʹ�ܳ���LDO */
    EnableRegularLDO();
    /* ʧ�� ��LDO */
    DisableWeakLDO();
    /* �����������е�ʱ�� */
    ClockSetToFullSpeedRun();
    /* ��SOC��ѹ����Ϊ1.15V */
    DCDC_AdjustTargetVoltage(DCDC, 0xe, 0x1);
}

