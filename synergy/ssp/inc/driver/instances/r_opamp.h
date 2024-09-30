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
* File Name    : r_opamp.h
* @brief       : Functions for configuring and using the OPAMP
***********************************************************************************************************************/

#ifndef R_OPAMP_H
#define R_OPAMP_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_opamp_cfg.h"
#include "r_opamp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup OPAMP Operational Amplifier (OPAMP)
 * @brief Driver for the Operational Amplifier (OPAMP).
 *
 * This module supports the OPAMP peripheral.  It implements
 * the following interfaces:
 *   - @ref OPAMP_API
 * @{
***********************************************************************************************************************/

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** Version of code that implements the API defined in this file */
#define OPAMP_CODE_VERSION_MAJOR  (2U)
#define OPAMP_CODE_VERSION_MINOR  (0U)

#define OPAMP_MASK_CHANNEL_0      (1U << 0)
#define OPAMP_MASK_CHANNEL_1      (1U << 1)
#define OPAMP_MASK_CHANNEL_2      (1U << 2)
#define OPAMP_MASK_CHANNEL_3      (1U << 3)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** Start and stop trigger for the op-amp. */
typedef enum e_opamp_trigger
{
    OPAMP_TRIGGER_SOFTWARE_START_SOFTWARE_STOP = 0,  ///< Start and stop with APIs
    OPAMP_TRIGGER_AGT_START_SOFTWARE_STOP      = 1,  ///< Start by AGT compare match and stop with API
    OPAMP_TRIGGER_AGT_START_ADC_STOP           = 3,  ///< Start by AGT compare match and stop after ADC conversion
} opamp_trigger_t;

/** Which AGT timer starts the op-amp. Only applies to channels if OPAMP_TRIGGER_AGT_START_SOFTWARE_STOP or
 * OPAMP_TRIGGER_AGT_START_ADC_STOP is selected for the channel. If OPAMP_TRIGGER_SOFTWARE_START_SOFTWARE_STOP is
 * selected for a channel, then no AGT compare match event will start that op-amp channel. */
typedef enum e_opamp_agt_link
{
    /** OPAMP channel 0 and 2 are started by AGT1 compare match.  OPAMP channel 1 and 3 are started by AGT0 compare
     * match. */
    OPAMP_AGT_LINK_AGT1_OPAMP_0_2_AGT0_OPAMP_1_3 = 0,

    /** OPAMP channel 0 and 1 are started by AGT1 compare match.  OPAMP channel 2 and 3 are started by AGT0 compare
     * match. */
    OPAMP_AGT_LINK_AGT1_OPAMP_0_1_AGT0_OPAMP_2_3 = 1,

    /** All OPAMP channels are started by AGT1 compare match. */
    OPAMP_AGT_LINK_AGT1_OPAMP_0_1_2_3            = 3,
} opamp_agt_link_t;

/** Op-amp mode. */
typedef enum e_opamp_mode
{
    OPAMP_MODE_LOW_POWER    = 0,   ///< Low power mode
    OPAMP_MODE_MIDDLE_SPEED = 1,   ///< Middle speed mode (not supported on all MCUs)
    OPAMP_MODE_HIGH_SPEED   = 3    ///< High speed mode
} opamp_mode_t;

/** Op-amp trim state. */
typedef enum e_opamp_priv_trim_state
{
    OPAMP_PRIV_TRIM_STATE_INVALID = -1, ///< Trim state invalid
    OPAMP_PRIV_TRIM_STATE_END   = 0,    ///< Trim state end
    OPAMP_PRIV_TRIM_STATE_BIT_0 = 0,    ///< Trim state bit 0
    OPAMP_PRIV_TRIM_STATE_BIT_1 = 1,    ///< Trim state bit 1
    OPAMP_PRIV_TRIM_STATE_BIT_2 = 2,    ///< Trim state bit 2
    OPAMP_PRIV_TRIM_STATE_BIT_3 = 3,    ///< Trim state bit 3
    OPAMP_PRIV_TRIM_STATE_BIT_4 = 4,    ///< Trim state bit 4
    OPAMP_PRIV_TRIM_STATE_BEGIN = 5,    ///< Trim state begin 
} opamp_priv_trim_state_t;

/** OPAMP configuration extension. This extension is required and must be provided in opamp_cfg_t::p_extend. */
typedef struct st_opamp_on_opamp_cfg
{
    /** Configure which AGT links are paired to which channel. Only applies to channels if
     * OPAMP_TRIGGER_AGT_START_SOFTWARE_STOP or OPAMP_TRIGGER_AGT_START_ADC_STOP is selected for the channel. */
    opamp_agt_link_t agt_link;
    opamp_mode_t     mode;              ///< Low power, middle speed, or high speed mode
    opamp_trigger_t  trigger_channel_0; ///< Start and stop triggers for channel 0
    opamp_trigger_t  trigger_channel_1; ///< Start and stop triggers for channel 1
    opamp_trigger_t  trigger_channel_2; ///< Start and stop triggers for channel 2
    opamp_trigger_t  trigger_channel_3; ///< Start and stop triggers for channel 3
} opamp_on_opamp_cfg_t;

/** OPAMP instance control block. DO NOT INITIALIZE.  Initialized in opamp_api_t::open(). */
typedef struct
{
    R_OPAMP_Type           * p_reg;           ///< Base register for this unit
    uint32_t                 opened;          ///< Boolean to verify that the Unit has been initialized
    uint8_t                  trim_capable;    ///< OPAMP has trim registers
    uint8_t                  switches;        ///< OPAMP has switches
    uint32_t                 valid_opamps;    ///< Mask of valid op-amps
    opamp_priv_trim_state_t  trim_state;      ///< Trim state for each op-amp
    uint8_t                  trim_channel;    ///< Channel
    opamp_trim_input_t       trim_input;      ///< Which input of the channel above
} opamp_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Interface Structure for user access */
extern const opamp_api_t g_opamp_on_opamp;
/** @endcond */


/*******************************************************************************************************************//**
 * @} (end defgroup OPAMP)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_OPAMP_H */
