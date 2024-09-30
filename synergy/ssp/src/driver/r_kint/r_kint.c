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
 * File Name    : r_kint.c
 * Description  : This file implements the driver for the Key Interrupt Module(KINT) for the KeyMatrix Interface
 **********************************************************************************************************************/



/***********************************************************************************************************************
 Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_kint_cfg.h"
#include "hw/hw_kint_private.h"
#include "r_keymatrix_api.h"
#include "r_kint.h"
#include "r_kint_private.h"
#include "r_kint_private_api.h"

/***********************************************************************************************************************
 Macro definitions
 **********************************************************************************************************************/

#define KINT_OPEN    (0x4B494E54ULL)

/** Macro for error logger. */
#ifndef KINT_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define KINT_ERROR_RETURN(a, err)  SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_kint_version)
#endif

/***********************************************************************************************************************
 Typedef definitions
 **********************************************************************************************************************/

#if KINT_CFG_PARAM_CHECKING_ENABLE
/***********************************************************************************************************************
 Private function prototypes
 **********************************************************************************************************************/
static ssp_err_t r_kint_keymatrix_open_param_check (kint_instance_ctrl_t const * const p_ctrl,
                                                    keymatrix_cfg_t const * const p_cfg);
#endif

/***********************************************************************************************************************
 Private global variables
 **********************************************************************************************************************/
/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "kint";
#endif

/** Storage for callback function and context */
void (* g_keymatrix_cb)(keymatrix_callback_args_t * cb_data) =  { NULL };
static void const * g_keymatrix_context = { NULL };

#if defined(__GNUC__)
/* This structure is affected by warnings from the GCC compiler bug. This pragma suppresses the warnings in this
 * structure only, and will be removed when the SSP compiler is updated to v5.3. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_kint_version =
{
        .api_version_minor  = KEYMATRIX_API_VERSION_MINOR,
        .api_version_major  = KEYMATRIX_API_VERSION_MAJOR,
        .code_version_major = KINT_CODE_VERSION_MAJOR,
        .code_version_minor = KINT_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/***********************************************************************************************************************
 Global Variables
 **********************************************************************************************************************/
/** KeyMatrix Implementation of Key Interrupt  */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const keymatrix_api_t g_keymatrix_on_kint =
{
        .open       = R_KINT_KEYMATRIX_Open,
        .enable     = R_KINT_KEYMATRIX_Enable,
        .disable    = R_KINT_KEYMATRIX_Disable,
        .triggerSet = R_KINT_KEYMATRIX_TriggerSet,
        .close      = R_KINT_KEYMATRIX_Close,
        .versionGet = R_KINT_VersionGet
};
/***********************************************************************************************************************
 Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @addtogroup KINT
 * @{
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Power on KINT, handle required initialization described in hardware manual. Implements keymatrix_api_t::open.
 *
 * The Open function configures all the Key Input (KINT) channels and provides a handle for use with the rest
 * of the KINT API functions. This function must be called at least once prior to calling any other KINT API functions.
 * After the peripheral is initialized, the Open function should not be called again without first calling the Close
 * function.
 *
 * @retval SSP_SUCCESS                Initialization was successful.
 * @retval SSP_ERR_ASSERTION          One of the following parameters may be NULL: p_cfg, or p_ctrl or the callback.
 * @retval SSP_ERR_INVALID_ARGUMENT   One of the following may be invalid:
 *                                     - p_cfg->channel: must be between 0 and KINT_MAX_NUM_CHANNELS
 *                                     - p_cfg->trigger not a valid value.
 * @retval SSP_ERR_HW_LOCKED          The API has already been opened. It must be closed before it can be opened again.
 * @return                            See @ref Common_Error_Codes or functions called by this function for other possible
 *                                    return codes. This function calls:
 *                                        * fmi_api_t::productFeatureGet
 *                                        * fmi_api_t::eventInfoGet
 * @note                              This function is not reentrant.
 **********************************************************************************************************************/
ssp_err_t R_KINT_KEYMATRIX_Open(keymatrix_ctrl_t * const p_api_ctrl,
        keymatrix_cfg_t const * const p_cfg)
{
    ssp_err_t err;
    kint_instance_ctrl_t * p_ctrl = (kint_instance_ctrl_t *) p_api_ctrl;

    /** Check to see that the arguments passed are not null pointers */
#if KINT_CFG_PARAM_CHECKING_ENABLE
    err = r_kint_keymatrix_open_param_check (p_ctrl, p_cfg);
    KINT_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;                       // Register base address is at channel 0
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_KEY;
    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    KINT_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = (R_KINT_Type *) info.ptr;

#if KINT_CFG_PARAM_CHECKING_ENABLE
    uint16_t channel_variant = (((uint8_t) info.variant_data >> 2) & 1U);
    uint32_t invalid_channel_mask = 0xFFFFFFFFU;
    if (1U == channel_variant)
    {
        invalid_channel_mask = 0xFFFFFF00U;
    }
    KINT_ERROR_RETURN((0U == (invalid_channel_mask & p_cfg->channels)), SSP_ERR_IP_CHANNEL_NOT_PRESENT);
#endif /* if KINT_CFG_PARAM_CHECKING_ENABLE */

    ssp_vector_info_t * p_vector_info;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_KEY_INT, &event_info);
    p_ctrl->irq = event_info.irq;
    KINT_ERROR_RETURN((SSP_INVALID_VECTOR != p_ctrl->irq), SSP_ERR_IRQ_BSP_DISABLED);

    R_SSP_VectorInfoGet(p_ctrl->irq, &p_vector_info);
    NVIC_SetPriority(p_ctrl->irq, p_cfg->irq_ipl);
    *(p_vector_info->pp_ctrl) = p_ctrl;

    /** Grab the hardware lock. If successful this indicates that the open was not previously called. */
    if (SSP_ERR_IN_USE == R_BSP_HardwareLock(&ssp_feature))
    {
        return SSP_ERR_HW_LOCKED;
    }

    /** Disable interrupts in the KINT peripheral*/
    HW_KINT_KRM_Set(p_ctrl->p_reg, HW_KINT_KRM_RESET);
    /** Clear any pending interrupt requests in the KINT peripheral*/
    HW_KINT_KRF_Set(p_ctrl->p_reg, HW_KINT_KRF_RESET);
    /** Clear the Interrupt Request in the ICU */
    R_BSP_IrqStatusClear(p_ctrl->irq);
    NVIC_ClearPendingIRQ(p_ctrl->irq);

    /** Configure the trigger edge.
     * The trigger edge can be modified in the TriggerSet function later if desired */
    if (p_cfg->trigger == KEYMATRIX_TRIG_FALLING)
    {
        HW_KINT_KRCTL_KREG_Set(p_ctrl->p_reg, HW_KINT_KRCTL_KREG_FALLING_EDGE_0);

    } else /**KEYMATRIX_TRIG_RISING condition */
    {
        HW_KINT_KRCTL_KREG_Set(p_ctrl->p_reg, HW_KINT_KRCTL_KREG_RISING_EDGE_1);
    }

    /** Configure the usage of key interrupt flags */
    HW_KINT_KRCTL_KRMD_Set(p_ctrl->p_reg, HW_KINT_KRCTL_KRMD_USES_KEY_INTERRUPT_FLAGS_1);

    /** Store the callback and context information */
    g_keymatrix_cb = p_cfg->p_callback;
    g_keymatrix_context = p_cfg->p_context;

    /** If interrupts are to be enabled now, set it up for the selected channels.
     * The channels can be changed later in the enable function but to modify
     * the callback and context, the API has to be closed and reopened with the new
     * callback and context.
     *  @note The KINT hardware only supports a single interrupt for all channels */
    if (true == p_cfg->autostart)
    {
        /** Enable interrupt for the selected channels after casting since KRM is an 8 bit register*/
        HW_KINT_KRM_Set(p_ctrl->p_reg, (uint8_t)p_cfg->channels);
        /** Enable interrupt */
        NVIC_EnableIRQ(p_ctrl->irq);
    }
    /** Store number of channels for use to the control block to use it later */
    p_ctrl->channels = p_cfg->channels;
    /** Mark driver as open by initializing it to "KINT" in its ASCII equivalent. */
    p_ctrl->open = KINT_OPEN;

    return SSP_SUCCESS;
} /** End of function R_KINT_KEYMATRIX_Open */

/*******************************************************************************************************************//**
 * @brief  Disable KINT. Implements keymatrix_api_t::close.
 *
 * The Close function disables the interrupts in the peripheral and the NVIC and clears any pending
 * interrupt requests. It also releases the hardware lock on the API.
 *
 * @retval SSP_SUCCESS        Successful close.
 * @retval SSP_ERR_ASSERTION  The parameter p_ctrl is NULL.
 * @retval SSP_ERR_NOT_OPEN   The channel is not opened.
 **********************************************************************************************************************/
ssp_err_t R_KINT_KEYMATRIX_Close(keymatrix_ctrl_t * const p_api_ctrl)
{
    kint_instance_ctrl_t * p_ctrl = (kint_instance_ctrl_t *) p_api_ctrl;

#if KINT_CFG_PARAM_CHECKING_ENABLE
    /** Check for null pointers */
    SSP_ASSERT(NULL != p_ctrl);
    /** Check that the Open function has been called else return SSP_ERR_NOT_OPEN */
    KINT_ERROR_RETURN(KINT_OPEN == p_ctrl->open , SSP_ERR_NOT_OPEN);
#endif

    /* The device is now considered closed */
    p_ctrl->open = 0U;
    /** Disable interrupt in ICU*/
    NVIC_DisableIRQ(p_ctrl->irq);
    /** Disable interrupts in the KINT peripheral*/
    HW_KINT_KRM_Set(p_ctrl->p_reg, HW_KINT_KRM_RESET);
    /** Clear any pending interrupt requests in the KINT peripheral*/
    HW_KINT_KRF_Set(p_ctrl->p_reg, HW_KINT_KRF_RESET);
    /** Clear the Interrupt Request bit */
    R_BSP_IrqStatusClear(p_ctrl->irq);

    /** Clear stored internal driver data */
    g_keymatrix_cb = NULL;
    g_keymatrix_context = NULL;

    /** Release the lock */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_KEY;
    R_BSP_HardwareUnlock(&ssp_feature);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Enable external irq for all the specified channel by R_KINT_KEYMATRIX_Open. Implements keymatrix_api_t::enable.
 *
 * This function enables interrupts for the KINT peripheral both at the interrupt level and in the NVIC
 * after clearing any pending requests in the KINT and ICU peripheral. All the channels specified by
 * R_KINT_KEYMATRIX_Open are enabled.
 *
 * @retval SSP_SUCCESS        Interrupt enabled successfully.
 * @retval SSP_ERR_ASSERTION  The p_ctrl parameter was null.
 * @retval SSP_ERR_NOT_OPEN   The peripheral is not opened.
 **********************************************************************************************************************/
ssp_err_t R_KINT_KEYMATRIX_Enable(keymatrix_ctrl_t * const p_api_ctrl)
{
    kint_instance_ctrl_t * p_ctrl = (kint_instance_ctrl_t *) p_api_ctrl;

#if KINT_CFG_PARAM_CHECKING_ENABLE
    /** Check for null pointer. */
    SSP_ASSERT(NULL != p_ctrl);
    /** Check that the Open function has been called else return SSP_ERR_NOT_OPEN */
    KINT_ERROR_RETURN(KINT_OPEN == p_ctrl->open , SSP_ERR_NOT_OPEN);
#endif
    /** Clear any pending interrupt requests in the KINT peripheral */
    HW_KINT_KRF_Set(p_ctrl->p_reg, HW_KINT_KRF_RESET);
    /** Clear the Interrupt Request Flag in the ICU */
    R_BSP_IrqStatusClear(p_ctrl->irq);
    /** Enable interrupt for the selected channels after casting since KRM is an 8 bit register */
    HW_KINT_KRM_Set(p_ctrl->p_reg, (uint8_t)p_ctrl->channels);
    /** Enable interrupt */
    NVIC_EnableIRQ(p_ctrl->irq);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Disable external irq for all the specified channel by R_KINT_KEYMATRIX_Open. Implements
 *         keymatrix_api_t::disable.
 *
 * This function disables interrupts for the KINT peripheral both at the interrupt level and in the NVIC.
 * All the channels specified by R_KINT_KEYMATRIX_Open are disabled.
 *
 * @retval SSP_SUCCESS        Interrupt disabled successfully.
 * @retval SSP_ERR_ASSERTION  The p_ctrl parameter was null.
 * @retval SSP_ERR_NOT_OPEN   The channel is not opened.
 **********************************************************************************************************************/
ssp_err_t R_KINT_KEYMATRIX_Disable(keymatrix_ctrl_t * const p_api_ctrl)
{
    kint_instance_ctrl_t * p_ctrl = (kint_instance_ctrl_t *) p_api_ctrl;

#if KINT_CFG_PARAM_CHECKING_ENABLE
    /** Check for null pointers */
    SSP_ASSERT(NULL != p_ctrl);
    /** Check that the Open function has been called else return SSP_ERR_NOT_OPEN */
    KINT_ERROR_RETURN(KINT_OPEN == p_ctrl->open , SSP_ERR_NOT_OPEN);
#endif

    /** Disable interrupts in the KINT peripheral */
    HW_KINT_KRM_Set(p_ctrl->p_reg, HW_KINT_KRM_RESET);
    /** Clear any pending interrupt requests in the KINT peripheral */
    HW_KINT_KRF_Set(p_ctrl->p_reg, HW_KINT_KRF_RESET);
    /** Disable interrupt in the ICU */
    NVIC_DisableIRQ(p_ctrl->irq);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Set trigger edge (falling or rising) provided. Implements keymatrix_api_t::triggerSet.
 *
 * This function changes trigger sense at run-time. The change is applied to all the channels specified by
 * R_KINT_KEYMATRIX_Open.
 *
 * @retval SSP_SUCCESS                Trigger value written successfully.
 * @retval SSP_ERR_ASSERTION          The p_ctrl parameter was null.
 * @retval SSP_ERR_INVALID_ARGUMENT   Trigger input invalid. hw_trigger must be one of the options from button_trigger_t.
 * @retval SSP_ERR_NOT_OPEN           The channel is not opened.
 * @note   This function expects to be called when the driver is disabled (the driver state before R_KINT_KEYMATRIX_Enable
 *         is called if the driver is opened in the non-auto start mode, or after R_KINT_KEYMATRIX_Disable is called if
 *         the driver is opened in the auto start mode). The driver does not restrict to call this API when the driver
 *         is enabled but users need to make sure the edge detection sense is instantly changed by this API call.
 **********************************************************************************************************************/
ssp_err_t R_KINT_KEYMATRIX_TriggerSet(keymatrix_ctrl_t * const p_api_ctrl,
        keymatrix_trigger_t hw_trigger)
{
    kint_instance_ctrl_t * p_ctrl = (kint_instance_ctrl_t *) p_api_ctrl;

#if KINT_CFG_PARAM_CHECKING_ENABLE
    /** Check for null pointers */
    SSP_ASSERT(NULL != p_ctrl);
    /** Check that the Open function has been called else return SSP_ERR_NOT_OPEN */
    KINT_ERROR_RETURN(KINT_OPEN == p_ctrl->open , SSP_ERR_NOT_OPEN);
    /** Check that the trigger argument is valid else return SSP_ERR_INVALID_ARG */
    KINT_ERROR_RETURN(!((hw_trigger != KEYMATRIX_TRIG_FALLING) && (hw_trigger != KEYMATRIX_TRIG_RISING)),
            SSP_ERR_INVALID_ARGUMENT);
#endif

    /** Configure the trigger edge */
    if (hw_trigger == KEYMATRIX_TRIG_FALLING)
    {
        HW_KINT_KRCTL_KREG_Set(p_ctrl->p_reg, HW_KINT_KRCTL_KREG_FALLING_EDGE_0);

    } else /** KEYMATRIX_TRIG_RISING condition */
    {
        HW_KINT_KRCTL_KREG_Set(p_ctrl->p_reg, HW_KINT_KRCTL_KREG_RISING_EDGE_1);
    }

    /** Configure the usage of key interrupt flags */
    HW_KINT_KRCTL_KRMD_Set(p_ctrl->p_reg, HW_KINT_KRCTL_KRMD_USES_KEY_INTERRUPT_FLAGS_1);
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Set driver version based on compile time macros.
 *
 * @retval     SSP_SUCCESS        Successful return.
 * @retval     SSP_ERR_ASSERTION  The parameter p_version is NULL.
 **********************************************************************************************************************/
ssp_err_t R_KINT_VersionGet(ssp_version_t * const p_version)
{
#if KINT_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_version);
#endif

    p_version->version_id = g_kint_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup KINT)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 Private Functions
 **********************************************************************************************************************/
#if KINT_CFG_PARAM_CHECKING_ENABLE
/*******************************************************************************************************************//**
 * @brief  Parameter check function for KINT driver open processing.
 *
 * @param[in]   p_ctrl       Pointer control structure.
 * @param[in]   p_cfg        Pointer to configuration structure. All elements of the structure must be set by user.
 * @retval SSP_SUCCESS                All the parameter are valid.
 * @retval SSP_ERR_IN_USE             The driver has been opened.
 * @retval SSP_ERR_ASSERTION          One of the following parameters is NULL: p_cfg, or p_ctrl or the callback.
 * @retval SSP_ERR_INVALID_ARGUMENT   One of the following parameters is invalid:
 *                                     - p_cfg->channel: must be between 0 and KINT_MAX_NUM_CHANNELS
 *                                     - p_cfg->hw_trigger not a valid value.
 **********************************************************************************************************************/
static ssp_err_t r_kint_keymatrix_open_param_check (kint_instance_ctrl_t const * const p_ctrl,
                                                    keymatrix_cfg_t const * const p_cfg)
{
    /** Check NULL pointers */
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_ctrl);
    /** Make sure that there is a valid callback passed */
    SSP_ASSERT(NULL != p_cfg->p_callback);
    /** If the input channel number exceeds KINT_MAX_NUM_CHANNELS return SSP_ERR_INVALID_ARGUMENT */
    KINT_ERROR_RETURN(!(p_cfg->channels > KINT_MAX_NUM_CHANNELS), SSP_ERR_INVALID_ARGUMENT);
    /** If the input channel number is less than or equal to 0 return SSP_ERR_INVALID_ARGUMENT */
    KINT_ERROR_RETURN(!(p_cfg->channels <= 0U), SSP_ERR_INVALID_ARGUMENT);
    /** If the trigger is not set to rising or falling edge return SSP_ERR_INVALID_ARGUMENT */
    KINT_ERROR_RETURN(!((p_cfg->trigger != KEYMATRIX_TRIG_FALLING)
                    && (p_cfg->trigger != KEYMATRIX_TRIG_RISING)),
            SSP_ERR_INVALID_ARGUMENT);
    return SSP_SUCCESS;
}
#endif

void key_int_isr(void);

/*******************************************************************************************************************//**
 * @brief  Key Interrupt ISR routine.
 *
 *          This function implements the KEY ISR. The function clears the interrupt request source on entry,
 *          populates the callback structure with the current status of the Key Return Flag register (KRF),
 *          which will hold the current status of the Key Input pins (at least one of which would have been
 *          in an active state causing the ISR to trigger) and calls the callback function with the argument.
 *
 **********************************************************************************************************************/
void key_int_isr(void)
{
    /** Save context if RTOS is used. */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    uint8_t krf_status = 0;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    kint_instance_ctrl_t * p_ctrl = (kint_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /** Clear the IRQ Request. */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    /** Read the state of the Key Input pins and store to structure for callback. */
    krf_status = HW_KINT_KRF_Get(p_ctrl->p_reg);

    /** Only clear the serviced channels in the KINT peripheral.
     *  Writing 0 to a KRF register bit clears the KRF bit. Writing 1 has no effect on a KRF register bit value. */
    HW_KINT_KRF_Set(p_ctrl->p_reg, (uint8_t)(~krf_status));

    /** Check if a valid callback and channel (ie KRF bit) are present.
     *  This interrupt could be triggered with no KRF bits set if a KRF bit was set
     *  during a previous occurrence of this interrupt after the call to R_BSP_IrqStatusClear()
     *  and before the call to HW_KINT_KRF_Get(). If this were to happen then the latest KRF bit
     *  would be serviced along with the previous channel(s), but the IR bit would still be set
     *  in the IELSRn register. This would trigger another occurrence of this interrupt where
     *  no KRF bits may be set. */
    if ((NULL != g_keymatrix_cb) && (krf_status))
    {
        /** Set data to identify callback to user. */
        keymatrix_callback_args_t cb_data;
        cb_data.channels = (keymatrix_channels_t)krf_status;
        /** Read the saved context information and store to structure for callback. */
        cb_data.p_context = g_keymatrix_context;
        /** Call the callback function. */
        g_keymatrix_cb(&cb_data);
    }

    /** Restore context if RTOS is used. */
    SF_CONTEXT_RESTORE
}
