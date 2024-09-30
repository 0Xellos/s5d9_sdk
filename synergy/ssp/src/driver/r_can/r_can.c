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
 * File Name    : r_can.c
 * Description  : SSP CAN Driver.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_can.h"
#include "r_can_cfg.h"
#include "r_cgc_api.h"
#include "r_cgc.h"
#include "r_can_private_api.h"
#include "./hw/hw_can_private.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** Macro for error logger. */
#ifndef CAN_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define CAN_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_can_version)
#endif

/** Non-zero value used to determine if the control block is open. */
#define CAN_OPEN    (0x5243414EU)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void can_irq_enable(can_instance_ctrl_t * p_ctrl, can_cfg_t const * const p_cfg, ssp_feature_t * p_feature);
static void can_irq_disable(can_instance_ctrl_t * p_ctrl);
static ssp_err_t can_open_parameters_check(can_instance_ctrl_t * const p_ctrl, can_cfg_t const * const p_cfg);
static ssp_err_t can_open_parameters_check_timing(can_cfg_t const * const p_cfg);
static ssp_err_t can_open_parameters_check_clock(can_cfg_t const * const p_cfg, bsp_feature_can_t can_feature);
static ssp_err_t can_write_parameters_check(can_ctrl_t * const p_ctrl, can_frame_t * const p_frame, uint32_t mailbox);
static ssp_err_t can_module_start(can_instance_ctrl_t * p_internal_ctrl, can_cfg_t const * const p_cfg, bsp_feature_can_t can_feature);
static void can_mailbox_configure(R_CAN0_Type * p_can_regs, can_cfg_t const * const p_cfg);
static ssp_err_t can_operate_mode(R_CAN0_Type * p_can_regs);
static ssp_err_t can_wake_and_init(can_instance_ctrl_t * p_internal_ctrl, can_bit_timing_cfg_t * const p_timing);
/***********************************************************************************************************************
 * ISR prototypes
 **********************************************************************************************************************/

static void can_error_interrupt(can_instance_ctrl_t * p_ctrl);
void can_error_isr(void);
static void can_receive_interrupt(can_instance_ctrl_t * p_ctrl);
void can_mailbox_rx_isr(void);
static void can_transmit_interrupt(can_instance_ctrl_t * p_ctrl);
void can_mailbox_tx_isr(void);

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
static const ssp_version_t g_can_version =
{
     .api_version_minor  = CAN_API_VERSION_MINOR,
     .api_version_major  = CAN_API_VERSION_MAJOR,
     .code_version_major = CAN_CODE_VERSION_MAJOR,
     .code_version_minor = CAN_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "can";
#endif

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/
/** CAN function pointers   */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const can_api_t g_can_on_can =
{
    .open       = R_CAN_Open,
    .close      = R_CAN_Close,
    .read       = R_CAN_Read,
    .write      = R_CAN_Write,
    .control    = R_CAN_Control,
    .infoGet    = R_CAN_InfoGet,
    .versionGet = R_CAN_VersionGet
};

/*******************************************************************************************************************//**
 * @addtogroup CAN
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/***************************************************************************************************************//**
 * @brief   Open and configure the CAN channel for operation. Implements can_api_t::open()
 * @retval SSP_SUCCESS                      Channel opened successfully
 * @retval SSP_ERR_INVALID_ARGUMENT         Invalid channel passed as argument.
 * @retval SSP_ERR_HW_LOCKED                Lock already owned by another user.
 * @retval SSP_ERR_CAN_MODE_SWITCH_FAILED   Channel failed to switch modes.
 * @retval SSP_ERR_CAN_INIT_FAILED          Channel failed to initialize.
 * @retval SSP_ERR_ASSERTION                Null pointer presented.
 *
 * @return See @ref Common_Error_Codes or functions called by this function for other possible return codes.
 *         This function calls:
 *                                  * fmi_api_t::productFeatureGet
 *                                  * fmi_api_t::eventInfoGet
 *                                  * cgc_api_t::clockCheck
 *****************************************************************************************************************/
ssp_err_t R_CAN_Open (can_ctrl_t * const p_ctrl, can_cfg_t const * const p_cfg)
{
    can_instance_ctrl_t * p_internal_ctrl = (can_instance_ctrl_t *) p_ctrl;
    ssp_err_t err;
    ssp_feature_t feature = { { (ssp_ip_t) 0U } };
    R_CAN0_Type * p_can_regs = NULL;

    bsp_feature_can_t can_feature = { 0U };
    R_BSP_FeatureCanGet(&can_feature);

    err = SSP_SUCCESS;

#if (CAN_CFG_PARAM_CHECKING_ENABLE)
    /** Check pointers for NULL values. */
    err = can_open_parameters_check(p_ctrl, p_cfg);
    CAN_ERROR_RETURN(SSP_SUCCESS == err, err);
    /** Check the bit timing settings are within range. */
    err = can_open_parameters_check_timing(p_cfg);
    CAN_ERROR_RETURN(SSP_SUCCESS == err, err);
    /** Check the hardware manual usage note requirements are met. */
    err = can_open_parameters_check_clock(p_cfg, can_feature);
    CAN_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif /* if    (CAN_CFG_PARAM_CHECKING_ENABLE) */

    /** Check for valid parameters. */
    /** Make sure the feature exists on this MCU. */
    feature.id = SSP_IP_CAN;
    feature.channel = (uint32_t)(p_cfg->channel & 0xFFFF);
    feature.unit = 0U;
    fmi_feature_info_t info = { 0U };
    err = g_fmi_on_fmi.productFeatureGet(&feature, &info);
    p_internal_ctrl->p_reg = info.ptr;
    p_can_regs = (R_CAN0_Type *) p_internal_ctrl->p_reg;

    /** Return if failed to get feature information. */
    CAN_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Try to get channel lock. */
    err = R_BSP_HardwareLock(&feature);
    /** Return if channel is already open so return error */
    CAN_ERROR_RETURN(SSP_SUCCESS == err, SSP_ERR_HW_LOCKED);

    /** Enter module start state. */
    R_BSP_ModuleStart(&feature);

    /** Disable interrupts while initializing */
    HW_CAN_ErrorInterruptDisable(p_can_regs);
    HW_CAN_SendReceiveInterruptsDisable(p_can_regs);

    /** Initialize and configure CAN module to run. */
    err = can_module_start(p_internal_ctrl, p_cfg, can_feature);

    /** Set channel, callback function, context, id mode, mailbox count, message mode, op mode and opened status. */
    if (SSP_SUCCESS == err)
    {
        /** If successful, Lookup and store IRQ numbers. Enable interrupts. */
        can_irq_enable(p_internal_ctrl, p_cfg, &feature);

        /** If successful, Mark the control block as open */
        p_internal_ctrl->open = CAN_OPEN;
    }
    else
    {
        /** If the device failed to initialize, disable interrupts, stop and unlock the hardware and
         * mark the control block as closed. */
        can_irq_disable(p_internal_ctrl);
        R_BSP_ModuleStop(&feature);
        R_BSP_HardwareUnlock(&feature);
        p_internal_ctrl->open = 0U;
    }

    /** Process errors before returning. */
    /** Log error or assertion. */
    CAN_ERROR_RETURN(SSP_SUCCESS == err, err);

    return err;
} /* End of function R_CAN_Open() */

/***************************************************************************************************************//**
 * @brief   Close the CAN channel. Implements can_api_t::close()
 * @retval SSP_SUCCESS          Channel closed successfully.
 * @retval SSP_ERR_NOT_OPEN     Control block not open.
 * @retval SSP_ERR_ASSERTION    Null pointer presented.
 *****************************************************************************************************************/
ssp_err_t R_CAN_Close (can_ctrl_t * const p_ctrl)
{
    can_instance_ctrl_t * p_internal_ctrl = (can_instance_ctrl_t *) p_ctrl;

#if    (CAN_CFG_PARAM_CHECKING_ENABLE)
    /** Check pointers for NULL values */
    SSP_ASSERT(p_ctrl);
    /** If channel is not open, return an error */
    CAN_ERROR_RETURN(p_internal_ctrl->open == CAN_OPEN, SSP_ERR_NOT_OPEN);
#endif /* if    (CAN_CFG_PARAM_CHECKING_ENABLE) */
    /** Mark the channel not open so other APIs cannot use it. */
    p_internal_ctrl->open = 0U;

    ssp_feature_t feature = { { (ssp_ip_t) 0U } };

    feature.id = SSP_IP_CAN;
    feature.unit = 0U;
    feature.channel = (uint32_t) (p_internal_ctrl->channel & 0xFFFF);

    /** Disable transmit, receive and error interrupts */
    can_irq_disable(p_internal_ctrl);

    /** Enable module stop for the CAN channel */
    R_BSP_ModuleStop(&feature);

    /** Unlock the CAN channel */
    R_BSP_HardwareUnlock(&feature);

    return SSP_SUCCESS;
} /* End of function R_CAN_Close() */

/***************************************************************************************************************//**
 * @brief  Read data from the CAN channel. Return up to eight bytes read from the channel mailbox.
 *         Implements can_api_t::read()
 * @retval SSP_SUCCESS                      Data successfully read.
 * @retval SSP_ERR_NOT_OPEN                 Control block not open.
 * @retval SSP_ERR_CAN_DATA_UNAVAILABLE     No data available.
 * @retval SSP_ERR_CAN_TRANSMIT_MAILBOX     Mailbox is not setup for receive.
 * @retval SSP_ERR_ASSERTION                Null pointer presented.
 *****************************************************************************************************************/
ssp_err_t R_CAN_Read (can_ctrl_t * const p_ctrl, uint32_t mailbox, can_frame_t * const p_frame)
{
    can_instance_ctrl_t * p_internal_ctrl = (can_instance_ctrl_t *) p_ctrl;
    R_CAN0_Type * p_can_regs;
#if    (CAN_CFG_PARAM_CHECKING_ENABLE)
    /** Check pointers for NULL values */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_frame);

    /** If channel is not open, return an error */
    CAN_ERROR_RETURN(p_internal_ctrl->open == CAN_OPEN, SSP_ERR_NOT_OPEN);

    /** Make sure mailbox is setup for receive. */
    p_can_regs = (R_CAN0_Type *) p_internal_ctrl->p_reg;
    CAN_ERROR_RETURN(CAN_MAILBOX_RECEIVE == HW_CAN_MailboxTypeGet(p_can_regs, mailbox), SSP_ERR_CAN_TRANSMIT_MAILBOX);
#else
    p_can_regs = (R_CAN0_Type *) p_internal_ctrl->p_reg;
#endif /* if    (CAN_CFG_PARAM_CHECKING_ENABLE) */

    /** Check for receive data */
    CAN_ERROR_RETURN(HW_CAN_ReceiveDataAvailable(p_can_regs, mailbox), SSP_ERR_CAN_DATA_UNAVAILABLE);

    /** Get frame data. */
    HW_CAN_ReceiveDataGet(p_can_regs, mailbox, p_internal_ctrl->id_mode, p_frame);

    /** Check for other mailboxes in an overrun state. */
    if((HW_CAN_MailboxMessageLost(p_internal_ctrl->p_reg)) && (CAN_MESSAGE_MODE_OVERRUN == p_internal_ctrl->message_mode))
    {
        NVIC_SetPendingIRQ(p_internal_ctrl->error_irq);
    }

    /** Check for other mailboxes with received messages pending. */
    if(HW_CAN_NewDataStatusFlag(p_internal_ctrl->p_reg))
    {
        NVIC_SetPendingIRQ(p_internal_ctrl->mailbox_rx_irq);
    }

    return SSP_SUCCESS;
} /* End of function R_CAN_Read() */

/***************************************************************************************************************//**
 * @brief  Write data to the CAN channel. Write up to eight bytes to the channel mailbox.
 *         Implements can_api_t::write()
 * @retval SSP_SUCCESS                      Operation succeeded.
 * @retval SSP_ERR_NOT_OPEN                 Control block not open.
 * @retval SSP_ERR_CAN_TRANSMIT_NOT_READY   Transmit in progress, cannot write data at this time.
 * @retval SSP_ERR_CAN_RECEIVE_MAILBOX      Mailbox is setup for receive and cannot send.
 * @retval SSP_ERR_INVALID_ARGUMENT         Data length or frame type invalid.
 * @retval SSP_ERR_ASSERTION                Null pointer presented
 *****************************************************************************************************************/
ssp_err_t R_CAN_Write (can_ctrl_t * const p_ctrl, uint32_t mailbox, can_frame_t * const p_frame)
{
    can_instance_ctrl_t * p_internal_ctrl = (can_instance_ctrl_t *) p_ctrl;

#if    (CAN_CFG_PARAM_CHECKING_ENABLE)
    ssp_err_t err;
    /** Check pointers for NULL values */
    err = can_write_parameters_check(p_ctrl, p_frame, mailbox);
    CAN_ERROR_RETURN(err == SSP_SUCCESS, err);
#endif /* if    (CAN_CFG_PARAM_CHECKING_ENABLE) */

    /** Check transmit ready flag. */
    R_CAN0_Type * p_can_regs = (R_CAN0_Type *) p_internal_ctrl->p_reg;

    /** Transmit ready flag is not set, return error/status. */
    CAN_ERROR_RETURN(HW_CAN_TransmitDataReady(p_can_regs, mailbox), SSP_ERR_CAN_TRANSMIT_NOT_READY);

    /** Transmit ready flag set, so clear it. */
    HW_CAN_TransmitDataClear(p_can_regs, mailbox);

    /** Send transmit frame. */
    HW_CAN_TransmitFrameSend(p_can_regs, mailbox, p_frame, p_internal_ctrl->id_mode);

    return SSP_SUCCESS;
}

/***************************************************************************************************************//**
 * @brief  CAN Control is used to control extended features. Implements can_api_t::control()
 * @retval SSP_SUCCESS                      Operation succeeded.
 * @retval SSP_ERR_NOT_OPEN                 Control block not open.
 * @retval SSP_ERR_INVALID_ARGUMENT         Invalid command.
 * @retval SSP_ERR_ASSERTION                Null pointer presented
 * @retval SSP_ERR_CAN_MODE_SWITCH_FAILED   Switching modes failed.
 *****************************************************************************************************************/
ssp_err_t R_CAN_Control (can_ctrl_t * const p_ctrl, can_command_t const command, void * p_data)
{
    can_instance_ctrl_t * p_internal_ctrl = (can_instance_ctrl_t *) p_ctrl;
    can_mode_t mode;
    ssp_err_t err = SSP_SUCCESS;

#if    (CAN_CFG_PARAM_CHECKING_ENABLE)
    /** Check pointers for NULL values */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_data);
    /** If channel is not open, return an error */
    CAN_ERROR_RETURN(p_internal_ctrl->open == CAN_OPEN, SSP_ERR_NOT_OPEN);
#endif /* if    (CAN_CFG_PARAM_CHECKING_ENABLE) */

    /** Verify command is CAN_COMMAND_MODE_SWITCH */
    CAN_ERROR_RETURN((CAN_COMMAND_MODE_SWITCH == command), SSP_ERR_INVALID_ARGUMENT);

    R_CAN0_Type * p_can_regs = (R_CAN0_Type *) p_internal_ctrl->p_reg;

    /** Change operating mode. Returns false if invalid mode or mode switch failed. */
    mode = *((can_mode_t *) p_data);
    err = HW_CAN_Control(p_can_regs, mode);
    CAN_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Save mode for diagnostic purposes. */
    p_internal_ctrl->operation_mode = mode;

    return err;
}

/***************************************************************************************************************//**
 * @brief   Get CAN state and status information for the channel. Implements can_api_t::infoGet()
 * @retval  SSP_SUCCESS                     Operation succeeded.
 * @retval  SSP_ERR_NOT_OPEN                Control block not open.
 * @retval  SSP_ERR_CAN_DATA_UNAVAILABLE    Channel failed to return info.
 * @retval  SSP_ERR_ASSERTION               Null pointer presented
 *****************************************************************************************************************/
ssp_err_t R_CAN_InfoGet (can_ctrl_t * const p_ctrl, can_info_t * const p_info)
{
    can_instance_ctrl_t * p_internal_ctrl = (can_instance_ctrl_t *) p_ctrl;

#if    (CAN_CFG_PARAM_CHECKING_ENABLE)
    /** Check pointers for NULL values */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_info);
    /** If channel is not open, return an error */
    CAN_ERROR_RETURN(p_internal_ctrl->open == CAN_OPEN, SSP_ERR_NOT_OPEN);
#endif /* if    (CAN_CFG_PARAM_CHECKING_ENABLE) */

    /** Get status for channel. */
    R_CAN0_Type * p_can_regs = (R_CAN0_Type *) p_internal_ctrl->p_reg;

    /** Error encountered when retrieving info. */
    CAN_ERROR_RETURN(HW_CAN_StatusGet(p_can_regs, p_info), SSP_ERR_CAN_DATA_UNAVAILABLE);

    /** Save the operation mode */
    p_info->operation_mode = p_internal_ctrl->operation_mode;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   Get CAN module code and API versions. Implements can_api_t::versionGet()
 * @retval  SSP_SUCCESS             Operation succeeded.
 * @retval  SSP_ERR_ASSERTION       Null pointer presented
 * note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_CAN_VersionGet (ssp_version_t * const p_version)
{
#if    (CAN_CFG_PARAM_CHECKING_ENABLE)
    /** Check pointer for NULL value */
    SSP_ASSERT(p_version);
#endif /* if    (CAN_CFG_PARAM_CHECKING_ENABLE) */

    /** Return module version information. */
    p_version->version_id = g_can_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup CAN)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief      CAN Common Error ISR.
 *
 * Gets errors, saves events and calls callback function, if used.
 *
 * @param[in]  p_ctrl    Pointer to CAN instance control block
 *
 **********************************************************************************************************************/

static void can_error_interrupt (can_instance_ctrl_t * p_ctrl)
{
    can_callback_args_t args = { 0U };
    can_interrrupt_status_t status = { 0U };
    uint32_t mailbox = { 0U };

    /** Get source of error interrupt. */
    R_CAN0_Type * p_can_regs = (R_CAN0_Type *) p_ctrl->p_reg;
    HW_CAN_InterruptErrorGet(p_can_regs, &status);

    if (status.int_status_b.bus_off_entry)                      ///< Check for bus off error.
    {
        args.event = CAN_EVENT_ERR_BUS_OFF;                     ///< Set event argument to bus error.
    }
    else if (status.int_status_b.bus_off_recovery)              ///< Check for bus recovery error.
    {
        args.event = CAN_EVENT_BUS_RECOVERY;                    ///< Set event argument to bus recovery.
    }
    else if (status.int_status_b.error_passive)                 ///< Check for bus error passive.
    {
        args.event = CAN_EVENT_ERR_PASSIVE;                     ///< Set event argument to error passive.
    }
    else if (status.int_status_b.error_warning)                 ///< Check for bus error warning.
    {
        args.event = CAN_EVENT_ERR_WARNING;                     ///< Set event argument to error warning.
    }
    else if (status.int_status_b.receive_overrun)               ///< Check for receive overwrite / overrrun.
    {
        args.event = CAN_EVENT_MAILBOX_OVERRUN;                 ///< Set event to overrrun error.
        HW_CAN_ErrorMailboxGet(p_can_regs, &mailbox);
    }
    else if(HW_CAN_MailboxMessageLost(p_ctrl->p_reg))
    {
        args.event = CAN_EVENT_MAILBOX_OVERRUN;                 ///< Set event to overrun error.
        HW_CAN_ErrorMailboxGet(p_can_regs, &mailbox);
    }
    else
    {
        /** Follow code rules */
    }

    /** Get user the callback, if set in the open function. */
    if (NULL != p_ctrl->p_callback)
    {
        args.channel = p_ctrl->channel;                 ///< Populate callback arguments accordingly.
        args.p_context = p_ctrl->p_context;
        args.mailbox = mailbox;
        p_ctrl->p_callback(&args);                      ///< Call the user callback function.
    }
}

/*******************************************************************************************************************//**
 * @brief      Error ISR.
 *
 * Saves context if RTOS is used, clears interrupts, calls common error function, and restores context if RTOS is used.
 **********************************************************************************************************************/
void can_error_isr (void)
{
    /** Save context if RTOS is used */
    SF_CONTEXT_SAVE

    /** Clear interrupt */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    can_instance_ctrl_t * p_ctrl = (can_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /** Check that the pointer is not NULL prior to using it */
    if ((p_ctrl != NULL) && (p_ctrl->p_reg != NULL))
    {
        can_error_interrupt(p_ctrl);
    }

    /** Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

/*******************************************************************************************************************//**
 * @brief      CAN Common Receive ISR.
 *
 * Identifies mailbox, saves receive event and calls callback function, if used.
 *
 * @param[in]  p_ctrl    Pointer to CAN instance control block
 *
 **********************************************************************************************************************/
static void can_receive_interrupt (can_instance_ctrl_t * p_ctrl)
{
    /** Get user the callback, if set in the open function. */
    if (NULL != p_ctrl->p_callback)
    {
        can_callback_args_t args;
        uint32_t mailbox = 0U;
        R_CAN0_Type * p_can_regs = (R_CAN0_Type *) p_ctrl->p_reg;
        HW_CAN_ReceiveMailboxGet(p_can_regs, &mailbox);

        args.event = CAN_EVENT_RX_COMPLETE;
        /** Save the receive mailbox number. */
        args.mailbox = mailbox;

        args.channel = p_ctrl->channel;                 ///< Populate callback arguments accordingly.
        args.p_context = p_ctrl->p_context;
        p_ctrl->p_callback(&args);                      ///< Call the user callback function.
    }
}

/*******************************************************************************************************************//**
 * @brief      Receive ISR.
 *
 * Saves context if RTOS is used, clears interrupts, calls common receive function
 * and restores context if RTOS is used.
 **********************************************************************************************************************/
void can_mailbox_rx_isr (void)
{
    /** Save context if RTOS is used */
    SF_CONTEXT_SAVE

    /** Clear interrupt */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    can_instance_ctrl_t * p_ctrl = (can_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /** Check that the pointer is not NULL prior to using it */
    if ((p_ctrl != NULL) && (p_ctrl->p_reg != NULL))
    {
        can_receive_interrupt(p_ctrl);
    }

    /** Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

/*******************************************************************************************************************//**
 * @brief      CAN Common Transmit ISR.
 *
 * Identifies mailbox, saves transmit event and calls callback function, if used.
 *
 * @param[in]  p_ctrl    Pointer to CAN instance control block
 *
 **********************************************************************************************************************/

static void can_transmit_interrupt (can_instance_ctrl_t * p_ctrl)
{
    /** Check CAN transmit. */

    /** Get user the callback, if set in the open function. */
    if (NULL != p_ctrl->p_callback)
    {
        can_callback_args_t args;
        uint32_t mailbox = 0U;
        ;
        R_CAN0_Type * p_can_regs = (R_CAN0_Type *) p_ctrl->p_reg;
        HW_CAN_TransmitMailboxGet(p_can_regs, &mailbox);

        HW_CAN_TransmitDataClear(p_can_regs, mailbox);  ///< clear SENTDATA and TRMREQ.

        /** Save the transmit mailbox number. */
        args.mailbox = mailbox;

        /**  Set event argument to transmit complete. */
        args.event = CAN_EVENT_TX_COMPLETE;
        args.channel = p_ctrl->channel;                 ///< Populate callback arguments accordingly.
        args.p_context = p_ctrl->p_context;
        p_ctrl->p_callback(&args);                      ///< Call the user callback function.
    }
}

/*******************************************************************************************************************//**
 * @brief      Transmit ISR.
 *
 * Saves context if RTOS is used, clears interrupts, calls common transmit function
 * and restores context if RTOS is used.
 **********************************************************************************************************************/
void can_mailbox_tx_isr (void)
{
    /** Save context if RTOS is used */
    SF_CONTEXT_SAVE

    /** Clear interrupt */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    can_instance_ctrl_t * p_ctrl = (can_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /** Check that the pointer is not NULL prior to using it */
    if ((p_ctrl != NULL) && (p_ctrl->p_reg != NULL))
    {
        can_transmit_interrupt(p_ctrl);

        /** Check for other mailboxes with pending transmit complete flags. */
        if(HW_CAN_SentDataStatusFlag(p_ctrl->p_reg))
        {
            NVIC_SetPendingIRQ(p_ctrl->mailbox_tx_irq);
        }
    }

    /** Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

/*******************************************************************************************************************//**
 * @brief      Disable and clear ISRs.
 *
 * Disables and clears CAN interrupts.
 *
 * @param[in]  p_ctrl      CAN Instance control block
 *
 **********************************************************************************************************************/
static void can_irq_disable (can_instance_ctrl_t * p_ctrl)
{
    R_CAN0_Type * p_can_regs = (R_CAN0_Type *) p_ctrl->p_reg;
    if (SSP_INVALID_VECTOR != p_ctrl->error_irq)
    {
        /** Disable and clear error interrupt. */
        NVIC_DisableIRQ(p_ctrl->error_irq);
        R_BSP_IrqStatusClear(p_ctrl->error_irq);
        NVIC_ClearPendingIRQ(p_ctrl->error_irq);
    }
    if (SSP_INVALID_VECTOR != p_ctrl->mailbox_rx_irq)
    {
        /** Disable and clear receive interrupt. */
        NVIC_DisableIRQ(p_ctrl->mailbox_rx_irq);
        R_BSP_IrqStatusClear(p_ctrl->mailbox_rx_irq);
        NVIC_ClearPendingIRQ(p_ctrl->mailbox_rx_irq);
    }
    if (SSP_INVALID_VECTOR != p_ctrl->mailbox_tx_irq)
    {
        /** Disable and clear transmit interrupt. */
        NVIC_DisableIRQ(p_ctrl->mailbox_tx_irq);
        R_BSP_IrqStatusClear(p_ctrl->mailbox_tx_irq);
        NVIC_ClearPendingIRQ(p_ctrl->mailbox_tx_irq);
    }
    HW_CAN_ErrorInterruptDisable(p_can_regs);
    HW_CAN_SendReceiveInterruptsDisable(p_can_regs);
}

/*******************************************************************************************************************//**
 * @brief Check for Open function NULL pointers and valid mailbox settings.
 *
 * @param[in] p_internal_ctrl       CAN instance control block
 * @param[in] p_cfg                 CAN configuration
 *
 * @return  SSP_SUCCESS             Pointers are valid. Mailbox count is valid. Control block is not currently open.
 * @return  SSP_ERR_ASSERTION       Invalid pointer
 * @return  SSP_ERR_CAN_INIT_FAILED Invalid mailbox count. Must be a multiple of 4 and maximum of 32.
 * @return  SSP_ERR_IN_USE          Control block is already open.
 **********************************************************************************************************************/
static ssp_err_t can_open_parameters_check (can_instance_ctrl_t * const p_internal_ctrl, can_cfg_t const * const p_cfg)
{
    uint32_t i;
    can_extended_cfg_t const * const extended_cfg = (can_extended_cfg_t *) p_cfg->p_extend;

    /** Check for null pointers */
    SSP_ASSERT(p_internal_ctrl);
    SSP_ASSERT(p_cfg);
    SSP_ASSERT(p_cfg->p_extend);
    SSP_ASSERT(p_cfg->p_callback);

    /** Check for valid mailbox count. Must be a multiple of 4 and maximum of 32. */
    CAN_ERROR_RETURN(p_cfg->mailbox_count <= CAN_MAX_NO_MAILBOXES, SSP_ERR_CAN_INIT_FAILED);
    CAN_ERROR_RETURN((p_cfg->mailbox_count % CAN_MAILBOX_GROUP_SIZE) == 0, SSP_ERR_CAN_INIT_FAILED);

    /** Check control block isn't already open */
    CAN_ERROR_RETURN(p_internal_ctrl->open != CAN_OPEN, SSP_ERR_IN_USE);

    for(i = 0U; i < (p_cfg->mailbox_count / CAN_MAILBOX_GROUP_SIZE); i++)
    {
        CAN_ERROR_RETURN(extended_cfg->p_mailbox_mask[i] <= CAN_DEFAULT_MASK, SSP_ERR_CAN_INIT_FAILED);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Check for Open function valid timing settings.
 *
 * @param[in] p_cfg     CAN Configuration
 *
 * @return  SSP_SUCCESS                 Valid timing settings
 *          SSP_ERR_CAN_INIT_FAILED     Invalid timing settings
 **********************************************************************************************************************/
static ssp_err_t can_open_parameters_check_timing (can_cfg_t const * const p_cfg)
{
    /** Check p_bit_timing is not null */
    SSP_ASSERT(p_cfg->p_bit_timing);

    /** Check baud Rate Prescaler. */
    CAN_ERROR_RETURN(p_cfg->p_bit_timing->baud_rate_prescaler <= CAN_BAUD_RATE_PRESCALER_MAX, SSP_ERR_CAN_INIT_FAILED);
    CAN_ERROR_RETURN(p_cfg->p_bit_timing->baud_rate_prescaler >= CAN_BAUD_RATE_PRESCALER_MIN, SSP_ERR_CAN_INIT_FAILED);

    /** Check Time Segment 1 is greater than Time Segment 2. */
    CAN_ERROR_RETURN((uint32_t)p_cfg->p_bit_timing->time_segment_1 > (uint32_t)p_cfg->p_bit_timing->time_segment_2, SSP_ERR_CAN_INIT_FAILED);

    /** Check Time Segment 2 is greater than or equal to the synchronization jump width */
    CAN_ERROR_RETURN((uint32_t)p_cfg->p_bit_timing->time_segment_2 >= (uint32_t)p_cfg->p_bit_timing->synchronization_jump_width,
            SSP_ERR_CAN_INIT_FAILED);

    return SSP_SUCCESS;
}


/*******************************************************************************************************************//**
 * @brief      Check for Open function valid clock settings.
 *
 * @param[in] p_cfg                 CAN Configuration
 * @param[in] can_feature           CAN features
 *
 * @return  SSP_SUCCESS                 Valid timing settings
 * @return  SSP_ERR_CAN_INIT_FAILED     Invalid timing settings
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                  * cgc_api_t::systemClockGet
 *                                  * cgc_api_t::systemClockFreqGet
 **********************************************************************************************************************/
static ssp_err_t can_open_parameters_check_clock (can_cfg_t const * const p_cfg, bsp_feature_can_t can_feature)
{
    can_extended_cfg_t const * const extended_cfg = (can_extended_cfg_t *) p_cfg->p_extend;
    const cgc_api_t * pcgc = &g_cgc_on_cgc;
    uint32_t pclka_iclk_frequency = 0U;
    uint32_t pclkb_frequency = 0U;
    ssp_err_t err = SSP_SUCCESS;

    /** Get the frequency of pclkb for later validation */
    err = pcgc->systemClockFreqGet(CGC_SYSTEM_CLOCKS_PCLKB, &pclkb_frequency);
    CAN_ERROR_RETURN(err == SSP_SUCCESS, err);

    /**  If the hardware manual states pclkb must be a 2:1 ratio of another clock */
    if(can_feature.check_pclkb_ratio == 1U)
    {
        /** Get the other clock defined by the hardware manual from the bsp and verify there is a 2:1 ratio */
        err = pcgc->systemClockFreqGet((cgc_system_clocks_t)can_feature.clock, &pclka_iclk_frequency);
        CAN_ERROR_RETURN(err == SSP_SUCCESS, err);
        CAN_ERROR_RETURN((pclka_iclk_frequency / pclkb_frequency) == CAN_PCLKB_RATIO, SSP_ERR_CAN_INIT_FAILED);
    }

    /** If the devices is configured for CANMCLK or can only use CANMCLK */
    if (can_feature.mclock_only || (CAN_CLOCK_SOURCE_CANMCLK == extended_cfg->clock_source))
    {
        /** Verify pclkb is greater than or equal to canmclk */
        CAN_ERROR_RETURN(pclkb_frequency >= (BSP_CFG_XTAL_HZ), SSP_ERR_CAN_INIT_FAILED);
    }
    else
    {
        /** Otherwise the device is configured for PCLKB. Verify the source clock is the PLL */
        cgc_system_clock_cfg_t clock_cfga;
        cgc_clock_t clock_source = (cgc_clock_t) 0U;
        err = pcgc->systemClockGet(&clock_source, &clock_cfga);
        CAN_ERROR_RETURN(err == SSP_SUCCESS, err);
        CAN_ERROR_RETURN(clock_source == CGC_CLOCK_PLL, SSP_ERR_CAN_INIT_FAILED);
    }
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Check for Write function for invalid parameters.
 *
 * @param[in] p_ctrl        CAN instance control block
 * @param[in] p_frame       CAN frame
 * @param[in] mailbox       CAN mailbox number
 *
 * @return  SSP_SUCCESS                 Control block open, vaild pointers, frame size, mailbox type and mailbox number.
 * @return  SSP_ERR_ASSERTION           Invalid pointer
 * @return  SSP_ERR_NOT_OPEN            Control block not open
 * @return  SSP_ERR_CAN_RECEIVE_MAILBOX Mailbox is configured to receive
 * @return  SSP_ERR_INVALID_ARGUMENT    Invalid frame size or mailbox number
 *
 **********************************************************************************************************************/
static ssp_err_t can_write_parameters_check (can_ctrl_t * const p_ctrl, can_frame_t * const p_frame, uint32_t mailbox)
{
    /** Check for null pointers */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_frame);

    /** Make sure mailbox is setup for transmit. */
    can_instance_ctrl_t * p_internal_ctrl = (can_instance_ctrl_t *) p_ctrl;
    R_CAN0_Type * p_can_regs = (R_CAN0_Type *) p_internal_ctrl->p_reg;

    /** If channel is not open, return an error */
    CAN_ERROR_RETURN(p_internal_ctrl->open == CAN_OPEN, SSP_ERR_NOT_OPEN);

    /** Check mailbox number */
    CAN_ERROR_RETURN((mailbox < CAN_MAX_NO_MAILBOXES), SSP_ERR_INVALID_ARGUMENT);

    /** Check Mailbox type */
    CAN_ERROR_RETURN(CAN_MAILBOX_TRANSMIT == HW_CAN_MailboxTypeGet(p_can_regs, mailbox), SSP_ERR_CAN_RECEIVE_MAILBOX);

    /** Check frame length is a maximum of 8 bytes. */
    CAN_ERROR_RETURN((p_frame->data_length_code <= CAN_MAX_DATA_LENGTH), SSP_ERR_INVALID_ARGUMENT);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Wake and initialize the device. Configure the mailboxes. Configure the device for normal mode operation.
 * @param[in] p_internal_ctrl       CAN instance control block
 * @param[in] p_cfg                 CAN configuration
 * @param[in] can_feature           CAN features
 * @return  SSP_SUCCESS         CAN device has been configured correctly
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                  * cgc_api_t::clockCheck
 **********************************************************************************************************************/
static ssp_err_t can_module_start(can_instance_ctrl_t * p_internal_ctrl, can_cfg_t const * const p_cfg, bsp_feature_can_t can_feature)
{
    ssp_err_t err = SSP_SUCCESS;

    /** Get the extended configuration */
    can_extended_cfg_t const * const extended_cfg = (can_extended_cfg_t *) p_cfg->p_extend;

    /** Initialize the control block */
    p_internal_ctrl->channel = p_cfg->channel;
    p_internal_ctrl->p_callback = p_cfg->p_callback;
    p_internal_ctrl->p_context = p_cfg->p_context;
    p_internal_ctrl->id_mode = p_cfg->id_mode;
    p_internal_ctrl->mailbox_count = p_cfg->mailbox_count;
    p_internal_ctrl->message_mode = p_cfg->message_mode;
    p_internal_ctrl->operation_mode = CAN_MODE_NORMAL;

    /** Check if only CANMCLK is supported */
    if (can_feature.mclock_only)
    {
        /** Set clock source to CANMCLK */
        p_internal_ctrl->clock_source = CAN_CLOCK_SOURCE_CANMCLK;
    }
    else
    {
        /** Set the clcok source to the user configured source. */
        p_internal_ctrl->clock_source = extended_cfg->clock_source;
    }

    /** Put the device in reset mode, configure the device, then go to halt mode. */
    err = can_wake_and_init(p_internal_ctrl, p_cfg->p_bit_timing);
    CAN_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Configure mailboxes. */
    can_mailbox_configure(p_internal_ctrl->p_reg, p_cfg);

    /** Go to normal operation. */
    err = can_operate_mode(p_internal_ctrl->p_reg);
    CAN_ERROR_RETURN(SSP_SUCCESS == err, err);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Set vectors.
 *
 * @param[in] p_ctrl                CAN instance control block
 * @param[in] p_cfg                 CAN configuration
 * @param[in] p_feature             CAN feature definition for this channel
 *                           This function calls:
 *                                  * fmi_api_t::eventInfoGet
 **********************************************************************************************************************/
static void can_irq_enable (can_instance_ctrl_t * p_ctrl, can_cfg_t const * const p_cfg, ssp_feature_t * p_feature)
{
    ssp_vector_info_t * p_vector_info;
    fmi_event_info_t event_info = { (IRQn_Type) 0 };
    R_CAN0_Type * p_can_regs = (R_CAN0_Type *) p_ctrl->p_reg;

    /** Check for a valid error interrupt vector */
    g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_CAN_ERROR, &event_info);
    p_ctrl->error_irq = event_info.irq;
    if (SSP_INVALID_VECTOR != p_ctrl->error_irq)
    {
        /** Enable error interrupt. */
        R_SSP_VectorInfoGet(p_ctrl->error_irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->error_irq, p_cfg->error_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
        NVIC_EnableIRQ(p_ctrl->error_irq);
        HW_CAN_ErrorInterruptEnable(p_can_regs);
    }

    /** Check for a valid receive interrupt vector */
    g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_CAN_MAILBOX_RX, &event_info);
    p_ctrl->mailbox_rx_irq = event_info.irq;
    if (SSP_INVALID_VECTOR != p_ctrl->mailbox_rx_irq)
    {
        /** Enable receive interrupt. */
        R_SSP_VectorInfoGet(p_ctrl->mailbox_rx_irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->mailbox_rx_irq, p_cfg->mailbox_rx_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
        NVIC_EnableIRQ(p_ctrl->mailbox_rx_irq);
    }

    /** Check for a valid transmit interrupt vector */
    g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_CAN_MAILBOX_TX, &event_info);
    p_ctrl->mailbox_tx_irq = event_info.irq;
    if (SSP_INVALID_VECTOR != p_ctrl->mailbox_tx_irq)
    {
        /** Enable transmit interrupt. */
        R_SSP_VectorInfoGet(p_ctrl->mailbox_tx_irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->mailbox_tx_irq, p_cfg->mailbox_tx_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
        NVIC_EnableIRQ(p_ctrl->mailbox_tx_irq);
    }

    /** If either the transmit or receive interrupts are enabled, enable TX/RX interrupts on the CAN device. */
    if ((SSP_INVALID_VECTOR != p_ctrl->mailbox_tx_irq) || (SSP_INVALID_VECTOR != p_ctrl->mailbox_rx_irq))
    {
        HW_CAN_SendReceiveInterruptsEnable(p_can_regs);
    }
}

/*******************************************************************************************************************//**
 * @brief      Go to CAN operate mode.
 * @param[in] p_can_regs    CAN registers
 * @return  SSP_SUCCESS                     CAN device has entered normal operation mode and reset the
 *                                              timestamp without errors.
 * @return  SSP_ERR_CAN_MODE_SWITCH_FAILED  CAN device failed to enter normal mode
 * @return  SSP_ERR_CAN_INIT_FAILED         CAN device failed to reset the timestamp or had an error
 **********************************************************************************************************************/
static ssp_err_t can_operate_mode (R_CAN0_Type * p_can_regs)
{
    ssp_err_t err = SSP_SUCCESS;

    /** Halt -> Normal OPERATION mode */
    err = HW_CAN_Control(p_can_regs, CAN_MODE_NORMAL);
    CAN_ERROR_RETURN(err == SSP_SUCCESS, err);

    /** Time Stamp Counter reset. Set the TSRC bit to 1 in CAN Operation mode. */
    CAN_ERROR_RETURN(HW_CAN_TimeStampReset(p_can_regs), SSP_ERR_CAN_INIT_FAILED);

    /** Check for errors so far, report, and clear. */
    CAN_ERROR_RETURN(false==HW_CAN_ErrorsGet(p_can_regs), SSP_ERR_CAN_INIT_FAILED);

    return err;
}

/*******************************************************************************************************************//**
 * @brief Checks that CANMCLK is running if that is the target clock. Wakes the device and puts it into reset mode.
 *        Initializes registers and clock settings. Puts the device in HALT mode.
 * @param[in] p_internal_ctrl       CAN instance control block
 * @param[in] p_timing              CAN timing configuration
 * @return  SSP_SUCCESS             Device has entered halt mode and initialized properly
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                  * cgc_api_t::clockCheck
 **********************************************************************************************************************/
static ssp_err_t can_wake_and_init (can_instance_ctrl_t * p_internal_ctrl, can_bit_timing_cfg_t * const p_timing)
{
    ssp_err_t err = SSP_SUCCESS;
    const cgc_api_t* pcgc = &g_cgc_on_cgc;
    R_CAN0_Type* p_can_regs = p_internal_ctrl->p_reg;

    /** If clock source is CANMCLK verify it is active before changing modes.
     * If it's not then chanigng modes could result in failing in an unknown mode. */
    if (CAN_CLOCK_SOURCE_CANMCLK == p_internal_ctrl->clock_source)
    {
        /** Check the clock has stabilized */
        err = pcgc->clockCheck(CGC_CLOCK_MAIN_OSC);

        /** SSP_ERR_STABILIZED is the expected return value for clockCheck with CGC_CLOCK_MAIN_OSC */
        if (SSP_ERR_STABILIZED == err)
        {
            err = SSP_SUCCESS;
        }
    }
    CAN_ERROR_RETURN(err == SSP_SUCCESS, err);

    /** Enter reset mode */
    err = HW_CAN_Control(p_can_regs, CAN_MODE_RESET);
    CAN_ERROR_RETURN(err == SSP_SUCCESS, err);

    /** BOM:    Bus Off recovery mode acc. to IEC11898-1 */
    HW_CAN_BusRecoveryModeSet(p_can_regs);

    /** MBM: Select normal mailbox mode. */
    HW_CAN_MailboxModeSet(p_can_regs);

    /** Select ID mode. Standard or extended */
    HW_CAN_IdModeSet(p_can_regs, p_internal_ctrl->id_mode);

    /** Select message overwrite or overrun mode */
    HW_CAN_MessageModeSet(p_can_regs, p_internal_ctrl->message_mode);

    /** Select transmission priority mode */
    HW_CAN_PriorityModeSet(p_can_regs);

    /** Set time stamp */
    HW_CAN_TimeStampSet(p_can_regs);

    /** Set timing bits */
    HW_CAN_BitTimingSet(p_can_regs, p_timing);

    /** Select the clock */
    HW_CAN_ClockSet(p_can_regs, p_internal_ctrl->clock_source);

    /** Enter halt mode */
    err = HW_CAN_Control(p_can_regs, CAN_MODE_HALT);
    CAN_ERROR_RETURN(err == SSP_SUCCESS, err);

    return err;
}

/*******************************************************************************************************************//**
 * @brief Configure CAN mailboxes
 * @param[in] p_can_regs    CAN registers
 * @param[in] p_cfg         CAN configuration
 **********************************************************************************************************************/
static void can_mailbox_configure (R_CAN0_Type * p_can_regs, can_cfg_t const * const p_cfg)
{
    can_extended_cfg_t const * const extended_cfg = (can_extended_cfg_t *) p_cfg->p_extend;

    /** Clear mailboxes in Halt mode. */
    HW_CAN_MailboxesClear(p_can_regs);

    /** Set Error Display mode in Halt mode. */
    HW_CAN_ErrorModeSet(p_can_regs);

    /** Set the IDs for each mailbox. */
    HW_CAN_MailboxIdSet(p_can_regs, p_cfg->mailbox_count, p_cfg->p_mailbox, p_cfg->id_mode);

    /** Set the masks for each mailbox group and initialize the mask invalid register. */
    HW_CAN_MailboxMaskSet(p_can_regs, p_cfg->mailbox_count, extended_cfg->p_mailbox_mask, p_cfg->id_mode);
}
