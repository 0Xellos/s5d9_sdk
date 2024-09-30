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
 * File Name    : r_pdc.h
 * Description  : PDC HAL driver for PDC interface
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup PDC PDC
 * @brief Driver for the Parallel Data Capture Unit (PDC).
 *
 * @section PDC_SUMMARY Summary
 * extends @ref PDC_API
 * @brief The PDC interface allows the capturing of an image from a camera module.
 * @{
 **********************************************************************************************************************/

#ifndef R_PDC_H
#define R_PDC_H

#include "r_pdc_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define PDC_CODE_VERSION_MAJOR (2U)
#define PDC_CODE_VERSION_MINOR (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** PDC instance control block. DO NOT INITIALIZE.  Initialization occurs when pdc_api_t::open is called. */
typedef struct st_pdc_instance_ctrl
{
    R_PDC_Type                * p_reg;                  ///< Pointer to PDC base address
    uint32_t                    open;                   ///< Indicates whether or not the driver is open
                                                        ///< called.
    uint8_t                     bytes_per_pixel;        ///< Number of bytes per pixel
    uint16_t                    x_resolution_pixels;    ///< Total number of horizontal pixels input to PDC
    uint16_t                    y_resolution_pixels;    ///< Total number of lines input to the PDC
    uint16_t                    x_capture_start_pixel;  ///< Horizontal position to start capture
    uint16_t                    x_capture_pixels;       ///< Number of horizontal pixels to capture
    uint16_t                    y_capture_start_pixel;  ///< Vertical position to start capture
    uint16_t                    y_capture_pixels;       ///< Number of vertical lines/pixels to capture
    pdc_endian_t                endian;                 ///< Endian of capture data
    pdc_hsync_polarity_t        hsync_polarity;         ///< Polarity of HSYNC input
    pdc_vsync_polarity_t        vsync_polarity;         ///< Polarity of VSYNC input
    uint8_t                   * p_current_buffer;       ///< Pointer to buffer currently in use
    bool                        transfer_in_progress;   ///< Indicates if a PDC transfer is already in progress
    transfer_instance_t const * p_lower_lvl_transfer;   ///< Pointer to the transfer instance the PDC should use
    transfer_info_t             info_transfer;          ///< Transfer info structure for low level Transfer interface
    void const                * p_context;              ///< Placeholder for user data.  Passed to the user callback in
                                                        ///< ::pdc_callback_args_t.
    void (* p_callback)(pdc_callback_args_t * p_args);  ///< Callback provided when a PDC transfer ISR occurs.
    IRQn_Type                   frame_end_irq;          ///< Frame end interrupt number
    IRQn_Type                   irq;                    ///< PDC interrupt number
} pdc_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const pdc_api_t g_pdc_on_pdc;
/** @endcond */

/**********************************************************************************************************************
 * Function Prototypes
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_PDC_H

/*******************************************************************************************************************//**
 * @} (end addtogroup R_PDC)
 **********************************************************************************************************************/
