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
* File Name    : r_acmphs.h
* Description  : High speed analog comparator instance header file.
***********************************************************************************************************************/

#ifndef R_ACMPHS_H
#define R_ACMPHS_H

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup ACMPHS High-Speed Analog Comparator
 * @brief Driver for the High-Speed Analog Comparator.
 *
 * @section ACMPHS_SUMMARY Summary
 * Extends @ref COMPARATOR_API.
 *
 * This module implements the @ref COMPARATOR_API using the high-speed analog comparator.
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"
#include "r_comparator_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define ACMPHS_CODE_VERSION_MAJOR   (2U)
#define ACMPHS_CODE_VERSION_MINOR   (0U)

/*********************************************************************************************************************
 * Typedef definitions
 *********************************************************************************************************************/
/** Channel instance control block. DO NOT INITIALIZE.  Initialization occurs in comparator_api_t::open. */
typedef struct st_acmphs_instance_ctrl
{
    uint32_t         open;    // Used to determine if channel control block is in use
    R_ACMPHS0_Type * p_reg;   // Pointer to register base address

    comparator_pin_output_t pin_output; // Whether to include output on output pin

    /* Callback provided when a comparator interrupt occurs.. */
    void      (* p_callback)(comparator_callback_args_t * p_args);

    /* Placeholder for user data.  Passed to the user callback in ::comparator_callback_args_t. */
    void const * p_context;

    IRQn_Type    irq;     // NVIC interrupt number
    uint8_t      channel; // Channel
} acmphs_instance_ctrl_t;

/**********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const comparator_api_t g_comparator_on_acmphs;
/** @endcond */

/*******************************************************************************************************************//**
 * @} (end defgroup ACMPHS)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_ACMPHS_H
