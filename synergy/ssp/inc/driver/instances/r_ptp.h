/***********************************************************************************************************************
 * Copyright [2019-2024] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : r_ptp.h
 * Description  : PTP instance header file
 **********************************************************************************************************************/

#ifndef R_PTP_H
#define R_PTP_H

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup PTP PTP
 * @brief Driver for the Precision time protocol(PTP).
 *
 * @section PTP_SUMMARY Summary
 * This module supports the PTP time synchronization functionality.
 *
 * Extends @ref PTP_API.
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_ptp_api.h"
#include "r_ptp_cfg.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
Macro definitions
 **********************************************************************************************************************/
/* Version of code that implements the API defined in this file */
#define PTP_CODE_VERSION_MAJOR   (2U)
#define PTP_CODE_VERSION_MINOR   (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** PTP instance control block.   DO NOT INITIALIZE. Initialized in ptp_api_t::open(). */
typedef struct st_ptp_instance_ctrl
{
    /** Callback provided when a PTP message is received.  NULL indicates no CPU interrupt. */
    ssp_err_t     (* p_callback)(ptp_callback_args_t * p_args);

    IRQn_Type                   mint_irq;                ///< MINT interrupt IRQ number
    ptp_device_t                device;                  ///< PTP clock type
    ptp_state_t                 state[2];                ///< PTP clock state
    ptp_delay_mechanism_t       delay[2];                ///< PTP delay correction mechanism
    ptp_frame_format_t          frame_format[2];         ///< PTP message frame format
    ptp_stca_mode_t             stca_mode;               ///< STCA synchronous mode
    ptp_address_t               address[2];              ///< IP and MAC address
    uint32_t                    open;                    ///< Flag to determine if the device is open
    uint8_t                     eptpc_flag[2];           ///< Flag to check the EPTPC channel
    uint8_t                     infabt_flag[2];          ///< Flag to check INFABT status
    void                      * p_reg_gen;               ///< Pointer to R_EPTPC_GEN_Type base register
    void                      * p_reg_cfg;               ///< Pointer to R_EPTPC_CFG_Type base register
    void                      * p_reg[2];                ///< Pointer to R_EPTPC0_Type base register
    void const                * p_context;               ///< Placeholder for user data.
    void const                * p_extend;                ///< Extension parameter for hardware specific settings
} ptp_instance_ctrl_t;

/**********************************************************************************************************************
Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const ptp_api_t g_ptp_on_ptp;
/** @endcond */

/*******************************************************************************************************************//**
 * @} (end defgroup PTP)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_PTP_H */

