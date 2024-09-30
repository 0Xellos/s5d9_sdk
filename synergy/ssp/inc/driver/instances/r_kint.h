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
* File Name    : r_kint.h
* Description  : Key Matrix Interrupt (KINT) Module instance header file.
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup KINT Key Interrupts
 *
 * @brief Driver for the Key Interrupt Function
 *
 * The Key input driver can be used for one to eight channels or in a matrix format.
 * This module implements the following interface: @ref KEYMATRIX_API.
 *
 * @{
 **********************************************************************************************************************/

#ifndef R_KINT_H
#define R_KINT_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"
#include "r_keymatrix_api.h"
#include "r_kint_cfg.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define KINT_CODE_VERSION_MAJOR   (2U)
#define KINT_CODE_VERSION_MINOR   (0U)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/** Channel instance control block. DO NOT INITIALIZE. Initialization occurs when keymatrix_api_t::open is called. */
typedef struct st_kint_instance_ctrl
{
    R_KINT_Type         * p_reg;      ///< Pointer to register base address
    keymatrix_channels_t  channels;   ///< Channel bitmask
    IRQn_Type             irq;        ///< Interrupt priority number
    uint32_t              open;	      ///< Flag to determine if the device is open
} kint_instance_ctrl_t;

/**********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const keymatrix_api_t g_keymatrix_on_kint;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_KINT_H

/*******************************************************************************************************************//**
 * @} (end defgroup KINT)
***********************************************************************************************************************/