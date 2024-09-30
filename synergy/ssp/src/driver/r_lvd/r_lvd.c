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
 * File Name    : r_lvd.c
 * Description  : LVD API implementation
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_lvd.h"
#include "r_lvd_api.h"
#include "r_lvd_private_api.h"
#include "hw/common/hw_lvd_common.h"
#include <string.h>

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#define NUMBER_OF_LVD_MONITORS  2

/** Macro for error logger. */
#ifndef LVD_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define LVD_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_lvd_version)
#endif

#define LVD_FIRST_MONITOR_NUMBER    (1U)

#define LVD_OPENED 0x4C5644U
#define LVD_CLOSED 0U

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

static void detection_response_configure (lvd_instance_ctrl_t * p_ctrl, lvd_cfg_t const * const p_cfg, IRQn_Type irq);
#if (0 != LVD_CFG_PARAM_CHECKING_ENABLE)
static ssp_err_t detection_response_check(lvd_cfg_t const * const p_cfg);
static ssp_err_t negation_delay_check(lvd_cfg_t const * const p_cfg);
#endif
static void interrupt_type_configure(lvd_instance_ctrl_t * p_ctrl, lvd_cfg_t const * const p_cfg);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
/** MCU specific LVD features. */
bsp_feature_lvd_t g_lvd_feature = {0U};

/** Stored context for NMI handler. */
static lvd_instance_ctrl_t * gp_ctrls[NUMBER_OF_LVD_MONITORS] = {NULL};

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "r_lvd";
#endif

#if defined(__GNUC__)
/* This structure is affected by warnings from the GCC compiler bug gcc.gnu.org/bugzilla/show_bug.cgi?id=60784
 * This pragma suppresses the warnings in this structure only, and will be removed when the SSP compiler is updated to
 * v5.3.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_lvd_version =
{
    .api_version_minor  = LVD_API_VERSION_MINOR,
    .api_version_major  = LVD_API_VERSION_MAJOR,
    .code_version_major = LVD_CODE_VERSION_MAJOR,
    .code_version_minor = LVD_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/

/** Instance of low voltage detection peripheral driver interface */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const lvd_api_t g_lvd_on_lvd =
{
    .open                               = R_LVD_Open,
    .statusGet                          = R_LVD_StatusGet,
    .statusClear                        = R_LVD_StatusClear,
    .close                              = R_LVD_Close,
    .versionGet                         = R_LVD_VersionGet,
};

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

static void lvd_nmi_handler (bsp_grp_irq_t irq);
#if (0 != LVD_CFG_PARAM_CHECKING_ENABLE)
static ssp_err_t lvd_open_parameter_check (lvd_instance_ctrl_t * p_ctrl, lvd_cfg_t const * const p_cfg);
#endif

/*******************************************************************************************************************//**
 * @addtogroup LVD
 * @{
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Initializes a low voltage detection driver according to the passed in configuration structure.
 * Enables an LVD peripheral based on configuration structure.
 * @par    Implements
 * - lvd_api_t::open.
 *
 * @note Digital filter is not to be used with standby modes
 *
 * @retval      SSP_SUCCESS             Successful
 * @retval      SSP_ERR_ASSERTION       One or more pointers passed as function argument point to NULL or the Requested
 *                                      configuration, detection, voltage, monitor number or sample clock configuration
 *                                      of a voltage monitor is invalid.
 * @retval      SSP_ERR_IN_USE          Unable to acquire the hardware lock.
 * @retval      SSP_ERR_INVALID_MODE    MOCO must be running for LVD_NEGATION_DELAY_FROM_RESET negation delay option
 * @return                              See @ref Common_Error_Codes or functions called by this function for other possible
 *                                      return codes. This function calls:
 *                                        * fmi_api_t::productFeatureGet
 *                                        * fmi_api_t::eventInfoGet
 **********************************************************************************************************************/
ssp_err_t R_LVD_Open (lvd_ctrl_t * const p_api_ctrl, lvd_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;
    lvd_instance_ctrl_t * p_ctrl = (lvd_instance_ctrl_t *) p_api_ctrl;

    R_BSP_FeatureLvdGet(&g_lvd_feature);

#if (0 != LVD_CFG_PARAM_CHECKING_ENABLE)
    err = lvd_open_parameter_check(p_ctrl, p_cfg);
    if(SSP_SUCCESS != err)
    {
        return err;
    }
#endif

    const uint32_t monitor_number = p_cfg->monitor_number;

    /* member 'channel' of the ssp_feature structure is a bit field of size 16 bit. The following variable
     * is used as the RValue for assignment to that member. uint16_t type is used to avoid LDRA violation 93 S */
    const uint16_t monitor_register_index = (uint16_t)(monitor_number - LVD_FIRST_MONITOR_NUMBER);

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_LVD;
    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    LVD_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = (R_SYSTEM_Type *) info.ptr;

    ssp_vector_info_t * p_vector_info;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    uint32_t signal_number = (uint32_t) SSP_SIGNAL_LVD_LVD1 + monitor_register_index;
    ssp_signal_t signal = (ssp_signal_t) signal_number;
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, signal, &event_info);

#if (0 != LVD_CFG_PARAM_CHECKING_ENABLE)
    if((LVD_RESPONSE_NMI        == p_cfg->detection_response)   ||
       (LVD_RESPONSE_INTERRUPT  == p_cfg->detection_response)
       )
    {
        SSP_ASSERT(SSP_INVALID_VECTOR != event_info.irq);
    }
#endif

    if (SSP_INVALID_VECTOR != event_info.irq)
    {
        R_SSP_VectorInfoGet(event_info.irq, &p_vector_info);
        NVIC_SetPriority(event_info.irq, p_cfg->monitor_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }

    p_ctrl->monitor_number = monitor_number;

    /** Verify channel is not already used */
    ssp_feature.channel = monitor_register_index;
    err = R_BSP_HardwareLock(&ssp_feature);
    LVD_ERROR_RETURN(SSP_SUCCESS == err, SSP_ERR_IN_USE);

    /* No early or error returns allowed past this point */
    gp_ctrls[monitor_register_index] = p_ctrl;

    HW_LVD_RegisterUnLock();

    /** Select the detection voltage by setting the LVDLVLR.LVDnLVL[m:0] bits. */
    HW_LVD_VoltageDetectionLevelSet(p_ctrl->p_reg, p_cfg);

    /** Enable voltage detection LVCMPCR.LVDnE = 1 */
    HW_LVD_LVCMPCR_LVDnE_Set(p_ctrl->p_reg, monitor_number);

    /** Configure the digital filter. */
    HW_LVD_DigitalFilterConfigure(p_ctrl->p_reg, p_cfg);

    /** Configure LVD monitor detection response. */
    detection_response_configure(p_ctrl, p_cfg, event_info.irq);

    /** Enable output of the results of comparison by voltage monitor LVDnCR0.CMPE = 1 */
    HW_LVD_LVDnCR0_CMPE_Set(p_ctrl->p_reg, monitor_number);

    HW_LVD_RegisterLock();

    /** Mark driver as opened by initializing it to "LVD" in its ASCII equivalent. */
    p_ctrl->opened = LVD_OPENED;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Disables the LVD peripheral. Closes the driver instance.
 * @par    Implements
 * - lvd_api_t::close.
 *
 * @retval      SSP_SUCCESS         Successful
 * @retval      SSP_ERR_ASSERTION   Pointers passed as function argument is NULL or the monitor number is invalid.
 * @retval      SSP_ERR_NOT_OPEN    Driver is not open.
 * @return                          See @ref Common_Error_Codes or functions called by this function for other possible
 *                                  return codes. This function calls:
 *                                      * fmi_api_t::eventInfoGet
 *
 **********************************************************************************************************************/
ssp_err_t R_LVD_Close (lvd_ctrl_t * const p_api_ctrl)
{
    lvd_instance_ctrl_t * p_ctrl = (lvd_instance_ctrl_t *) p_api_ctrl;

#if (0 != LVD_CFG_PARAM_CHECKING_ENABLE)
    LVD_ERROR_RETURN((NULL != p_ctrl), SSP_ERR_ASSERTION);
    LVD_ERROR_RETURN((true == HW_LVD_MonitorNumberCheck(p_ctrl->monitor_number)), SSP_ERR_ASSERTION);
    LVD_ERROR_RETURN((LVD_OPENED == p_ctrl->opened), SSP_ERR_NOT_OPEN);
#endif

    const uint32_t monitor_number = p_ctrl->monitor_number;

    /* member 'channel' of the ssp_feature structure is a bit field of size 16 bit. The following variable
     * is used as the RValue for assignment to that member. uint16_t type is used to avoid LDRA violation 93 S */
    const uint16_t monitor_register_index = (uint16_t)(monitor_number - LVD_FIRST_MONITOR_NUMBER);

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_LVD;

    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    uint32_t signal_number = (uint32_t) SSP_SIGNAL_LVD_LVD1 + monitor_register_index;
    ssp_signal_t signal = (ssp_signal_t) signal_number;
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, signal, &event_info);
    if (SSP_INVALID_VECTOR != event_info.irq)
    {
        HW_LVD_IrqDisable(event_info.irq);
    }

    HW_LVD_RegisterUnLock();

    HW_LVD_NmiDisable(monitor_number);

    /** Disable voltage monitor comparison result output */
    HW_LVD_LVDnCR0_CMPE_Clear(p_ctrl->p_reg, monitor_number);

    p_ctrl->p_callback = NULL;
    p_ctrl->lvd_callback_args.p_context = NULL;
    p_ctrl->lvd_callback_args.status.crossing_detected =
                                                        LVD_THRESHOLD_CROSSING_NOT_DETECTED;
    p_ctrl->lvd_callback_args.status.current_state =
                                                        LVD_CURRENT_STATE_ABOVE_THRESHOLD;

    HW_LVD_DelayAfterComparisonOutputDisable(p_ctrl->p_reg, p_ctrl);

    /** Disable reset/interrupt event */
    HW_LVD_LVDnCR0_RIE_Clear(p_ctrl->p_reg, monitor_number);

    /** Clear low voltage detection flag LVDnSR.DET = 0 */
    HW_LVD_LVDnSR_DET_Clear(p_ctrl->p_reg, monitor_number);

    /** Disable digital filtering */
    HW_LVD_LVDnCR0_DFDIS_Set(p_ctrl->p_reg, monitor_number);

    /** Disable voltage monitor */
    HW_LVD_LVCMPCR_LVDnE_Clear(p_ctrl->p_reg, monitor_number);

    HW_LVD_RegisterLock();

    gp_ctrls[monitor_register_index] = NULL;
    p_ctrl->opened = LVD_CLOSED;

    ssp_feature.channel = monitor_register_index;
    R_BSP_HardwareUnlock(&ssp_feature);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Get the current state of the monitor, (threshold crossing detected, voltage currently within range)
 * @par    Implements
 * - lvd_api_t::statusGet.
 *
 * @retval          SSP_SUCCESS         Successful
 * @retval          SSP_ERR_ASSERTION   One or more pointers passed as function argument point to NULL, invalid LVD monitor number
 * @retval          SSP_ERR_NOT_OPEN    Driver is not open, status will not be valid
 *
 **********************************************************************************************************************/
ssp_err_t R_LVD_StatusGet (lvd_ctrl_t * const p_api_ctrl, lvd_status_t * p_lvd_status)
{
    lvd_instance_ctrl_t * p_ctrl = (lvd_instance_ctrl_t *) p_api_ctrl;

#if (0 != LVD_CFG_PARAM_CHECKING_ENABLE)
    LVD_ERROR_RETURN((NULL != p_ctrl), SSP_ERR_ASSERTION);
    LVD_ERROR_RETURN((NULL != p_lvd_status), SSP_ERR_ASSERTION);
    LVD_ERROR_RETURN((true == HW_LVD_MonitorNumberCheck(p_ctrl->monitor_number)), SSP_ERR_ASSERTION);
    LVD_ERROR_RETURN((LVD_OPENED == p_ctrl->opened), SSP_ERR_NOT_OPEN);
#endif

    /** Get the current state of the monitor */
    p_lvd_status->crossing_detected = (lvd_threshold_crossing_t)HW_LVD_LVDnSR_DET_Get(p_ctrl->p_reg, p_ctrl->monitor_number);
    p_lvd_status->current_state = (lvd_current_state_t)HW_LVD_LVDnSR_MON_Get(p_ctrl->p_reg, p_ctrl->monitor_number);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Clears the latched status of the monitor.
 * @par    Implements
 * - lvd_api_t::statusClear.
 *
 * @retval      SSP_SUCCESS         Successful
 * @retval      SSP_ERR_ASSERTION   Pointers passed as function argument point to NULL, invalid LVD monitor number
 * @retval      SSP_ERR_NOT_OPEN    Driver is not open, status will not be valid
 *
 **********************************************************************************************************************/
ssp_err_t R_LVD_StatusClear (lvd_ctrl_t * const p_api_ctrl)
{
    lvd_instance_ctrl_t * p_ctrl = (lvd_instance_ctrl_t *) p_api_ctrl;

#if (0 != LVD_CFG_PARAM_CHECKING_ENABLE)
    LVD_ERROR_RETURN((NULL != p_ctrl), SSP_ERR_ASSERTION);
    LVD_ERROR_RETURN((true == HW_LVD_MonitorNumberCheck(p_ctrl->monitor_number)), SSP_ERR_ASSERTION);
    LVD_ERROR_RETURN((LVD_OPENED == p_ctrl->opened), SSP_ERR_NOT_OPEN);
#endif

    HW_LVD_RegisterUnLock();

    /** Clears the latched status of the monitor */
    HW_LVD_LVDnSR_DET_Clear(p_ctrl->p_reg, p_ctrl->monitor_number);

    HW_LVD_RegisterLock();

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Returns the LVD driver version based on compile time macros.
 * @par    Implements
 * - lvd_api_t::versionGet.
 *
 * @retval          SSP_SUCCESS         Successful
 * @retval          SSP_ERR_ASSERTION   p_version was NULL
 *
 **********************************************************************************************************************/
ssp_err_t R_LVD_VersionGet (ssp_version_t * const p_version)
{
#if (0 != LVD_CFG_PARAM_CHECKING_ENABLE)
    LVD_ERROR_RETURN((NULL != p_version), SSP_ERR_ASSERTION);
#endif

    p_version->version_id = g_lvd_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup LVD)
 **********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @brief Common code needed for all LVD ISRs.
 * @param[in] p_ctrl            Pointer to the control block structure for the driver instance
 * @param[in] monitor_number    Identifier for the monitor which generated the interrupt
 *
 * @note Do not call this function directly. To be used internally by LVD driver.
 *
 **********************************************************************************************************************/
void lvd_common_isr_handler(lvd_instance_ctrl_t * p_ctrl, uint32_t monitor_number);
void lvd_common_isr_handler(lvd_instance_ctrl_t * p_ctrl, uint32_t monitor_number)
{
    /** If a callback is specified, call it */
    if (NULL != p_ctrl->p_callback)
    {
        p_ctrl->lvd_callback_args.status.crossing_detected =
                (lvd_threshold_crossing_t)HW_LVD_LVDnSR_DET_Get(p_ctrl->p_reg, monitor_number);

        p_ctrl->lvd_callback_args.status.current_state =
                (lvd_current_state_t)HW_LVD_LVDnSR_MON_Get(p_ctrl->p_reg, monitor_number);

        p_ctrl->lvd_callback_args.monitor_number = monitor_number;

        p_ctrl->p_callback(&(p_ctrl->lvd_callback_args));
    }
    HW_LVD_RegisterUnLock();
    HW_LVD_LVDnSR_DET_Clear(p_ctrl->p_reg, monitor_number);
    HW_LVD_RegisterLock();
}

/*******************************************************************************************************************//**
 * @brief ISR for LVD
 * @note Do not call this function directly. To be used internally by LVD driver.
 *
 **********************************************************************************************************************/
void lvd_lvd_isr (void);
void lvd_lvd_isr (void)
{
    SF_CONTEXT_SAVE        /** Save context if RTOS is used     */

    /** Clear the Interrupt Request */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    lvd_instance_ctrl_t * p_ctrl = (lvd_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    uint32_t monitor_number = p_ctrl->monitor_number;

    lvd_common_isr_handler(p_ctrl, monitor_number);

    SF_CONTEXT_RESTORE     /** Restore context if RTOS is used  */
}

/*******************************************************************************************************************//**
 * @brief NMI ISR handler for LVD 1 and 2
 * @param[in] irq         BSP group IRQ identifier
 *
 * @note Do not call this function directly. To be used internally by LVD driver.
 *
 **********************************************************************************************************************/
static void lvd_nmi_handler (bsp_grp_irq_t irq)
{
    SF_CONTEXT_SAVE        /** Save context if RTOS is used     */

    if(BSP_GRP_IRQ_LVD1 == irq)
    {
        const uint32_t monitor_number = 1U;
        lvd_common_isr_handler(gp_ctrls[0U], monitor_number);
    }

    if(BSP_GRP_IRQ_LVD2 == irq)
    {
        const uint32_t monitor_number = 2U;
        lvd_common_isr_handler(gp_ctrls[1U], monitor_number);
    }

    SF_CONTEXT_RESTORE     /** Restore context if RTOS is used  */
}

/*******************************************************************************************************************//**
 * @brief      Configure LVD monitor detection response. Internal function, do not use directly.
 * @param[in]   p_ctrl              Pointer to the control block structure for the driver instance
 * @param[in]   p_cfg               Pointer to the configuration structure for the driver instance
 * @param[in]   irq                 IRQ associated with the monitor
 *
 * @note Do not call this function directly. To be used internally by LVD driver.
 *
 **********************************************************************************************************************/
static void detection_response_configure (lvd_instance_ctrl_t * p_ctrl, lvd_cfg_t const * const p_cfg, IRQn_Type irq)
{
    if(LVD_RESPONSE_NONE != p_cfg->detection_response)
    {
        const uint32_t monitor_number = p_cfg->monitor_number;

        if((LVD_RESPONSE_INTERRUPT  == p_cfg->detection_response)   ||
           (LVD_RESPONSE_NMI        == p_cfg->detection_response)
           )
        {
            /* Choose interrupt generation LVD2CR0.RI */
            HW_LVD_LVDnCR0_RI_Set(p_ctrl->p_reg, monitor_number, LVD_DETECTION_RESPONSE_INTERRUPT);

            /* Store interrupt callback function address */
            p_ctrl->p_callback = p_cfg->p_callback;

            /* Store the user context */
            p_ctrl->lvd_callback_args.p_context = p_cfg->p_context;

            if(LVD_RESPONSE_NMI == p_cfg->detection_response)
            {
                HW_LVD_NmiEnable(monitor_number, lvd_nmi_handler);
            }
            else
            {
                /* Enable interrupt in NVIC */
                HW_LVD_IrqEnable(irq);
            }
        }
        else
        {
            /* Choose reset generation LVD2CR0.RI */
            HW_LVD_LVDnCR0_RI_Set(p_ctrl->p_reg, monitor_number, LVD_DETECTION_RESPONSE_RESET);
        }

        /* Select rising or falling voltage by setting the LVDnCR1.IDTSEL[1:0] bits. */
        HW_LVD_LVDnCR1_IDTSEL_Set(p_ctrl->p_reg, monitor_number, p_cfg->voltage_slope);

        /* Select the type of interrupt (maskable/NMI) by setting the LVDnCR1.IRQSEL bit. */
        interrupt_type_configure(p_ctrl, p_cfg);

        /* Clear low voltage detection flag LVDnSR.DET = 0 */
        HW_LVD_LVDnSR_DET_Clear(p_ctrl->p_reg, monitor_number);

        if(LVD_RESPONSE_RESET == p_cfg->detection_response)
        {
            /* Negation of reset follows reset or voltage in range LVDnCR0.RN */
            HW_LVD_LVDnCR0_RN_Set(p_ctrl->p_reg, monitor_number, ((lvd_extend_t*)(p_cfg->p_extend))->negation_delay);
        }

        /* Enable reset/interrupt generation LVDnCR0.RIE = 1 */
        HW_LVD_LVDnCR0_RIE_Set(p_ctrl->p_reg, monitor_number);
    }
}

#if (0 != LVD_CFG_PARAM_CHECKING_ENABLE)
/*******************************************************************************************************************//**
 * @brief  Verify the detection response. Internal function, do not use directly.
 *
 * @param[in]   p_cfg               Pointer to the configuration structure for the driver instance
 * @retval      SSP_SUCCESS         detection response configuration is valid
 * @retval      SSP_ERR_ASSERTION   detection response configuration is invalid
 *
 **********************************************************************************************************************/
static ssp_err_t detection_response_check(lvd_cfg_t const * const p_cfg)
{
    if(LVD_RESPONSE_RESET == p_cfg->detection_response)
    {
        LVD_ERROR_RETURN((SSP_SUCCESS == negation_delay_check(p_cfg)),
                          SSP_ERR_ASSERTION);
    }
    else if((LVD_RESPONSE_NMI        != p_cfg->detection_response)   &&
           (LVD_RESPONSE_INTERRUPT  != p_cfg->detection_response))
    {
        LVD_ERROR_RETURN(LVD_RESPONSE_NONE == p_cfg->detection_response, SSP_ERR_ASSERTION);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Verify the LVD signal negation delay. Internal function, do not use directly.
 *
 * @param[in]   p_cfg               Pointer to the configuration structure for the driver instance
 * @retval      SSP_SUCCESS         signal negation delay is valid
 * @retval      SSP_ERR_ASSERTION   signal negation delay is invalid
 *
 **********************************************************************************************************************/
static ssp_err_t negation_delay_check(lvd_cfg_t const * const p_cfg)
{
    LVD_ERROR_RETURN(((LVD_NEGATION_DELAY_FROM_VOLTAGE == ((lvd_extend_t*)(p_cfg->p_extend))->negation_delay)  ||
                      (LVD_NEGATION_DELAY_FROM_RESET   == ((lvd_extend_t*)(p_cfg->p_extend))->negation_delay)  ),
                      SSP_ERR_ASSERTION);

    return SSP_SUCCESS;
}
#endif

/*******************************************************************************************************************//**
 * @brief  Configure the interrupt response type. Internal function, do not use directly.
 *
 * @param[in]   p_ctrl              Pointer to the control block structure for the driver instance
 * @param[in]   p_cfg               Pointer to the configuration structure for the driver instance
 * @retval      none
 *
 **********************************************************************************************************************/
static void interrupt_type_configure(lvd_instance_ctrl_t * p_ctrl, lvd_cfg_t const * const p_cfg)
{
    if(LVD_RESPONSE_INTERRUPT == p_cfg->detection_response)
    {
        HW_LVD_LVDnCR1_IRQSEL_Set(p_ctrl->p_reg, p_cfg->monitor_number, LVD_IRQ_MASKING_MASKABLE);
    }
    else if(LVD_RESPONSE_NMI == p_cfg->detection_response)
    {
        HW_LVD_LVDnCR1_IRQSEL_Set(p_ctrl->p_reg, p_cfg->monitor_number, LVD_IRQ_MASKING_NON_MASKABLE);
    }
    else
    {
        /* Response is reset, nothing to do */
    }
}

#if (0 != LVD_CFG_PARAM_CHECKING_ENABLE)
/*******************************************************************************************************************//**
 * @brief  Helper function to do parameter checking for the Open function for LVD module
 *
 * @param[in]   p_ctrl                  Pointer to the control block structure for the driver instance
 * @param[in]   p_cfg                   Pointer to the configuration structure for the driver instance
 *
 * @retval      SSP_SUCCESS             If all parameter checks are success
 * @retval      SSP_ERR_ASSERTION       If any of the parameter checks fails
 * @retval      SSP_ERR_INVALID_MODE    If the attempted mode is invalid for this configuration
 **********************************************************************************************************************/
static ssp_err_t lvd_open_parameter_check (lvd_instance_ctrl_t * p_ctrl, lvd_cfg_t const * const p_cfg)
{
       LVD_ERROR_RETURN((NULL != p_cfg), SSP_ERR_ASSERTION);
       LVD_ERROR_RETURN((NULL != p_ctrl), SSP_ERR_ASSERTION);
       LVD_ERROR_RETURN((true == HW_LVD_MonitorNumberCheck(p_cfg->monitor_number)), SSP_ERR_ASSERTION);
       LVD_ERROR_RETURN((true == HW_LVD_VoltageThresholdCheck(p_cfg)), SSP_ERR_ASSERTION);
       LVD_ERROR_RETURN((NULL != p_cfg->p_extend), SSP_ERR_ASSERTION);
       LVD_ERROR_RETURN((SSP_SUCCESS == detection_response_check(p_cfg)), SSP_ERR_ASSERTION);
       LVD_ERROR_RETURN((true == HW_LVD_SampleClockSelectCheck(p_cfg)), SSP_ERR_ASSERTION);
       LVD_ERROR_RETURN((true == HW_LVD_ResetDelayClockCheck(p_cfg)), SSP_ERR_INVALID_MODE);
       return SSP_SUCCESS;
}
#endif

