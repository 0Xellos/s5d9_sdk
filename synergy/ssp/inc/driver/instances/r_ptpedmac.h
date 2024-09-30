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
 * File Name    : r_ptpedmac.h
 * Description  : PTPEDMAC instance header file
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup PTPEDMAC PTPEDMAC
 * @brief DMA controller for PTP driver
 *
 * @section PTPEDMAC_SUMMARY Summary
 * This module implements the following interface: @ref PTPEDMAC_API.
 *
 * @{
 **********************************************************************************************************************/
#ifndef R_PTPEDMAC_H
#define R_PTPEDMAC_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_ptpedmac_api.h"
#include "r_ptpedmac_cfg.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
Macro definitions
 **********************************************************************************************************************/
/* Version of code that implements the API defined in this file */
#define PTPEDMAC_CODE_VERSION_MAJOR   (2U)
#define PTPEDMAC_CODE_VERSION_MINOR   (0U)

/* Number of receive descriptors */
#define PTPEDMAC_NUM_RX_DESCRIPTORS   (PTPEDMAC_CFG_NUM_RX_DESCRIPTORS)
/* Total number of PTPEDMAC buffers */
#define PTPEDMAC_BUFFER_SIZE          (PTPEDMAC_CFG_BUFFER_SIZE)
/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/* Ethernet buffer type */
typedef struct st_ptpedmac_ether_buffer
{
    /* PTPEDMAC Ethernet buffer aligned to 32 bytes */
    uint8_t  buffer[PTPEDMAC_NUM_RX_DESCRIPTORS][PTPEDMAC_BUFFER_SIZE] BSP_ALIGN_VARIABLE_V2(32);
}ptpedmac_ether_buffer_t;

/** PTPEDMAC instance control block.  DO NOT INITIALIZE. Initialized in ptpedmac_api_t::open().*/
typedef struct st_ptpedmac_instance_ctrl
{
    /** Callback provided when a PTP message is received.  NULL indicates no CPU interrupt. */
    ssp_err_t     (* p_callback)(ptpedmac_callback_args_t * p_args);

    void const            * p_context;                                          ///< Pointer to user interrupt context data
    IRQn_Type               pint_irq;                                           ///< PINT interrupt IRQ number
    void                  * p_reg;                                              ///< Pointer to R_PTPEDMAC_Type base register
    uint32_t                open;                                               ///< Flag to determine if the device is open
    ptpedmac_trans_t        transfer_flag;                                      ///< Flag to determine if the PTP host interface is linked
    ptpedmac_ether_buffer_t p_ptpedmac_buffer;                                  ///< Pointer to Ethernet buffer
    ptpedmac_descriptor_t   p_rx_descriptors[PTPEDMAC_NUM_RX_DESCRIPTORS] BSP_ALIGN_VARIABLE_V2(16);    ///< Pointer to receive descriptor aligned to 16 bytes
    ptpedmac_descriptor_t * p_app_ptp_rx_desc;                                  ///< Pointer to application descriptor
} ptpedmac_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const ptpedmac_api_t g_ptpedmac_on_ptpedmac;
/** @endcond */

/*******************************************************************************************************************//**
 * @} (end defgroup PTPEDMAC)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_PTPEDMAC_H */
