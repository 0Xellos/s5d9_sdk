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
 * File Name    : r_pdc.c
 * Description  : Parallel Data Capture (PDC) HAL API implementation.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_pdc_api.h"
#include "r_pdc_private.h"
#include "r_pdc_private_api.h"
#include "hw/hw_pdc_private.h"
#include "bsp_api.h"
#include "r_pdc_cfg.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** Macro for error logger. */
#ifndef PDC_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define PDC_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_pdc_version)
#endif

/** "PDC" in ASCII, used to determine if driver is open. */
#define PDC_OPEN                (0x00504443ULL)

#define PDC_TRANSFER_SIZE       4u  /* 32 bit transfer size from PCDR register */
#define PDC_TRANSFERS_PER_BLOCK 8u  /* PDC peripheral documentation, do 8 32bit transfers per PDC data ready interrupt
                                    **/
#define PDC_TIMEOUT             0xFFFFul
#define MAX_PIXEL_RESOLUTION    0x1000U

void pdc_frame_end_isr (void);
void pdc_int_isr (void);

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void      r_pdc_transfer_callback       (transfer_callback_args_t * p_args);
static void      r_pdc_error_handler           (pdc_instance_ctrl_t * p_ctrl);
static ssp_err_t pdc_open_assert_check         (pdc_ctrl_t * const p_ctrl,
                                                pdc_cfg_t const * const p_cfg);
static ssp_err_t pdc_capturestart_assert_check (pdc_instance_ctrl_t * const p_ctrl,
                                                uint8_t * const p_buffer);
static void      nvic_pdc_interrupts_enable    (pdc_instance_ctrl_t * const p_ctrl);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "pdc";
#endif

#if defined(__GNUC__)
/*LDRA_INSPECTED 119 S */
/* This structure is affected by warnings from the GCC compiler bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60784
 * This pragma suppresses the warnings in this structure only, and will be removed when the SSP compiler is updated to
 * v5.3.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_pdc_version =
{
    .api_version_minor  = PDC_API_VERSION_MINOR,
    .api_version_major  = PDC_API_VERSION_MAJOR,
    .code_version_major = PDC_CODE_VERSION_MAJOR,
    .code_version_minor = PDC_CODE_VERSION_MINOR,
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
/** PDC implementation of PDC Driver  */
/*LDRA_INSPECTED 27 D */
const pdc_api_t g_pdc_on_pdc =
{
    .open         = R_PDC_Open,
    .close        = R_PDC_Close,
    .captureStart = R_PDC_CaptureStart,
    .stateGet     = R_PDC_StateGet,
    .versionGet   = R_PDC_VersionGet,
};

/*******************************************************************************************************************//**
 * @addtogroup PDC
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Powers on PDC, handles required initialization described in the hardware manual. Implements pdc_api_t::open.
 *
 * The Open function provides initial configuration for the PDC module. It powers on the module and enables the PCLKO
 * output and the PIXCLK input. Further initialization requires the PIXCLK input to be running in order to be able to
 * reset the PDC as part of its initialization. This clock is input from a camera module and so the reset and further
 * initialization is performed in pdc_api_t::captureStart. This function should be called once prior to calling any
 * other PDC API functions. After the PDC is opened the Open function should not be called again without first
 * calling the Close function.
 *
 * @retval SSP_SUCCESS              Initialization was successful.
 * @retval SSP_ERR_ASSERTION        One of the following parameters is incorrect.  Either
 *                                  - p_cfg is NULL, OR
 *                                  - p_api_ctrl is NULL, OR
 *                                  - The pointer to the transfer interface in the p_cfg parameter is NULL
 *
 * @retval SSP_ERR_INVALID_ARGUMENT One of the following configuration parameters is incorrect.  Either
 *                                  - bytes_per_pixel is zero, OR
 *                                  - x_capture_pixels is zero, OR
 *                                  - y_capture_pixels is zero, OR
 *                                  - x_capture_start_pixel + x_capture_pixels is greater than 4095, OR
 *                                  - y_capture_start_pixel + y_capture_pixels is greater than 4095
 * @retval SSP_ERR_HW_LOCKED        Unable to reserve BSP hardware lock for this module.
 * @return                          See @ref Common_Error_Codes or functions called by this function for other possible
 *                                  return codes. This function calls:
 *                                  * fmi_api_t::productFeatureGet
 *                                  * fmi_api_t::eventInfoGet
 *
 *
 * @note This function is not reentrant.
 **********************************************************************************************************************/
ssp_err_t R_PDC_Open (pdc_ctrl_t * const p_api_ctrl, pdc_cfg_t const * const p_cfg)
{
    pdc_instance_ctrl_t * p_ctrl = (pdc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err;

    SSP_PARAMETER_NOT_USED(g_pdc_version);

#if (1 == PDC_CFG_PARAM_CHECKING_ENABLE)
    err = pdc_open_assert_check(p_ctrl, p_cfg);
    PDC_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif /* if (1 == PDC_CFG_PARAM_CHECKING_ENABLE) */

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_PDC;
    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    PDC_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Lock the PDC Hardware Resource */
    err = R_BSP_HardwareLock(&ssp_feature);
    PDC_ERROR_RETURN(err == SSP_SUCCESS, SSP_ERR_HW_LOCKED);

    p_ctrl->p_reg = (R_PDC_Type *) info.ptr;

    ssp_vector_info_t * p_vector_info;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    err = g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_PDC_INT, &event_info);
    p_ctrl->irq = event_info.irq;
    if (SSP_INVALID_VECTOR != p_ctrl->irq)
    {
        R_SSP_VectorInfoGet(p_ctrl->irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->irq, p_cfg->irq_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_PDC_FRAME_END, &event_info);
    p_ctrl->frame_end_irq = event_info.irq;
    if (SSP_INVALID_VECTOR != p_ctrl->frame_end_irq)
    {
        R_SSP_VectorInfoGet(p_ctrl->frame_end_irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->frame_end_irq, p_cfg->frame_end_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }

    p_ctrl->bytes_per_pixel       = p_cfg->bytes_per_pixel;
    p_ctrl->p_current_buffer      = p_cfg->p_buffer;
    p_ctrl->x_capture_pixels      = p_cfg->x_capture_pixels;
    p_ctrl->x_capture_start_pixel = p_cfg->x_capture_start_pixel;
    p_ctrl->y_capture_pixels      = p_cfg->y_capture_pixels;
    p_ctrl->y_capture_start_pixel = p_cfg->y_capture_start_pixel;
    p_ctrl->hsync_polarity        = p_cfg->hsync_polarity;
    p_ctrl->vsync_polarity        = p_cfg->vsync_polarity;
    p_ctrl->endian                = p_cfg->endian;
    p_ctrl->p_lower_lvl_transfer  = p_cfg->p_lower_lvl_transfer;
    p_ctrl->transfer_in_progress  = false;
    p_ctrl->p_callback            = p_cfg->p_callback;

    /** Disable module stop mode for PDC */
    R_BSP_ModuleStart(&ssp_feature);

    /* Disable PDC */
    HW_PDC_Disable(p_ctrl->p_reg);

    /** Set PCLKB divider */
    HW_PDC_DividerSet(p_ctrl->p_reg, p_cfg->clock_division);

    /** Enable PCLKO output */
    HW_PDC_PCKOEnable(p_ctrl->p_reg);

    /** Enable the PIXCLK input */
    HW_PDC_PixclkEnable(p_ctrl->p_reg);

    /** Mark driver as open by initializing it to "PDC" in its ASCII equivalent */
    p_ctrl->open = PDC_OPEN;

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_PDC_Open
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Stops and closes the transfer interface, disables the PDC, powers off the PDC, clears internal driver data
 * and disables interrupts. Implements pdc_api_t::close.
 *
 * @retval SSP_SUCCESS           Successful close.
 * @retval SSP_ERR_ASSERTION     One of the following parameters is incorrect.  Either
 *                                 - p_api_ctrl is NULL, OR
 *                                 - low level transfer is not assigned, OR
 *                                 - low level transfer APIs are not assigned
 * @retval SSP_ERR_NOT_OPEN      Open has not been successfully called.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                  * transfer_api_t::disable
 *                                  * transfer_api_t::close
 * @note This API will close the PDC driver. If a capture is in progress it will be stopped. This function is
 * reentrant.
 **********************************************************************************************************************/
ssp_err_t R_PDC_Close (pdc_ctrl_t * const p_api_ctrl)
{
    pdc_instance_ctrl_t * p_ctrl = (pdc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err;

#if (1 == PDC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_ctrl->p_lower_lvl_transfer);
    SSP_ASSERT(NULL != p_ctrl->p_lower_lvl_transfer->p_api);
#endif

    /* Check driver is open */
    PDC_ERROR_RETURN((PDC_OPEN == p_ctrl->open), SSP_ERR_NOT_OPEN);

    /** Disable all interrupts. */
    HW_PDC_InterruptSet(p_ctrl->p_reg, PDC_INTERRUPT_NONE);

    if (SSP_INVALID_VECTOR != p_ctrl->frame_end_irq)
    {
        NVIC_DisableIRQ(p_ctrl->frame_end_irq);
    }
    if (SSP_INVALID_VECTOR != p_ctrl->irq)
    {
        NVIC_DisableIRQ(p_ctrl->irq);
    }

    /* Stop any transfer in progress. */
    err = p_ctrl->p_lower_lvl_transfer->p_api->disable(p_ctrl->p_lower_lvl_transfer->p_ctrl);
    if (err == SSP_SUCCESS)
    {
        /* Transfer stopped. Close the interface. */
        err = p_ctrl->p_lower_lvl_transfer->p_api->close(p_ctrl->p_lower_lvl_transfer->p_ctrl);
    }
    else
    {
        /* Either an assertion has occurred due to an incorrect parameter or the interface is already closed. If the
         * interface is already closed this is not treated as an error. */
        if (SSP_ERR_NOT_OPEN == err)
        {
            err = SSP_SUCCESS;
        }
    }

    /* Disable PDC */
    HW_PDC_Disable(p_ctrl->p_reg);

    /** Enable module stop mode for PDC */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_PDC;
    R_BSP_ModuleStop(&ssp_feature);

    /* Mark driver as closed */
    p_ctrl->open = 0U;

    /* Mark transfer as not in progress */
    p_ctrl->transfer_in_progress = false;

    /** Unlock the PDC Hardware Resource */
    R_BSP_HardwareUnlock(&ssp_feature);

    return err;
}

/******************************************************************************
 * End of function R_PDC_Close
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Starts a capture. Enables interrupts. Implements pdc_api_t::captureStart.
 *
 * Sets up the transfer interface to transfer data from the PDC into the specified buffer. Configures the PDC settings
 * as previously set by the pdc_api_t::open API. These settings are configured here as the PIXCLK input must be active
 * for the PDC reset operation.
 * When a capture is complete the callback registered during pdc_api_t::open API call will be called.
 *
 * @retval SSP_SUCCESS           Capture start successful.
 * @retval SSP_ERR_ASSERTION     One of the following parameters is incorrect.  Either
 *                                 - p_api_ctrl is NULL, OR
 *                                 - low level transfer is not assigned, OR
 *                                 - low level transfer APIs are not assigned
 *                                 - buffer is not assigned, assign buffer
 * @retval SSP_ERR_NOT_OPEN      Open has not been successfully called.
 * @retval SSP_ERR_IN_USE        Pdc transfer is already in progress, wait for transfer to complete.
 * @retval SSP_ERR_TIMEOUT       Reset operation timed out.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                  * transfer_api_t::open
 *                                  * transfer_api_t::enable
 *
 * @note If the PIXCLK is being generated by a camera module the camera must be configured after the call to
 * pdc_api_t::open and before the call to pdc_api_t::captureStart.
 * This function is not reentrant.
 *
 * The user is responsible to ensuring that the memory pointed to by p_buffer is both valid and large enough to store
 * a complete image. The amount of space required, in bytes can be calculated as shown:
 *
 * size (bytes) = image width (pixels) * image height (lines) * number of bytes per pixel
 **********************************************************************************************************************/
ssp_err_t R_PDC_CaptureStart (pdc_ctrl_t * const p_api_ctrl, uint8_t * const p_buffer)
{
    pdc_instance_ctrl_t * p_ctrl = (pdc_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;
#if (1 == PDC_CFG_PARAM_CHECKING_ENABLE)
    err = pdc_capturestart_assert_check (p_ctrl, p_buffer);
    PDC_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    transfer_cfg_t    pdc_transfer_cfg;
    uint32_t timeout     = PDC_TIMEOUT;
    uint32_t reset_state = 0U;
    uint32_t num_blocks  = 0U;
    uint32_t interrupt_setting = 0U;

    /* Check driver is open */
    PDC_ERROR_RETURN((PDC_OPEN == p_ctrl->open), SSP_ERR_NOT_OPEN);

    /* Check if a transfer is already in progress */
    PDC_ERROR_RETURN((p_ctrl->transfer_in_progress == false), SSP_ERR_IN_USE);

    /** Set up transfer interface */
    p_ctrl->p_lower_lvl_transfer->p_cfg->p_info->chain_mode     = TRANSFER_CHAIN_MODE_DISABLED;
    p_ctrl->p_lower_lvl_transfer->p_cfg->p_info->dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
    p_ctrl->p_lower_lvl_transfer->p_cfg->p_info->irq            = TRANSFER_IRQ_END;
    p_ctrl->p_lower_lvl_transfer->p_cfg->p_info->length         = 8U;
    p_ctrl->p_lower_lvl_transfer->p_cfg->p_info->mode           = TRANSFER_MODE_BLOCK;

    if (NULL != p_buffer)
    {
        p_ctrl->p_current_buffer = p_buffer;
    }

    /** Configure the transfer interface  */
    p_ctrl->p_lower_lvl_transfer->p_cfg->p_info->p_dest           = p_ctrl->p_current_buffer;
    p_ctrl->p_lower_lvl_transfer->p_cfg->p_info->p_src            = (void const *) &R_PDC->PCDR;
    p_ctrl->p_lower_lvl_transfer->p_cfg->p_info->repeat_area      = TRANSFER_REPEAT_AREA_SOURCE;
    p_ctrl->p_lower_lvl_transfer->p_cfg->p_info->size             = TRANSFER_SIZE_4_BYTE;
    p_ctrl->p_lower_lvl_transfer->p_cfg->p_info->src_addr_mode    = TRANSFER_ADDR_MODE_FIXED;

    num_blocks                                                    = (uint32_t) p_ctrl->x_capture_pixels;
    num_blocks                                                   *= p_ctrl->bytes_per_pixel;
    num_blocks                                                   *= p_ctrl->y_capture_pixels;
    num_blocks                                                   /= PDC_TRANSFER_SIZE;
    num_blocks                                                   /= PDC_TRANSFERS_PER_BLOCK;
    p_ctrl->p_lower_lvl_transfer->p_cfg->p_info->num_blocks       = (uint16_t) num_blocks;

    pdc_transfer_cfg.p_info            = p_ctrl->p_lower_lvl_transfer->p_cfg->p_info;
    pdc_transfer_cfg.activation_source = ELC_EVENT_PDC_RECEIVE_DATA_READY;
    pdc_transfer_cfg.auto_enable       = true;
    pdc_transfer_cfg.p_context         = p_ctrl;
    pdc_transfer_cfg.p_callback        = r_pdc_transfer_callback;
    pdc_transfer_cfg.p_extend          = p_ctrl->p_lower_lvl_transfer->p_cfg->p_extend;
    pdc_transfer_cfg.irq_ipl           = p_ctrl->p_lower_lvl_transfer->p_cfg->irq_ipl;

    /** Open transfer interface */
    err = p_ctrl->p_lower_lvl_transfer->p_api->open(p_ctrl->p_lower_lvl_transfer->p_ctrl, &pdc_transfer_cfg);
    PDC_ERROR_RETURN((err == SSP_SUCCESS), err);

    /* Mark transfer as in progress */
    p_ctrl->transfer_in_progress = true;

    /* Reset the PDC */
    HW_PDC_Reset(p_ctrl->p_reg);

    /** Wait for reset to complete */
    timeout     = PDC_TIMEOUT;
    reset_state = HW_PDC_GetResetState(p_ctrl->p_reg);
    while ((timeout > 0u) && (1u == reset_state))
    {
        reset_state = HW_PDC_GetResetState(p_ctrl->p_reg);
        timeout--;
    }
    if (0u == timeout)
    {
        return SSP_ERR_TIMEOUT;
    }
    /** Set horizontal capture range */
    HW_PDC_HSTSet(p_ctrl->p_reg, (uint32_t) (p_ctrl->x_capture_start_pixel * p_ctrl->bytes_per_pixel));

    /** Set horizontal capture size */
    HW_PDC_HSZSet(p_ctrl->p_reg, (uint32_t) (p_ctrl->x_capture_pixels * p_ctrl->bytes_per_pixel));

    /** Set vertical capture range */
    HW_PDC_VSTSet(p_ctrl->p_reg, p_ctrl->y_capture_start_pixel);

    /** Set vertical capture size */
    HW_PDC_VSZSet(p_ctrl->p_reg, p_ctrl->y_capture_pixels);

    /** Set VSYNC polarity */
    HW_PDC_VPSSet(p_ctrl->p_reg, p_ctrl->vsync_polarity);

    /** Set HSYNC polarity */
    HW_PDC_HPSSet(p_ctrl->p_reg, p_ctrl->hsync_polarity);

    /** Set endianess of capture data */
    HW_PDC_EndianSet(p_ctrl->p_reg, p_ctrl->endian);

    /** Enable interrupts:
     *  Receive data ready interrupt,
     *  Underrun interrupt,
     *  Overrun interrupt,
     *  Frame end interrupt,
     *  Vertical line number setting error interrupt,
     *  Horizontal byte number setting error interrupt */
    interrupt_setting = (uint32_t)( PDC_INTERRUPT_DFIE | PDC_INTERRUPT_UDRIE | PDC_INTERRUPT_OVIE |
            PDC_INTERRUPT_FEIE | PDC_INTERRUPT_VERIE | PDC_INTERRUPT_HERIE);
    HW_PDC_InterruptSet(p_ctrl->p_reg, interrupt_setting);

    nvic_pdc_interrupts_enable(p_ctrl);

    err = p_ctrl->p_lower_lvl_transfer->p_api->enable(p_ctrl->p_lower_lvl_transfer->p_ctrl);
    PDC_ERROR_RETURN((err == SSP_SUCCESS), err);

    /* Set PCCR1.PCE as 1 */
    HW_PDC_Enable(p_ctrl->p_reg);

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_PDC_CaptureStart
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Returns the state of the VSYNC and HSYNC pins.Implements pdc_api_t::stateGet.
 *
 * @retval SSP_SUCCESS           State read successful.
 * @retval SSP_ERR_ASSERTION     p_api_ctrl is NULL OR p_state is NULL
 * @retval SSP_ERR_NOT_OPEN      Open has not been successfully called.
 *
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_PDC_StateGet (pdc_ctrl_t * const p_api_ctrl, pdc_state_t * p_state)
{
    pdc_instance_ctrl_t * p_ctrl = (pdc_instance_ctrl_t *) p_api_ctrl;

#if (1 == PDC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_state);
#endif

    uint32_t pcmonr = 0;

    /** Check if the driver is open */
    PDC_ERROR_RETURN((PDC_OPEN == p_ctrl->open), SSP_ERR_NOT_OPEN);

    /** Get the contents of PCMONR register */
    pcmonr = HW_PDC_PCMONRGet(p_ctrl->p_reg);

    /** Update vsync signal state */
    if (0UL == (PDC_SIGNAL_STATUS_FLAGS_VSYNC & pcmonr))
    {
        p_state->vsync = PDC_VSYNC_STATE_LOW;
    }
    else
    {
        p_state->vsync = PDC_VSYNC_STATE_HIGH;
    }

    /** Update hsync signal state */
    if (0UL == (PDC_SIGNAL_STATUS_FLAGS_HSYNC & pcmonr))
    {
        p_state->hsync = PDC_HSYNC_STATE_LOW;
    }
    else
    {
        p_state->hsync = PDC_HSYNC_STATE_HIGH;
    }

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_PDC_StateGet
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief Return PDC HAL driver version. Implements pdc_api_t::versionGet.
 *
 * @retval      SSP_SUCCESS             Version information successfully read.
 * @retval      SSP_ERR_ASSERTION       Null pointer passed as a parameter
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_PDC_VersionGet (ssp_version_t * const p_data)
{
#if (1 == PDC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_data != NULL);
#endif

    p_data->version_id = g_pdc_version.version_id;

    return SSP_SUCCESS;
}

/******************************************************************************
 * End of function R_PDC_VersionGet
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @} (end addtogroup PDC)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Internal transfer complete callback for PDC driver..
 *
 *
 * @param[in]      p_args Transfer interface callback arguments.
 **********************************************************************************************************************/
static void r_pdc_transfer_callback (transfer_callback_args_t * p_args)
{
    pdc_instance_ctrl_t * p_ctrl = (pdc_instance_ctrl_t *) p_args->p_context;
    pdc_callback_args_t pdc_args;

    if (NULL != p_ctrl)
    {
        p_ctrl->transfer_in_progress = false;
        p_ctrl->p_lower_lvl_transfer->p_api->close(p_ctrl->p_lower_lvl_transfer->p_ctrl);
        if (NULL != p_ctrl->p_callback)
        {
            pdc_args.p_context = p_ctrl;
            pdc_args.event     = PDC_EVENT_TRANSFER_COMPLETE;
            pdc_args.p_buffer  = p_ctrl->p_current_buffer;
            p_ctrl->p_callback(&pdc_args);
        }
    }
}

/******************************************************************************
 * End of function r_pdc_transfer_callback
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  PDC frame end ISR.
 **********************************************************************************************************************/
void pdc_frame_end_isr (void)
{
    uint16_t timeout = 0xFFFFu;

    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    pdc_instance_ctrl_t * p_ctrl = (pdc_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    timeout = 0xFFFFu;
    while ((0UL == (PDC_STATUS_FLAGS_FEMPF & HW_PDC_StatusGet(p_ctrl->p_reg))) && (0UL != timeout) &&
            (0UL == (PDC_STATUS_FLAGS_UDRF & HW_PDC_StatusGet(p_ctrl->p_reg))))
    {
        /* Wait for FIFO empty flag to be set and DTC/DMAC to transfer last data block.*/
        timeout--;
    }

    if (0UL == (PDC_STATUS_FLAGS_UDRF & HW_PDC_StatusGet(p_ctrl->p_reg)))
    {
        /* No underrun error */

        /* Stop the PDC */
        HW_PDC_Disable(p_ctrl->p_reg);

        /* Clear FEF frame end flag */
        HW_PDC_StatusClear(p_ctrl->p_reg,PDC_STATUS_FLAGS_FEF);

        R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

        /* Disable interrupts */
        NVIC_DisableIRQ(R_SSP_CurrentIrqGet());
        NVIC_DisableIRQ(p_ctrl->irq);
    }
    else
    {
        /* Underrun error has occurred */

        /* Clear FEF frame end flag */
        HW_PDC_StatusClear(p_ctrl->p_reg,PDC_STATUS_FLAGS_FEF);

        /* Call the error handler */
        r_pdc_error_handler(p_ctrl);

        R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());
        NVIC_DisableIRQ(R_SSP_CurrentIrqGet());
    }

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

/******************************************************************************
 * End of function pdc_frame_end_isr
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  PDC error ISR
 **********************************************************************************************************************/
void pdc_int_isr (void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    pdc_instance_ctrl_t * p_ctrl = (pdc_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /* Call the error handler */
    r_pdc_error_handler(p_ctrl);

    /* Check for underrun interrupt after error handling */
    if ((uint32_t)PDC_STATUS_FLAGS_UDRF == (HW_PDC_StatusGet(p_ctrl->p_reg) & PDC_STATUS_FLAGS_UDRF))
    {
        r_pdc_error_handler(p_ctrl);
    }

    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    /* Disable interrupt */
    NVIC_DisableIRQ(R_SSP_CurrentIrqGet());

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}
/******************************************************************************
 * End of function pdc_int_isr
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  PDC error handler
 * @param[in]   p_ctrl   Pointer to PDC Specific Control Structure
 **********************************************************************************************************************/
static void r_pdc_error_handler (pdc_instance_ctrl_t * p_ctrl)
{
    pdc_callback_args_t pdc_args;
    uint32_t            pdc_status;
    uint32_t            event = 0;

    /* Stop the PDC */
    HW_PDC_Disable(p_ctrl->p_reg);

    /* Disable the transfer */
    p_ctrl->p_lower_lvl_transfer->p_api->disable(p_ctrl->p_lower_lvl_transfer->p_ctrl);

    if (NULL != p_ctrl->p_callback)
    {
        /* Get PDC status flags */
        pdc_status     = HW_PDC_StatusGet(p_ctrl->p_reg);
        if ((uint32_t)PDC_STATUS_FLAGS_OVRF == (pdc_status & PDC_STATUS_FLAGS_OVRF))
        {
            event |= (uint32_t)PDC_EVENT_ERR_OVERRUN;
            HW_PDC_StatusClear(p_ctrl->p_reg, PDC_STATUS_FLAGS_OVRF);
        }

        if ((uint32_t)PDC_STATUS_FLAGS_UDRF == (pdc_status & PDC_STATUS_FLAGS_UDRF))
        {
            event |= (uint32_t)PDC_EVENT_ERR_UNDERRUN;
            HW_PDC_StatusClear(p_ctrl->p_reg, PDC_STATUS_FLAGS_UDRF);
        }

        if ((uint32_t)PDC_STATUS_FLAGS_VERF == (pdc_status & PDC_STATUS_FLAGS_VERF))
        {
            event |= (uint32_t)PDC_EVENT_ERR_V_SET;
            HW_PDC_StatusClear(p_ctrl->p_reg, PDC_STATUS_FLAGS_VERF);
        }

        if ((uint32_t)PDC_STATUS_FLAGS_HERF == (pdc_status & PDC_STATUS_FLAGS_HERF))
        {
            event |= (uint32_t)PDC_EVENT_ERR_H_SET;
            HW_PDC_StatusClear(p_ctrl->p_reg, PDC_STATUS_FLAGS_HERF);
        }

        pdc_args.event     = (pdc_event_t)event;
        pdc_args.p_context = p_ctrl->p_context;
        p_ctrl->p_callback(&pdc_args);
    }
}
/******************************************************************************
 * End of function r_pdc_error_handler
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Checks for any input parameter error.
 *
 * @param[in]   p_ctrl   Pointer to PDC Control Structure
 * @param[in]   p_cfg    Pointer to PDC Configuration Structure
 *
 * @retval SSP_SUCCESS              Input Parameters are Valid.
 * @retval SSP_ERR_ASSERTION        One of the following parameters is incorrect.  Either
 *                                  - p_ctrl is NULL, OR
 *                                  - p_cfg is NULL, OR
 *                                  - low level transfer is not assigned
 * @retval SSP_ERR_INVALID_ARGUMENT One of the following configuration parameters is incorrect.  Either
 *                                  - bytes_per_pixel is zero, OR
 *                                  - x_capture_pixels is zero, OR
 *                                  - y_capture_pixels is zero, OR
 *                                  - x_capture_start_pixel + x_capture_pixels is greater than 4095, OR
 *                                  - y_capture_start_pixel + y_capture_pixels is greater than 4095
 **********************************************************************************************************************/
static ssp_err_t pdc_open_assert_check(pdc_ctrl_t * const p_ctrl,
                                       pdc_cfg_t const * const p_cfg)
{
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_cfg->p_lower_lvl_transfer);
    PDC_ERROR_RETURN((p_cfg->bytes_per_pixel  != 0UL), SSP_ERR_INVALID_ARGUMENT);
    PDC_ERROR_RETURN((p_cfg->x_capture_pixels != (uint16_t)0UL), SSP_ERR_INVALID_ARGUMENT);
    PDC_ERROR_RETURN((p_cfg->y_capture_pixels != (uint16_t)0UL), SSP_ERR_INVALID_ARGUMENT);
    PDC_ERROR_RETURN(((p_cfg->y_capture_start_pixel + p_cfg->y_capture_pixels) < (uint16_t)MAX_PIXEL_RESOLUTION),
                       SSP_ERR_INVALID_ARGUMENT);
    PDC_ERROR_RETURN(((p_cfg->x_capture_start_pixel + p_cfg->x_capture_pixels) < (uint16_t)MAX_PIXEL_RESOLUTION),
                       SSP_ERR_INVALID_ARGUMENT);

    return SSP_SUCCESS;
} /* End of function pdc_open_assert_check */
/******************************************************************************
 * End of function pdc_open_assert_check
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Checks for any input parameter error.
 *
 * @param[in]   p_ctrl   Pointer to PDC Specific Control Structure
 * @param[in]   p_buffer   Pointer to buffer
 *
 * @retval SSP_SUCCESS              Input Parameters are Valid.
 * @retval SSP_ERR_ASSERTION       One of the following parameters is incorrect.  Either
 *                                 - p_ctrl is NULL, OR
 *                                 - low level transfer is not assigned, OR
 *                                 - low level transfer APIs are not assigned
 **********************************************************************************************************************/
static ssp_err_t pdc_capturestart_assert_check(pdc_instance_ctrl_t * const p_ctrl,
                                               uint8_t * const p_buffer)
{
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_ctrl->p_lower_lvl_transfer);
    SSP_ASSERT(NULL != p_ctrl->p_lower_lvl_transfer->p_api);

    if (NULL == p_buffer)
    {
        SSP_ASSERT(NULL != p_ctrl->p_current_buffer);
    }

    return SSP_SUCCESS;
} /* End of function pdc_capturestart_assert_check */
/******************************************************************************
 * End of function pdc_capturestart_assert_check
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Sets the NVIC vector table for IRQ.
 * @param[in]   p_ctrl   Pointer to PDC Specific Control Structure
 **********************************************************************************************************************/
static void nvic_pdc_interrupts_enable(pdc_instance_ctrl_t * const p_ctrl)
{
    if (SSP_INVALID_VECTOR != p_ctrl->frame_end_irq)
    {
        NVIC_EnableIRQ(p_ctrl->frame_end_irq);
    }
    if (SSP_INVALID_VECTOR != p_ctrl->irq)
    {
        NVIC_EnableIRQ(p_ctrl->irq);
    }

} /* End of function nvic_pdc_interrupts_enable */
/******************************************************************************
 * End of function nvic_pdc_interrupts_enable
 ******************************************************************************/
