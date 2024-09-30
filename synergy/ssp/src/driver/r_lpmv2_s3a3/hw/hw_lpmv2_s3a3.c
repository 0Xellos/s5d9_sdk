/***********************************************************************************************************************
 * Copyright [2016-2024] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 * 
 * This file is part of Renesas SynergyTM Software Package (SSP)
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas SSP license agreement. Unless otherwise agreed in an SSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * File Name    : hw_lpmv2_s3a3.c
 * Description  : LPMV2 S3A3 hardware implementation.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include "bsp_api.h"

#if defined(BSP_MCU_GROUP_S3A3)

#include "r_lpmv2_s3a3.h"
#include "hw_lpmv2_s3a3.h"
#include "r_cgc.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if LPMV2_CFG_PARAM_CHECKING_ENABLE
static bool check_low_power_mode(lpmv2_low_power_mode_t low_power_mode);
static ssp_err_t check_snooze_config(lpmv2_snooze_dtc_t dtc_state_in_snooze,
                                     lpmv2_snooze_request_t snooze_request_source,
                                     lpmv2_snooze_end_bits_t snooze_end_sources);
static bool check_output_port_config(lpmv2_output_port_enable_t output_port_enable);
#endif /* LPMV2_CFG_PARAM_CHECKING_ENABLE */

static ssp_err_t check_snooze_trigger_and_clocks(void);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

#if (0 != BSP_CFG_ERROR_LOG)
    extern const char g_lpmv2_module_name[];
    extern const ssp_version_t g_lpmv2_version;
#endif

/*******************************************************************************************************************//**
 * @addtogroup LPMV2_S3A3
 * @{
 **********************************************************************************************************************/

#if LPMV2_CFG_PARAM_CHECKING_ENABLE
/*******************************************************************************************************************//**
 * @brief Verifies all MCU specific settings related to low power modes
 *
 * @param p_cfg the MCU specific configuration
 *
 * @return SSP_SUCCESS              configuration is valid
 * @return SSP_ERR_INVALID_MODE     invalid low power mode
 * @return SSP_ERR_INVALID_POINTER  NULL p_extend when low power mode is not Sleep
 * @return SSP_ERR_INVALID_ARGUMENT invalid snooze entry source
 * @return SSP_ERR_INVALID_ARGUMENT invalid snooze end sources
 * @return SSP_ERR_INVALID_MODE     invalid DTC option for snooze mode
 * @return SSP_ERR_INVALID_MODE     invalid output port state setting for standby or deep standby
 * @return SSP_ERR_INVALID_MODE     invalid sources for wake from standby mode
 *
 **********************************************************************************************************************/
ssp_err_t HW_LPMV2_MCUSpecificLowPowerCheck(lpmv2_cfg_t const * const p_cfg)
{
    LPMV2_ERROR_RETURN(true == check_low_power_mode(p_cfg->low_power_mode),
                       SSP_ERR_INVALID_MODE);

    if(LPMV2_LOW_POWER_MODE_SLEEP != p_cfg->low_power_mode)
    {
        LPMV2_ERROR_RETURN(NULL != p_cfg->p_extend, SSP_ERR_INVALID_POINTER);

        lpmv2_mcu_cfg_t * p_lpmv2_s3a3_cfg = (lpmv2_mcu_cfg_t * )(p_cfg->p_extend);

        if(LPMV2_LOW_POWER_MODE_STANDBY_SNOOZE == p_cfg->low_power_mode)
        {
            ssp_err_t err = check_snooze_config(p_lpmv2_s3a3_cfg->dtc_state_in_snooze,
                                                p_lpmv2_s3a3_cfg->snooze_request_source,
                                                p_lpmv2_s3a3_cfg->snooze_end_sources);
            LPMV2_ERROR_RETURN(SSP_SUCCESS == err, err);
        }

        /* All reserved bits in WUPEN are 0 after reset. All reserved bits should be written as 0. */
        LPMV2_ERROR_RETURN(0UL == ((uint32_t)p_lpmv2_s3a3_cfg->standby_wake_sources & ~HW_LPMV2_STANDBY_WAKE_SOURCE_MASK), SSP_ERR_INVALID_MODE);

        LPMV2_ERROR_RETURN(true == check_output_port_config(p_lpmv2_s3a3_cfg->output_port_enable),
                           SSP_ERR_INVALID_MODE);
    }
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Verifies the requested low power mode.  Internal function, do not use directly.
 *
 * @param low_power_mode the requested low power mode
 *
 * @return true     valid low power mode
 * @return false    invalid low power mode
 *
 **********************************************************************************************************************/
static bool check_low_power_mode(lpmv2_low_power_mode_t low_power_mode)
{
    bool valid = false;

    if((LPMV2_LOW_POWER_MODE_SLEEP          == low_power_mode)  ||
       (LPMV2_LOW_POWER_MODE_STANDBY        == low_power_mode)  ||
       (LPMV2_LOW_POWER_MODE_STANDBY_SNOOZE == low_power_mode)
       )
    {
        valid = true;
    }

    return valid;
}

/*******************************************************************************************************************//**
 * @brief Verifies MCU specific settings related to snooze mode. Internal function, do not use directly.
 *
 * @param dtc_state_in_snooze   DTC enabled or disabled in snooze mode
 * @param snooze_request_source Snoozed request source
 * @param snooze_end_sources    Snooze end source list
 *
 * @return SSP_SUCCESS              configuration is valid
 * @return SSP_ERR_INVALID_MODE     invalid DTC option for snooze mode
 * @return SSP_ERR_INVALID_ARGUMENT invalid snooze entry source
 * @return SSP_ERR_INVALID_ARGUMENT invalid snooze end sources
 *
 **********************************************************************************************************************/
static ssp_err_t check_snooze_config(lpmv2_snooze_dtc_t dtc_state_in_snooze,
                                     lpmv2_snooze_request_t snooze_request_source,
                                     lpmv2_snooze_end_bits_t snooze_end_sources)
{
    LPMV2_ERROR_RETURN(0U == ((uint32_t)dtc_state_in_snooze & (uint32_t)~(uint32_t)1U), SSP_ERR_INVALID_MODE);
    LPMV2_ERROR_RETURN(0U == ((uint32_t)snooze_request_source & ~HW_LPMV2_SNOOZE_REQUEST_MASK), SSP_ERR_INVALID_ARGUMENT);
    LPMV2_ERROR_RETURN(0U == ((uint32_t)snooze_end_sources & ~HW_LPMV2_SNOOZE_END_MASK), SSP_ERR_INVALID_ARGUMENT);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Verifies MCU specific settings related to output ports. Internal function, do not use directly.
 *
 * @param output_port_enable   output port mode
 *
 * @return true     output port mode is valid
 * @return false    invalid output port mode
 *
 **********************************************************************************************************************/
static bool check_output_port_config(lpmv2_output_port_enable_t output_port_enable)
{
    bool valid = false;

    if((LPMV2_OUTPUT_PORT_ENABLE_HIGH_IMPEDANCE == output_port_enable) ||
       (LPMV2_OUTPUT_PORT_ENABLE_RETAIN         == output_port_enable)
       )
    {
        valid = true;
    }

    return valid;
}
#endif /* LPMV2_CFG_PARAM_CHECKING_ENABLE */

/*******************************************************************************************************************//**
 * @brief Configures all MCU specific settings related to low power modes
 *
 * @param p_cfg the MCU specific configuration
 *
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
void HW_LPMV2_MCUSpecificConfigure(lpmv2_cfg_t const * const p_cfg)
{
    R_SYSTEM->SBYCR &= (uint16_t)~SBYCR_SSBY_MASK;

    R_SYSTEM->SNZCR &= (uint8_t)~SNZCR_SNZE_MASK;

    if((LPMV2_LOW_POWER_MODE_SLEEP != p_cfg->low_power_mode) &&
       (NULL != p_cfg->p_extend))
    {
        lpmv2_mcu_cfg_t * p_lpmv2_s3a3_cfg = (lpmv2_mcu_cfg_t * )(p_cfg->p_extend);

        HW_LPMV2_WUPENSet(p_lpmv2_s3a3_cfg->standby_wake_sources);

        R_SYSTEM->SBYCR_b.OPE = SBYCR_OPE_MASK & p_lpmv2_s3a3_cfg->output_port_enable;

        if(LPMV2_LOW_POWER_MODE_STANDBY_SNOOZE == p_cfg->low_power_mode)
        {
            HW_LPMV2_SnoozeConfigure(p_lpmv2_s3a3_cfg->dtc_state_in_snooze,
                                     p_lpmv2_s3a3_cfg->snooze_request_source,
                                     p_lpmv2_s3a3_cfg->snooze_end_sources,
                                     p_lpmv2_s3a3_cfg->snooze_cancel_sources);
            R_SYSTEM->SNZCR |= SNZCR_SNZE_MASK;
        }

        R_SYSTEM->SBYCR |= SBYCR_SSBY_MASK;
    }
}

/*******************************************************************************************************************//**
 * @brief Perform pre-WFI execution tasks, enter low power mode, Perform post-WFI execution tasks
 *
 * @note This function will unlock and lock registers as needed
 *
 * @return  SSP_SUCCESS                     Successfully entered and woke from low power mode.
 * @retval  SSP_ERR_INVALID_MODE            HOCO was not system clock when using snooze mode with SCI0/RXD0.
 * @retval  SSP_ERR_INVALID_MODE            MOCO was running when using snooze mode with SCI0/RXD0.
 * @retval  SSP_ERR_INVALID_MODE            MAIN OSCILLATOR was running when using snooze mode with SCI0/RXD0.
 * @retval  SSP_ERR_INVALID_MODE            PLL was running when using snooze mode with SCI0/RXD0.
 **********************************************************************************************************************/
ssp_err_t HW_LPMV2_LowPowerModeEnter (void)
{
    uint32_t ostdcr_ostde_prewfi = R_SYSTEM->OSTDCR_b.OSTDE;
    ssp_err_t ssp_err = SSP_SUCCESS;

    /* If snooze is enabled, and the snooze trigger is RXD0/DRX0, check a bunch of caveats */
    ssp_err = check_snooze_trigger_and_clocks();
    LPMV2_ERROR_RETURN(SSP_SUCCESS == ssp_err, ssp_err);

    /* If oscillator stop is enabled, disable then restore. */
    if((1U == R_SYSTEM->SBYCR_b.SSBY) && (1U == ostdcr_ostde_prewfi))
    {
        R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_CGC);
        R_SYSTEM->OSTDCR_b.OSTDE = 0U;
        R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_CGC);
        LPMV2_ERROR_RETURN(0U == R_SYSTEM->OSTDCR_b.OSTDE, SSP_ERR_INVALID_MODE);
    }

    /**
     * DSB should be last instruction executed before WFI
     * infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHICBGB.html
     */
     /*LDRA_INSPECTED 496 S Function call with no prior declaration. : __DSB MISRA-C:2012 R.17.3  */
    __DSB();

    /*LDRA_NOANALYSIS CMSIS related functions are not maintained by Renesas,
     *  so LDRA analysis is skipped for these files. */
    __WFI();
    /*LDRA_ANALYSIS */

    if((1U == R_SYSTEM->SBYCR_b.SSBY) && (1U == ostdcr_ostde_prewfi))
    {
        R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_CGC);

        /* Revert OSTDCR.OSTDE */
        R_SYSTEM->OSTDCR_b.OSTDE = 0x1U;

        R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_CGC);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Configure and enable snooze mode
 * This function enables the triggers that cause transition from standby mode to snooze mode.
 *
 * @note Snooze should be disabled before calling this function, and enabled after
 *
 * @retval SSP_SUCCESS             Snooze mode successfully enabled
 *
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
void HW_LPMV2_SnoozeConfigure (lpmv2_snooze_dtc_t dtc_state_in_snooze,
                               lpmv2_snooze_request_t request_source,
                               lpmv2_snooze_end_bits_t end_sources,
                               lpmv2_snooze_cancel_t cancel_sources)
{
    /* Configure RXD0 falling edge detect */
    if(LPMV2_SNOOZE_REQUEST_RXD0_FALLING == request_source)
    {
        R_SYSTEM->SNZCR |= SNZCR_RXDREQEN_MASK;
        /* Clear all other the request conditions that can trigger entry in to snooze mode */
        HW_LPMV2_SNZREQCRSet(0U);
    }
    else
    {
        R_SYSTEM->SNZCR &= (uint8_t)~SNZCR_RXDREQEN_MASK;
        /* Set the request condition that can trigger entry in to snooze mode */
        HW_LPMV2_SNZREQCRSet((uint32_t)request_source);
    }
    R_SYSTEM->SNZCR_b.SNZDTCEN = SNZCR_SNZDTCEN_MASK & (uint32_t)dtc_state_in_snooze;  /* Enable/disable DTC operation */
    HW_LPMV2_SELSR0Set(cancel_sources);  /* Set the source that can cause an exit from snooze to normal mode */
    HW_LPMV2_SNZEDCRSet((uint8_t)end_sources);  /* Set all sources that can cause an exit from snooze to standby mode */
}

/*******************************************************************************************************************//**
 * @brief  Wait for opccr and sopccr transition flags to clear. Internal function, do not use directly.
 *
 * @retval true     transition flags cleared
 * @retval false    transition flags not cleared
 *
 **********************************************************************************************************************/
bool HW_LPMV2_WaitForOperatingModeFlags(void)
{
    bool stable = false;
    int32_t timeout = 0xFFFF;
    /*SSP_LDRA_EXECUTION_INSPECTED */
    /* Cannot test, based on operating power control mode hardware flags */
    while(((0U != R_SYSTEM->OPCCR_b.OPCMTSF) || (0U != R_SYSTEM->SOPCCR_b.SOPCMTSF)) && (0 < timeout))
    {
        /* Wait for transition flags to clear */
        timeout--;
    }
    if(0 < timeout)
    {
        stable = true;
    }

    return stable;
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/* If snooze is enabled, and the snooze trigger is RXD0/DRX0, check a bunch of caveats */
static ssp_err_t check_snooze_trigger_and_clocks(void)
{
    cgc_clock_t clock_source = (cgc_clock_t)(0U - 1U);
    cgc_system_clock_cfg_t set_clock_cfg =
    {
        .pclka_div = CGC_SYS_CLOCK_DIV_1,
        .pclkb_div = CGC_SYS_CLOCK_DIV_1,
        .pclkc_div = CGC_SYS_CLOCK_DIV_1,
        .pclkd_div = CGC_SYS_CLOCK_DIV_1,
        .bclk_div = CGC_SYS_CLOCK_DIV_1,
        .fclk_div = CGC_SYS_CLOCK_DIV_1,
        .iclk_div = CGC_SYS_CLOCK_DIV_1,
    };
    /* If snooze is enabled, and the snooze trigger is RXD0/DRX0, check a bunch of caveats */
    if((1U == R_SYSTEM->SBYCR_b.SSBY) && (1U == R_SYSTEM->SNZCR_b.SNZE) && (0U == R_SYSTEM->SNZREQCR))
    {
        g_cgc_on_cgc.systemClockGet(&clock_source, &set_clock_cfg);
        LPMV2_ERROR_RETURN(CGC_CLOCK_HOCO == clock_source, SSP_ERR_INVALID_MODE);
        LPMV2_ERROR_RETURN(SSP_ERR_CLOCK_INACTIVE == g_cgc_on_cgc.clockCheck(CGC_CLOCK_MOCO), SSP_ERR_INVALID_MODE);
        LPMV2_ERROR_RETURN(SSP_ERR_CLOCK_INACTIVE == g_cgc_on_cgc.clockCheck(CGC_CLOCK_MAIN_OSC), SSP_ERR_INVALID_MODE);
        LPMV2_ERROR_RETURN(SSP_ERR_CLOCK_INACTIVE == g_cgc_on_cgc.clockCheck(CGC_CLOCK_PLL), SSP_ERR_INVALID_MODE);
    }
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup LPMV2_S3A3)
 **********************************************************************************************************************/

#endif
