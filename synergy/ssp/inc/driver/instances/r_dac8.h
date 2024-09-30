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
 * File Name    : r_dac8.h
 * Description  : D/A Converter (DAC8) Module instance header file.
 **********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup DAC8 DAC8
 * @brief Driver for the 8-Bit D/C Converter (DAC8).
 *
 * @section DAC8_SUMMARY Summary
 * This module implements the following interface: @ref DAC_API.
 *
 * @{
 **********************************************************************************************************************/

#ifndef R_DAC8_H
#define R_DAC8_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include "bsp_api.h"
#include "r_dac8_cfg.h"
#include "r_dac_api.h"
#include "r_transfer_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define DAC8_CODE_VERSION_MAJOR (2U)
#define DAC8_CODE_VERSION_MINOR (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** DAC8 instance control block.  DO NOT INITIALIZE.  */
typedef struct st_dac8_instance_ctrl
{
    void         *    p_reg;               ///< Pointer to DAC base register
    uint8_t           channel;             ///< ID associated with this DAC channel
    uint8_t           channel_started;     ///< DAC operation on channel started
    uint32_t          channel_opened;      ///< DAC channel open
    dac_data_format_t data_format;         ///< DAC data format
} dac8_instance_ctrl_t;

/** DAC8 mode */
typedef enum e_dac8_mode
{
    DAC8_MODE_NORMAL,           ///< DAC Normal mode
    DAC8_MODE_REAL_TIME         ///< DAC Real-time (event link) mode
} dac8_mode_t;

/** DAC8 extended configuration */
typedef struct st_dac8_extended_cfg
{
    bool           enable_charge_pump;     ///< Enable DAC charge pump
    dac8_mode_t    dac_mode;               ///< DAC mode
} dac8_extended_cfg_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const dac_api_t g_dac_on_dac8;
/** @endcond */

/*******************************************************************************************************************//**
 * @} (end defgroup DAC)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_DAC8_H

