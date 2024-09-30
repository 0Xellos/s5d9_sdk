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
 * File Name    : r_dac8.c
 * Description  : DAC8 HLD implementation
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_dac8.h"
#include "r_dac8_private.h"
#include "r_dac8_private_api.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** Macro for error logger. */
#ifndef DAC8_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define DAC8_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_dac8_version)
#endif

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if DAC8_CFG_PARAM_CHECKING_ENABLE
static ssp_err_t r_dac8_validate_channel(dac_cfg_t const * const p_cfg, dac8_variant_info_t variant_info);
static ssp_err_t r_dac8_validate_config(dac_cfg_t const * const p_cfg, dac8_variant_info_t variant_info);
#endif

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const dac_api_t g_dac_on_dac8 =
{
    .open            = R_DAC8_Open,
    .close           = R_DAC8_Close,
    .write           = R_DAC8_Write,
    .start           = R_DAC8_Start,
    .stop            = R_DAC8_Stop,
    .versionGet      = R_DAC8_VersionGet,
    .infoGet         = R_DAC8_InfoGet
};

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug. This pragma suppresses the warnings in this
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_dac8_version =
{
    .api_version_minor  = DAC_API_VERSION_MINOR,
    .api_version_major  = DAC_API_VERSION_MAJOR,
    .code_version_major = DAC8_CODE_VERSION_MAJOR,
    .code_version_minor = DAC8_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "dac8";
#endif

/*******************************************************************************************************************//**
 * @addtogroup DAC8
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/******************************************************************************************************************//**
 * @brief Perform required initialization described in hardware manual.  Implements dac_api_t::open.
 * Configures a single DAC channel, starts the channel, and provides a handle for use with the
 * DAC API Write and Close functions.  Must be called once prior to calling any other DAC API
 * functions.  After a channel is opened, Open should not be called again for the same channel
 * without calling Close first.
 *
 * @retval SSP_SUCCESS                      The channel was successfully opened.
 * @retval SSP_ERR_ASSERTION                One or both of the following parameters may be NULL: p_api_ctrl or p_cfg
 * @retval SSP_ERR_IN_USE                   DAC8 resource is locked.
 * @retval SSP_ERR_IP_CHANNEL_NOT_PRESENT   An invalid channel was requested.
 * @retval SSP_ERR_UNSUPPORTED              Output amplifier is not supported.
 *                                          Real time mode is not supported.
 *                                          Charge pump is not supported.
 *                                          A/D - D/A synchronization is not supported.
 * @return                                  See @ref Common_Error_Codes or functions called by this function for other possible
 *                                              return codes. This function calls:
 *                                              * fmi_api_t::productFeatureGet
 *
 * @note This function is reentrant for different channels.  It is not reentrant for the same channel.
 **********************************************************************************************************************/
ssp_err_t R_DAC8_Open(dac_ctrl_t * p_api_ctrl, dac_cfg_t const * const p_cfg)
{
    dac8_instance_ctrl_t * p_ctrl = (dac8_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err;

    /** Validate the input parameter. */
#if DAC8_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_ctrl);
#endif /* if DAC8_CFG_PARAM_CHECKING_ENABLE */

    /** Make sure the peripheral exists. */
    /* Note: The DAC8 module is unit 0 with respect to module start
     * and unit 1 with respect to FMI. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 1U;
    ssp_feature.id = SSP_IP_DAC;
    fmi_feature_info_t info = {0U};

    err = g_fmi_on_fmi.productFeatureGet (&ssp_feature, &info);
    DAC8_ERROR_RETURN(SSP_SUCCESS == err, err);

    dac8_variant_info_t variant_info;
    variant_info.data = (uint16_t) info.variant_data;

#if DAC8_CFG_PARAM_CHECKING_ENABLE
    err = r_dac8_validate_config(p_cfg, variant_info);
    DAC8_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif /* if DAC8_CFG_PARAM_CHECKING_ENABLE */

    /** Lock the DAC Hardware Resource. */
    /* Note: Hardware locks are provided for each channel on the DAC, but the DAC is otherwise viewed as a single
     * peripheral with no channels by the FMI and R_BSP_ModuleStart APIs. */
    ssp_feature.channel = p_cfg->channel;
    err = R_BSP_HardwareLock (&ssp_feature);
    ssp_feature.channel = 0U;
    DAC8_ERROR_RETURN((SSP_SUCCESS == err), err);

    p_ctrl->p_reg = (void *) info.ptr;

    /** Power on the DAC device. */
    R_BSP_ModuleStart (&ssp_feature);

    /** Stop the channel. */
    R_DAC8_Type * p_dac_reg = (R_DAC8_Type *) p_ctrl->p_reg;
    HW_DAC8_Control (p_dac_reg, p_cfg->channel, HW_DAC8_CONTROL_DISABLE);

    /** Set defaults. */
    dac8_mode_t dac_mode = DAC8_MODE_NORMAL;
    bool charge_pump_enabled = false;

    if (p_cfg->p_extend)
    {
        const dac8_extended_cfg_t *pextend = (dac8_extended_cfg_t *) p_cfg->p_extend;

        charge_pump_enabled = pextend->enable_charge_pump;
        dac_mode = pextend->dac_mode;
    }

    /** Configure the charge pump. */
    if (variant_info.charge_pump_present)
    {
        HW_DAC8_ChargePumpCfg (p_dac_reg, charge_pump_enabled);
    }

    /** Configure the DAC mode. */
    if (variant_info.real_time_support)
    {
        HW_DAC8_ControlMode (p_dac_reg, p_cfg->channel, dac_mode);
    }

    /** Configure DA/AD mode. */
    if (variant_info.ad_sync_supported)
    {
        HW_DAC8_DaAdSyncCfg (p_dac_reg, (uint8_t) p_cfg->ad_da_synchronized);
    }

    /** Initialize the channel state information. */
    p_ctrl->channel         = p_cfg->channel;
    p_ctrl->channel_started = false;
    p_ctrl->data_format     = p_cfg->data_format;
    p_ctrl->channel_opened  = DAC8_OPEN;

    /** All done.  Return. */
    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief    Stop the D/A conversion, stop output, and close the DAC channel.
 * @retval   SSP_SUCCESS           The channel is successfully closed.
 * @retval   SSP_ERR_ASSERTION     p_api_ctrl is NULL.
 * @retval   SSP_ERR_NOT_OPEN      Channel associated with p_ctrl has not been opened.
 **********************************************************************************************************************/
ssp_err_t R_DAC8_Close(dac_ctrl_t * p_api_ctrl)
{
    dac8_instance_ctrl_t * p_ctrl = (dac8_instance_ctrl_t *) p_api_ctrl;

#if DAC8_CFG_PARAM_CHECKING_ENABLE
    /** Validate the handle parameter */
    SSP_ASSERT(NULL != p_ctrl);

    /** Validate that the channel is opened. */
    DAC8_ERROR_RETURN(DAC8_OPEN == p_ctrl->channel_opened, SSP_ERR_NOT_OPEN);
#endif

    /** Stop the channel */
    R_DAC8_Type * p_dac_reg = (R_DAC8_Type *) p_ctrl->p_reg;
    HW_DAC8_Control (p_dac_reg, p_ctrl->channel, HW_DAC8_CONTROL_DISABLE);

    /** Update the channel state information. */
    p_ctrl->channel_opened = 0U;
    p_ctrl->channel_started = false;

    /** Update DAC Hardware Resource information. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U }};
    ssp_feature.channel = p_ctrl->channel;
    ssp_feature.id = SSP_IP_DAC;

    /** Unlock the DAC Hardware Resource */
    ssp_feature.unit = 1U;
    R_BSP_HardwareUnlock (&ssp_feature);

    /* All done, return success. */
    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief    Write data to the D/A converter and enable the output if it has not been enabled.
 *
 * @retval   SSP_SUCCESS           Data is successfully written to the D/A Converter.
 * @retval   SSP_ERR_ASSERTION     p_api_ctrl is NULL.
 * @retval   SSP_ERR_NOT_OPEN      Channel associated with p_ctrl has not been opened.
 * @retval   SSP_ERR_OVERFLOW      Data overflow when data value exceeds 8-bit limit.
 *
 * @note     Write function automatically starts the D/A conversion after data is successfully written
 *           to the channel.
 **********************************************************************************************************************/
ssp_err_t R_DAC8_Write(dac_ctrl_t * p_api_ctrl, dac_size_t value)
{
    dac8_instance_ctrl_t * p_ctrl = (dac8_instance_ctrl_t *) p_api_ctrl;

#if DAC8_CFG_PARAM_CHECKING_ENABLE
    /** Validate the handle parameter */
    SSP_ASSERT(NULL != p_ctrl);

    /** Validate that the channel is opened. */
    DAC8_ERROR_RETURN(DAC8_OPEN == p_ctrl->channel_opened, SSP_ERR_NOT_OPEN);
#endif

    /** Handle data format. */
    if (DAC_DATA_FORMAT_FLUSH_LEFT == p_ctrl->data_format)
    {
        value = value >> 8;
    }
    else
    {
#if DAC8_CFG_PARAM_CHECKING_ENABLE
        /** Validate that data does not overflow 8 bits. */
        DAC8_ERROR_RETURN(value <= 0xFFU, SSP_ERR_OVERFLOW);
#endif
    }
    /** Convert to 8 bits. */
    uint8_t dac_value = (uint8_t)value & 0xFFU;

    /** Write the value to D/A converter. */
    R_DAC8_Type * p_dac_reg = (R_DAC8_Type *) p_ctrl->p_reg;
    HW_DAC8_DataWrite (p_dac_reg, p_ctrl->channel, dac_value);

    /** Start the converter if it has been idle. */
    if (!p_ctrl->channel_started)
    {
        /** Start the channel */
        HW_DAC8_Control (p_dac_reg, p_ctrl->channel, HW_DAC8_CONTROL_ENABLE);

        p_ctrl->channel_started = true;
    }

    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief    Start the D/A conversion output.
 *
 * @retval   SSP_SUCCESS           The channel is started successfully.
 * @retval   SSP_ERR_ASSERTION     p_api_ctrl is NULL.
 * @retval   SSP_ERR_NOT_OPEN      Channel associated with p_ctrl has not been opened.
 **********************************************************************************************************************/
ssp_err_t R_DAC8_Start(dac_ctrl_t * p_api_ctrl)
{
    dac8_instance_ctrl_t * p_ctrl = (dac8_instance_ctrl_t *) p_api_ctrl;

#if DAC8_CFG_PARAM_CHECKING_ENABLE
    /** Validate the handle parameter */
    SSP_ASSERT(NULL != p_ctrl);

    /** Validate that the channel is opened. */
    DAC8_ERROR_RETURN(DAC8_OPEN == p_ctrl->channel_opened, SSP_ERR_NOT_OPEN);
#endif

    /** Enable the output. */
    R_DAC8_Type * p_dac_reg = (R_DAC8_Type *) p_ctrl->p_reg;
    HW_DAC8_Control (p_dac_reg, p_ctrl->channel, HW_DAC8_CONTROL_ENABLE);

    /** Update the internal state. */
    p_ctrl->channel_started = true;

    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief  Stop the D/A conversion and disable the output signal.
 *
 * @retval  SSP_SUCCESS           The control is successfully stopped.
 * @retval  SSP_ERR_ASSERTION     p_api_ctrl is NULL.
 * @retval  SSP_ERR_NOT_OPEN      Channel associated with p_ctrl has not been opened.
 **********************************************************************************************************************/
ssp_err_t R_DAC8_Stop(dac_ctrl_t * p_api_ctrl)
{
    dac8_instance_ctrl_t * p_ctrl = (dac8_instance_ctrl_t *) p_api_ctrl;

#if DAC8_CFG_PARAM_CHECKING_ENABLE
    /** Validate the handle parameter */
    SSP_ASSERT(NULL != p_ctrl);

    /** Validate that the channel is opened. */
    DAC8_ERROR_RETURN(DAC8_OPEN == p_ctrl->channel_opened, SSP_ERR_NOT_OPEN);
#endif

    /** Disable the output. */
    R_DAC8_Type * p_dac_reg = (R_DAC8_Type *) p_ctrl->p_reg;
    HW_DAC8_Control (p_dac_reg, p_ctrl->channel, HW_DAC8_CONTROL_DISABLE);

    /** Mark the internal state. */
    p_ctrl->channel_started = false;

    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief  Get version and store it in provided pointer p_version.
 * @retval  SSP_SUCCESS           Successfully retrieved version information.
 * @retval  SSP_ERR_ASSERTION     p_version is NULL.
 **********************************************************************************************************************/
ssp_err_t R_DAC8_VersionGet(ssp_version_t * p_version)
{
#if DAC8_CFG_PARAM_CHECKING_ENABLE
    /** Verify pointer argument is not NULL. */
    SSP_ASSERT(NULL != p_version);
#endif

    /** Return the version number */
    p_version->version_id = g_dac8_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Get information about DAC Resolution and store it in provided pointer p_info.
 *
 * @retval SSP_SUCCESS           Value of DAC resolution written to caller's
 *                               structure successfully.
 * @retval SSP_ERR_ASSERTION     The p_info parameter was null.
 **********************************************************************************************************************/
ssp_err_t R_DAC8_InfoGet (dac_info_t    * const p_info)
{
#if DAC8_CFG_PARAM_CHECKING_ENABLE
    /** Make sure parameters are valid. */
    SSP_ASSERT(NULL != p_info);
#endif

    /** Update DAC resolution as 8-bit */
    p_info->bit_width = 8U;

    return SSP_SUCCESS;
} /* End of function R_DAC8_InfoGet */

/*******************************************************************************************************************//**
 * @} (end addtogroup DAC8)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

#if DAC8_CFG_PARAM_CHECKING_ENABLE

/******************************************************************************************************************//**
 * @brief Validate requested channel.
 * @param[in]   p_cfg                            Pointer to DAC configuration.
 * @param[in]   variant_info                     To validate the number of channels supports.
 * @retval      SSP_SUCCESS                      The parameters are valid.
 * @retval      SSP_ERR_IP_CHANNEL_NOT_PRESENT   An invalid channel was requested.
 *
 **********************************************************************************************************************/
static ssp_err_t r_dac8_validate_channel(dac_cfg_t const * const p_cfg, dac8_variant_info_t variant_info)
{
    /** Validate channel is supported */
    uint32_t max_channels = variant_info.channels_count;
    /* FMI defines a channel count of zero as two supported channels. */
    if (0U == max_channels)
    {
        max_channels = 2U;
    }
    if (p_cfg->channel >= max_channels)
    {
        return SSP_ERR_IP_CHANNEL_NOT_PRESENT;
    }
    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief Validate user input parameters.
 * @param[in]   p_cfg                            Pointer to DAC configuration
 * @param[in]   variant_info                     To validate the DAC8 supported modes.
 * @retval      SSP_SUCCESS                      The parameters are valid.
 * @retval      SSP_ERR_IP_CHANNEL_NOT_PRESENT   An invalid channel was requested.
 * @retval      SSP_ERR_UNSUPPORTED              Output amplifier is not supported.
 *                                               Real time mode is not supported.
 *                                               Charge pump is not supported.
 *                                               A/D - D/A synchronization is not supported.
 *
 **********************************************************************************************************************/
static ssp_err_t r_dac8_validate_config(dac_cfg_t const * const p_cfg, dac8_variant_info_t variant_info)
{
    /** Output amplifier is not supported by hardware */
    if (false != p_cfg->output_amplifier_enabled)
    {
        return SSP_ERR_UNSUPPORTED;
    }

    /** Validate p_extend if present */
    if (NULL != p_cfg->p_extend)
    {
        const dac8_extended_cfg_t *pextend = (dac8_extended_cfg_t *) p_cfg->p_extend;

        /** Validate mode is supported */
        if ((pextend->dac_mode == DAC8_MODE_REAL_TIME) && (!variant_info.real_time_support))
        {
            return SSP_ERR_UNSUPPORTED;
        }

        /** Validate charge pump is supported */
        if (pextend->enable_charge_pump && (!variant_info.charge_pump_present))
        {
            return SSP_ERR_UNSUPPORTED;
        }
    }

    /** Validate A/D D/A synchronization is supported */
    if (p_cfg->ad_da_synchronized && (!variant_info.ad_sync_supported))
    {
        return SSP_ERR_UNSUPPORTED;
    }

    return r_dac8_validate_channel(p_cfg, variant_info);
}

#endif
