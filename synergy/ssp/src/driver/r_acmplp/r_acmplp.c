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
* File Name    : r_acmplp.c
* Description  : Primary source code for Low Power Analog Comparator
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/

#include "bsp_api.h"
#include "r_acmplp_cfg.h"
#include "r_acmplp.h"

#include "hw/hw_acmplp_private.h"
#include "r_cgc.h"
#include "r_acmplp_private_api.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define ACMPLP_OPEN                        (0x434d504CU)

#define ACMPLP_STABILIZATION_WAIT_TIME_US  (100U)

/** Macro for error logger. */
#ifndef ACMPLP_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define ACMPLP_ERROR_RETURN(a, err)  SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_acmplp_version)
#endif

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
void comp_lp_int_isr(void);

/** Version data structure used by error logger macro. */
static const ssp_version_t g_acmplp_version =
{
    .api_version_minor  = COMPARATOR_API_VERSION_MINOR,
    .api_version_major  = COMPARATOR_API_VERSION_MAJOR,
    .code_version_major = ACMPLP_CODE_VERSION_MAJOR,
    .code_version_minor = ACMPLP_CODE_VERSION_MINOR
};

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "acmplp";
#endif

/***********************************************************************************************************************
 Global Variables
 **********************************************************************************************************************/

/** ACMPLP Implementation of comparator interface. */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const comparator_api_t g_comparator_on_acmplp =
{
    .open                   = R_ACMPLP_Open,
    .outputEnable           = R_ACMPLP_OutputEnable,
    .infoGet                = R_ACMPLP_InfoGet,
    .statusGet              = R_ACMPLP_StatusGet,
    .close                  = R_ACMPLP_Close,
    .versionGet             = R_ACMPLP_VersionGet
};

/*******************************************************************************************************************//**
 * @addtogroup ACMPLP
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Configures the comparator and starts operation. Callbacks and pin output are not active until outputEnable() is
 * called. comparator_api_t::outputEnable() should be called after the output has stabilized. Implements
 * comparator_api_t::open().
 *
 * Comparator inputs must be configured in the application code prior to calling this function.
 *
 * @retval SSP_SUCCESS                    Open successful.
 * @retval SSP_ERR_ASSERTION              An input pointer is NULL
 * @retval SSP_ERR_INVALID_ARGUMENT       An argument is invalid. Window mode (COMPARATOR_MODE_WINDOW) and filter of 1
 *                                        (COMPARATOR_FILTER_1) are not supported in this implementation.
 * @retval SSP_ERR_IN_USE                 The control block is already open or the hardware lock is taken.
 * @return                                See @ref Common_Error_Codes or functions called by this function for other
 *                                        possible return codes. This function calls:
 *                                              * fmi_api_t::productFeatureGet
 **********************************************************************************************************************/
ssp_err_t R_ACMPLP_Open (comparator_ctrl_t      * const p_api_ctrl,
                                 comparator_cfg_t const * const p_cfg)
{
    acmplp_instance_ctrl_t * p_ctrl = (acmplp_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;

#if ACMPLP_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_ctrl);
#endif

    /** Verify the control block has not already been initialized. */
    ACMPLP_ERROR_RETURN(ACMPLP_OPEN != p_ctrl->open, SSP_ERR_IN_USE);

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_COMP_LP;
    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    ACMPLP_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = (R_ACMPLP_Type *) info.ptr;

    /* Acquire hardware lock for this channel of the ACMPLP */
    ssp_feature.channel = p_cfg->channel;
    err = R_BSP_HardwareLock(&ssp_feature);
    ACMPLP_ERROR_RETURN((SSP_SUCCESS == err), err);

    /* Initialize control block. */
    p_ctrl->p_callback  = p_cfg->p_callback;
    p_ctrl->p_context   = p_cfg->p_context;
    p_ctrl->channel     = p_cfg->channel;
    p_ctrl->pin_output  = p_cfg->pin_output;
    p_ctrl->invert      = p_cfg->invert;

    p_ctrl->output_enabled  = 0U;

    /** Configure interrupt priority. The interrupt is disabled until comparator_api_t::outputEnable() is called. */
    ssp_vector_info_t * p_vector_info;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_COMP_LP_INT, &event_info);
    p_ctrl->irq = event_info.irq;
    if (SSP_INVALID_VECTOR != p_ctrl->irq)
    {
        NVIC_DisableIRQ(p_ctrl->irq);
        R_SSP_VectorInfoGet(p_ctrl->irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->irq, p_cfg->irq_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }

    /** Enable clocks to the ACMPLP hardware. */
    R_BSP_ModuleStart(&ssp_feature);

    /** Set registers controlled by this channel to their default values. */
    HW_ACMPLP_RegisterReset(p_ctrl->p_reg, p_ctrl->channel);

    /** Set the mode. */
    HW_ACMPLP_ModeSet(p_ctrl->p_reg, p_ctrl->channel, p_cfg->mode);

    /** Configure the output polarity. */
    HW_ACMPLP_PolaritySet(p_ctrl->p_reg, p_ctrl->channel, p_cfg->invert);

    /** Configure the trigger edge. */
    HW_ACMPLP_TriggerSet(p_ctrl->p_reg, p_ctrl->channel, p_cfg->trigger);

    /** Configure the hardware debounce filter. */
    HW_ACMPLP_FilterSet(p_ctrl->p_reg, p_ctrl->channel, p_cfg->filter);

    /** Enable the comparator. */
    HW_ACMPLP_Start(p_ctrl->p_reg, p_ctrl->channel);

    /* Mark the control block as open */
    p_ctrl->open = ACMPLP_OPEN;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Provides the minimum stabilization wait time in microseconds. Implements comparator_api_t::infoGet().
 *
 * @retval  SSP_SUCCESS                Information stored in p_info.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_ACMPLP_InfoGet(comparator_ctrl_t * const p_api_ctrl, comparator_info_t * const p_info)
{
    acmplp_instance_ctrl_t * p_ctrl = (acmplp_instance_ctrl_t *) p_api_ctrl;

     /** Perform parameter checking  */
#if (1 == ACMPLP_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ACMPLP is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_info);
    ACMPLP_ERROR_RETURN(ACMPLP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#else
    SSP_PARAMETER_NOT_USED (p_ctrl);
#endif

    /* This comes from the Operation section in the hardware manual. */
    p_info->min_stabilization_wait_us = ACMPLP_STABILIZATION_WAIT_TIME_US;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Enables the comparator output, which can be polled using comparator_api_t::statusGet(). Also enables pin output and
 * interrupts as configured during comparator_api_t::open(). Implements comparator_api_t::outputEnable().
 *
 * @retval  SSP_SUCCESS                Comparator output is enabled.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_ACMPLP_OutputEnable(comparator_ctrl_t * const p_api_ctrl)
{
    acmplp_instance_ctrl_t * p_ctrl = (acmplp_instance_ctrl_t *) p_api_ctrl;

#if (1 == ACMPLP_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ACMPLP is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    ACMPLP_ERROR_RETURN(ACMPLP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** Set the VCOUT output setting for this channel (enabled or disabled). */
    HW_ACMPLP_PinOutputEnable(p_ctrl->p_reg, p_ctrl->channel, p_ctrl->pin_output);

    /** Enable interrupts for this channel. */
    if (SSP_INVALID_VECTOR != p_ctrl->irq)
    {
        R_BSP_IrqStatusClear(p_ctrl->irq);
        NVIC_ClearPendingIRQ(p_ctrl->irq);
        NVIC_EnableIRQ(p_ctrl->irq);
    }

    p_ctrl->output_enabled = 1U;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Provides the operating status of the comparator. Implements comparator_api_t::statusGet().
 *
 * @retval  SSP_SUCCESS                Operating status of the comparator is provided in p_status.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_ACMPLP_StatusGet(comparator_ctrl_t * const p_api_ctrl, comparator_status_t * const p_status)
{
    acmplp_instance_ctrl_t * p_ctrl = (acmplp_instance_ctrl_t *) p_api_ctrl;

#if (1 == ACMPLP_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ACMPLP is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_status);
    ACMPLP_ERROR_RETURN(ACMPLP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    if (0U == p_ctrl->output_enabled)
    {
        p_status->state = COMPARATOR_STATE_OUTPUT_DISABLED;
    }
    else
    {
        /** Read the operating status of the comparator. */
        uint8_t state = HW_ACMPLP_StatusGet(p_ctrl->p_reg, p_ctrl->channel);
        if (COMPARATOR_POLARITY_INVERT_ON == p_ctrl->invert)
        {
            state = !state;
        }
        if (1U == state)
        {
            p_status->state = COMPARATOR_STATE_OUTPUT_HIGH;
        }
        else
        {
            p_status->state = COMPARATOR_STATE_OUTPUT_LOW;
        }
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Stops the comparator. Implements comparator_api_t::close().
 *
 * @retval  SSP_SUCCESS                Instance control block closed successfully.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_ACMPLP_Close(comparator_ctrl_t * p_api_ctrl)
{
    acmplp_instance_ctrl_t * p_ctrl = (acmplp_instance_ctrl_t *) p_api_ctrl;

    /** Perform parameter checking*/
#if (1 == ACMPLP_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ACMPLP is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    ACMPLP_ERROR_RETURN(ACMPLP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** Mark driver as closed   */
    p_ctrl->open = 0U;

    /** Disable interrupts. */
    if (SSP_INVALID_VECTOR != p_ctrl->irq)
    {
        NVIC_DisableIRQ(p_ctrl->irq);
        ssp_vector_info_t * p_vector_info;
        R_SSP_VectorInfoGet(p_ctrl->irq, &p_vector_info);
        *(p_vector_info->pp_ctrl) = NULL;
    }

    /** Stop the comparator and disable output to VCOUT. */
    HW_ACMPLP_Stop(p_ctrl->p_reg, p_ctrl->channel);

    /** Enter the module-stop state. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.id = SSP_IP_COMP_LP;
    ssp_feature.channel = p_ctrl->channel;
    ssp_feature.unit = 0U;
    R_BSP_ModuleStop(&ssp_feature);

    /** Release the hardware lock */
    R_BSP_HardwareUnlock(&ssp_feature);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Gets the API and code version. Implements comparator_api_t::versionGet().
 *
 * @retval  SSP_SUCCESS        Version information available in p_version.
 * @retval  SSP_ERR_ASSERTION  The parameter p_version is NULL.
***********************************************************************************************************************/
ssp_err_t R_ACMPLP_VersionGet(ssp_version_t * const p_version)
{
#if (1 == ACMPLP_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointer is not NULL. */
    SSP_ASSERT(NULL != p_version);
#endif

    /** Return the version number */
    p_version->version_id =  g_acmplp_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup ACMPLP)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 Private Functions
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * Comparator interrupt
 **********************************************************************************************************************/
void comp_lp_int_isr(void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    /* Clear the IRQ flag. */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    acmplp_instance_ctrl_t * p_ctrl = (acmplp_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    if (NULL != p_ctrl)
    {
        if (NULL != p_ctrl->p_callback)
        {
            /** Call user callback if one was provided. */
            comparator_callback_args_t args;
            args.channel   = p_ctrl->channel;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
    }

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

