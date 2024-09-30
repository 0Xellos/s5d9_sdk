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
* File Name    : r_slcdc.h
* Description  : Segment LCD Controller (SLCDC) Module instance header file.
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup SLCDC SLCDC
 * @brief Driver for the Segment LCD Controller (SLCDC).
 *
 * @section SLCDC_SUMMARY Summary
 * Extends @ref SLCDC_API.
 * @{
 **********************************************************************************************************************/

#ifndef R_SLCDC_H
#define R_SLCDC_H

#include "bsp_api.h"
#include "r_slcdc_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/** SLCDC control block. DO NOT INITIALIZE.  Initialization occurs when slcdc_api_t::open is called */
typedef struct st_slcdc_instance_ctrl
{
    slcdc_display_state_t  state;                         ///< Status of SLCD module
    slcdc_cfg_t            info;                          ///< SLCDC config info
    void const           * p_context;                     ///< Pointer to the higher level device context
    R_LCD_Type           * p_reg;                         ///< Pointer to register base address
} slcdc_instance_ctrl_t;

/**********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
extern const slcdc_api_t g_slcdc_on_slcdc;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_SLCDC_H

/*******************************************************************************************************************//**
 * @} (end addtogroup SLCDC)
***********************************************************************************************************************/
