/***********************************************************************************************************************
 * Copyright [2015-2024] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : r_slcdc.c
 * Description  : Driver for segment LCD controller (SLCDC)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_cgc.h"
#include "r_slcdc.h"
#include "r_slcdc_cfg.h"
#include "r_slcdc_private.h"
#include "r_slcdc_api.h"
#include "r_slcdc_private_api.h"
#include "./hw/common/hw_slcdc_common.h"
#include "string.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#ifndef SLCDC_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define SLCDC_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &module_version)
#endif

/* Control registers address defines */
/*LDRA_INSPECTED 78 S Parentheses cannot be added around macro parameters since they are used to construct variable
 * and section names that do not allow parentheses.*/
/*LDRA_INSPECTED 340 S inline functions doesn't return any value*/
#define SEG_ADR(n) ((volatile slcdc_size_t *) HW_SLCDC_GetSEG0Addr(p_ctrl->p_reg) + (0x01 * n))

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

static ssp_err_t r_slcdc_check_display_mode (slcdc_cfg_t const * const p_cfg);

static void r_slcdc_write_reg (slcdc_instance_ctrl_t * p_ctrl,
                               slcdc_size_t start_segment,
                               slcdc_size_t const * const p_data,
                               slcdc_size_t segment_count);
static ssp_err_t  r_slcdc_driver_config(slcdc_cfg_t const * const p_cfg,
                                      slcdc_instance_ctrl_t * p_ctrl, ssp_feature_t ssp_feature);
static ssp_err_t  r_check_support_for_wave_a(slcdc_cfg_t const * const p_cfg);
static ssp_err_t  r_condition_check_for_wave_a(slcdc_cfg_t const * const p_cfg);
static ssp_err_t  r_check_support_for_wave_b(slcdc_cfg_t const * const p_cfg);
static ssp_err_t  r_slcdc_clock_operation(slcdc_cfg_t const * const p_cfg,
                                      slcdc_instance_ctrl_t * p_ctrl, ssp_feature_t ssp_feature);
/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "slcdc";
#endif

#if defined(__GNUC__)
/* This structure is affected by warnings from the GCC compiler bug gcc.gnu.org/bugzilla/show_bug.cgi?id=60784
 * This pragma suppresses the warnings in this structure only, and will be removed when the SSP compiler is updated to
 * v5.3.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** SLCDC HAL module version data structure */
static const ssp_version_t module_version =
{
    .api_version_minor  = SLCDC_API_VERSION_MINOR,
    .api_version_major  = SLCDC_API_VERSION_MAJOR,
    .code_version_major = SLCDC_CODE_VERSION_MAJOR,
    .code_version_minor = SLCDC_CODE_VERSION_MINOR,
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/** LCD clock selection mapping. */
static const cgc_clock_t g_lcd_clock_mapping[] =
{
    [SLCDC_CLOCK_LOCO]      = CGC_CLOCK_LOCO,        ///<  Display clock source LOCO
    [SLCDC_CLOCK_SOSC]      = CGC_CLOCK_SUBCLOCK,    ///<  Display clock source SOSC
    [SLCDC_CLOCK_MOSC]      = CGC_CLOCK_MAIN_OSC,    ///<  Display clock source MOSC
    [SLCDC_CLOCK_HOCO]      = CGC_CLOCK_HOCO,        ///<  Display clock source HOCO
};

/** SLCDC HAL module API function pointer list */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const slcdc_api_t g_slcdc_on_slcdc =
{
    .open             = R_SLCDC_Open,
    .write            = R_SLCDC_Write,
    .modify           = R_SLCDC_Modify,
    .start            = R_SLCDC_Start,
    .stop             = R_SLCDC_Stop,
    .contrastIncrease = R_SLCDC_ContrastIncrease,
    .contrastDecrease = R_SLCDC_ContrastDecrease,
    .setdisplayArea   = R_SLCDC_SetDisplayArea,
    .close            = R_SLCDC_Close,
    .versionGet       = R_SLCDC_VersionGet,
};

/*******************************************************************************************************************//**
 * @addtogroup SLCDC
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @retval  SSP_SUCCESS                    Device was opened successfully.
 * @retval  SSP_ERR_ASSERTION              Pointer to the control block or the configuration structure is NULL.
 * @retval  SSP_ERR_HW_LOCKED              SLCDC resource is locked.
 * @return                                 See @ref Common_Error_Codes for other possible return codes. This function calls
 *                                         * fmi_api_t::productFeatureGet
 **********************************************************************************************************************/
ssp_err_t R_SLCDC_Open (slcdc_ctrl_t * const p_api_ctrl, slcdc_cfg_t const * const p_cfg)
{
    slcdc_instance_ctrl_t * p_ctrl = (slcdc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err     = SSP_SUCCESS;

    p_ctrl->state = SLCDC_DISPLAY_STATE_CLOSED;

#if (SLCDC_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_cfg);
#endif /* if (SLCDC_CFG_PARAM_CHECKING_ENABLE) */

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_SLCDC;
    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    SLCDC_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = (R_LCD_Type *) info.ptr;

    /** Configure the SLCDC driver */
    err = r_slcdc_driver_config(p_cfg, p_ctrl,ssp_feature );
    if(err != SSP_SUCCESS)
    {
        return err;
    }

    /** Record the configuration on the device for later use */
    memcpy(&p_ctrl->info, p_cfg, sizeof(p_ctrl->info));

    /** Mark control block state open so subsequent calls know the device is open. */
    p_ctrl->state = SLCDC_DISPLAY_STATE_OPENED;

    return err;
}  /* End of function R_SLCDC_Open() */

/*******************************************************************************************************************//**
 * @retval  SSP_SUCCESS                      Device was opened successfully.
 * @retval  SSP_ERR_ASSERTION                Pointer to the control block structure and display data is NULL
 * @retval  SSP_ERR_INVALID_ARGUMENT         Invalid parameter in the argument.
 * @retval  SSP_ERR_NOT_OPEN                 Device is not opened or initialized
 **********************************************************************************************************************/
ssp_err_t R_SLCDC_Write (slcdc_ctrl_t * const p_api_ctrl, slcdc_size_t const start_segment,
                                      slcdc_size_t const * const p_data, slcdc_size_t const segment_count)
{
    slcdc_instance_ctrl_t * p_ctrl = (slcdc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err = SSP_SUCCESS;

#if (SLCDC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(NULL != p_data);
    SLCDC_ERROR_RETURN(SLCDC_DISPLAY_STATE_CLOSED != p_ctrl->state, SSP_ERR_NOT_OPEN);

    if (((uint8_t)(MAX_NUM_SEG+1U)) < (start_segment + segment_count))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
#else
    SSP_PARAMETER_NOT_USED(p_ctrl);
#endif

    /** Get the register address of the specified segment and write data to it. */
    r_slcdc_write_reg(p_ctrl, start_segment, p_data, segment_count);

    return err;
}

/*******************************************************************************************************************//**
 * @retval  SSP_SUCCESS                      Device was opened successfully.
 * @retval  SSP_ERR_ASSERTION                Pointer to the control block structure is NULL.
 * @retval  SSP_ERR_INVALID_ARGUMENT         Invalid parameter in the argument.
 * @retval  SSP_ERR_NOT_OPEN                 Device is not opened or initialized
 **********************************************************************************************************************/
/*LDRA_INSPECTED 37 S Procedure parameter has a type and identifier too.*/
ssp_err_t R_SLCDC_Modify (slcdc_ctrl_t * const p_api_ctrl, slcdc_size_t const segment_number,
                                      slcdc_size_t const data, slcdc_size_t const data_mask)
{
    slcdc_instance_ctrl_t * p_ctrl = (slcdc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err = SSP_SUCCESS;

#if (SLCDC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_ctrl);
    SLCDC_ERROR_RETURN(SLCDC_DISPLAY_STATE_CLOSED != p_ctrl->state, SSP_ERR_NOT_OPEN);

    if (MAX_NUM_SEG < segment_number)
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
#else
    SSP_PARAMETER_NOT_USED(p_ctrl);
#endif

    volatile slcdc_size_t * const pSEG_ADR = SEG_ADR(segment_number);

    /** Masks the data being displayed. */
    *pSEG_ADR &= data_mask;

    /** Specifies the data to rewrite. */
    *pSEG_ADR |= data;

    return err;
}

/*******************************************************************************************************************//**
 * @retval  SSP_SUCCESS                      Device was opened successfully.
 * @retval  SSP_ERR_ASSERTION                Pointer to the control block structure is NULL.
 * @retval  SSP_ERR_NOT_OPEN                 Device is not opened or initialized
 **********************************************************************************************************************/
ssp_err_t R_SLCDC_Start (slcdc_ctrl_t * const p_api_ctrl)
{
    slcdc_instance_ctrl_t * p_ctrl = (slcdc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err = SSP_SUCCESS;

#if (SLCDC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_ctrl);
    SLCDC_ERROR_RETURN(SLCDC_DISPLAY_STATE_CLOSED != p_ctrl->state, SSP_ERR_NOT_OPEN);
#endif

    /** Enable the voltage boost circuit or capacitor split circuit. */
    if ((SLCDC_VOLT_INTERNAL == p_ctrl->info.drive_volt_gen) || (SLCDC_VOLT_CAPACITOR == p_ctrl->info.drive_volt_gen))
    {
        HW_SLCDC_VoltageGeneratorCircuitEnable(p_ctrl->p_reg);
    }

    /** Enable the LCD display. */
    HW_SLCDC_LcdON(p_ctrl->p_reg);

    return err;
}

/*******************************************************************************************************************//**
 * @retval  SSP_SUCCESS                      Device was opened successfully.
 * @retval  SSP_ERR_ASSERTION                Pointer to the control block structure is NULL.
 * @retval  SSP_ERR_NOT_OPEN                 Device is not opened or initialized
 **********************************************************************************************************************/
ssp_err_t R_SLCDC_Stop (slcdc_ctrl_t * const p_api_ctrl)
{
    slcdc_instance_ctrl_t * p_ctrl = (slcdc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err = SSP_SUCCESS;

#if (SLCDC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_ctrl);
    SLCDC_ERROR_RETURN(SLCDC_DISPLAY_STATE_CLOSED != p_ctrl->state, SSP_ERR_NOT_OPEN);
#endif

    /** Disable the LCD display */
    HW_SLCDC_LcdOFF(p_ctrl->p_reg);

    if ((SLCDC_VOLT_INTERNAL == p_ctrl->info.drive_volt_gen) || (SLCDC_VOLT_CAPACITOR == p_ctrl->info.drive_volt_gen))
    {
        /** Disable the voltage boost circuit or capacitor split circuit. */
        HW_SLCDC_VoltageGeneratorCircuitDisable(p_ctrl->p_reg);
    }

    return err;
}

/*******************************************************************************************************************//**
 * @retval  SSP_SUCCESS                      Device was opened successfully.
 * @retval  SSP_ERR_ASSERTION                Pointer to the control block structure is NULL.
 * @retval  SSP_ERR_NOT_OPEN                 Device is not opened or initialized
 * @retval  SSP_ERR_UNSUPPORTED              Unsupported operation
 **********************************************************************************************************************/
ssp_err_t R_SLCDC_ContrastIncrease (slcdc_ctrl_t * const p_api_ctrl)
{
    slcdc_instance_ctrl_t * p_ctrl = (slcdc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err = SSP_SUCCESS;
    slcdc_size_t   slcd_voltage;
#if (SLCDC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_ctrl);
    SLCDC_ERROR_RETURN(SLCDC_DISPLAY_STATE_CLOSED != p_ctrl->state, SSP_ERR_NOT_OPEN);
#endif

    /** The VLCD setting is valid only when the voltage boost circuit is operating */
    if (SLCDC_VOLT_INTERNAL != p_ctrl->info.drive_volt_gen)
    {
        return SSP_ERR_UNSUPPORTED;
    }

    /** Verify the new volt is within the range. */
    slcd_voltage = HW_SLCDC_GetRefVolatge(p_ctrl->p_reg);

    slcd_voltage++;

    if ((SLCDC_BIAS_2 == p_ctrl->info.bias_method) || (SLCDC_BIAS_3 == p_ctrl->info.bias_method))
    {
        if (((uint8_t)SLCDC_VOL_MAX) < slcd_voltage)
        {
            return SSP_ERR_UNSUPPORTED;
        }
    }
    /* This condition is for "SLCDC_BIAS_4" */
    else
    {
        if (((uint8_t)SLCDC_VOL_MAX_4BIAS) < slcd_voltage)
        {
            return SSP_ERR_UNSUPPORTED;
        }
    }

    /** Stop the internal voltage boost/capacitor split circuit. */
    HW_SLCDC_VoltageGeneratorCircuitDisable(p_ctrl->p_reg);

    /** Set new voltage value. */
    HW_SLCDC_SetRefVolatge(p_ctrl->p_reg, slcd_voltage);

    /** Wait 5ms minimum as per HW manual */
    R_BSP_SoftwareDelay((uint32_t)SLCDC_SETUP_WAIT, BSP_DELAY_UNITS_MILLISECONDS);

    /** Enable the voltage boost circuit or capacitor split circuit. */
    HW_SLCDC_VoltageGeneratorCircuitEnable(p_ctrl->p_reg);

    return err;
}

/*******************************************************************************************************************//**
 * @retval  SSP_SUCCESS                      Device was opened successfully.
 * @retval  SSP_ERR_ASSERTION                Pointer to the control block structure is NULL.
 * @retval  SSP_ERR_NOT_OPEN                 Device is not opened or initialized
 * @retval  SSP_ERR_UNSUPPORTED              Unsupported operation
 **********************************************************************************************************************/
ssp_err_t R_SLCDC_ContrastDecrease (slcdc_ctrl_t * const p_api_ctrl)
{
    slcdc_instance_ctrl_t * p_ctrl = (slcdc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err = SSP_SUCCESS;
    slcdc_size_t   slcd_voltage;
#if (SLCDC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_ctrl);
    SLCDC_ERROR_RETURN(SLCDC_DISPLAY_STATE_CLOSED != p_ctrl->state, SSP_ERR_NOT_OPEN);
#endif

    /** The VLCD setting is valid only when the voltage boost circuit is operating */
    if (SLCDC_VOLT_INTERNAL != p_ctrl->info.drive_volt_gen)
    {
        return SSP_ERR_UNSUPPORTED;
    }

    /** Verify the new volt is within the range. */
    slcd_voltage = HW_SLCDC_GetRefVolatge(p_ctrl->p_reg);

    slcd_voltage--;

    if (((uint8_t)SLCDC_VOL_MIN) > slcd_voltage)
    {
        return SSP_ERR_UNSUPPORTED;
    }

    /** Stop the internal voltage boost/capacitor split circuit. */
    HW_SLCDC_VoltageGeneratorCircuitDisable(p_ctrl->p_reg);

    /** Set new voltage value. */
    HW_SLCDC_SetRefVolatge(p_ctrl->p_reg, slcd_voltage);

    /** Wait 5ms minimum as per HW manual */
    R_BSP_SoftwareDelay((uint32_t)SLCDC_SETUP_WAIT, BSP_DELAY_UNITS_MILLISECONDS);

    /** Enable the voltage boost circuit or capacitor split circuit. */
    HW_SLCDC_VoltageGeneratorCircuitEnable(p_ctrl->p_reg);

    return err;
}

/*******************************************************************************************************************//**
 * @retval  SSP_SUCCESS                      Device was opened successfully.
 * @retval  SSP_ERR_ASSERTION                Pointer to the control block structure is NULL.
 * @retval  SSP_ERR_UNSUPPORTED              Unsupported operation
 * @retval  SSP_ERR_NOT_OPEN                 Device is not opened or initialized
 * @retval  SSP_ERR_NOT_ENABLED              RTC not enabled for blink operation
 * @return                                   See @ref Common_Error_Codes for other possible return codes. This function calls
 *                                           * fmi_api_t::productFeatureGet
 **********************************************************************************************************************/
ssp_err_t R_SLCDC_SetDisplayArea (slcdc_ctrl_t * const p_api_ctrl, slcdc_display_area_t const display_area)
{
    slcdc_instance_ctrl_t * p_ctrl = (slcdc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err = SSP_SUCCESS;

#if (SLCDC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_ctrl);
    SLCDC_ERROR_RETURN(SLCDC_DISPLAY_STATE_CLOSED != p_ctrl->state, SSP_ERR_NOT_OPEN);
#endif

    /** When the number of time slices is eight, LCD display data registers
     * (A-pattern, B-pattern, or blinking display) cannot be selected. */
    if (SLCDC_SLICE_8 ==  p_ctrl->info.time_slice)
    {
        return SSP_ERR_UNSUPPORTED;
    }

    /** Set the LCD display data area. */

    if ((SLCDC_DISP_A == display_area) || (SLCDC_DISP_B == display_area))
    {
        HW_SLCDC_BlinkOFF(p_ctrl->p_reg);
        HW_SLCDC_SetDisplayArea(p_ctrl->p_reg, display_area);
    }
    /* This condition is for "SLCDC_DISP_BLINK" */
    else
    {
#if (SLCDC_CFG_PARAM_CHECKING_ENABLE)
        /* Check RTC Periodic interrupt is enabled for blinking*/
        ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
        ssp_feature.channel = 0U;
        ssp_feature.unit = 0U;
        ssp_feature.id = SSP_IP_RTC;
        fmi_feature_info_t info = {0U};
        err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
        SLCDC_ERROR_RETURN(SSP_SUCCESS == err, err);
        if (false == HW_SLCDC_GetRtcPIEbit((R_RTC_Type *) info.ptr))
        {
            return SSP_ERR_NOT_ENABLED;
        }
#endif

        HW_SLCDC_BlinkON(p_ctrl->p_reg);
        HW_SLCDC_SetDisplayArea(p_ctrl->p_reg, SLCDC_DISP_A);     /* Does't matter A or B, if BLON =1*/
    }

    return err;
}

/*******************************************************************************************************************//**
 * @retval  SSP_SUCCESS                      Device was opened successfully.
 * @retval  SSP_ERR_ASSERTION                Pointer to the control block structure is NULL.
 * @retval  SSP_ERR_NOT_OPEN                 Device is not opened or initialized
 **********************************************************************************************************************/
ssp_err_t R_SLCDC_Close (slcdc_ctrl_t * const p_api_ctrl)
{
    slcdc_instance_ctrl_t * p_ctrl = (slcdc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err = SSP_SUCCESS;
    slcdc_size_t seg_reset_data = 0x00;

#if (SLCDC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_ctrl);
    SLCDC_ERROR_RETURN(SLCDC_DISPLAY_STATE_CLOSED != p_ctrl->state, SSP_ERR_NOT_OPEN);
#endif

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_SLCDC;

    for (slcdc_size_t cnt = (uint8_t)0; cnt <= ((uint8_t)MAX_NUM_SEG); cnt++)
    {
        /** Clear all the data segment registers. */
        r_slcdc_write_reg(p_ctrl, 0x00, &seg_reset_data, (uint8_t)1);
    }

    /** Disable the LCD display area. Segment pin outputs de-select signal */
    HW_SLCDC_LcdOFF(p_ctrl->p_reg);

    if ((SLCDC_VOLT_INTERNAL == p_ctrl->info.drive_volt_gen) || (SLCDC_VOLT_CAPACITOR == p_ctrl->info.drive_volt_gen))
    {
        /** Disable voltage circuit */
        HW_SLCDC_VoltageGeneratorCircuitDisable(p_ctrl->p_reg);

        /** Set voltage generator to External */
        HW_SLCDC_DriveVoltageGenerator(p_ctrl->p_reg, SLCDC_VOLT_EXTERNAL);
    }

    /** Protect OFF of CGC. */
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_CGC);

    /** Disable LCD clock */
    g_cgc_on_cgc.lcdClockDisable();

    /** Protect ON of CGC. */
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_CGC);

    /** Release hardware lock for SLCD */
    R_BSP_HardwareUnlock(&ssp_feature);

    /** SLCDC Power off - enter module-stop state for the SLCDC */
    R_BSP_ModuleStop(&ssp_feature);

    /** Mark control block close. */
    p_ctrl->state = SLCDC_DISPLAY_STATE_CLOSED;

    return err;
}

/*******************************************************************************************************************//**
 * @brief   Retrieve the API version number.
 *
 * @retval  SSP_SUCCESS        Successful return.
 * @retval  SSP_ERR_ASSERTION  The parameter p_version is NULL.
***********************************************************************************************************************/
ssp_err_t R_SLCDC_VersionGet (ssp_version_t * const p_version)
{
#if (1 == SLCDC_CFG_PARAM_CHECKING_ENABLE)
    /** Verify parameters are valid */
    SSP_ASSERT(NULL != p_version);
#endif

    *p_version = module_version;
    return SSP_SUCCESS;
}  /* End of function R_SLCDC_VersionGet() */

/*******************************************************************************************************************//**
 * @} (end addtogroup R_SLCDC)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @brief   This is the subroutine to check the combination of display mode. Refer Table 50.7 of HW
 *manual(R01UH0583EU0070 Rev.0.70) for details.
 * @param[in]     p_cfg   Pointer to the configuration structure for slcdc interface
 * @retval  SSP_SUCCESS                      Valid combination of display mode.
 * @retval  SSP_ERR_UNSUPPORTED              Invalid combination of display mode.
 **********************************************************************************************************************/
static ssp_err_t r_slcdc_check_display_mode (slcdc_cfg_t const * const p_cfg)
{
    ssp_err_t err     = SSP_SUCCESS;
    if ((p_cfg->wave_form) == SLCDC_WAVE_A )
    {
        /*Check combination of WAVE-A*/
        err = r_check_support_for_wave_a(p_cfg);
    }
    else
    {
        /*Check combination of WAVE-B*/
        err = r_check_support_for_wave_b(p_cfg);
    }
    SLCDC_ERROR_RETURN(SSP_SUCCESS == err, err);

    return SSP_SUCCESS;
}  /* End of function r_slcdc_check_display_mode() */

/*******************************************************************************************************************//**
 * @brief   This is the subroutine to get the register address of the specified segment and write data to it. Take start
 * segment address and write to count number of segment registers.
 * @param[in]     p_ctrl          Control structure
 * @param[in]     start_segment   Start segment number
 * @param[in]     p_data           Pointer to segment data
 * @param[in]     segment_count   count
 * @retval  None
 **********************************************************************************************************************/
static void r_slcdc_write_reg (slcdc_instance_ctrl_t * p_ctrl,
                               slcdc_size_t start_segment,
                               slcdc_size_t const * const p_data,
                               slcdc_size_t segment_count)
{
    slcdc_size_t count;
    volatile slcdc_size_t * pSEG_ADR = SEG_ADR(start_segment);
    slcdc_size_t *data = (slcdc_size_t *)p_data;

    /* The display data is stored in the display data register. */
    for(count=(uint8_t)0; count< segment_count; count++)
    {
       *pSEG_ADR = *data;
       pSEG_ADR++;
       data++;
    }
}

/*******************************************************************************************************************//**
 * @brief   This is the subroutine to configure the SLCDC driver.
 *
 * @retval  SSP_SUCCESS                      Device was opened successfully.
 * @retval  SSP_ERR_HW_LOCKED                SLCDC resource is locked.
**********************************************************************************************************************/
static ssp_err_t  r_slcdc_driver_config(slcdc_cfg_t const * const p_cfg,
                                      slcdc_instance_ctrl_t * p_ctrl, ssp_feature_t ssp_feature)
{
    ssp_err_t err     = SSP_SUCCESS;

    /* Check combination of display mode */
    err = r_slcdc_check_display_mode(p_cfg);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

     /** Lock the SLCD resource */
    if (SSP_SUCCESS != R_BSP_HardwareLock(&ssp_feature))
    {
        return SSP_ERR_HW_LOCKED;
    }

    R_BSP_ModuleStart(&ssp_feature);

    /* Specifies the voltage boosting pin initial value switching control bit */
    if (SLCDC_VOLT_INTERNAL == p_cfg->drive_volt_gen)
    {
         HW_SLCDC_VoltageBoostPinInitSet(p_ctrl->p_reg);
    }

    /* Set Waveform*/
    HW_SLCDC_SetWaveform(p_ctrl->p_reg, p_cfg->wave_form);

    /* Set time slice*/
    HW_SLCDC_SetTimeSlice(p_ctrl->p_reg, p_cfg->time_slice);

    /* Set bias method*/
    HW_SLCDC_SetBiasemethod(p_ctrl->p_reg, p_cfg->bias_method);

    /* Set drive voltage generator*/
    HW_SLCDC_DriveVoltageGenerator(p_ctrl->p_reg, p_cfg->drive_volt_gen);

    /* Specifies the display data area. */
    if (SLCDC_SLICE_8 !=  p_cfg->time_slice)
    {
        HW_SLCDC_BlinkOFF(p_ctrl->p_reg);
        HW_SLCDC_SetDisplayArea(p_ctrl->p_reg, SLCDC_DISP_A);
    }

    /* Do clock operations required for SLCDC */
    err = r_slcdc_clock_operation(p_cfg, p_ctrl, ssp_feature);
    SLCDC_ERROR_RETURN((SSP_SUCCESS == err), err);

    /* Specifies the voltage level. */
    if (SLCDC_VOLT_INTERNAL == p_cfg->drive_volt_gen)
    {
        /* Disable voltage circuit*/
        HW_SLCDC_VoltageGeneratorCircuitDisable(p_ctrl->p_reg);

        HW_SLCDC_SetRefVolatge(p_ctrl->p_reg,(uint32_t)SLCDC_CFG_REF_VCC);

        /* Wait 5ms minimum as per HW manual*/
        R_BSP_SoftwareDelay((uint32_t)SLCDC_SETUP_WAIT, BSP_DELAY_UNITS_MILLISECONDS);
    }

    /* Enable the voltage boost circuit or capacitor split circuit. */
    if ((SLCDC_VOLT_INTERNAL == p_cfg->drive_volt_gen) || (SLCDC_VOLT_CAPACITOR == p_cfg->drive_volt_gen))
    {
        HW_SLCDC_VoltageGeneratorCircuitEnable(p_ctrl->p_reg);
    }

     return err;
 }

/*******************************************************************************************************************//**
 * @brief   Checks the supported features with respect to wave form A

  * @retval  SSP_SUCCESS                      Valid combination of display mode.
 * @retval  SSP_ERR_UNSUPPORTED              Invalid combination of display mode.
 **********************************************************************************************************************/
static ssp_err_t  r_check_support_for_wave_a(slcdc_cfg_t const * const p_cfg)
{
    if ((SLCDC_SLICE_8 == p_cfg->time_slice) && (SLCDC_BIAS_4 == p_cfg->bias_method)
                 && ((SLCDC_VOLT_EXTERNAL == p_cfg->drive_volt_gen) || (SLCDC_VOLT_INTERNAL == p_cfg->drive_volt_gen)))
    {
        /* Do nothing */
    }

    else if ((SLCDC_SLICE_4 == p_cfg->time_slice) && (SLCDC_BIAS_3 == p_cfg->bias_method))
    {
        /* Do nothing */
    }
    else if (SSP_SUCCESS != r_condition_check_for_wave_a(p_cfg))
    {
        return SSP_ERR_UNSUPPORTED;
    }
    return SSP_SUCCESS;
 }

/*******************************************************************************************************************//**
 * @brief   Checks the supported features with respect to wave form A

 * @retval  SSP_SUCCESS                      Valid combination of display mode.
 * @retval  SSP_ERR_UNSUPPORTED              Invalid combination of display mode.
 **********************************************************************************************************************/
static ssp_err_t r_condition_check_for_wave_a(slcdc_cfg_t const * const p_cfg)
{

    if ((SLCDC_SLICE_3 == p_cfg->time_slice) && ((SLCDC_BIAS_3 == p_cfg->bias_method)
            || ((SLCDC_BIAS_2 == p_cfg->bias_method) && (SLCDC_VOLT_EXTERNAL == p_cfg->drive_volt_gen))))
    {
        /* Do nothing */
    }

    else if ((SLCDC_SLICE_2 == p_cfg->time_slice) && (SLCDC_BIAS_2 == p_cfg->bias_method)
             && (SLCDC_VOLT_EXTERNAL == p_cfg->drive_volt_gen))
    {
        /* Do nothing */
    }
    else if ((SLCDC_STATIC == p_cfg->time_slice) && (SLCDC_VOLT_EXTERNAL == p_cfg->drive_volt_gen))
    {
        /* Do nothing */
    }
    else
    {
        return SSP_ERR_UNSUPPORTED;
    }
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   Checks the supported features with respect to wave form B

  * @retval  SSP_SUCCESS                      Valid combination of display mode.
 * @retval  SSP_ERR_UNSUPPORTED               Invalid combination of display mode.
 **********************************************************************************************************************/
static ssp_err_t  r_check_support_for_wave_b(slcdc_cfg_t const * const p_cfg)
{
    if ((SLCDC_SLICE_8 == p_cfg->time_slice) && (SLCDC_BIAS_4 == p_cfg->bias_method) &&
            ((SLCDC_VOLT_EXTERNAL == p_cfg->drive_volt_gen) || (SLCDC_VOLT_INTERNAL == p_cfg->drive_volt_gen)))
    {
        /* Do nothing */
    }
    else if ((SLCDC_SLICE_4 == p_cfg->time_slice) && (SLCDC_BIAS_3 == p_cfg->bias_method))
    {
        /* Do nothing */
    }
    else
    {
        return SSP_ERR_UNSUPPORTED;
    }
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   Starts the HOCO clock when SLCDC clock source is selected as HOCO. Set other clock operations required
 * 	        for SLCDC.
 *
 * @retval  SSP_SUCCESS       HOCO clock activated successfully.
 * @retval  SSP_ERR_TIMEOUT   Not configured within the mentioned time.
 * @return                    See @ref Common_Error_Codes for other possible return codes. This function calls
 *                            * cgc_api_t.clockStart
 *                            * cgc_api_t.lcdClockCfg
 *                            * cgc_api_t.lcdClockDisable
 *                            * cgc_api_t.lcdClockEnable
***********************************************************************************************************************/

static ssp_err_t r_slcdc_clock_operation(slcdc_cfg_t const * const p_cfg,
                                      slcdc_instance_ctrl_t * p_ctrl, ssp_feature_t ssp_feature)
{
    ssp_err_t err = SSP_SUCCESS;

    cgc_clock_cfg_t clk_cfg;

    /*Start HOCO clock*/
    if (SLCDC_CLOCK_HOCO == p_cfg->slcdc_clock)
    {
        err = g_cgc_on_cgc.clockStart(CGC_CLOCK_HOCO, &clk_cfg);

        if((SSP_SUCCESS != err) && (SSP_ERR_CLOCK_ACTIVE != err))
        {
            /* Release hardware lock for SLCD before leaving*/
            R_BSP_HardwareUnlock(&ssp_feature);

            /* SLCDC Power off - enter module-stop state for the SLCDC */
            R_BSP_ModuleStop(&ssp_feature);
            return err;
        }
    }

    /*Disable LCD clock*/
    g_cgc_on_cgc.lcdClockDisable();

    /* Set Division of LCDC clock source. */
    HW_SLCDC_SetClockDivision(p_ctrl->p_reg,(uint32_t)p_cfg->slcdc_clock_setting);

    /* Set clock source for LCDC. */
    err = g_cgc_on_cgc.lcdClockCfg(g_lcd_clock_mapping[p_cfg->slcdc_clock]);

    if (SSP_SUCCESS != err)
    {
        /* Release hardware lock for SLCD before leaving*/
        R_BSP_HardwareUnlock(&ssp_feature);

        /* SLCDC Power off - enter module-stop state for the SLCDC */
        R_BSP_ModuleStop(&ssp_feature);
    }
    SLCDC_ERROR_RETURN(SSP_SUCCESS == err, SSP_ERR_TIMEOUT);

    /*Enable LCD clock*/
    g_cgc_on_cgc.lcdClockEnable();

    return err;
}
