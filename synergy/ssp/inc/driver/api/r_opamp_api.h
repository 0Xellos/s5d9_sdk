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
* File Name    : r_opamp_api.h
* Description  : OPAMP driver interface file.
***********************************************************************************************************************/

#ifndef R_OPAMP_API_H
#define R_OPAMP_API_H

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup OPAMP_API OPAMP Interface
 * @brief Interface for Operational Amplifiers.
 *
 * @section OPAMP_API_SUMMARY Summary
 * The OPAMP interface provides standard operational amplifier functionality, including starting and stopping the
 * amplifier.
 *
 * Implemented by:
 * @ref OPAMP
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * OPAMP Interface description: @ref HALOPAMPInterface
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/** Includes board and MCU related header files. */
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** Version Number of API.  */
#define OPAMP_API_VERSION_MAJOR   (2U)
#define OPAMP_API_VERSION_MINOR   (0U)

/*****************************************************************************
Typedef definitions
******************************************************************************/
/** Trim command. */
typedef enum e_opamp_trim_cmd
{
    OPAMP_TRIM_CMD_START,                      ///< Initialize trim state machine
    OPAMP_TRIM_CMD_NEXT_STEP,                  ///< Move to next step in state machine
    OPAMP_TRIM_CMD_CLEAR_BIT,                  ///< Clear trim bit
} opamp_trim_cmd_t;

/** Trim input. */
typedef enum e_opamp_trim_input
{
    OPAMP_TRIM_INPUT_PCH   = 0U,     ///< Trim non-inverting (+) input
    OPAMP_TRIM_INPUT_NCH   = 1U,     ///< Trim inverting (-) input
} opamp_trim_input_t;

/** OPAMP trim arguments. */
typedef struct st_opamp_trim_args
{
    uint8_t            channel;            ///< Channel
    opamp_trim_input_t input;              ///< Which input of the channel above
} opamp_trim_args_t;

/** OPAMP information. */
typedef struct st_opamp_info
{
    uint32_t    min_stabilization_wait_us; ///< Minimum stabilization wait time in microseconds
} opamp_info_t;

/** OPAMP status. */
typedef struct st_opamp_status
{
    uint32_t    operating_channel_mask;    ///< Bitmask of channels currently operating
} opamp_status_t;

/** OPAMP general configuration. */
typedef struct st_opamp_cfg
{
    void const     * p_extend;         ///< Extension parameter for hardware specific settings
} opamp_cfg_t;

/** OPAMP control block. Allocate using driver instance control structure from driver instance header file. */
typedef void opamp_ctrl_t;

/** OPAMP functions implemented at the HAL layer will follow this API. */
typedef struct st_opamp_api
{
    /** Initialize the operational amplifier.
     * @par Implemented as
     *  - R_OPAMP_Open()
     *
     * @param[in]  p_ctrl  Pointer to instance control block
     * @param[in]  p_cfg   Pointer to configuration
     */
    ssp_err_t (* open)(opamp_ctrl_t * const p_ctrl,  opamp_cfg_t const * const p_cfg);

    /** Start the op-amp(s).
     * @par Implemented as
     *  - R_OPAMP_Start()
     *
     * @param[in]  p_ctrl         Pointer to instance control block
     * @param[in]  channel_mask   Bitmask of channels to start
     */
    ssp_err_t (* start)(opamp_ctrl_t * const p_ctrl, uint32_t const channel_mask);

    /** Stop the op-amp(s).
     * @par Implemented as
     *  - R_OPAMP_Stop()
     *
     * @param[in]  p_ctrl         Pointer to instance control block
     * @param[in]  channel_mask   Bitmask of channels to stop
     */
    ssp_err_t (* stop)(opamp_ctrl_t * const p_ctrl, uint32_t const channel_mask);

    /** Trim the op-amp(s). Not supported on all MCUs. See implementation for procedure details.
     * @par Implemented as
     *  - R_OPAMP_Trim()
     *
     * @param[in]  p_ctrl         Pointer to instance control block
     * @param[in]  cmd            Trim command
     * @param[in]  p_args         Pointer to arguments for the command
     */
    ssp_err_t (* trim)(opamp_ctrl_t * const p_ctrl, opamp_trim_cmd_t const cmd, opamp_trim_args_t const * const p_args);

    /** Provide information such as the recommended minimum stabilization wait time.
     * @par Implemented as
     *  - R_OPAMP_InfoGet()
     *
     * @param[in]   p_ctrl       Pointer to instance control block
     * @param[out]  p_info       OPAMP information stored here
     */
    ssp_err_t (* infoGet) (opamp_ctrl_t * const p_ctrl, opamp_info_t * const p_info);

    /** Provide status of each op-amp channel.
     * @par Implemented as
     *  - R_OPAMP_StatusGet()
     *
     * @param[in]   p_ctrl       Pointer to instance control block
     * @param[out]  p_status     Status stored here
     */
    ssp_err_t (* statusGet) (opamp_ctrl_t * const p_ctrl, opamp_status_t * const p_status);

    /** Close the specified OPAMP unit by ending any scan in progress, disabling interrupts, and removing power to the
     * specified A/D unit.
     * @par Implemented as
     *  - R_OPAMP_Close()
     *
     * @param[in]  p_ctrl   Pointer to instance control block
     */
    ssp_err_t (* close)(opamp_ctrl_t * const p_ctrl);

    /** Retrieve the API version.
     * @par Implemented as
     *  - R_OPAMP_VersionGet()
     *
     * @pre This function retrieves the API version.
     * @param[in]  p_version   Pointer to version structure
     */
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);

} opamp_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_opamp_instance
{
    opamp_ctrl_t              * p_ctrl;         ///< Pointer to the control structure for this instance
    opamp_cfg_t         const * p_cfg;          ///< Pointer to the configuration structure for this instance
    opamp_api_t         const * p_api;          ///< Pointer to the API structure for this instance
} opamp_instance_t;

/*******************************************************************************************************************//**
 * @} (end defgroup OPAMP_API)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_OPAMP_API_H */
