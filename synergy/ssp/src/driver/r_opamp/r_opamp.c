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
* File Name    : r_opamp.c
* Description  : Primary source code for Operational Amplifier
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/

#include "bsp_api.h"
#include "r_opamp_api.h"
/* Configuration for this package. */
#include "r_opamp_cfg.h"
/* Private header file for this package. */
#include "r_opamp.h"

#include "hw/hw_opamp_private.h"
#include "r_cgc.h"
#include "r_opamp_private_api.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @addtogroup OPAMP
 * @{
 **********************************************************************************************************************/
#define OPAMP_OPEN                        (0x4f414d50U)

/** Macro for error logger. */
#ifndef OPAMP_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define OPAMP_ERROR_RETURN(a, err)  SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_opamp_version)
#endif

#define OPAMP_PRIV_VARIANT_UNUSED_LSBS          (2U)
#define OPAMP_PRIV_VARIANT_TRIM_BIT             (2U)
#define OPAMP_PRIV_VARIANT_MASK                 (3U)
#define OPAMP_PRIV_VARIANT_4_AMP_NO_SWITCHES    (1U)
#define OPAMP_PRIV_VARIANT_3_AMP_WITH_SWITCHES  (2U)

#define OPAMP_PRIV_4CH_VARIANT_AMPMC_MASK       (0x80U)
#define OPAMP_PRIV_4CH_VARIANT_CHANNEL_MASK     (0xFU)
#define OPAMP_PRIV_3CH_VARIANT_CHANNEL_MASK     (0x7U)

#define OPAMP_PRIV_AMPMC_LOW_POWER              (0U)
#define OPAMP_PRIV_AMPMC_MIDDLE_SPEED           (0x40U)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
#if (1 == OPAMP_CFG_PARAM_CHECKING_ENABLE)
static ssp_err_t r_opamp_trim_param_check(opamp_instance_ctrl_t * const p_ctrl, opamp_trim_cmd_t const cmd, opamp_trim_args_t const * const p_args);
#endif

/** Version data structure used by error logger macro. */
static const ssp_version_t g_opamp_version =
{
    .api_version_minor  = OPAMP_API_VERSION_MINOR,
    .api_version_major  = OPAMP_API_VERSION_MAJOR,
    .code_version_major = OPAMP_CODE_VERSION_MAJOR,
    .code_version_minor = OPAMP_CODE_VERSION_MINOR
};

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "opamp";
#endif

/***********************************************************************************************************************
 Global Variables
 **********************************************************************************************************************/

/** OPAMP Implementation of OPAMP interface. */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const opamp_api_t g_opamp_on_opamp =
{
    .open                   = R_OPAMP_Open,
    .start                  = R_OPAMP_Start,
    .stop                   = R_OPAMP_Stop,
    .trim                   = R_OPAMP_Trim,
    .infoGet                = R_OPAMP_InfoGet,
    .statusGet              = R_OPAMP_StatusGet,
    .close                  = R_OPAMP_Close,
    .versionGet             = R_OPAMP_VersionGet
};

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Applies power to the OPAMP and initializes the hardware based on the user configuration. Implements
 * opamp_api_t::open().
 *
 * The op-amp is not operational until the opamp_api_t::start() is called. If the op-amp is configured to start after
 * AGT compare match, the op-amp is not operational until opamp_api_t::start() and the associated AGT compare match
 * event occurs.
 *
 * Some MCUs have switches that must be set before starting the op-amp.  These switches must be set in the application
 * code after opamp_api_t::open() and before opamp_api_t::start().
 *
 * @retval  SSP_SUCCESS                Configuration successful.
 * @retval  SSP_ERR_ASSERTION          An input pointer is NULL.
 * @retval  SSP_ERR_INVALID_ARGUMENT   An input argument is invalid.
 * @retval  SSP_ERR_IN_USE             Control block is already opened.
 * @return                             See @ref Common_Error_Codes or functions called by this function for other
 *                                     possible return codes. This function calls:
 *                                        * fmi_api_t::productFeatureGet
***********************************************************************************************************************/
ssp_err_t R_OPAMP_Open(opamp_ctrl_t * const p_api_ctrl,  opamp_cfg_t const * const p_cfg)
{
    opamp_instance_ctrl_t * p_ctrl = (opamp_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;

#if OPAMP_CFG_PARAM_CHECKING_ENABLE
    /* Verify the pointers are not NULL. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_cfg->p_extend);
#endif

    /** Verify the control block has not already been initialized. */
    OPAMP_ERROR_RETURN(OPAMP_OPEN != p_ctrl->opened, SSP_ERR_IN_USE);

    /** Confirm the OPAMP exists on this MCU. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_OPAMP;

    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    OPAMP_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = info.ptr;

    uint8_t amptrm_val = 0U;
    opamp_on_opamp_cfg_t * p_cfg_extend = (opamp_on_opamp_cfg_t *) p_cfg->p_extend;
    amptrm_val |= (uint8_t) p_cfg_extend->trigger_channel_0;
    amptrm_val |= (uint8_t) (p_cfg_extend->trigger_channel_1 << 2U);
    amptrm_val |= (uint8_t) (p_cfg_extend->trigger_channel_2 << 4U);

    uint8_t ampmc_val = (uint8_t) (p_cfg_extend->mode << 6U);

    /* Store the variant data. */
    uint8_t variant_data = (uint8_t) (info.variant_data >> OPAMP_PRIV_VARIANT_UNUSED_LSBS);
    p_ctrl->trim_capable = (variant_data >> OPAMP_PRIV_VARIANT_TRIM_BIT) & 1U;
    if (OPAMP_PRIV_VARIANT_3_AMP_WITH_SWITCHES == (variant_data & OPAMP_PRIV_VARIANT_MASK))
    {
        p_ctrl->valid_opamps = OPAMP_PRIV_3CH_VARIANT_CHANNEL_MASK;
        p_ctrl->switches = 1U;
    }
    else
    {
#if OPAMP_CFG_PARAM_CHECKING_ENABLE
        /* This variant does not support middle speed mode. */
        OPAMP_ERROR_RETURN(OPAMP_MODE_MIDDLE_SPEED != p_cfg_extend->mode, SSP_ERR_INVALID_ARGUMENT);
#endif

        p_ctrl->valid_opamps = OPAMP_PRIV_4CH_VARIANT_CHANNEL_MASK;
        p_ctrl->switches = 0U;

        /* This variant has channel 3. */
        amptrm_val |= (uint8_t) (p_cfg_extend->trigger_channel_3 << 6U);

        /* This variant only uses bit 7 for power mode. */
        ampmc_val &= OPAMP_PRIV_4CH_VARIANT_AMPMC_MASK;
    }

    p_ctrl->trim_state = OPAMP_PRIV_TRIM_STATE_INVALID;

    /** Lock the OPAMP */
    err = R_BSP_HardwareLock(&ssp_feature);
    OPAMP_ERROR_RETURN((SSP_SUCCESS == err), err);

    /** Initialize the hardware based on the configuration. */

    R_BSP_ModuleStart(&ssp_feature);

    /** Stop operation of all op-amps. */
    HW_OPAMP_StopAll(p_ctrl->p_reg);

    /** Initialize trim registers to factory trim values. */
    if (p_ctrl->trim_capable)
    {
        HW_OPAMP_TrimDisable(p_ctrl->p_reg);
    }

    HW_OPAMP_PowerSpeedSet(p_ctrl->p_reg, ampmc_val);
    HW_OPAMP_AgtLinkSet(p_ctrl->p_reg, p_cfg_extend->agt_link);
    HW_OPAMP_TriggerSet(p_ctrl->p_reg, amptrm_val);

    p_ctrl->opened = OPAMP_OPEN;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Provides the minimum stabilization wait time in microseconds. Implements opamp_api_t::infoGet().
 *
 * @retval  SSP_SUCCESS                Information stored in p_info.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_OPAMP_InfoGet(opamp_ctrl_t * const p_api_ctrl, opamp_info_t * const p_info)
{
    opamp_instance_ctrl_t * p_ctrl = (opamp_instance_ctrl_t *) p_api_ctrl;

     /** Perform parameter checking  */
#if (1 == OPAMP_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the OPAMP is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_info);
    OPAMP_ERROR_RETURN(OPAMP_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);
#endif

    uint8_t power_mode = HW_OPAMP_PowerSpeedGet(p_ctrl->p_reg);

    bsp_feature_opamp_t feature = {0U};
    R_BSP_FeatureOpampGet(&feature);

    if (OPAMP_PRIV_AMPMC_LOW_POWER == power_mode)
    {
        p_info->min_stabilization_wait_us = feature.min_wait_time_lp_us;
    }
    else if (OPAMP_PRIV_AMPMC_MIDDLE_SPEED == power_mode)
    {
        p_info->min_stabilization_wait_us = feature.min_wait_time_ms_us;
    }
    else
    {
        p_info->min_stabilization_wait_us = feature.min_wait_time_hs_us;
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief If the OPAMP is configured for hardware triggers, enables hardware triggers.  Otherwise, starts the op-amp.
 * Implements opamp_api_t::start().
 *
 * Some MCUs have switches that must be set before starting the op-amp.  These switches must be set in the application
 * code after opamp_api_t::open() and before opamp_api_t::start().
 *
 * @retval  SSP_SUCCESS                Op-amp started or hardware triggers enabled successfully.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
 * @retval  SSP_ERR_INVALID_ARGUMENT   channel_mask includes a channel that does not exist on this MCU.
***********************************************************************************************************************/
ssp_err_t R_OPAMP_Start(opamp_ctrl_t * const p_api_ctrl, uint32_t const channel_mask)
{
    opamp_instance_ctrl_t * p_ctrl = (opamp_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;

#if (1 == OPAMP_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the OPAMP is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    OPAMP_ERROR_RETURN(OPAMP_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);

    /* Make sure all channels in the mask exist. */
    OPAMP_ERROR_RETURN(p_ctrl->valid_opamps == (p_ctrl->valid_opamps | channel_mask), SSP_ERR_INVALID_ARGUMENT);
#endif

    /** Enable AGT start and ADC conversion end triggers or start the op-amp channel(s). */
    HW_OPAMP_Start(p_ctrl->p_reg, (uint8_t) channel_mask);

    /** Return the error code */
    return err;
}

/*******************************************************************************************************************//**
 * @brief Stops the op-amp. If the OPAMP is configured for hardware triggers, disables hardware triggers. Implements
 * opamp_api_t::stop().
 *
 * @retval  SSP_SUCCESS                Op-amp stopped or hardware triggers disabled successfully.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
 * @retval  SSP_ERR_INVALID_ARGUMENT   channel_mask includes a channel that does not exist on this MCU.
***********************************************************************************************************************/
ssp_err_t R_OPAMP_Stop(opamp_ctrl_t * const p_api_ctrl, uint32_t const channel_mask)
{
    opamp_instance_ctrl_t * p_ctrl = (opamp_instance_ctrl_t *) p_api_ctrl;

#if (1 == OPAMP_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the OPAMP is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    OPAMP_ERROR_RETURN(OPAMP_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);

    /* Make sure all channels in the mask exist. */
    OPAMP_ERROR_RETURN(p_ctrl->valid_opamps == (p_ctrl->valid_opamps | channel_mask), SSP_ERR_INVALID_ARGUMENT);
#endif

    /** Disable AGT start and ADC conversion end triggers and stop the op-amp channel(s). */
    HW_OPAMP_Stop(p_ctrl->p_reg, (uint8_t) channel_mask);

    /** Return the error code */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Provides the operating status for each op-amp in a bitmask. This bit is set when operation begins, before the
 * stabilization wait time has elapsed. Implements opamp_api_t::statusGet().
 *
 * @retval  SSP_SUCCESS                Operating status of each op-amp provided in p_status.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_OPAMP_StatusGet(opamp_ctrl_t * const p_api_ctrl, opamp_status_t * const p_status)
{
    opamp_instance_ctrl_t * p_ctrl = (opamp_instance_ctrl_t *) p_api_ctrl;

#if (1 == OPAMP_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the OPAMP is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_status);
    OPAMP_ERROR_RETURN(OPAMP_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);
#endif

    /** Read the operating status of the op-amps. */
    p_status->operating_channel_mask = HW_OPAMP_StatusGet(p_ctrl->p_reg);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief On MCUs that support trimming, the op-amp trim register is set to the factory default after open(). This function
 * allows the application to trim the operational amplifier to a user setting, which overwrites the factory default
 * factory trim values.
 *
 * Not supported on all MCUs. See hardware manual for details. Not supported if configured for low power mode
 * (OPAMP_MODE_LOW_POWER).
 *
 * This function is not reentrant.  Only one side of one op-amp can be trimmed at a time. Complete the procedure for
 * one side of one channel before calling trim() with command OPAMP_TRIM_CMD_START again.
 *
 * Implements opamp_api_t::trim().
 *
 * The trim procedure works as follows:
 *  -# Call trim() for the Pch (+) side input with command OPAMP_TRIM_CMD_START.
 *  -# Connect a fixed voltage to the Pch (+) input.
 *  -# Connect the Nch (-) input to the op-amp output to create a voltage follower.
 *  -# Ensure the op-amp is operating and stabilized.
 *  -# Call trim() for the Pch (+) side input with command OPAMP_TRIM_CMD_START.
 *  -# Measure the fixed voltage connected to the Pch (+) input using the SAR ADC and save the value (referred to as A
 *    later in this procedure).
 *  -# Iterate over the following loop 5 times:
 *      -# Call trim() for the Pch (+) side input with command OPAMP_TRIM_CMD_NEXT_STEP.
 *      -# Measure the op-amp output using the SAR ADC (referred to as B in the next step).
 *      -# If A <= B, call trim() for the Pch (+) side input with command OPAMP_TRIM_CMD_CLEAR_BIT.
 *  -# Call trim() for the Nch (-) side input with command OPAMP_TRIM_CMD_START.
 *  -# Measure the fixed voltage connected to the Pch (+) input using the SAR ADC and save the value (referred to as A
 *    later in this procedure).
 *  -# Iterate over the following loop 5 times:
 *         -# Call trim() for the Nch (-) side input with command OPAMP_TRIM_CMD_NEXT_STEP.
 *         -# Measure the op-amp output using the SAR ADC (referred to as B in the next step).
 *         -# If A <= B, call trim() for the Nch (-) side input with command OPAMP_TRIM_CMD_CLEAR_BIT.
 *
 * @retval  SSP_SUCCESS                Conversion result in p_data.
 * @retval  SSP_ERR_UNSUPPORTED        Trimming is not supported on this MCU.
 * @retval  SSP_ERR_INVALID_STATE      The command is not valid in the current state of the trim state machine.
 * @retval  SSP_ERR_INVALID_ARGUMENT   The requested channel is not operating or the trim procedure is not in progress
 *                                     for this channel/input combination.
 * @retval  SSP_ERR_INVALID_MODE       Trim is not allowed in low power mode.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_OPAMP_Trim(opamp_ctrl_t * const p_api_ctrl, opamp_trim_cmd_t const cmd, opamp_trim_args_t const * const p_args)
{
    opamp_instance_ctrl_t * p_ctrl = (opamp_instance_ctrl_t *) p_api_ctrl;

#if (1 == OPAMP_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_args);
    ssp_err_t err = r_opamp_trim_param_check(p_ctrl, cmd, p_args);
    OPAMP_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    /* Enable trimming. */
    HW_OPAMP_TrimEnable(p_ctrl->p_reg, p_args->channel);

    /* Get the trim register for the requested channel and input. */
    uint8_t volatile * p_trim_reg = NULL;
    HW_OPAMP_TrimRegisterGet(p_ctrl->p_reg, p_args->channel, p_args->input, &p_trim_reg);
    if (OPAMP_TRIM_CMD_START == cmd)
    {
        /* OPAMP_PRIV_TRIM_STATE_BEGIN resets the trim procedure and can be called at any time. */
        p_ctrl->trim_state = OPAMP_PRIV_TRIM_STATE_BEGIN;
        p_ctrl->trim_channel = p_args->channel;
        p_ctrl->trim_input = p_args->input;

        /** Initialize the trim register to 0 during OPAMP_TRIM_CMD_START. */
        *p_trim_reg = 0U;
    }
    else
    {
        OPAMP_ERROR_RETURN(OPAMP_PRIV_TRIM_STATE_INVALID != p_ctrl->trim_state, SSP_ERR_INVALID_STATE);
        if (OPAMP_TRIM_CMD_NEXT_STEP == cmd)
        {
            /* OPAMP_TRIM_CMD_NEXT_STEP can only be called 5 times (one for each trim bit). */
            OPAMP_ERROR_RETURN(OPAMP_PRIV_TRIM_STATE_END != p_ctrl->trim_state, SSP_ERR_INVALID_STATE);
            p_ctrl->trim_state--;

            /** Set the next trim bit during OPAMP_TRIM_CMD_NEXT_STEP. */
            uint8_t mask = (uint8_t) (1U << p_ctrl->trim_state);
            *p_trim_reg |= mask;
        }
        else /* (OPAMP_TRIM_CMD_CLEAR_BIT == cmd) */
        {
            /* OPAMP_TRIM_CMD_CLEAR_BIT cannot be used until OPAMP_TRIM_CMD_NEXT_STEP is used at least once. */
            OPAMP_ERROR_RETURN(OPAMP_PRIV_TRIM_STATE_BEGIN != p_ctrl->trim_state, SSP_ERR_INVALID_STATE);

            /* If the current trim bit is already cleared, return an error. */
            uint8_t mask = (uint8_t) (1U << p_ctrl->trim_state);
            OPAMP_ERROR_RETURN(0U != (*p_trim_reg & mask), SSP_ERR_INVALID_STATE);

            /** Clear the current trim bit during OPAMP_TRIM_CMD_CLEAR_BIT. */
            *p_trim_reg &= (uint8_t) (~mask);
        }
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Stops the op-amps. Implements opamp_api_t::close().
 *
 * @retval  SSP_SUCCESS                Instance control block closed successfully.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_OPAMP_Close(opamp_ctrl_t * const p_api_ctrl)
{
    opamp_instance_ctrl_t * p_ctrl = (opamp_instance_ctrl_t *) p_api_ctrl;

    /** Perform parameter checking*/
#if (1 == OPAMP_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the OPAMP is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    OPAMP_ERROR_RETURN(OPAMP_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);
#endif

    /** Set all OPAMP units and the reference current generator to be stopped. */
    HW_OPAMP_StopAll(p_ctrl->p_reg);

    /** Mark driver as closed   */
    p_ctrl->opened = 0U;

    /** Enter the module-stop state. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.id = SSP_IP_OPAMP;
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    R_BSP_ModuleStop(&ssp_feature);

    /** Release the hardware lock */
    R_BSP_HardwareUnlock(&ssp_feature);

    /** Return the error code */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Gets the API and code version. Implements opamp_api_t::versionGet().
 *
 * @retval  SSP_SUCCESS        Version information available in p_version.
 * @retval  SSP_ERR_ASSERTION  The parameter p_version is NULL.
***********************************************************************************************************************/
ssp_err_t R_OPAMP_VersionGet(ssp_version_t * const p_version)
{
#if (1 == OPAMP_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointer is not NULL. */
    SSP_ASSERT(NULL != p_version);
#endif

    /** Return the version number */
    p_version->version_id =  g_opamp_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup OPAMP)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 Private Functions
 **********************************************************************************************************************/
#if (1 == OPAMP_CFG_PARAM_CHECKING_ENABLE)
/*******************************************************************************************************************//**
 * Parameter checking for trim().
 *
 * @param[in]  p_ctrl         Pointer to instance control block
 * @param[in]  cmd            Trim command
 * @param[in]  p_args         Pointer to arguments for the command
 *
 * @retval  SSP_SUCCESS                No invalid arguments or conditions found.
 * @retval  SSP_ERR_UNSUPPORTED        Trimming is not supported on this MCU.
 * @retval  SSP_ERR_INVALID_ARGUMENT   The requested channel is not operating or the trim procedure is not in progress
 *                                     for this channel/input combination.
 * @retval  SSP_ERR_INVALID_MODE       Trim is not allowed in low power mode.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
static ssp_err_t r_opamp_trim_param_check(opamp_instance_ctrl_t * const p_ctrl, opamp_trim_cmd_t const cmd, opamp_trim_args_t const * const p_args)
{

    /* Verify the pointers are not NULL and ensure the OPAMP is already open. */
    OPAMP_ERROR_RETURN(OPAMP_OPEN == p_ctrl->opened, SSP_ERR_NOT_OPEN);
    OPAMP_ERROR_RETURN(1U == p_ctrl->trim_capable, SSP_ERR_UNSUPPORTED);

    /* Trim not supported in low power mode. */
    OPAMP_ERROR_RETURN(OPAMP_PRIV_AMPMC_LOW_POWER != HW_OPAMP_PowerSpeedGet(p_ctrl->p_reg), SSP_ERR_INVALID_MODE);

    /* Make sure the channel exists. */
    OPAMP_ERROR_RETURN(p_ctrl->valid_opamps == (p_ctrl->valid_opamps | (1U << p_args->channel)), SSP_ERR_INVALID_ARGUMENT);
    if (OPAMP_TRIM_CMD_START != cmd)
    {
        /* Make sure the channel is operating. */
        uint8_t operating_channels = HW_OPAMP_StatusGet(p_ctrl->p_reg);
        OPAMP_ERROR_RETURN(operating_channels == (operating_channels | (1U << (uint8_t) p_args->channel)), SSP_ERR_INVALID_ARGUMENT);

        /* Make sure the trim procedure was most recently started for this channel and input. */
        OPAMP_ERROR_RETURN(p_ctrl->trim_channel == p_args->channel, SSP_ERR_INVALID_ARGUMENT);
        OPAMP_ERROR_RETURN(p_ctrl->trim_input == p_args->input, SSP_ERR_INVALID_ARGUMENT);
    }
    return SSP_SUCCESS;
}
#endif
