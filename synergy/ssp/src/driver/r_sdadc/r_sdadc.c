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
/***********************************************************************************************************************
* File Name    : r_sdadc.c
* Description  : Primary source code for Sigma Delta A/D Converter driver.
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/

#include "bsp_api.h"
#include "r_adc_api.h"
/* Configuration for this package. */
#include "r_sdadc_cfg.h"
/* Private header file for this package. */
#include "r_sdadc.h"

#include "hw/hw_sdadc_private.h"
#include "r_cgc.h"
#include "r_sdadc_private_api.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#if BSP_CFG_SDADCCLK_DIV == 8
#define SDADC_PRIV_CLK_DIV  (16U)
#elif BSP_CFG_SDADCCLK_DIV == 7
#define SDADC_PRIV_CLK_DIV  (12U)
#elif BSP_CFG_SDADCCLK_DIV == 6
#define SDADC_PRIV_CLK_DIV  (8U)
#else
#define SDADC_PRIV_CLK_DIV  (BSP_CFG_SDADCCLK_DIV + 1U)
#endif

#if BSP_CFG_SDADC_CLOCK_SOURCE == 0
#define SDADC_PRIV_CLK_INPUT (BSP_HOCO_HZ)
#else
#define SDADC_PRIV_CLK_INPUT (BSP_CFG_XTAL_HZ)
#endif

#define SDADC_PRIV_CLK_HZ                  (SDADC_PRIV_CLK_INPUT / SDADC_PRIV_CLK_DIV)
#define SDADC_PRIV_NORMAL_MODE_CLK_HZ      (4000000U)
#define SDADC_PRIV_LOW_POWER_MODE_CLK_HZ   (500000U)

#if (SDADC_PRIV_CLK_HZ != SDADC_PRIV_NORMAL_MODE_CLK_HZ) && (SDADC_PRIV_CLK_HZ != SDADC_PRIV_LOW_POWER_MODE_CLK_HZ)
#error "SDADC clock must be 4 MHz"
#endif

#define SDADC_PRIV_PGAC_PGAGC_S2_SHIFT (0)
#define SDADC_PRIV_PGAC_PGAGC_S1_SHIFT (2)
#define SDADC_PRIV_PGAC_PGAOSR_SHIFT   (5)
#define SDADC_PRIV_PGAC_PGAOFS_SHIFT   (8)
#define SDADC_PRIV_PGAC_PGAPOL_SHIFT   (14)
#define SDADC_PRIV_PGAC_PGASEL_SHIFT   (15)
#define SDADC_PRIV_PGAC_PGACTM_SHIFT   (16)
#define SDADC_PRIV_PGAC_PGACTN_SHIFT   (21)
#define SDADC_PRIV_PGAC_PGAAVN_SHIFT   (24)
#define SDADC_PRIV_PGAC_PGAREV_SHIFT   (28)
#define SDADC_PRIV_PGAC_PGACVE_SHIFT   (30)
#define SDADC_PRIV_PGAC_PGAASN_SHIFT   (31)

#define SDADC_PRIV_PGAC_DEFAULT   (0x00010040U)

#define SDADC_PRIV_ADC1_SDADBMP_SHIFT (8U)
#define SDADC_PRIV_ADC1_SDADTMD_SHIFT (4U)

/* Calibration was measured to take 3.4 ms per channel.  A 10% buffer is added.  So the timeout is 3.4 ms * 5 * 1.10 = 19 ms. */
#define SDADC_PRIV_CALIBRATION_TIMEOUT_US (19000U)

#define SDADC_PRIV_8_BITS         (8U)
#define SDADC_PRIV_16_BITS        (16U)

/** Macro for error logger. */
#ifndef SDADC_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define SDADC_ERROR_RETURN(a, err)  SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_sdadc_version)
#endif

#define SDADC_OPEN                        (0x53444144U)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
static ssp_err_t r_sdadc_open_param_check(sdadc_instance_ctrl_t * const p_ctrl, adc_cfg_t const * const p_cfg);

static ssp_err_t r_sdadc_open_param_check_2(adc_cfg_t const * const p_cfg);

static ssp_err_t r_sdadc_open_param_check_3(adc_cfg_t const * const p_cfg);

static ssp_err_t r_sdadc_calibrate_param_check(sdadc_instance_ctrl_t * const p_ctrl, sdadc_calibrate_args_t * p_calibrate);
#endif

static ssp_err_t r_sdadc_open_prepare(sdadc_instance_ctrl_t * const p_ctrl, adc_cfg_t const * const p_cfg);

static void r_sdadc_open_vref_voltage_set(sdadc_instance_ctrl_t * p_ctrl, sdadc_vref_voltage_t voltage);

static ssp_err_t r_sdadc_open_wait_for_calibration(sdadc_instance_ctrl_t * const p_ctrl);

static ssp_err_t r_sdadc_open_irq_cfg(sdadc_instance_ctrl_t * const p_ctrl, adc_cfg_t const * const p_cfg, ssp_feature_t * p_feature);

static IRQn_Type r_sdadc_open_irq_cfg_sub(sdadc_instance_ctrl_t * const p_ctrl,
                                       ssp_feature_t         * const p_feature,
                                       ssp_signal_t                  signal,
                                       uint8_t                       priority);

static void r_sdadc_scan_configure(sdadc_instance_ctrl_t * const p_ctrl, uint32_t scan_mask, adc_mode_t mode, adc_trigger_t trigger);

static void r_sdadc_calibrate_end_restore(sdadc_instance_ctrl_t * const p_ctrl);

static void r_sdadc_disable_irq(IRQn_Type irq);

void sdadc_adi_isr(void);
void sdadc_scanend_isr(void);
void sdadc_caliend_isr(void);

/** Version data structure used by error logger macro. */
static const ssp_version_t g_sdadc_version =
{
        .api_version_minor  = ADC_API_VERSION_MINOR,
        .api_version_major  = ADC_API_VERSION_MAJOR,
        .code_version_major = SDADC_CODE_VERSION_MAJOR,
        .code_version_minor = SDADC_CODE_VERSION_MINOR
};

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "sdadc";
#endif

/***********************************************************************************************************************
 Global Variables
 **********************************************************************************************************************/

/** SDADC Implementation of ADC interface. */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const adc_api_t g_adc_on_sdadc =
{
    .open                   = R_SDADC_Open,
    .scanCfg                = R_SDADC_ScanConfigure,
    .infoGet                = R_SDADC_InfoGet,
    .scanStart              = R_SDADC_ScanStart,
    .scanStop               = R_SDADC_ScanStop,
    .scanStatusGet          = R_SDADC_CheckScanDone,
    .sampleStateCountSet    = R_SDADC_SampleStateCountSet,
    .offsetSet              = R_SDADC_OffsetSet,
    .read                   = R_SDADC_Read,
    .read32                 = R_SDADC_Read32,
    .calibrate              = R_SDADC_Calibrate,
    .close                  = R_SDADC_Close,
    .versionGet             = R_SDADC_VersionGet
};

/*******************************************************************************************************************//**
 * @addtogroup SDADC
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Applies power to the SDADC and initializes the hardware based on the user configuration. As part of this
 * initialization, the SDADC clock is configured and enabled.  If an interrupt priority is non-zero, enables an
 * interrupt which will call a callback to notify the user when a conversion, scan, or calibration is complete.  For
 * all channels that are configured in differential mode, calibration is performed unless it is disabled in the user
 * configuration. Implements adc_api_t::open().
 *
 * @retval  SSP_SUCCESS                Configuration and calibration successful.
 * @retval  SSP_ERR_CALIBRATE_FAILED   Calibration failed.
 * @retval  SSP_ERR_ASSERTION          An input pointer is NULL.
 * @retval  SSP_ERR_INVALID_ARGUMENT   An input argument is invalid.
 * @retval  SSP_ERR_IN_USE             Control block is already open.
 * @retval  SSP_ERR_IRQ_BSP_DISABLED   A required interrupt is disabled
 * @return                             See @ref Common_Error_Codes or functions called by this function for other possible
 *                                     return codes. This function calls:
 *                                        * fmi_api_t::productFeatureGet
***********************************************************************************************************************/
ssp_err_t R_SDADC_Open(adc_ctrl_t * p_api_ctrl,  adc_cfg_t const * const p_cfg)
{
    sdadc_instance_ctrl_t * p_ctrl = (sdadc_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;

    /** Ensure the SDADC exists on the hardware, enable interrupts, and reserve the SDADC hardware lock. */
    err = r_sdadc_open_prepare(p_ctrl, p_cfg);
    SDADC_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* SDADC clock must be disabled to begin power supply activation. */
    g_cgc_on_cgc.sdadcClockDisable();

    /** Initialize the hardware based on the configuration. */

    /** Start the SDADC. */
    ssp_feature_t adc_feature = {{(ssp_ip_t) 0U}};
    adc_feature.id = SSP_IP_SDADC;
    adc_feature.channel = p_cfg->unit;
    adc_feature.unit = 0U;
    R_BSP_ModuleStart(&adc_feature);

    /* Apply default settings to unused registers. */
    HW_SDADC_RegisterReset(p_ctrl->p_reg);

    /** Configure the SDADC clock source and divisor to the clock source selected in the clock configuration. */
    g_cgc_on_cgc.sdadcClockCfg((cgc_clock_t) BSP_CFG_SDADC_CLOCK_SOURCE);
    HW_SDADC_ClkdivSet(p_ctrl->p_reg, BSP_CFG_SDADCCLK_DIV);

    /** Set the reference voltage for sensors (internal or external VREF mode). */
    adc_on_sdadc_cfg_t const * p_cfg_extend = p_cfg->p_extend;
    HW_SDADC_VrefSourceSet(p_ctrl->p_reg, p_cfg_extend->vref_src);
    r_sdadc_open_vref_voltage_set(p_ctrl, p_cfg_extend->vref_voltage);

    /** Set the A/D conversion operation mode (normal or low power mode). */
    HW_SDADC_OperationModeSet(p_ctrl->p_reg, SDADC_POWER_MODE_NORMAL);

    /** Supply the 24-bit sigma-delta A/D converter clock (SDADCCLK). */
    g_cgc_on_cgc.sdadcClockEnable();

    /** Turn on the power of ADBGR, SBIAS, and ADREG. */
    HW_SDADC_BgrPowerOn(p_ctrl->p_reg);

    /** Stabilization wait time of 2 ms is required between power on of ADBGR/SBIAS/ADREG and VBIAS/channel/SDADC. */
    R_BSP_SoftwareDelay(2U, BSP_DELAY_UNITS_MILLISECONDS);

    /** Turn on the power of VBIAS, channel, and sigma-delta A/D converter. */
    HW_SDADC_AdcPowerOn(p_ctrl->p_reg);

    /** For each channel: */
    uint32_t scan_mask = 0U;
    bool calibrate = false;
    sdadc_calibrate_args_t calibrate_args;
    calibrate_args.mode = SDADC_CALIBRATION_INTERNAL_GAIN_OFFSET;
    calibrate_args.channel = ADC_REG_CHANNEL_0;
    for (uint32_t i = 0U; i < SDADC_MAX_NUM_CHANNELS; i++)
    {
        p_ctrl->results.results_32[i] = 0U;
        if (NULL != p_cfg_extend->p_channel_cfgs[i])
        {
            /** -# Set the oversampling ratio. */
            uint32_t pgac_register_value = 0U;
            pgac_register_value = (uint32_t) p_cfg_extend->p_channel_cfgs[i]->oversampling << SDADC_PRIV_PGAC_PGAOSR_SHIFT;

            /** -# Set the gain. */
            pgac_register_value |= (uint32_t) p_cfg_extend->p_channel_cfgs[i]->stage_1_gain << SDADC_PRIV_PGAC_PGAGC_S1_SHIFT;
            pgac_register_value |= (uint32_t) p_cfg_extend->p_channel_cfgs[i]->stage_2_gain << SDADC_PRIV_PGAC_PGAGC_S2_SHIFT;

            /** -# Select single-end input/differential input. */
            pgac_register_value |= (uint32_t) p_cfg_extend->p_channel_cfgs[i]->input << SDADC_PRIV_PGAC_PGASEL_SHIFT;

            /** -# If differential mode is used and calibration during open is not skipped, enable calibration on this channel. */
            if ((SDADC_CHANNEL_INPUT_DIFFERENTIAL == p_cfg_extend->p_channel_cfgs[i]->input) &&
                    (!p_cfg_extend->skip_internal_calibration))
            {
                pgac_register_value |= 1U << SDADC_PRIV_PGAC_PGACVE_SHIFT;
                calibrate = true;

                /* All channels configured for differential mode will be calibrated because all channels have the
                 * PGACVE bit set above. The only channel specific settings required for calibration are setting
                 * PGACVE and setting PGAOFS to 0.  PGAOFS is set to the default of 0 when PGAC is written below.
                 * The channel passed to the calibrate API must be one of the enabled channels. */
                calibrate_args.channel = (adc_register_t) i;
            }

            /** -# Select the polarity of single-end input (only for single-end mode). */
            pgac_register_value |= (uint32_t) p_cfg_extend->p_channel_cfgs[i]->polarity << SDADC_PRIV_PGAC_PGAPOL_SHIFT;

            /** Set the A/D conversion count. */
            pgac_register_value |= (uint32_t) p_cfg_extend->p_channel_cfgs[i]->coefficient_m << SDADC_PRIV_PGAC_PGACTM_SHIFT;
            pgac_register_value |= (uint32_t) p_cfg_extend->p_channel_cfgs[i]->coefficient_n << SDADC_PRIV_PGAC_PGACTN_SHIFT;
            pgac_register_value |= (uint32_t) p_cfg_extend->p_channel_cfgs[i]->count_formula << SDADC_PRIV_PGAC_PGAASN_SHIFT;

            /** -# Select the averaging operation. Select the average data count. */
            pgac_register_value |= (uint32_t) p_cfg_extend->p_channel_cfgs[i]->average << SDADC_PRIV_PGAC_PGAAVN_SHIFT;

            /** -# Select whether to reverse the A/D conversion results of single-end input (only for negative-side single
             * end mode). */
            pgac_register_value |= (uint32_t) p_cfg_extend->p_channel_cfgs[i]->invert << SDADC_PRIV_PGAC_PGAREV_SHIFT;

            HW_SDADC_ChannelCfg(p_ctrl->p_reg, (adc_register_t) i, pgac_register_value);

            /** -# Enable conversion for the channel. */
            scan_mask |= (1U << i);
        }
        else
        {
            HW_SDADC_ChannelCfg(p_ctrl->p_reg, (adc_register_t) i, SDADC_PRIV_PGAC_DEFAULT);
        }
    }

    p_ctrl->scan_mask = scan_mask;
    p_ctrl->scan_cfg_mask = scan_mask;

    /** Configure enabled channels and autoscan mode. */
    /** If the A/D conversion trigger is ELC hardware events, the hardware events are enabled in
     *  adc_api_t::scanStart(). */
    r_sdadc_scan_configure(p_ctrl, p_ctrl->scan_mask, p_cfg->mode, ADC_TRIGGER_SOFTWARE);

    /** Mark driver as open by initializing it to "SDAD" - its ASCII equivalent. */
    p_ctrl->opened = SDADC_OPEN;

    if (calibrate)
    {
        /** If at least one channel is configured for differential mode and calibration is not disabled in the user
         * configuration, calibrate all PGAs configured for differential mode. */
        err = g_adc_on_sdadc.calibrate(p_ctrl, &calibrate_args);
        if (SSP_SUCCESS == err)
        {
            /** The calibration takes approximately 3.4 ms per channel in normal mode or 27 ms in low power mode.
             *  The open() API blocks waiting for calibration to complete unless calibration is skipped. Calibration
             *  during open can be skipped and handled in at the application level by setting
             *  adc_on_sdadc_cfg_t::skip_internal_calibration to true, then calling adc_api_t::calibrate(). */
            err = r_sdadc_open_wait_for_calibration(p_ctrl);
        }
        if (SSP_SUCCESS != err)
        {
            g_adc_on_sdadc.close(p_ctrl);
        }
        SDADC_ERROR_RETURN(SSP_SUCCESS == err, SSP_ERR_CALIBRATE_FAILED);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Configures the enabled channels of the ADC. Only adc_channel_cfg_t::scan_mask is used. Implements
 * adc_api_t::scanCfg().
 *
 * @retval  SSP_SUCCESS                Information stored in p_adc_info.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
 * @retval  SSP_ERR_INVALID_ARGUMENT   An input argument is invalid.
***********************************************************************************************************************/
ssp_err_t R_SDADC_ScanConfigure(adc_ctrl_t * p_api_ctrl, adc_channel_cfg_t const * const p_channel_cfg)
{
    sdadc_instance_ctrl_t * p_ctrl = (sdadc_instance_ctrl_t *) p_api_ctrl;

#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ADC unit is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_channel_cfg);
    SDADC_ERROR_RETURN(SDADC_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);

    /* Verify all channels to enable are configured. */
    SDADC_ERROR_RETURN(p_ctrl->scan_cfg_mask == (p_channel_cfg->scan_mask | p_ctrl->scan_cfg_mask), SSP_ERR_INVALID_ARGUMENT);
#endif

    /** Update the enabled channels. */
    p_ctrl->scan_mask = p_channel_cfg->scan_mask;
    r_sdadc_scan_configure(p_ctrl, p_ctrl->scan_mask, p_ctrl->mode, (adc_trigger_t) p_ctrl->trigger_enabled);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * adc_api_t::sampleStateCountSet is not supported on the SDADC.
 *
 * @retval  SSP_ERR_UNSUPPORTED  This API is not supported.
***********************************************************************************************************************/
ssp_err_t R_SDADC_SampleStateCountSet(adc_ctrl_t * p_api_ctrl,  adc_sample_state_t  * p_sample)
{
    SSP_PARAMETER_NOT_USED(p_api_ctrl);
    SSP_PARAMETER_NOT_USED(p_sample);

    /** Return the unsupported error. */
    return SSP_ERR_UNSUPPORTED;
}

/*******************************************************************************************************************//**
 * Returns the address of the lowest number configured channel, the total number of results to be read in order to
 * read the results of all configured channels, the size of each result, and the ELC event enumerations. Implements
 * adc_api_t::infoGet().
 *
 * @retval  SSP_SUCCESS                Information stored in p_adc_info.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_SDADC_InfoGet(adc_ctrl_t * p_api_ctrl, adc_info_t * p_adc_info)
{
    sdadc_instance_ctrl_t * p_ctrl = (sdadc_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;

#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ADC unit is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_adc_info);
    SDADC_ERROR_RETURN(SDADC_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);
#endif

    /** Retrieve the scan mask of active channels from the control block */
    uint32_t scan_mask = p_ctrl->scan_mask;

    if (0U == scan_mask)
    {
        p_adc_info->length = 0U;
        p_adc_info->p_address = NULL;
        p_adc_info->transfer_size = TRANSFER_SIZE_2_BYTE;
    }
    else
    {
        /** Determine the lowest channel that is configured. */
        uint32_t temp_mask = 0U;
        int32_t lowest_channel = -1;
        while (0U == temp_mask)
        {
            lowest_channel++;
            temp_mask = (uint32_t)(scan_mask & (1U << lowest_channel));
        }

        /** Determine the highest channel that is configured. */
        int32_t highest_channel = 32;
        temp_mask = 0U;
        while (0U == temp_mask)
        {
            highest_channel--;
            temp_mask = (uint32_t)(scan_mask & (1U << highest_channel));
        }

        /** Determine the size of data that must be read to read all the channels between and including the
         * highest and lowest channels. */
        p_adc_info->length = (uint32_t) ((highest_channel - lowest_channel) + 1);

        /** Determine the base address and transfer size. */
        if (ADC_RESOLUTION_24_BIT == p_ctrl->resolution)
        {
            p_adc_info->p_address = (uint16_t *) &p_ctrl->results.results_32[lowest_channel];
            p_adc_info->transfer_size = TRANSFER_SIZE_4_BYTE;
        }
        else
        {
            p_adc_info->p_address = &p_ctrl->results.results_16[lowest_channel];
            p_adc_info->transfer_size = TRANSFER_SIZE_2_BYTE;
        }
    }

    /** Specify the peripheral name in the ELC list */
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_ctrl->unit;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_SDADC;
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_SDADC_SCANEND, &event_info);
    p_adc_info->elc_event = event_info.event;
    p_adc_info->elc_peripheral = ELC_PERIPHERAL_SDADC0;

    /** Set sensor information to invalid value */
    p_adc_info->calibration_data = 0xFFFFFFFFU;
    p_adc_info->slope_microvolts = 0U;
    return err;
}

/*******************************************************************************************************************//**
 * If the SDADC is configured for hardware triggers, enables hardware triggers.  Otherwise, starts a scan.
 * Implements adc_api_t::scanStart().
 *
 * @retval  SSP_SUCCESS                Scan started or hardware triggers enabled successfully.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
 * @retval  SSP_ERR_IN_USE             A conversion or calibration is in progress.
***********************************************************************************************************************/
ssp_err_t R_SDADC_ScanStart(adc_ctrl_t * p_api_ctrl)
{
    sdadc_instance_ctrl_t * p_ctrl = (sdadc_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;

#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ADC unit is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SDADC_ERROR_RETURN(SDADC_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);
#endif

    /** Conversion cannot be performed if conversion or calibration is already in progress. */
    SDADC_ERROR_RETURN(0U == p_ctrl->calib_status, SSP_ERR_IN_USE);
    SDADC_ERROR_RETURN(0U == HW_SDADC_ConversionStatusGet(p_ctrl->p_reg), SSP_ERR_IN_USE);

    /** If the unit is configured for software triggering, trigger a scan. */
    if (ADC_TRIGGER_SOFTWARE == p_ctrl->trigger)
    {
        HW_SDADC_SoftwareStart(p_ctrl->p_reg);
    }
    else
    {
        /** Otherwise, enable hardware triggers. */
        HW_SDADC_ElcTriggerEnable(p_ctrl->p_reg);
    }

    /** Return the error code */
    return err;
}

/*******************************************************************************************************************//**
 * If the SDADC is configured for hardware triggers, disables hardware triggers.  Otherwise, stops any in-progress scan
 * started by software. Implements adc_api_t::scanStop().
 *
 * @retval  SSP_SUCCESS                Scan stopped or hardware triggers disabled successfully.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_SDADC_ScanStop(adc_ctrl_t * p_api_ctrl)
{
    sdadc_instance_ctrl_t * p_ctrl = (sdadc_instance_ctrl_t *) p_api_ctrl;

#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ADC unit is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SDADC_ERROR_RETURN(SDADC_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);
#endif

    /** If the unit is configured for software triggering, stop the scan. */
    if (ADC_TRIGGER_SOFTWARE == p_ctrl->trigger)
    {
        HW_SDADC_SoftwareStop(p_ctrl->p_reg);
    }
    else
    {
        /** Otherwise, disable hardware triggers. */
        HW_SDADC_ElcTriggerDisable(p_ctrl->p_reg);
    }

    /** Return the error code */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Returns the status of a scan started by software, including calibration scans.  It is not possible to determine the
 * status of a scan started by a hardware trigger. Implements adc_api_t::scanStatusGet().
 *
 * @retval  SSP_SUCCESS                No software scan or calibration is in progress.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_IN_USE             A conversion or calibration is in progress.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_SDADC_CheckScanDone(adc_ctrl_t * p_api_ctrl)
{
    sdadc_instance_ctrl_t * p_ctrl = (sdadc_instance_ctrl_t *) p_api_ctrl;

#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ADC unit is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SDADC_ERROR_RETURN(SDADC_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);
#endif

    /** If calibration is in progress, return an error. */
    SDADC_ERROR_RETURN(0U == p_ctrl->calib_status, SSP_ERR_IN_USE);

    /** Return the scan status of a scan triggered by software. */
    SDADC_ERROR_RETURN(0U == HW_SDADC_ConversionStatusGet(p_ctrl->p_reg), SSP_ERR_IN_USE);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Reads the most recent conversion result from a channel.  Truncates 24-bit results to the upper 16 bits.  Implements
 * adc_api_t::read().
 *
 * @retval  SSP_SUCCESS                Conversion result in p_data.
 * @retval  SSP_ERR_INVALID_ARGUMENT   An input argument was outside the valid range.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_SDADC_Read(adc_ctrl_t * p_api_ctrl, adc_register_t const  reg_id, uint16_t * const p_data)
{
    sdadc_instance_ctrl_t * p_ctrl = (sdadc_instance_ctrl_t *) p_api_ctrl;

#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ADC unit is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_data);
    SDADC_ERROR_RETURN(SDADC_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);
    SDADC_ERROR_RETURN(0U != ((1U << (uint32_t) reg_id) & p_ctrl->scan_mask), SSP_ERR_INVALID_ARGUMENT);
#endif

    /** Read the most recent conversion value into a 16-bit result. */
    if (ADC_RESOLUTION_24_BIT == p_ctrl->resolution)
    {
        if (ADC_ALIGNMENT_LEFT == p_ctrl->alignment)
        {
            *p_data = (uint16_t) (p_ctrl->results.results_32[reg_id] >> SDADC_PRIV_16_BITS);
        }
        else
        {
            *p_data = (uint16_t) (p_ctrl->results.results_32[reg_id] >> SDADC_PRIV_8_BITS);
        }
    }
    else
    {
        *p_data = p_ctrl->results.results_16[reg_id];
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Reads the most recent conversion result from a channel. Implements adc_api_t::read32().
 *
 * @retval  SSP_SUCCESS                Conversion result in p_data.
 * @retval  SSP_ERR_INVALID_ARGUMENT   An input argument was outside the valid range.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_SDADC_Read32(adc_ctrl_t * p_api_ctrl, adc_register_t const  reg_id, uint32_t * const p_data)
{
    sdadc_instance_ctrl_t * p_ctrl = (sdadc_instance_ctrl_t *) p_api_ctrl;

#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ADC unit is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_data);
    SDADC_ERROR_RETURN(SDADC_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);
    SDADC_ERROR_RETURN(0U != ((1U << (uint32_t) reg_id) & p_ctrl->scan_mask), SSP_ERR_INVALID_ARGUMENT);
#endif

    /** Read the most recent conversion value into a 32-bit result. */
    uint32_t result = 0U;
    if (ADC_RESOLUTION_24_BIT == p_ctrl->resolution)
    {
        result = p_ctrl->results.results_32[reg_id];
    }
    else
    {
        result = (uint32_t) p_ctrl->results.results_16[reg_id];
        if (ADC_ALIGNMENT_LEFT == p_ctrl->alignment)
        {
            result <<= SDADC_PRIV_16_BITS;
        }
    }
    *p_data = result;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Sets the offset.  Offset is applied after stage 1 of the input channel.  Offset can only be applied when the channel is
 * configured for differential input.  Implements adc_api_t::offsetSet().
 *
 * Note: The offset is cleared if adc_api_t::calibrate() is called.  The offset can be re-applied if necessary after the
 * the callback with event ADC_EVENT_CALIBRATION_COMPLETE is called.
 *
 * @param[in]  p_api_ctrl  See p_ctrl in adc_api_t::offsetSet().
 * @param[in]  reg_id      See reg_id in adc_api_t::offsetSet().
 * @param[in]  offset      Must be between -15 and 15, offset (mV) = 10.9376 mV * offset_steps / stage 1 gain.
 *
 * @retval  SSP_SUCCESS                Offset updated successfully.
 * @retval  SSP_ERR_INVALID_ARGUMENT   An input argument was outside the valid range.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_IN_USE             A conversion or calibration is in progress.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_SDADC_OffsetSet(adc_ctrl_t * const p_api_ctrl, adc_register_t const reg_id, int32_t const offset)
{
    sdadc_instance_ctrl_t * p_ctrl = (sdadc_instance_ctrl_t *) p_api_ctrl;

#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ADC unit is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SDADC_ERROR_RETURN((offset >= -15) && (offset <= 15), SSP_ERR_INVALID_ARGUMENT);
    SDADC_ERROR_RETURN(SDADC_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);
    SDADC_ERROR_RETURN(0U != ((1U << (uint32_t) reg_id) & p_ctrl->scan_mask), SSP_ERR_INVALID_ARGUMENT);

    /** The channel must be configured for differential input. */
    SDADC_ERROR_RETURN(SDADC_CHANNEL_INPUT_DIFFERENTIAL == HW_SDADC_ChannelInputGet(p_ctrl->p_reg, reg_id), SSP_ERR_INVALID_ARGUMENT);
#endif

    /** Offset cannot be updated if conversion or calibration is in progress. */
    SDADC_ERROR_RETURN(0U == p_ctrl->calib_status, SSP_ERR_IN_USE);
    SDADC_ERROR_RETURN(0U == HW_SDADC_ConversionStatusGet(p_ctrl->p_reg), SSP_ERR_IN_USE);

    /** Set the offset. */
    HW_SDADC_ChannelOffsetSet(p_ctrl->p_reg, reg_id, offset);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Requires ::sdadc_calibrate_args_t passed to p_extend. Calibrates the specified channel. Calibration is not required or
 * supported for single-ended mode. Internal calibration is automatically done during open() unless it is disabled in
 * the user configuration.  This API is provided primarily for external calibration. A callback with the event
 * ADC_EVENT_CALIBRATION_COMPLETE is called when calibration completes. The calibration status can also be monitored
 * using adc_api_t::statusGet(). Implements adc_api_t::calibrate().
 *
 * During external offset calibration, apply a differential voltage of 0 to ANSDnP - ANSDnN, where n is the input channel
 * and ANSDnP is OPAMP0 for channel 4 and ANSDnN is OPAMP1 for channel 4.  Complete external offset calibration before
 * external gain calibration.
 *
 * During external gain calibration apply a voltage between 0.4 V / total_gain and 0.8 V / total_gain.  The
 * differential voltage applied during calibration is corrected to a conversion result of 0x7FFFFF, which is the
 * maximum possible positive differential measurement.
 *
 * This function clears the offset value. If offset is required after calibration, it must be reapplied after
 * calibration is complete using adc_api_t::offsetSet.
 *
 * @retval  SSP_SUCCESS                Calibration began successfully.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_IN_USE             A conversion or calibration is in progress.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_SDADC_Calibrate(adc_ctrl_t * const p_api_ctrl, void * const p_extend)
{
    sdadc_instance_ctrl_t * p_ctrl = (sdadc_instance_ctrl_t *) p_api_ctrl;
    sdadc_calibrate_args_t * p_calibrate = (sdadc_calibrate_args_t *) p_extend;

#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
    ssp_err_t err = r_sdadc_calibrate_param_check(p_ctrl, p_calibrate);
    SDADC_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    /** Calibration cannot be performed if conversion or calibration is already in progress. */
    SDADC_ERROR_RETURN(0U == p_ctrl->calib_status, SSP_ERR_IN_USE);
    SDADC_ERROR_RETURN(0U == HW_SDADC_ConversionStatusGet(p_ctrl->p_reg), SSP_ERR_IN_USE);

    p_ctrl->calib_status = 1U;

    /** Select software trigger. */
    /** Select single scan mode. */
    p_ctrl->trigger_enabled = HW_SDADC_ElcTriggerGet(p_ctrl->p_reg);
    r_sdadc_scan_configure(p_ctrl, 0U, ADC_MODE_SINGLE_SCAN, ADC_TRIGGER_SOFTWARE);

    /** Enable calibration. */
    HW_SDADC_ChannelCalibrateEnable(p_ctrl->p_reg, p_calibrate->channel);

    /** Set the offset voltage to 0 mV. */
    HW_SDADC_ChannelOffsetSet(p_ctrl->p_reg, p_calibrate->channel, 0);

    /** Set the calibration mode. */
    HW_SDADC_CalibrateModeSet(p_ctrl->p_reg, (sdadc_calibration_t) p_calibrate->mode);

    /** Start calibration. */
    HW_SDADC_CalibrateStart(p_ctrl->p_reg);

    /** Confirm that calibration started. */
    /* After calibration is started, the calibration status flag is set after 2 PCLKB + 3 SDADC clock cycles.
     * This should never time out. */
    uint32_t timeout = 0xFFFF;
    uint8_t calibration_started = 0U;
    while ((0U == calibration_started) && (timeout > 0U))
    {
        calibration_started = HW_SDADC_CalibrateStatusGet(p_ctrl->p_reg);
        timeout--;
    }
    if (0U == calibration_started)
    {
        /* Error occurred, restore settings to pre-calibration settings. */
        r_sdadc_calibrate_end_restore(p_ctrl);
    }
    SDADC_ERROR_RETURN(1U == calibration_started, SSP_ERR_IN_USE);

    /** Return the error code */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Stops any scan in progress, disables interrupts, and powers down the SDADC peripheral. Implements
 * adc_api_t::close().
 *
 * @retval  SSP_SUCCESS                Instance control block closed successfully.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_SDADC_Close(adc_ctrl_t * p_api_ctrl)
{
    sdadc_instance_ctrl_t * p_ctrl = (sdadc_instance_ctrl_t *) p_api_ctrl;

    /** Perform parameter checking*/
#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ADC unit is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SDADC_ERROR_RETURN(SDADC_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);
#endif

    /** Mark driver as closed   */
    p_ctrl->opened = 0U;

    /** Disable hardware triggers. */
    HW_SDADC_ElcTriggerDisable(p_ctrl->p_reg);

    /** Stop A/D conversion. */
    HW_SDADC_SoftwareStop(p_ctrl->p_reg);

    /** Disable interrupts. */
    r_sdadc_disable_irq(p_ctrl->conv_end_irq);
    r_sdadc_disable_irq(p_ctrl->scan_end_irq);
    r_sdadc_disable_irq(p_ctrl->calib_end_irq);

    /** Wait 3 us in normal mode as required by the hardware manual. */
    R_BSP_SoftwareDelay(3U, BSP_DELAY_UNITS_MICROSECONDS);

    /** Turn off the power of VBIAS, channel, and sigma-delta A/D converter. */
    HW_SDADC_AdcPowerOff(p_ctrl->p_reg);

    /** Turn off the power of ADBGR, SBIAS, and ADREG. */
    HW_SDADC_BgrPowerOff(p_ctrl->p_reg);

    /** Stop the input clock for the 24-bit sigma-delta A/D converter (SDADCCLK). */
    g_cgc_on_cgc.sdadcClockDisable();

    /** Enter the module-stop state. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.id = SSP_IP_SDADC;
    ssp_feature.channel = p_ctrl->unit;
    ssp_feature.unit = 0U;
    R_BSP_ModuleStop(&ssp_feature);

    /** Release the hardware lock */
    R_BSP_HardwareUnlock(&ssp_feature);

    /** Return the error code */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Gets the API and code version. Implements adc_api_t::versionGet().
 *
 * @retval  SSP_SUCCESS        Version information available in p_version.
 * @retval  SSP_ERR_ASSERTION  The parameter p_version is NULL.
***********************************************************************************************************************/
ssp_err_t R_SDADC_VersionGet(ssp_version_t * const p_version)
{
#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointer is not NULL. */
    SSP_ASSERT(NULL != p_version);
#endif

    /** Return the version number */
    p_version->version_id =  g_sdadc_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup SDADC)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 Private Functions
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * Configures and enables an SDADC interrupt.
 *
 * @param[in]  p_ctrl             Pointer to instance control block
 * @param[in]  p_feature          Pointer to SDADC feature
 * @param[in]  signal             Signal for this interrupt
 * @param[in]  priority           Priority for this interrupt
 *
 * @return IRQ number for the interrupt, SSP_INVALID_VECTOR if the interrupt is not in the vector table.
***********************************************************************************************************************/
static IRQn_Type r_sdadc_open_irq_cfg_sub(sdadc_instance_ctrl_t * const p_ctrl,
                                       ssp_feature_t         * const p_feature,
                                       ssp_signal_t                  signal,
                                       uint8_t                       priority)
{
    ssp_vector_info_t * p_vector_info;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    g_fmi_on_fmi.eventInfoGet(p_feature, signal, &event_info);
    if (SSP_INVALID_VECTOR != event_info.irq)
    {
        /** Set the interrupt priority. */
        R_SSP_VectorInfoGet(event_info.irq, &p_vector_info);
        NVIC_SetPriority(event_info.irq, priority);
        *(p_vector_info->pp_ctrl) = p_ctrl;

        /** Clear any pending interrupt requests in the NVIC or the peripheral. */
        R_BSP_IrqStatusClear(event_info.irq);
        NVIC_ClearPendingIRQ(event_info.irq);

        /** Enable the interrupt. */
        NVIC_EnableIRQ(event_info.irq);
    }

    return event_info.irq;
}

#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
/*******************************************************************************************************************//**
 * Validates the input parameters to open().
 *
 * @param[in]  p_cfg   Pointer to configuration structure
 *
 * @retval  SSP_SUCCESS                No invalid configurations identified.
 * @retval  SSP_ERR_INVALID_ARGUMENT   An input argument is invalid.
***********************************************************************************************************************/
static ssp_err_t r_sdadc_open_param_check_3(adc_cfg_t const * const p_cfg)
{

    /** External trigger mode is not supported. */
    SDADC_ERROR_RETURN(ADC_TRIGGER_ASYNC_EXT_TRG0 != p_cfg->trigger, SSP_ERR_INVALID_ARGUMENT);

    /** Hardware triggers cannot be used in continuous scan mode. */
    if (ADC_TRIGGER_SYNC_ELC == p_cfg->trigger)
    {
        SDADC_ERROR_RETURN(ADC_MODE_SINGLE_SCAN == p_cfg->mode, SSP_ERR_INVALID_ARGUMENT);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Validates the input parameters to open().
 *
 * @param[in]  p_cfg   Pointer to configuration structure
 *
 * @retval  SSP_SUCCESS                No invalid configurations identified.
 * @retval  SSP_ERR_INVALID_ARGUMENT   An input argument is invalid.
***********************************************************************************************************************/
static ssp_err_t r_sdadc_open_param_check_2(adc_cfg_t const * const p_cfg)
{

    /** When single-ended input is selected, oversampling must be 256 and gain at each stage must be 1. */
    adc_on_sdadc_cfg_t const * p_cfg_extend = (adc_on_sdadc_cfg_t *) p_cfg->p_extend;
    uint32_t channels_configured = 0U;
    for (uint32_t i = 0U; i < SDADC_MAX_NUM_CHANNELS; i++)
    {
        if (NULL != p_cfg_extend->p_channel_cfgs[i])
        {
            channels_configured++;
            if (SDADC_CHANNEL_INPUT_SINGLE_ENDED == p_cfg_extend->p_channel_cfgs[i]->input)
            {
                SDADC_ERROR_RETURN(SDADC_CHANNEL_OVERSAMPLING_256 == p_cfg_extend->p_channel_cfgs[i]->oversampling, SSP_ERR_INVALID_ARGUMENT);
                SDADC_ERROR_RETURN(SDADC_CHANNEL_STAGE_1_GAIN_1 == p_cfg_extend->p_channel_cfgs[i]->stage_1_gain, SSP_ERR_INVALID_ARGUMENT);
                SDADC_ERROR_RETURN(SDADC_CHANNEL_STAGE_2_GAIN_1 == p_cfg_extend->p_channel_cfgs[i]->stage_2_gain, SSP_ERR_INVALID_ARGUMENT);
            }
        }
    }

    /** At least one channel must be configured */
    SDADC_ERROR_RETURN(channels_configured > 0U, SSP_ERR_INVALID_ARGUMENT);

    ssp_err_t err = r_sdadc_open_param_check_3(p_cfg);
    SDADC_ERROR_RETURN(SSP_SUCCESS == err, err);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Validates the input parameters to open().
 *
 * @param[in]  p_ctrl  Pointer to instance control block
 * @param[in]  p_cfg   Pointer to configuration structure
 *
 * @retval  SSP_SUCCESS                No invalid configurations identified.
 * @retval  SSP_ERR_ASSERTION          An input pointer is NULL.
 * @retval  SSP_ERR_INVALID_ARGUMENT   An input argument is invalid.
***********************************************************************************************************************/
static ssp_err_t r_sdadc_open_param_check(sdadc_instance_ctrl_t * const p_ctrl, adc_cfg_t const * const p_cfg)
{

    /* Verify the pointers are not NULL. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_cfg->p_extend);

    /** Resolution must be 24-bit or 16-bit. */
    SDADC_ERROR_RETURN((ADC_RESOLUTION_24_BIT == p_cfg->resolution) || (ADC_RESOLUTION_16_BIT == p_cfg->resolution),
            SSP_ERR_INVALID_ARGUMENT);

    /** Group scan mode is not supported. */
    SDADC_ERROR_RETURN(ADC_MODE_GROUP_SCAN != p_cfg->mode, SSP_ERR_INVALID_ARGUMENT);

    /** Adding not supported. Averaging is supported per channel in sdadc_channel_cfg_t. */
    SDADC_ERROR_RETURN(ADC_ADD_OFF == p_cfg->add_average_count, SSP_ERR_INVALID_ARGUMENT);

    /** Clearing is not supported. */
    SDADC_ERROR_RETURN(ADC_CLEAR_AFTER_READ_OFF == p_cfg->clearing, SSP_ERR_INVALID_ARGUMENT);

    ssp_err_t err = r_sdadc_open_param_check_2(p_cfg);
    SDADC_ERROR_RETURN(SSP_SUCCESS == err, err);

    return SSP_SUCCESS;
}
#endif

/*******************************************************************************************************************//**
 * Configures interrupts and ensures required interrupts are enabled.
 *
 * @param[in]  p_ctrl     Pointer to instance control block
 * @param[in]  p_cfg      Pointer to configuration structure
 * @param[in]  p_feature  Pointer to SDADC feature
 *
 * @retval  SSP_SUCCESS                All required interrupts enabled.
 * @retval  SSP_ERR_IRQ_BSP_DISABLED   A required interrupt is disabled
***********************************************************************************************************************/
static ssp_err_t r_sdadc_open_irq_cfg(sdadc_instance_ctrl_t * const p_ctrl, adc_cfg_t const * const p_cfg, ssp_feature_t * p_feature)
{
    adc_on_sdadc_cfg_t const * p_cfg_extend = (adc_on_sdadc_cfg_t *) p_cfg->p_extend;
    p_ctrl->conv_end_irq = r_sdadc_open_irq_cfg_sub(p_ctrl, p_feature, SSP_SIGNAL_SDADC_ADI, p_cfg_extend->conversion_end_ipl);
    SDADC_ERROR_RETURN(SSP_INVALID_VECTOR != p_ctrl->conv_end_irq, SSP_ERR_IRQ_BSP_DISABLED);
    p_ctrl->calib_end_irq = r_sdadc_open_irq_cfg_sub(p_ctrl, p_feature, SSP_SIGNAL_SDADC_CALIEND, p_cfg_extend->calibration_end_ipl);

    /* Calibration interrupt is required if any channel is configured for differential mode. */
    for (uint32_t i = 0U; i < SDADC_MAX_NUM_CHANNELS; i++)
    {
        if (NULL != p_cfg_extend->p_channel_cfgs[i])
        {
            if (SDADC_CHANNEL_INPUT_DIFFERENTIAL == p_cfg_extend->p_channel_cfgs[i]->input)
            {
                SDADC_ERROR_RETURN(SSP_INVALID_VECTOR != p_ctrl->calib_end_irq, SSP_ERR_IRQ_BSP_DISABLED);
            }
        }
    }
    p_ctrl->scan_end_irq = r_sdadc_open_irq_cfg_sub(p_ctrl, p_feature, SSP_SIGNAL_SDADC_SCANEND, p_cfg->scan_end_ipl);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Validates the input parameters to open(), confirms the SDADC exists on the MCU, and enables interrupts.
 *
 * @param[in]  p_ctrl  Pointer to instance control block
 * @param[in]  p_cfg   Pointer to configuration structure
 *
 * @retval  SSP_SUCCESS                Ready to proceed with hardware initialization.
 * @retval  SSP_ERR_INVALID_ARGUMENT   An input argument is invalid.
 * @retval  SSP_ERR_IN_USE             Control block is already open.
 * @retval  SSP_ERR_IRQ_BSP_DISABLED   A required interrupt is disabled
 * @return                             See @ref Common_Error_Codes or functions called by this function for other possible
 *                                     return codes. This function calls:
 *                                        * fmi_api_t::productFeatureGet
***********************************************************************************************************************/
static ssp_err_t r_sdadc_open_prepare(sdadc_instance_ctrl_t * const p_ctrl, adc_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;
#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
    err = r_sdadc_open_param_check(p_ctrl, p_cfg);
    SDADC_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    /** Verify the control block has not already been initialized. */
    SDADC_ERROR_RETURN(SDADC_OPEN != p_ctrl->opened, SSP_ERR_IN_USE);

    /* Set all p_ctrl fields prior to using it in any functions. */
    p_ctrl->p_callback = p_cfg->p_callback;
    p_ctrl->p_context = p_cfg->p_context;
    p_ctrl->mode = p_cfg->mode;
    p_ctrl->trigger = p_cfg->trigger;
    p_ctrl->alignment = p_cfg->alignment;
    p_ctrl->resolution = p_cfg->resolution;
    p_ctrl->unit = p_cfg->unit;
    p_ctrl->trigger_enabled = 0U;

    /** Confirm the SDADC exists on this MCU. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_ctrl->unit;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_SDADC;
    /** Confirm the requested unit exists on this MCU. */
    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    SDADC_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = info.ptr;

    /** Lock specified ADC channel */
    err = R_BSP_HardwareLock(&ssp_feature);
    SDADC_ERROR_RETURN((SSP_SUCCESS == err), err);

    /** Configure and enable interrupts. */
    err = r_sdadc_open_irq_cfg(p_ctrl, p_cfg, &ssp_feature);
    if (SSP_SUCCESS != err)
    {
        R_BSP_HardwareUnlock(&ssp_feature);
    }
    SDADC_ERROR_RETURN(SSP_SUCCESS == err, err);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Sets the Vref voltage, waiting 80 us between each step as required by the hardware manual.
 *
 * @param[in]  p_ctrl   Pointer to instance control block
 * @param[in]  voltage  Voltage to set
***********************************************************************************************************************/
static void r_sdadc_open_vref_voltage_set(sdadc_instance_ctrl_t * p_ctrl, sdadc_vref_voltage_t voltage)
{
    sdadc_vref_voltage_t current_vref = HW_SDADC_VrefVoltageGet(p_ctrl->p_reg);
    while (current_vref != voltage)
    {
        if (current_vref < voltage)
        {
            /** If the voltage is less than the desired voltage, increment the voltage. */
            current_vref++;
            HW_SDADC_VrefVoltageSet(p_ctrl->p_reg, current_vref);
        }
        else
        {
            /** If the voltage is more than the desired voltage, decrement the voltage. */
            current_vref--;
            HW_SDADC_VrefVoltageSet(p_ctrl->p_reg, current_vref);
        }
        if (current_vref != voltage)
        {
            /** Wait 80 us between each step. */
            R_BSP_SoftwareDelay(80U, BSP_DELAY_UNITS_MICROSECONDS);
        }
    }
}

/*******************************************************************************************************************//**
 * Waits for calibration to complete.
 *
 * @param[in]  p_ctrl        Pointer to instance control block
 *
 * @retval  SSP_SUCCESS                Calibration is complete.
 * @retval  SSP_ERR_TIMEOUT            Calibration timed out.
***********************************************************************************************************************/
static ssp_err_t r_sdadc_open_wait_for_calibration(sdadc_instance_ctrl_t * const p_ctrl)
{
    uint32_t timeout_us = SDADC_PRIV_CALIBRATION_TIMEOUT_US;
    while ((1U == p_ctrl->calib_status) && (timeout_us > 0U))
    {
        R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MICROSECONDS);
        timeout_us--;
    }
    SDADC_ERROR_RETURN(0U != timeout_us, SSP_ERR_TIMEOUT);
    return SSP_SUCCESS;
}

#if (1 == SDADC_CFG_PARAM_CHECKING_ENABLE)
/*******************************************************************************************************************//**
 * Validates the input parameters to calibrate().
 *
 * @param[in]  p_ctrl        Pointer to instance control block
 * @param[in]  p_calibrate   Pointer to calibration configuration structure
 *
 * @retval  SSP_SUCCESS                No invalid configurations identified.
 * @retval  SSP_ERR_ASSERTION          A required input pointer is NULL.
 * @retval  SSP_ERR_NOT_OPEN           The instance control block has not been opened.
 * @retval  SSP_ERR_INVALID_ARGUMENT   An input argument is invalid.
***********************************************************************************************************************/
static ssp_err_t r_sdadc_calibrate_param_check(sdadc_instance_ctrl_t * const p_ctrl, sdadc_calibrate_args_t * p_calibrate)
{

    /* Verify the pointers are not NULL and ensure the ADC unit is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_calibrate);
    SDADC_ERROR_RETURN(SDADC_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);

    /** The channel must be enabled during open. */
    uint32_t channel = (uint32_t) p_calibrate->channel;
    uint32_t mask = 1U << channel;
    SDADC_ERROR_RETURN(0U != (mask & p_ctrl->scan_mask), SSP_ERR_INVALID_ARGUMENT);

    /** The channel must be configured for differential input. */
    SDADC_ERROR_RETURN(SDADC_CHANNEL_INPUT_DIFFERENTIAL == HW_SDADC_ChannelInputGet(p_ctrl->p_reg, (adc_register_t) p_calibrate->channel), SSP_ERR_INVALID_ARGUMENT);

    return SSP_SUCCESS;
}
#endif

/*******************************************************************************************************************//**
 * Sets the scan mask, scan mode, and trigger.
 *
 * @param[in]  p_ctrl     Pointer to instance control block
 * @param[in]  scan_mask  Bitmask of enabled channels
 * @param[in]  mode       Single scan or continuous scan mode.
 * @param[in]  trigger    ELC or software trigger.
***********************************************************************************************************************/
static void r_sdadc_scan_configure(sdadc_instance_ctrl_t * const p_ctrl,
                                   uint32_t                      scan_mask,
                                   adc_mode_t                    mode,
                                   adc_trigger_t                 trigger)
{
    /* Select which channels to enable conversion on. */
    uint32_t adc1_setting = (~scan_mask & 0x1FU) << SDADC_PRIV_ADC1_SDADBMP_SHIFT;

    /** Select AUTOSCAN mode (single scan or continuous scan). */
    uint32_t adc1_mode_bit = !((uint32_t) mode);
    adc1_setting |= adc1_mode_bit;

    /** Select software or hardware (ELC event) trigger. */
    uint32_t adc1_trigger_bit = (uint32_t) trigger & 0x1U;
    adc1_trigger_bit <<= SDADC_PRIV_ADC1_SDADTMD_SHIFT;
    adc1_setting |= adc1_trigger_bit;

    HW_SDADC_ScanCfg(p_ctrl->p_reg, adc1_setting);
}

/*******************************************************************************************************************//**
 * Restores settings after calibration.
 *
 * @param[in]  p_ctrl     Pointer to instance control block
***********************************************************************************************************************/
static void r_sdadc_calibrate_end_restore(sdadc_instance_ctrl_t * const p_ctrl)
{
    /** Disable calibration for all channels. */
    for (uint32_t i = 0U; i < SDADC_MAX_NUM_CHANNELS; i++)
    {
        HW_SDADC_ChannelCalibrateDisable(p_ctrl->p_reg, (adc_register_t) i);
    }

    /** Restore scan mask, mode, and trigger. */
    r_sdadc_scan_configure(p_ctrl, p_ctrl->scan_mask, p_ctrl->mode, (adc_trigger_t) p_ctrl->trigger_enabled);

    /** Clear the calibration in progress software flag only after restoring scan mask, mode, and trigger settings. */
    p_ctrl->calib_status = 0U;
}

/*******************************************************************************************************************//**
 * Disables an interrupt.
 *
 * @param[in]  irq     Interrupt to disable
***********************************************************************************************************************/
static void r_sdadc_disable_irq(IRQn_Type irq)
{
    ssp_vector_info_t * p_vector_info;
    if (SSP_INVALID_VECTOR != irq)
    {
        NVIC_DisableIRQ (irq);
        NVIC_ClearPendingIRQ (irq);
        R_BSP_IrqStatusClear (irq);
        R_SSP_VectorInfoGet(irq, &p_vector_info);
        *(p_vector_info->pp_ctrl) = NULL;
    }
}

/*******************************************************************************************************************//**
 * Scan complete interrupt.
***********************************************************************************************************************/
void sdadc_scanend_isr(void)
{
    /** Save context if RTOS is used */
    SF_CONTEXT_SAVE;

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    sdadc_instance_ctrl_t * p_ctrl = (adc_ctrl_t *) *(p_vector_info->pp_ctrl);
    adc_callback_args_t   args;
    args.unit = 0U;

    /** Clear the BSP IRQ Flag     */
    R_BSP_IrqStatusClear (R_SSP_CurrentIrqGet());

    /** Check that the pointer is not NULL prior to using it */
    if (NULL != p_ctrl)
    {
        /** Store the event into the callback argument */
        args.event = ADC_EVENT_SCAN_COMPLETE;

        /** If a callback was provided, call it with the argument */
        args.p_context = p_ctrl->p_context;
        if (NULL != p_ctrl->p_callback)
        {
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
    }

    /** Restore context if RTOS is used */
    SF_CONTEXT_RESTORE;
}

/*******************************************************************************************************************//**
 * Conversion complete interrupt.
***********************************************************************************************************************/
void sdadc_adi_isr(void)
{
    /** Save context if RTOS is used */
    SF_CONTEXT_SAVE;

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    sdadc_instance_ctrl_t * p_ctrl = (adc_ctrl_t *) *(p_vector_info->pp_ctrl);
    adc_callback_args_t   args;
    args.unit = 0U;

    /** Clear the BSP IRQ Flag     */
    R_BSP_IrqStatusClear (R_SSP_CurrentIrqGet());

    /** Check that the pointer is not NULL prior to using it */
    if (NULL != p_ctrl)
    {
        /** Store the event into the callback argument */
        args.event = ADC_EVENT_CONVERSION_COMPLETE;

        /** Read the converted result. */
        uint32_t result = HW_SDADC_ResultGet(p_ctrl->p_reg);
        adc_register_t channel = (adc_register_t) ((result >> 25) - 1U);
        if (HW_SDADC_IsChannelAverageEnabled(p_ctrl->p_reg, channel))
        {
            /** If averaging is enabled, read the averaged result. */
            result = HW_SDADC_AveragedResultGet(p_ctrl->p_reg);
            channel = (adc_register_t) ((result >> 25) - 1U);
        }
        args.channel = channel;

        /** Mask off the upper 8 bits, which are not part of the result. */
        result = result & 0xFFFFFFU;

        /** Store the result according to the configured resolution. */
        if (ADC_RESOLUTION_24_BIT == p_ctrl->resolution)
        {
            if (ADC_ALIGNMENT_LEFT == p_ctrl->alignment)
            {
                result <<= SDADC_PRIV_8_BITS;
            }
            p_ctrl->results.results_32[channel] = result;
        }
        else
        {
            result = result >> SDADC_PRIV_8_BITS;
            p_ctrl->results.results_16[channel] = (uint16_t) result;
        }

        /** If a callback was provided, call it with the argument */
        if (NULL != p_ctrl->p_callback)
        {
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
    }

    /** Restore context if RTOS is used */
    SF_CONTEXT_RESTORE;
}

/*******************************************************************************************************************//**
 * Calibration complete interrupt.
***********************************************************************************************************************/
void sdadc_caliend_isr(void)
{
    /** Save context if RTOS is used */
    SF_CONTEXT_SAVE;

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    sdadc_instance_ctrl_t * p_ctrl = (adc_ctrl_t *) *(p_vector_info->pp_ctrl);
    adc_callback_args_t   args;
    args.unit = 0U;

    /** Clear the BSP IRQ Flag     */
    R_BSP_IrqStatusClear (R_SSP_CurrentIrqGet());

    /** Check that the pointer is not NULL prior to using it */
    if (NULL != p_ctrl)
    {
        /** Restore settings to their pre-calibration values. */
        r_sdadc_calibrate_end_restore(p_ctrl);

        /** Store the event into the callback argument */
        args.event = ADC_EVENT_CALIBRATION_COMPLETE;

        /** If a callback was provided, call it with the argument */
        if (NULL != p_ctrl->p_callback)
        {
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
    }

    /** Restore context if RTOS is used */
    SF_CONTEXT_RESTORE;
}
