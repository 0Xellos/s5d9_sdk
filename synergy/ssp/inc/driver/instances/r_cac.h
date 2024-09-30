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
* File Name    : r_cac.h
* Description  : Clock Accuracy Circuit (CAC) Module instance header file.
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @file r_cac.h
 **********************************************************************************************************************/

#ifndef R_CAC_H
#define R_CAC_H

#include "bsp_api.h"
#include "r_cac_cfg.h"
#include "r_cac_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup CAC CAC
 * @brief Driver for the Clock Frequency Accuracy Measurement Circuit (CAC).
 *
 * @section CAC_HAL_Library_SUMMARY Summary
 * This module supports the CAC peripheral.
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define CAC_CODE_VERSION_MAJOR   (2U)
#define CAC_CODE_VERSION_MINOR   (0U)

/**********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const cac_api_t g_cac_on_cac;
/** @endcond */

/** CAC instance control block.  DO NOT INITIALIZE. */
typedef struct st_cac_instance_ctrl
{
    void                  * p_reg;                      ///< Pointer to register base address
    void                 (* p_callback)(cac_callback_args_t * cb_data); ///< Called from the ISR.
    void const            * p_context;                  ///< Passed to the callback.
    IRQn_Type               frequency_error_irq;        ///< Frequency error IRQ number
    IRQn_Type               measurement_end_irq;        ///< Measurement end IRQ number
    IRQn_Type               overflow_irq;               ///< Overflow IRQ number
    uint32_t                cac_api_open;         ///< Set to "CAC" once API has been successfully opened.
    bool                    cac_continous_mode;   ///< Set as a result of the Open() call
    bsp_lock_t              cac_lock;             ///< CAC commands software lock
    cac_clock_source_t      measurement_clock;    ///< Clock specified in Open() as the measurement clock
    cac_clock_source_t      reference_clock;      ///< Clock specified in Open() as the reference clock
    void const            * p_extend;
} cac_instance_ctrl_t;

/*******************************************************************************************************************//**
 * @} (end defgroup CAC)
***********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_CAC_H


