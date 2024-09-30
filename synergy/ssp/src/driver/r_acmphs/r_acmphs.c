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
* File Name    : r_acmphs.c
* Description  : Primary source code for High-Speed Analog Comparator
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/

#include "bsp_api.h"
#include "r_acmphs_cfg.h"
#include "r_acmphs.h"

#include "hw/hw_acmphs_private.h"
#include "r_cgc.h"
#include "r_acmphs_private_api.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define ACMPHS_OPEN                        (0x434d5048U)

/** Macro for error logger. */
#ifndef ACMPHS_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define ACMPHS_ERROR_RETURN(a, err)  SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_acmphs_version)
#endif

#define ACMPHS_PRIV_MAX_STATUS_RETRIES     (1024U)

#define ACMPHS_PRIV_US_PER_S               (1000000U)
#define ACMPHS_PRIV_FILTER_WAIT_CLOCKS     (4U)
#define ACMPHS_PRIV_FILTER_DIVISOR_BASE    (4U)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
void comp_hs_int_isr(void);

/** Version data structure used by error logger macro. */
static const ssp_version_t g_acmphs_version =
{
    .api_version_minor  = COMPARATOR_API_VERSION_MINOR,
    .api_version_major  = COMPARATOR_API_VERSION_MAJOR,
    .code_version_major = ACMPHS_CODE_VERSION_MAJOR,
    .code_version_minor = ACMPHS_CODE_VERSION_MINOR
};

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "acmphs";
#endif

/***********************************************************************************************************************
 Global Variables
 **********************************************************************************************************************/

/** ACMPHS Implementation of comparator interface. */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const comparator_api_t g_comparator_on_acmphs =
{
    .open                   = R_ACMPHS_Open,
    .outputEnable           = R_ACMPHS_OutputEnable,
    .infoGet                = R_ACMPHS_InfoGet,
    .statusGet              = R_ACMPHS_StatusGet,
    .close                  = R_ACMPHS_Close,
    .versionGet             = R_ACMPHS_VersionGet
};

/*******************************************************************************************************************//**
 * @addtogroup ACMPHS
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
ssp_err_t R_ACMPHS_Open (comparator_ctrl_t      * const p_api_ctrl,
                                 comparator_cfg_t const * const p_cfg)
{
    acmphs_instance_ctrl_t * p_ctrl = (acmphs_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;

#if ACMPHS_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_ctrl);

    /* COMPARATOR_FILTER_1 and COMPARATOR_MODE_WINDOW are not supported in this implementation. */
    ACMPHS_ERROR_RETURN(COMPARATOR_FILTER_1 != p_cfg->filter, SSP_ERR_INVALID_ARGUMENT);
    ACMPHS_ERROR_RETURN(COMPARATOR_MODE_WINDOW != p_cfg->mode, SSP_ERR_INVALID_ARGUMENT);
#endif

    /** Verify the control block has not already been initialized. */
    ACMPHS_ERROR_RETURN(ACMPHS_OPEN != p_ctrl->open, SSP_ERR_IN_USE);

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_cfg->channel;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_COMP_HS;
    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    ACMPHS_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = (R_ACMPHS0_Type *) info.ptr;

    /* Acquire hardware lock for this channel of the ACMPHS */
    err = R_BSP_HardwareLock(&ssp_feature);
    ACMPHS_ERROR_RETURN((SSP_SUCCESS == err), SSP_ERR_IN_USE);

    /* Initialize control block. */
    p_ctrl->p_callback  = p_cfg->p_callback;
    p_ctrl->p_context   = p_cfg->p_context;
    p_ctrl->channel     = p_cfg->channel;
    p_ctrl->pin_output  = p_cfg->pin_output;

    /** Configure interrupt priority. The interrupt is disabled until comparator_api_t::outputEnable() is called. */
    ssp_vector_info_t * p_vector_info;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_COMP_HS_INT, &event_info);
    p_ctrl->irq = event_info.irq;
    if (SSP_INVALID_VECTOR != p_ctrl->irq)
    {
        NVIC_DisableIRQ(p_ctrl->irq);
        R_SSP_VectorInfoGet(p_ctrl->irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->irq, p_cfg->irq_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }

    /** Enable clocks to the ACMPHS hardware. */
    R_BSP_ModuleStart(&ssp_feature);

    /** Set registers controlled by this driver to their default values. */
    HW_ACMPHS_Stop(p_ctrl->p_reg);

    /** Configure the output polarity. */
    HW_ACMPHS_PolaritySet(p_ctrl->p_reg, p_cfg->invert);

    /** Configure the trigger edge. */
    HW_ACMPHS_TriggerSet(p_ctrl->p_reg, p_cfg->trigger);

    /** Configure the hardware debounce filter. */
    HW_ACMPHS_FilterSet(p_ctrl->p_reg, p_cfg->filter);

    /** Enable the comparator. */
    HW_ACMPHS_Start(p_ctrl->p_reg);

    /* Mark the control block as open */
    p_ctrl->open = ACMPHS_OPEN;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Provides the minimum stabilization wait time in microseconds. Implements comparator_api_t::infoGet().
 *
 * @retval  SSP_SUCCESS                Information stored in p_info.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
***********************************************************************************************************************/
ssp_err_t R_ACMPHS_InfoGet(comparator_ctrl_t * const p_api_ctrl, comparator_info_t * const p_info)
{
    acmphs_instance_ctrl_t * p_ctrl = (acmphs_instance_ctrl_t *) p_api_ctrl;

     /** Perform parameter checking  */
#if (1 == ACMPHS_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ACMPHS is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_info);
    ACMPHS_ERROR_RETURN(ACMPHS_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    bsp_feature_acmphs_t feature = {0U};
    R_BSP_FeatureAcmphsGet(&feature);

    /** Get the base stabilization time. */
    uint32_t filter_stabilization_us = 0U;
    comparator_filter_t filter = HW_ACMPHS_FilterGet(p_ctrl->p_reg);
    if (COMPARATOR_FILTER_OFF != filter)
    {
        uint32_t pclk_freq_hz = 0;
        g_cgc_on_cgc.systemClockFreqGet(CGC_SYSTEM_CLOCKS_PCLKB, &pclk_freq_hz);

        /** Add 4 filter clocks if the filter is enabled. */
        uint32_t divisor;
        divisor = ACMPHS_PRIV_FILTER_DIVISOR_BASE << (uint32_t) filter;
        filter_stabilization_us = (ACMPHS_PRIV_FILTER_WAIT_CLOCKS * ACMPHS_PRIV_US_PER_S * divisor) / pclk_freq_hz;

        /* Round up. */
        filter_stabilization_us += 1U;
    }

    p_info->min_stabilization_wait_us = feature.min_wait_time_us + filter_stabilization_us;

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
ssp_err_t R_ACMPHS_OutputEnable(comparator_ctrl_t * const p_api_ctrl)
{
    acmphs_instance_ctrl_t * p_ctrl = (acmphs_instance_ctrl_t *) p_api_ctrl;

#if (1 == ACMPHS_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ACMPHS is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    ACMPHS_ERROR_RETURN(ACMPHS_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** Enable the ACMPHS output. */
    HW_ACMPHS_OutputEnable(p_ctrl->p_reg);

    /** Set the VCOUT output setting for this channel (enabled or disabled). */
    HW_ACMPHS_PinOutputEnable(p_ctrl->p_reg, p_ctrl->pin_output);

    /** Enable interrupts for this channel. */
    if (SSP_INVALID_VECTOR != p_ctrl->irq)
    {
        R_BSP_IrqStatusClear(p_ctrl->irq);
        NVIC_ClearPendingIRQ(p_ctrl->irq);
        NVIC_EnableIRQ(p_ctrl->irq);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Provides the operating status of the comparator. Implements comparator_api_t::statusGet().
 *
 * @retval  SSP_SUCCESS                Operating status of the comparator is provided in p_status.
 * @retval  SSP_ERR_ASSERTION          An input pointer was NULL.
 * @retval  SSP_ERR_NOT_OPEN           Instance control block is not open.
 * @retval  SSP_ERR_TIMEOUT            The debounce filter is off and 2 consecutive matching values were not read within
 *                                     1024 attempts.
***********************************************************************************************************************/
ssp_err_t R_ACMPHS_StatusGet(comparator_ctrl_t * const p_api_ctrl, comparator_status_t * const p_status)
{
    acmphs_instance_ctrl_t * p_ctrl = (acmphs_instance_ctrl_t *) p_api_ctrl;

#if (1 == ACMPHS_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ACMPHS is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_status);
    ACMPHS_ERROR_RETURN(ACMPHS_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** Read the operating status of the comparator. */
    if (0U == HW_ACMPHS_OutputStatusGet(p_ctrl->p_reg))
    {
        p_status->state = COMPARATOR_STATE_OUTPUT_DISABLED;
    }
    else
    {
        uint8_t state = HW_ACMPHS_StatusGet(p_ctrl->p_reg);
        if (COMPARATOR_FILTER_OFF == HW_ACMPHS_FilterGet(p_ctrl->p_reg))
        {
            /* Software debounce of 2 consecutive reads is required by the hardware manual if the hardware debounce filter is off. */
            uint8_t state2 = HW_ACMPHS_StatusGet(p_ctrl->p_reg);
            uint32_t retries = ACMPHS_PRIV_MAX_STATUS_RETRIES;
            while ((state2 != state) && (retries > 0U))
            {
                state = state2;
                state2 = HW_ACMPHS_StatusGet(p_ctrl->p_reg);
                retries--;
            }
            ACMPHS_ERROR_RETURN(0U != retries, SSP_ERR_TIMEOUT);
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
ssp_err_t R_ACMPHS_Close(comparator_ctrl_t * p_api_ctrl)
{
    acmphs_instance_ctrl_t * p_ctrl = (acmphs_instance_ctrl_t *) p_api_ctrl;

    /** Perform parameter checking*/
#if (1 == ACMPHS_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ACMPHS is already open. */
    SSP_ASSERT(NULL != p_ctrl);
    ACMPHS_ERROR_RETURN(ACMPHS_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
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
    HW_ACMPHS_Stop(p_ctrl->p_reg);

    /** Enter the module-stop state. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.id = SSP_IP_COMP_HS;
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
ssp_err_t R_ACMPHS_VersionGet(ssp_version_t * const p_version)
{
#if (1 == ACMPHS_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointer is not NULL. */
    SSP_ASSERT(NULL != p_version);
#endif

    /** Return the version number */
    p_version->version_id =  g_acmphs_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup ACMPHS)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 Private Functions
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * Comparator interrupt
 **********************************************************************************************************************/
void comp_hs_int_isr(void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    /* Clear the IRQ flag. */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    acmphs_instance_ctrl_t * p_ctrl = (acmphs_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

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

