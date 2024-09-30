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
 * File Name    : r_agt_input_capture.c
 * Description  :  AGT functions used to implement the input capture interface.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_agt_input_capture.h"
#include "r_agt_input_capture_cfg.h"
#include "hw/hw_agt_private.h"
#include "r_agt_input_capture_private_api.h"
#include "r_cgc_api.h"
#include "r_cgc.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** Macro for error logger. */
#ifndef AGT_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define AGT_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_agt_input_capture_version)
#endif

/** The maximum count of a 16 bit AGT. */
#define AGT_MAX_CLOCK_COUNTS_16 (0xFFFFUL)

/** "R_AIC" in ASCII, used to determine if channel is open. */
#define R_AIC_OPEN              (0x52414943ULL)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void agt_input_capture_hardware_initialize (agt_input_capture_instance_ctrl_t      * const p_ctrl,
                                                   input_capture_cfg_t const * const p_cfg,
                                                   ssp_feature_t       const * const p_feature);
void agt_input_capture_counter_overflow_isr (void);
void agt_input_capture_compare_isr (void);
static ssp_err_t agt_input_capture_check_count_source(input_capture_cfg_t const * const p_cfg);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug.  This pragma suppresses the warnings in this 
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_agt_input_capture_version =
{
    .api_version_minor  = INPUT_CAPTURE_API_VERSION_MINOR,
    .api_version_major  = INPUT_CAPTURE_API_VERSION_MAJOR,
    .code_version_major = AGT_INPUT_CAPTURE_CODE_VERSION_MAJOR,
    .code_version_minor = AGT_INPUT_CAPTURE_CODE_VERSION_MINOR,
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "agt_input_capture";
#endif

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/
/** AGT Implementation of Input Capture Driver  */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const input_capture_api_t g_input_capture_on_agt =
{
    .open           = R_AGT_InputCaptureOpen,
    .close          = R_AGT_InputCaptureClose,
    .versionGet     = R_AGT_InputCaptureVersionGet,
    .disable        = R_AGT_InputCaptureDisable,
    .enable         = R_AGT_InputCaptureEnable,
    .infoGet        = R_AGT_InputCaptureInfoGet,
    .lastCaptureGet = R_AGT_InputCaptureLastCaptureGet,
};

/*******************************************************************************************************************//**
 * @addtogroup AGT_INPUT_CAPTURE
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Open an AGT Timer for Input Capture. Implements input_capture_api_t::open.
 *
 * The Open function configures a single AGT channel for input capture and provides a handle for use with the other
 * Input Capture API functions. This function must be called once prior to calling any other Input Capture API function.
 * After a channel is opened, the Open function should not be called again for the same channel without first calling
 * the associated Close function.
 *
 * @retval SSP_SUCCESS              Initialization was successful.
 * @retval SSP_ERR_ASSERTION        One of the parameters is NULL: p_cfg, p_ctrl, p_extend.
 * @retval SSP_ERR_IRQ_BSP_DISABLED A required interrupt does not exist in the vector table.
 * @retval SSP_ERR_IN_USE           The channel specified has already been opened. No configurations were changed. Call
 *                                  the associated Close function or use associated Control commands to reconfigure the
 *                                  channel.
 * @return                          See @ref Common_Error_Codes or functions called by this function for other possible
 *                                  return codes. This function calls:
 *                                   * fmi_api_t::productFeatureGet
 *                                   * fmi_api_t::eventInfoGet
 *
 * @note This function is reentrant for different channels.  It is not reentrant for the same channel.
 **********************************************************************************************************************/
ssp_err_t R_AGT_InputCaptureOpen (input_capture_ctrl_t      * const p_api_ctrl,
                                  input_capture_cfg_t const * const p_cfg)
{
    agt_input_capture_instance_ctrl_t * p_ctrl = (agt_input_capture_instance_ctrl_t *) p_api_ctrl;
#if AGT_INPUT_CAPTURE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_cfg->p_extend);
#endif  // AGT_INPUT_CAPTURE_CFG_PARAM_CHECKING_ENABLE

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};

    /** Get fmi feature information for AGT. */
    ssp_feature.id = SSP_IP_AGT;
    ssp_feature.channel = p_cfg->channel;
    ssp_feature.unit = 0U;
    fmi_feature_info_t info = {0};
    ssp_err_t error = SSP_SUCCESS;
    error = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    AGT_ERROR_RETURN(SSP_SUCCESS == error, error);
    p_ctrl->p_reg = info.ptr;

    /** If count source clock is not operational, return error. */
    error = agt_input_capture_check_count_source(p_cfg);
    AGT_ERROR_RETURN((SSP_ERR_CLOCK_ACTIVE == error), error);

    /** Verify channel is not already used */
    error = R_BSP_HardwareLock(&ssp_feature);
    AGT_ERROR_RETURN((SSP_SUCCESS == error), SSP_ERR_IN_USE);

    /** Get fmi measurement and overflow event information for AGT. */
    ssp_vector_info_t * p_vector_info;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    error = g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_AGT_INT, &event_info);
    p_ctrl->overflow_irq = event_info.irq;
    if (SSP_SUCCESS != error)
    {
        /* Release hardware lock. */
        R_BSP_HardwareUnlock(&ssp_feature);
        /* Return error if received irq is invalid. */
        SSP_ERROR_LOG(SSP_ERR_IRQ_BSP_DISABLED, &g_module_name[0], g_agt_input_capture_version);
        return error;
    }

    /** Set measurement and overflow event interrupt priority and vector info. */
    R_SSP_VectorInfoGet(p_ctrl->overflow_irq, &p_vector_info);
    NVIC_SetPriority(p_ctrl->overflow_irq, p_cfg->overflow_irq_ipl);
    *(p_vector_info->pp_ctrl) = p_ctrl;

    /** Get fmi compare match event information for AGT. */
    error = g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_AGT_COMPARE_A, &event_info);
    p_ctrl->capture_irq = event_info.irq;
    if (SSP_SUCCESS != error)
    {
        /* Release hardware lock. */
        R_BSP_HardwareUnlock(&ssp_feature);
        /* Return error if received irq is invalid. */
        SSP_ERROR_LOG(SSP_ERR_IRQ_BSP_DISABLED, &g_module_name[0], g_agt_input_capture_version);
        return error;
    }

    /** Set compare match event interrupt priority and vector info. */
    R_SSP_VectorInfoGet(p_ctrl->capture_irq, &p_vector_info);
    NVIC_SetPriority(p_ctrl->capture_irq, p_cfg->capture_irq_ipl);
    *(p_vector_info->pp_ctrl) = p_ctrl;

    /** Initialize control block. */
    p_ctrl->channel                 = p_cfg->channel;
    p_ctrl->mode                    = p_cfg->mode;
    p_ctrl->repetition              = p_cfg->repetition;
    p_ctrl->overflows_current       = 0U;
    p_ctrl->p_callback              = p_cfg->p_callback;
    p_ctrl->p_context               = p_cfg->p_context;
    p_ctrl->pulse_period_first_edge = 1U;

    /** Perform hardware initializations based on configuration. */
    agt_input_capture_hardware_initialize(p_ctrl, p_cfg, &ssp_feature);

    /** Mark channel as open, by initializing it to R_AIC ASCII equivalent. */
    p_ctrl->open = R_AIC_OPEN;

    return SSP_SUCCESS;
} /* End of function R_AGT_InputCaptureOpen */

/*******************************************************************************************************************//**
 * @brief  Close a AGT Timer Channel for Input Capture. Implements input_capture_api_t::close.
 *
 * Clears Timer settings, disables interrupts, and clears internal driver data.
 *
 * @retval SSP_SUCCESS          Successful close.
 * @retval SSP_ERR_ASSERTION    The parameter p_ctrl is NULL.
 * @retval SSP_ERR_NOT_OPEN     The channel is not opened.
 **********************************************************************************************************************/
ssp_err_t R_AGT_InputCaptureClose (input_capture_ctrl_t * const p_api_ctrl)
{
    agt_input_capture_instance_ctrl_t * p_ctrl = (agt_input_capture_instance_ctrl_t *) p_api_ctrl;
    ssp_vector_info_t * p_vector_info     = NULL;
    ssp_vector_info_t * p_vector_info_cmp = NULL;

#if AGT_INPUT_CAPTURE_CFG_PARAM_CHECKING_ENABLE
    /** Make sure channel is open.  If not open, return. */
    SSP_ASSERT(NULL != p_ctrl);
    AGT_ERROR_RETURN(R_AIC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_AGT0_Type * p_agt_reg = (R_AGT0_Type * ) p_ctrl->p_reg;

    /* Gets the vector information */
    R_SSP_VectorInfoGet(p_ctrl->overflow_irq, &p_vector_info);
    R_SSP_VectorInfoGet(p_ctrl->capture_irq, &p_vector_info_cmp);

    /** Cleanup. Disable interrupts and stop measurements. */
    NVIC_DisableIRQ(p_ctrl->overflow_irq);
    R_BSP_IrqStatusClear(p_ctrl->overflow_irq);
    NVIC_ClearPendingIRQ(p_ctrl->overflow_irq);

    NVIC_DisableIRQ(p_ctrl->capture_irq);
    R_BSP_IrqStatusClear(p_ctrl->capture_irq);
    NVIC_ClearPendingIRQ(p_ctrl->capture_irq);

    /* disables the compare match used by pulse counter */
    HW_AGT_CompareMatchDisable(p_agt_reg, AGT_AGTCMA);

    /* Stops the counter */
    HW_AGT_CounterStartStop(p_agt_reg, AGT_STOP);

    /* Force Stops and reset counter */
    HW_AGT_CounterForceStop (p_agt_reg);

    /** Unlock channel */
    ssp_feature_t ssp_feature  = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_ctrl->channel;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_AGT;
    R_BSP_HardwareUnlock(&ssp_feature);

    /** Clear stored internal driver data */
    p_ctrl->p_callback = NULL;
    p_ctrl->p_context  = NULL;
    (*p_vector_info->pp_ctrl) = NULL;
    (*p_vector_info_cmp->pp_ctrl) = NULL;

    p_ctrl->open              = 0U;

    return SSP_SUCCESS;
} /* End of function R_AGT_InputCaptureClose */

/*******************************************************************************************************************//**
 * @brief      Gets driver version based on compile time macros. Implements input_capture_api_t::versionGet.
 *
 * @retval SSP_SUCCESS          Success.
 * @retval SSP_ERR_ASSERTION    The parameter p_version is NULL.
 **********************************************************************************************************************/
ssp_err_t R_AGT_InputCaptureVersionGet (ssp_version_t * const p_version)
{
#if AGT_INPUT_CAPTURE_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid */
    SSP_ASSERT(NULL != p_version);
#endif

    p_version->version_id = g_agt_input_capture_version.version_id;

    return SSP_SUCCESS;
} /* End of function R_AGT_InputCaptureVersionGet */

/*******************************************************************************************************************//**
 * @brief  Stops the Input capture and disables its interrupts for specified channel at NVIC. Implements
 * input_capture_api_t::disable.
 *
 * @retval SSP_SUCCESS              Interrupt disabled successfully.
 * @retval SSP_ERR_ASSERTION        The p_ctrl parameter was null.
 * @retval SSP_ERR_NOT_OPEN         The channel is not opened.
 **********************************************************************************************************************/
ssp_err_t R_AGT_InputCaptureDisable (input_capture_ctrl_t const * const p_api_ctrl)
{
    agt_input_capture_instance_ctrl_t * p_ctrl = (agt_input_capture_instance_ctrl_t *) p_api_ctrl;
#if AGT_INPUT_CAPTURE_CFG_PARAM_CHECKING_ENABLE
    /** Make sure handle is valid. */
    SSP_ASSERT(NULL != p_ctrl);
    AGT_ERROR_RETURN(R_AIC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif
    R_AGT0_Type * p_agt_reg = (R_AGT0_Type * ) p_ctrl->p_reg;

    /* Stops the counter */
    HW_AGT_CounterStartStop(p_agt_reg, AGT_STOP);

    /** Disable interrupts */
    NVIC_DisableIRQ(p_ctrl->capture_irq);
    NVIC_DisableIRQ(p_ctrl->overflow_irq);

    return SSP_SUCCESS;
} /* End of function R_AGT_InputCaptureIrqDisable */

/*******************************************************************************************************************//**
 * @brief  Enables its interrupts for specified channel at NVIC, and starts the Input capture. Implements
 * input_capture_api_t::enable.
 *
 * @retval SSP_SUCCESS              Interrupt enabled successfully.
 * @retval SSP_ERR_ASSERTION        The p_ctrl parameter was null.
 * @retval SSP_ERR_NOT_OPEN         The channel is not opened.
  **********************************************************************************************************************/
ssp_err_t R_AGT_InputCaptureEnable (input_capture_ctrl_t const * const p_api_ctrl)
{
    agt_input_capture_instance_ctrl_t * p_ctrl = (agt_input_capture_instance_ctrl_t *) p_api_ctrl;
#if AGT_INPUT_CAPTURE_CFG_PARAM_CHECKING_ENABLE
    /** Make sure handle is valid. */
    SSP_ASSERT(NULL != p_ctrl);
    AGT_ERROR_RETURN(R_AIC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif
    R_AGT0_Type * p_agt_reg = (R_AGT0_Type *) p_ctrl->p_reg;

    /* Sets the count register with initial value, for next capture. */
    HW_AGT_CounterSet(p_agt_reg, AGT_MAX_CLOCK_COUNTS_16);

    /* Sets the first edge for next period capture. */
    if (INPUT_CAPTURE_MODE_PERIOD == p_ctrl->mode)
    {
        p_ctrl->pulse_period_first_edge = 1U;
    }

    /** Enabling the measurement overflow and compare match interrupt. */
    R_BSP_IrqStatusClear(p_ctrl->overflow_irq);
    NVIC_ClearPendingIRQ(p_ctrl->overflow_irq);
    NVIC_EnableIRQ(p_ctrl->overflow_irq);

    R_BSP_IrqStatusClear(p_ctrl->capture_irq);
    NVIC_ClearPendingIRQ(p_ctrl->capture_irq);
    NVIC_EnableIRQ(p_ctrl->capture_irq);

    /* Starts the counter */
    HW_AGT_CounterStartStop(p_agt_reg, AGT_START);

    return SSP_SUCCESS;
} /* End of function R_AGT_InputCaptureIrqEnable */

/*******************************************************************************************************************//**
 * @brief  Gets status into provided p_info pointer. Implements input_capture_api_t::infoGet.
 *
 * @retval SSP_SUCCESS              Success.
 * @retval SSP_ERR_ASSERTION        The p_ctrl parameter was null.
 * @retval SSP_ERR_NOT_OPEN         The channel is not opened.
 **********************************************************************************************************************/
ssp_err_t R_AGT_InputCaptureInfoGet (input_capture_ctrl_t const * const p_api_ctrl,
                                     input_capture_info_t       * const p_info)
{
    agt_input_capture_instance_ctrl_t * p_ctrl = (agt_input_capture_instance_ctrl_t *) p_api_ctrl;
#if AGT_INPUT_CAPTURE_CFG_PARAM_CHECKING_ENABLE
    /** Make sure parameters are valid. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_info);
    AGT_ERROR_RETURN(R_AIC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** Gets the input capture status. */
    R_AGT0_Type * p_agt_reg = (R_AGT0_Type *) p_ctrl->p_reg;
    bool status = HW_AGT_CountStatusGet(p_agt_reg);

    if (true == status)
    {
        p_info->status = INPUT_CAPTURE_STATUS_CAPTURING;
    }
    else
    {
        p_info->status = INPUT_CAPTURE_STATUS_IDLE;
    }

    p_info->variant = INPUT_CAPTURE_VARIANT_16_BIT;

    return SSP_SUCCESS;
} /* End of function R_AGT_InputCaptureStatusGet */

/*******************************************************************************************************************//**
 * @brief  Update the last captured value and overflow count, in provided p_capture pointer.
 * Implements input_capture_api_t::lastCaptureGet.
 *
 * @retval SSP_SUCCESS           Period value written successfully.
 * @retval SSP_ERR_ASSERTION     The p_ctrl or p_value parameter was null.
 * @retval SSP_ERR_NOT_OPEN      The channel is not opened.
 **********************************************************************************************************************/
ssp_err_t R_AGT_InputCaptureLastCaptureGet (input_capture_ctrl_t const * const p_api_ctrl,
                                             input_capture_capture_t    * const p_capture)
{
    agt_input_capture_instance_ctrl_t * p_ctrl = (agt_input_capture_instance_ctrl_t *) p_api_ctrl;
#if AGT_INPUT_CAPTURE_CFG_PARAM_CHECKING_ENABLE
    /** Validate the input parameter. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_capture);
    AGT_ERROR_RETURN(R_AIC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** Gets the captured value */
    /* For pulse count mode get the value from hardware register, as it does not receive callback for each
     * count, hence the overflow callback variable would not be an updated one. */
    if (INPUT_CAPTURE_MODE_PULSE_COUNT == p_ctrl->mode)
    {
        uint16_t count = 0U;
        R_AGT0_Type *  p_agt_reg = (R_AGT0_Type * ) p_ctrl->p_reg;
        count = HW_AGT_CounterGet(p_agt_reg);
        p_capture->counter = AGT_MAX_CLOCK_COUNTS_16 - count;
    }
    else
    {
        p_capture->counter = p_ctrl->capture_count;
    }

    p_capture->overflows = p_ctrl->overflows_current;

    return SSP_SUCCESS;
} /* End of function R_AGT_InputCaptureLastCaptureGet */

/*******************************************************************************************************************//**
 * @} (end addtogroup AGT_INPUT_CAPTURE)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Performs hardware initialization of the AGT for Input Capture.
 * @pre    All parameter checking should be done prior to calling this function.
 *
 * @param[in]  p_cfg  Pointer to configuration struct, which points to extension struct.
 * @param[in]  p_ctrl Pointer to control struct
 * @param[in]  p_feature Features structure
 **********************************************************************************************************************/
static void agt_input_capture_hardware_initialize (agt_input_capture_instance_ctrl_t      * const p_ctrl,
                                                   input_capture_cfg_t const * const p_cfg,
                                                   ssp_feature_t       const * const p_feature)
{
    agt_input_capture_extend_t * p_extend = (agt_input_capture_extend_t *) p_cfg->p_extend;

    /** Power on AGT before setting any hardware registers. Make sure the counter is stopped before setting registers.
    **/
    R_AGT0_Type * p_agt_reg = (R_AGT0_Type *) p_ctrl->p_reg;

    R_BSP_ModuleStart(p_feature);

    /** Stops the counter*/
    HW_AGT_CounterStartStop(p_agt_reg, AGT_STOP);

    /** Set all necessary hardware register. */
    /* Set the count source, clock divider, signal filter, and pin select. */
    HW_AGT_CountSourceSet(p_agt_reg, p_extend->count_source);
    HW_AGT_ClockDivideSet(p_agt_reg, p_extend->clock_divider);
    HW_AGT_InputFilterSet(p_agt_reg, p_extend->signal_filter);
    HW_AGT_PinSelect(p_agt_reg, p_extend->pin_select);

    /* Sets the capture counter with initial value, and clears its event flags. */
    HW_AGT_CounterSet(p_agt_reg, AGT_MAX_CLOCK_COUNTS_16);
    HW_AGT_CounterFlagsClear(p_agt_reg, AGT_INPUT_CAPTURE_ACTIVE_EDGE_FLAG);
    HW_AGT_CounterFlagsClear(p_agt_reg, AGT_INPUT_CAPTURE_UNDERFLOW_FLAG);

    /* Set the input capture mode and edge. */
    if (INPUT_CAPTURE_MODE_PULSE_WIDTH == p_cfg->mode)
    {
        HW_AGT_ModeSet(p_agt_reg, AGT_INPUT_CAPTURE_MODE_PULSE_WIDTH);
        if (INPUT_CAPTURE_SIGNAL_EDGE_RISING == p_cfg->edge)
        {
            HW_AGT_IOTEdgeSelectSet(p_agt_reg, 1U);
        }
        else
        {
            HW_AGT_IOTEdgeSelectSet(p_agt_reg, 0U);
        }
    }
    else if (INPUT_CAPTURE_MODE_PERIOD == p_cfg->mode)
    {
        HW_AGT_ModeSet(p_agt_reg, AGT_INPUT_CAPTURE_MODE_PERIOD);
        HW_AGT_IOTEdgeSelectSet(p_agt_reg, p_cfg->edge);
    }
    else if (INPUT_CAPTURE_MODE_PULSE_COUNT == p_cfg->mode)
    {
        uint16_t compare_reg_value = (uint16_t) (AGT_MAX_CLOCK_COUNTS_16 - (uint16_t)(p_extend->pulse_count_value - 1));
        HW_AGT_ModeSet(p_agt_reg, AGT_INPUT_CAPTURE_MODE_PULSE_COUNT);
        HW_AGT_IOTEdgeSelectSet(p_agt_reg, p_cfg->edge);

        /* For event counter set the compare match register with expected value, and enable compare match. */
        HW_AGT_CompareMatchSet(p_agt_reg, AGT_AGTCMA, compare_reg_value);
        HW_AGT_CompareMatchEnable(p_agt_reg, AGT_AGTCMA);

        /* Set the edge polarity for pulse count mode. */
        HW_AGT_CountEdgePolaritySet(p_agt_reg, p_extend->count_edge);
    }

    if (true == p_cfg->autostart)
    {
        /** Starts the counter*/
        HW_AGT_CounterStartStop(p_agt_reg, AGT_START);

        /** Enabling the overflow and capture registers. */
        R_BSP_IrqStatusClear(p_ctrl->overflow_irq);
        NVIC_ClearPendingIRQ(p_ctrl->overflow_irq);
        NVIC_EnableIRQ(p_ctrl->overflow_irq);

        R_BSP_IrqStatusClear(p_ctrl->capture_irq);
        NVIC_ClearPendingIRQ(p_ctrl->capture_irq);
        NVIC_EnableIRQ(p_ctrl->capture_irq);
    }
} /* End of function agt_input_capture_hardware_initialize */

/*******************************************************************************************************************//**
 * @brief   Counter compare match interrupt procedure.
 **********************************************************************************************************************/
void agt_input_capture_compare_isr (void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    IRQn_Type irq = R_SSP_CurrentIrqGet();
    R_SSP_VectorInfoGet(irq, &p_vector_info);
    agt_input_capture_instance_ctrl_t * p_ctrl = (agt_input_capture_instance_ctrl_t *) *(p_vector_info->pp_ctrl);
    uint8_t channel = p_ctrl->channel;
    R_AGT0_Type *  p_agt_reg = (R_AGT0_Type * ) p_ctrl->p_reg;
    input_capture_event_t events = INPUT_CAPTURE_EVENT_MEASUREMENT;
    uint16_t count = 0U;

    /** Clear pending IRQ to make sure it doesn't fire again after exiting */
    R_BSP_IrqStatusClear(irq);

    /* Check if the interrupt was triggered for pulse count mode, then turn off future interrupts. */
    if (INPUT_CAPTURE_MODE_PULSE_COUNT == p_ctrl->mode)
    {
        NVIC_ClearPendingIRQ(p_ctrl->capture_irq);
        NVIC_DisableIRQ(p_ctrl->capture_irq);
    }

    /* Read the event flag. */
    p_ctrl->flags = HW_AGT_CounterFlagsGet(p_agt_reg);

    /* Check if the interrupt was triggered for a compare match event. */
    if (0U != (p_ctrl->flags & (uint8_t)AGT_INPUT_CAPTURE_COMPARE_A_FLAG))
    {
        HW_AGT_CounterFlagsClear(p_agt_reg, AGT_INPUT_CAPTURE_COMPARE_A_FLAG);

        events = INPUT_CAPTURE_EVENT_MEASUREMENT;

        /* Get captured count value. */
        count = HW_AGT_CounterGet(p_agt_reg);
        p_ctrl->capture_count = AGT_MAX_CLOCK_COUNTS_16 - count;
    }

    if (NULL != p_ctrl->p_callback)
    {
        /** Set data to identify callback to user, then call user callback. */
        input_capture_callback_args_t callback_args;
        callback_args.channel   = channel;
        callback_args.event     = events;
        callback_args.counter   = p_ctrl->capture_count;
        callback_args.overflows = p_ctrl->overflows_current;
        callback_args.p_context = p_ctrl->p_context;
        p_ctrl->p_callback(&callback_args);
    }

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
} /* End of function agt_input_capture_compare_isr */

/*******************************************************************************************************************//**
 * @brief   Counter measurement and overflow interrupt procedure.
 **********************************************************************************************************************/
void agt_input_capture_counter_overflow_isr (void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    IRQn_Type irq = R_SSP_CurrentIrqGet();
    R_SSP_VectorInfoGet(irq, &p_vector_info);
    agt_input_capture_instance_ctrl_t * p_ctrl = (agt_input_capture_instance_ctrl_t *) *(p_vector_info->pp_ctrl);
    uint8_t channel = p_ctrl->channel;
    R_AGT0_Type *  p_agt_reg = (R_AGT0_Type * ) p_ctrl->p_reg;
    input_capture_event_t events = INPUT_CAPTURE_EVENT_MEASUREMENT;
    uint16_t count = 0U;

    /** Clear pending IRQ to make sure it doesn't fire again after exiting */
    R_BSP_IrqStatusClear(irq);

    /** If we are capturing a single pulse (i.e. one-shot mode), then turn off future interrupts. */
    if (INPUT_CAPTURE_REPETITION_ONE_SHOT == p_ctrl->repetition)
    {
        if ((INPUT_CAPTURE_MODE_PULSE_WIDTH == p_ctrl->mode) ||
                ((INPUT_CAPTURE_MODE_PERIOD == p_ctrl->mode) && (0U == p_ctrl->pulse_period_first_edge)))
        {
            NVIC_DisableIRQ(irq);
        }
    }

    /** Read the event flag. */
    p_ctrl->flags = HW_AGT_CounterFlagsGet(p_agt_reg);

    /* Check if the interrupt was triggered for an measurement event. */
    if (0U != (p_ctrl->flags & (uint8_t)AGT_INPUT_CAPTURE_ACTIVE_EDGE_FLAG))
    {
        HW_AGT_CounterFlagsClear(p_agt_reg, AGT_INPUT_CAPTURE_ACTIVE_EDGE_FLAG);

        events = INPUT_CAPTURE_EVENT_MEASUREMENT;

        /* Get captured count value. */
        count = HW_AGT_CounterGet(p_agt_reg);

        /* When the first edge is received for period mode, clear the first edge flag and return, as we need
         * another edge to measure period. */
        if ((INPUT_CAPTURE_MODE_PERIOD == p_ctrl->mode) && (1U == p_ctrl->pulse_period_first_edge))
        {
            p_ctrl->pulse_period_first_edge = 0U;
            p_ctrl->overflows_current = 0U;
            return;
        }

        /* Get captured count value. After reading the pulse width count, set the count register to initial
         * value for next measurement, don't have set count register for period mode, as it will be reseted by
         * hardware */
        if (INPUT_CAPTURE_MODE_PULSE_WIDTH == p_ctrl->mode)
        {
            p_ctrl->capture_count = AGT_MAX_CLOCK_COUNTS_16 - count;
            HW_AGT_CounterSet(p_agt_reg, AGT_MAX_CLOCK_COUNTS_16);
        }
        else if (INPUT_CAPTURE_MODE_PERIOD == p_ctrl->mode)
        {
            p_ctrl->capture_count = (AGT_MAX_CLOCK_COUNTS_16 - count) + 1;
        }
    }
    /* Check if the interrupt was triggered for an overflow event. */
    else if (0U != (p_ctrl->flags & (uint8_t)AGT_INPUT_CAPTURE_UNDERFLOW_FLAG))
    {
        HW_AGT_CounterFlagsClear(p_agt_reg, AGT_INPUT_CAPTURE_UNDERFLOW_FLAG);

        events = INPUT_CAPTURE_EVENT_OVERFLOW;

        /* Increment the current number of overflows. */
        p_ctrl->overflows_current++;
    }

    if (NULL != p_ctrl->p_callback)
    {
        /** Set data to identify callback to user, then call user callback. */
        input_capture_callback_args_t callback_args;
        callback_args.channel   = channel;
        callback_args.event     = events;
        callback_args.counter   = p_ctrl->capture_count;
        callback_args.overflows = p_ctrl->overflows_current;
        callback_args.p_context = p_ctrl->p_context;
        p_ctrl->p_callback(&callback_args);
    }

    if (0U != (p_ctrl->flags & (uint8_t)AGT_INPUT_CAPTURE_ACTIVE_EDGE_FLAG))
    {
        p_ctrl->overflows_current = 0U;
    }

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
} /* End of function agt_input_capture_counter_overflow_isr */

/*******************************************************************************************************************//**
 * @brief  Checks whether count source is operational or not.
 * @pre    All parameter checking should be done prior to calling this function.
 *
 * @param[in]  p_cfg  Pointer to configuration struct, which points to extension struct.
 **********************************************************************************************************************/
static ssp_err_t agt_input_capture_check_count_source(input_capture_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_ERR_CLOCK_ACTIVE;
    const cgc_api_t * pcgc = &g_cgc_on_cgc;

    /** If the source clock is SUBCLOCK or LOCO, return the run state of the source clock */
    if (AGT_CLOCK_INPUT_CAPTURE_FSUB == ((agt_input_capture_extend_t *)(p_cfg->p_extend))->count_source)
    {
    	err = pcgc->clockCheck(CGC_CLOCK_SUBCLOCK);
    }
    else if (AGT_INPUT_CAPTURE_CLOCK_LOCO == ((agt_input_capture_extend_t *)(p_cfg->p_extend))->count_source)
    {
    	err = pcgc->clockCheck(CGC_CLOCK_LOCO);
    }

    return err;
} /* End of function agt_input_capture_check_count_source */
