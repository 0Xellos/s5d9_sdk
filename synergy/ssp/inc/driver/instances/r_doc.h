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
 * File Name    : r_doc.h
 * Description  : Prototypes of Data Operation Circuit (DOC) functions used to implement the DOC interface.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup DOC DOC
 * @brief Driver for the Data Operation Circuit (DOC).
 *
 * @section DOC_SUMMARY Summary
 * This module implements the @ref DOC_API using the Data Operation Circuit (DOC).
 * @{
 **********************************************************************************************************************/

#ifndef R_DOC_H
#define R_DOC_H

#include "bsp_api.h"
#include "r_doc_api.h"
#include "r_doc_cfg.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define DOC_CODE_VERSION_MAJOR (2U)
#define DOC_CODE_VERSION_MINOR (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** DOC instance control block. Do not initialize. Initialization occurs when the doc_api_t::open function is called. */
typedef struct st_doc_instance_ctrl
{
    uint32_t  open;                                 ///< Used by driver to check if the control structure is valid

    /** Callback provided when a DOC ISR occurs. NULL indicates no CPU interrupt.  */
    void (* p_callback)(doc_callback_args_t * p_args);

    doc_event_t  event;                             ///< The event DOC is configured for. Passed in ISR callback.

    /** Placeholder for user data. Passed to the user callback in ::doc_callback_args_t. */
    void const * p_context;
    void       * p_reg;                             ///< Base register
} doc_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const doc_api_t g_doc_on_doc;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_DOC_H

/*******************************************************************************************************************//**
 * @} (end addtogroup DOC)
 **********************************************************************************************************************/
