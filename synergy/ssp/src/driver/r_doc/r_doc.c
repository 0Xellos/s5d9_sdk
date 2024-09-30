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
 * File Name    : r_doc.c
 * Description  : DOC functions to implement the interface.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_doc_api.h"
#include "hw/hw_doc_private.h"
#include "r_doc_private_api.h"
#include "bsp_api.h"
#include "bsp_cfg.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** "DOCO" in ASCII, used to identify Data Operation Circuit (DOC) configuration */
#define DOC_OPEN (0x444F434fU)

/** Macro for error logger. */
#ifndef DOC_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define DOC_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_doc_version)
#endif

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
void doc_int_isr (void);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "doc";
#endif

#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug. This pragma suppresses the warnings in this 
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_doc_version =
{
	.api_version_major  = DOC_API_VERSION_MAJOR,
	.code_version_minor = DOC_CODE_VERSION_MINOR,
    .code_version_major = DOC_CODE_VERSION_MAJOR,
	.api_version_minor  = DOC_API_VERSION_MINOR,
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
/* DOC implementation of DOC Driver  */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const doc_api_t g_doc_on_doc =
{
    .open               = R_DOC_Open,
    .close              = R_DOC_Close,
    .statusGet          = R_DOC_StatusGet,
    .statusClear        = R_DOC_StatusClear,
    .write              = R_DOC_Write,
    .inputRegisterWrite = R_DOC_InputRegisterWrite,
    .versionGet         = R_DOC_VersionGet,
};

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @addtogroup DOC
 * @{
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Configure the Data Operation Circuit (DOC) in comparison, addition or subtraction mode. Implements
 * doc_api_t::open.
 *
 * If callback is not NULL in the configuration structure the DOC IRQ will be enabled.
 *
 *
 * @retval SSP_SUCCESS                  DOC successfully configured.
 * @retval SSP_ERR_IN_USE               Module already open.
 * @retval SSP_ERR_ASSERTION            One or more pointers point to NULL.
 * @retval SSP_ERR_INVALID_ARGUMENT     ISR is not enabled. Enable the ISR in bsp_irq_cfg.h.
 * @retval SSP_ERR_HW_LOCKED            DOC resource is locked.
 * @return                              See @ref Common_Error_Codes or functions called by this function for other possible
 *                                      return codes. This function calls:
 *                                        * fmi_api_t::productFeatureGet
 *                                        * fmi_api_t::eventInfoGet
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_DOC_Open (doc_ctrl_t * const p_api_ctrl, doc_cfg_t const * const p_cfg)
{
    doc_instance_ctrl_t * p_ctrl = (doc_instance_ctrl_t *) p_api_ctrl;

    /** Validate the parameters and check if the module is initialized */
#if DOC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_cfg);
    DOC_ERROR_RETURN(DOC_OPEN != p_ctrl->open, SSP_ERR_IN_USE);
#endif

    /** Make sure the DOC exists on this part. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_DOC;
    fmi_feature_info_t info = {0U};
    ssp_err_t err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    /* This statement returns an error if the DOC does not exist on the MCU.  All current MCUs have a DOC, so this
     * cannot be tested yet. */
    /*SSP_LDRA_EXECUTION_INSPECTED */
    DOC_ERROR_RETURN(SSP_SUCCESS == err, err);

    p_ctrl->p_reg = info.ptr;
    R_DOC_Type * p_doc_reg = (R_DOC_Type *) p_ctrl->p_reg;

    ssp_vector_info_t * p_vector_info;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_DOC_INT, &event_info);
    if (NULL != p_cfg->p_callback)
    {
        /* Interrupt must be enabled if a callback is requested. */
        DOC_ERROR_RETURN(SSP_INVALID_VECTOR != event_info.irq, SSP_ERR_INVALID_ARGUMENT);
    }

    /** Lock the DOC Hardware Resource */
    DOC_ERROR_RETURN(SSP_SUCCESS == R_BSP_HardwareLock(&ssp_feature), SSP_ERR_HW_LOCKED);

    if (SSP_INVALID_VECTOR != event_info.irq)
    {
        R_SSP_VectorInfoGet(event_info.irq, &p_vector_info);
        NVIC_SetPriority(event_info.irq, p_cfg->irq_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
        R_BSP_IrqStatusClear(event_info.irq);
        NVIC_ClearPendingIRQ(event_info.irq);
        NVIC_DisableIRQ(event_info.irq);
    }

    R_BSP_ModuleStart(&ssp_feature);

    /** Configure the DOC via DOCR register. */
    HW_DOC_DOCRWrite(p_doc_reg, p_cfg->event);

    R_DOC_StatusClear(p_ctrl);


    p_ctrl->p_callback = p_cfg->p_callback;
    p_ctrl->p_context  = p_cfg->p_context;
    p_ctrl->event      = p_cfg->event;

    /** If callback parameter is not NULL configure the IRQ */
    if (NULL != p_cfg->p_callback)
    {
        NVIC_EnableIRQ(event_info.irq);
    }

    /** Mark driver as open by initializing it to "DOCO" in its ASCII equivalent. */
    p_ctrl->open = DOC_OPEN;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Close the module driver. Enable module stop mode. Implements doc_api_t::close.
 *
 * To close the DOC it must have been opened via the open API. When opened a control structure of type
 * doc_ctrl_t is passed to the open API. The same control structure must be passed to the close API.
 *
 * @retval SSP_SUCCESS          Module successfully closed.
 * @retval SSP_ERR_NOT_OPEN     Driver not open.
 * @retval SSP_ERR_ASSERTION    Pointer pointing to NULL.
 * @return                      See @ref Common_Error_Codes or functions called by this function for other possible
 *                              return codes. This function calls:
 *                                   * fmi_api_t::eventInfoGet
 *
 * @note This function is reentrant.
 * @note This function will disable the DOC interrupt in the NVIC.
 **********************************************************************************************************************/
ssp_err_t R_DOC_Close (doc_ctrl_t * const p_api_ctrl)
{
    doc_instance_ctrl_t * p_ctrl = (doc_instance_ctrl_t *) p_api_ctrl;

    /** Validate the parameter and check if the module is initialized */
#if DOC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    DOC_ERROR_RETURN(DOC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_DOC;

    /** Disable the IRQ in the NVIC in case it has been left enabled. */
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_DOC_INT, &event_info);
    if (SSP_INVALID_VECTOR != event_info.irq)
    {
        NVIC_DisableIRQ(event_info.irq);
    }

    /** Clear DOPCF in DOCR  */
    R_DOC_StatusClear(p_ctrl);

    if (SSP_INVALID_VECTOR != event_info.irq)
    {
        R_BSP_IrqStatusClear(event_info.irq);
        NVIC_ClearPendingIRQ(event_info.irq);
        NVIC_DisableIRQ(event_info.irq);
    }

    R_BSP_ModuleStop(&ssp_feature);

    /** Mark driver as closed.  */
    p_ctrl->open = 0u;

    /** Unlock the DOC Hardware Resource */
    R_BSP_HardwareUnlock(&ssp_feature);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Return the comparison/addition/subtraction status. Implements doc_api_t::statusGet.
 *
 * The status is read from the Data Operation Circuit Flag (DOPCF).
 *
 * @retval SSP_SUCCESS          Status successfully read.
 * @retval SSP_ERR_NOT_OPEN     Driver not open.
 * @retval SSP_ERR_ASSERTION    One or more pointers point to NULL.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_DOC_StatusGet (doc_ctrl_t * const p_api_ctrl, doc_status_t * p_status)
{
    doc_instance_ctrl_t * p_ctrl = (doc_instance_ctrl_t *) p_api_ctrl;

    /** Validate the parameters and check if the module is intialized */
#if DOC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_status);
    DOC_ERROR_RETURN(DOC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /* Cast to prevent compiler warning. doc_status_t contains enumerations of 0 and 1. */
    R_DOC_Type * p_doc_reg = (R_DOC_Type *) p_ctrl->p_reg;
    /**Read the comparison or addition or subtraction status from the register and store in the user supplied location */
    *p_status = (doc_status_t)HW_DOC_INTRead(p_doc_reg);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Clear the DOPCF status flag at the hardware layer. This flag indicates the result of a DOC operation.
 * Implements doc_api_t::statusClear.
 *
 * @retval SSP_SUCCESS          Interrupt successfully cleared.
 * @retval SSP_ERR_NOT_OPEN     Driver not open.
 * @retval SSP_ERR_ASSERTION    Pointer pointing to NULL.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_DOC_StatusClear (doc_ctrl_t * const p_api_ctrl)
{
    doc_instance_ctrl_t * p_ctrl = (doc_instance_ctrl_t *) p_api_ctrl;

    /** Validate the parameter and check if the module is initialized */
#if DOC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    DOC_ERROR_RETURN(DOC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_DOC_Type * p_doc_reg = (R_DOC_Type *) p_ctrl->p_reg;
    /** Clear the hardware status flag */
    HW_DOC_INTClear(p_doc_reg);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   Write to the DODIR and DODSR registers. Implements doc_api_t::write,
 *
 * In comparison mode the 16-bit reference data is written to the DODSR register and the data for the comparison
 * written to the DODIR.
 * In addition mode the initial data is written to the DODSR and the data to be added to the DODIR.
 * In subtraction mode the initial data is written to the DODSR and the data to be subtracted to the DODIR.
 *
 * When changing both the DODSR and DODIR the DODSR should be updated first.
 *
 * @retval SSP_SUCCESS          Values successfully written to the registers.
 * @retval SSP_ERR_NOT_OPEN     Driver not open.
 * @retval SSP_ERR_ASSERTION    One or more pointers point to NULL.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_DOC_Write (doc_ctrl_t * const p_api_ctrl, doc_data_t * const p_data)
{
    doc_instance_ctrl_t * p_ctrl = (doc_instance_ctrl_t *) p_api_ctrl;

    /** Validate the parameters and check if the module is initialized */
#if DOC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_data);
    DOC_ERROR_RETURN(DOC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_DOC_Type * p_doc_reg = (R_DOC_Type *) p_ctrl->p_reg;
    /** Writes the user supplied data to the DODIR(DOC Data Input Register) and DODSR(DOC Data Setting Register) registers */
    HW_DOC_Write(p_doc_reg, p_data);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   Write to the DODIR register. Implements doc_api_t::inputRegisterWrite,
 *
 * Writes to the DODIR register only.
 *
 * @retval SSP_SUCCESS          Value successfully written to the DODIR register.
 * @retval SSP_ERR_NOT_OPEN     Driver not open.
 * @retval SSP_ERR_ASSERTION    One or more pointers point to NULL.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_DOC_InputRegisterWrite (doc_ctrl_t * const p_api_ctrl, doc_size_t data)
{
    doc_instance_ctrl_t * p_ctrl = (doc_instance_ctrl_t *) p_api_ctrl;
    
    /** Validate the parameter and check if the module is intialized */
#if DOC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    DOC_ERROR_RETURN(DOC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_DOC_Type * p_doc_reg = (R_DOC_Type *) p_ctrl->p_reg;
    /** Writes the user supplied data to the DODIR register for data operation in Comparison, Addition and subtraction modes */
    HW_DOC_DODIRWrite(p_doc_reg, data);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Return DOC HAL driver version. Implements doc_api_t::versionGet.
 *
 * @retval SSP_SUCCESS          Version information successfully read.
 * @retval SSP_ERR_ASSERTION    Pointer pointing to NULL.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_DOC_VersionGet (ssp_version_t     * const p_version)
{
    /** Validate the parameter */
#if DOC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_version);
#endif

    /** Store the version id from version data structure to the user supplied location */
    p_version->version_id = g_doc_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup DOC)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  DOC ISR.
 *
 * Saves context if RTOS is used, clears interrupts, calls callback if one was provided in the open function
 * and restores context if RTOS is used.
 **********************************************************************************************************************/
void doc_int_isr (void)
{
    /* Save context if RTOS is used. */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    doc_instance_ctrl_t * p_ctrl = (doc_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

   /* Clear the IR flag in the ICU */
    R_BSP_IrqStatusClear (R_SSP_CurrentIrqGet());

    /* Call callback if provided */
    if ((NULL != p_ctrl) && (NULL != p_ctrl->p_callback))
    {
        doc_callback_args_t cb_data;

        /** Set data to identify callback to the user. */
        cb_data.event     = p_ctrl->event;
        cb_data.p_context = p_ctrl->p_context;

        /** Call the callback.  */
        p_ctrl->p_callback(&cb_data);
    }

    /* Restore context if RTOS is used. */
    SF_CONTEXT_RESTORE
}
