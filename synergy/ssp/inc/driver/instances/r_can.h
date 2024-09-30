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
 * File Name    : r_can.h
 * Description  : CAN Module instance header file.
 **********************************************************************************************************************/

#ifndef R_CAN_H
#define R_CAN_H

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup CAN CAN
 * @brief Driver for CAN, Controller Area Network.
 *
 * This module supports the Controller Area Network peripheral. It implements
 * the following interfaces:
 *   - @ref CAN_API
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_can_cfg.h"
#include "r_can_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define CAN_CODE_VERSION_MAJOR (2U)
#define CAN_CODE_VERSION_MINOR (0U)

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const can_api_t g_can_on_can;
/** @endcond */

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** CAN Instance Control Block   */
typedef struct st_can_instance_ctrl
{
    /** Parameters to control CAN peripheral device */
    uint32_t            channel;                                    ///< Channel number
    uint32_t            open;                                       ///< Open status of channel.
    can_mode_t          operation_mode;                             ///< Can operation mode.
    can_id_mode_t       id_mode;                                    ///< Standard or Extended ID mode.
    uint32_t            mailbox_count;                              ///< Number of mailboxes.
    can_mailbox_t     * p_mailbox;                                  ///< Pointer to mailboxes.
    can_message_mode_t  message_mode;                               ///< Overwrite message or overrun.
    can_clock_source_t  clock_source;                               ///< Clock source. CANMCLK or PCLKB.
    /** Parameters to process CAN Event */
    void             (* p_callback)(can_callback_args_t * p_args);  ///< Pointer to callback function
    void const        * p_context;                                  ///< Pointer to the higher level device context
    void              * p_reg;                                      ///< Pointer to register base address
    IRQn_Type           error_irq;                                  ///< Error IRQ number
    IRQn_Type           mailbox_rx_irq;                             ///< Receive mailbox IRQ number
    IRQn_Type           mailbox_tx_irq;                             ///< Transmit mailbox IRQ number
} can_instance_ctrl_t;

/** CAN clock configuration and mailbox mask to be pointed to by p_extend. */
typedef struct st_can_extended_cfg
{
    can_clock_source_t      clock_source;                    ///< Source of the CAN clock.
    uint32_t              * p_mailbox_mask;                  ///< Mailbox mask, one for every 4 mailboxes.
} can_extended_cfg_t;


/*******************************************************************************************************************//**
 * @} (end defgroup CAN)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

/* R_CAN_H */
#endif // ifndef R_CAN_H
