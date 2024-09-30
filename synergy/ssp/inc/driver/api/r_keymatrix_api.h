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
 * File Name    : r_keymatrix_api.h
 * Description  : API for key matrix applications. Allows usage of the KINT peripheral
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup KEYMATRIX_API Key Matrix Interface
 * @brief Interface for key matrix functions.
 *
 * @section KEYMATRIX_API_SUMMARY Summary
 * The KEYMATRIX interface provides standard KeyMatrix functionality including event generation on a rising or falling
 * edge for one or more channels at the same time. The generated event indicates all channels that are active in
 * that instant via a bit mask. This allows the interface to be used with a matrix configuration or a one-to-one
 * hardware implementation that is triggered on either a rising or a falling edge.
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * Key Matrix Interface description: @ref HALKINTInterface
 *
 * @{
 **********************************************************************************************************************/

#ifndef DRV_KEYMATRIX_API_H
#define DRV_KEYMATRIX_API_H

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
/* Includes board and MCU related header files. */
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 *********************************************************************************************************************/
#define KEYMATRIX_API_VERSION_MAJOR (2U)    ///< KEY MATRIX API version number (Major)
#define KEYMATRIX_API_VERSION_MINOR (0U)    ///< KEY MATRIX API version number (Minor)

/**********************************************************************************************************************
 * Typedef definitions
 *********************************************************************************************************************/
/** Channel definition. This is a bit mask with each bit from 0-7 representing channels 0-7 respectively. */
typedef uint32_t keymatrix_channels_t;

/** Key matrix control block.  Allocate an instance specific control block to pass into the key matrix API calls.
 * @par Implemented as
 * - kint_instance_ctrl_t
 */
typedef void keymatrix_ctrl_t;

/** Trigger type: rising edge, falling edge */
typedef enum e_keymatrix_trigger
{
    KEYMATRIX_TRIG_FALLING = 0,  ///< Falling edge trigger
    KEYMATRIX_TRIG_RISING  = 1,  ///< Rising edge trigger
} keymatrix_trigger_t;

/** Callback function parameter data */
typedef struct st_keymatrix_calback_args
{
    void const * p_context;   ///< Holder for user data. Set in keymatrix_api_t::open function in ::keymatrix_cfg_t.

    /** Bit vector representing the physical hardware channel(s) that caused the interrupt.  The bit vector is
     *  used for compatibility with matrix designs where more than one input will be active at once.
     *  @note Not all HAL drivers support matrix mode.  See r_kint.h for details. */
    keymatrix_channels_t channels;
} keymatrix_callback_args_t;

/** User configuration structure, used in open function */
typedef struct st_keymatrix_cfg
{
    keymatrix_channels_t  channels;    ///< Key Input channel(s). Bit mask of channels to open.
    keymatrix_trigger_t   trigger;     ///< Key Input trigger setting.
    bool                  autostart;   ///< Start operation and enable interrupts during open().
    void               (* p_callback)(keymatrix_callback_args_t * p_args); ///< Callback for key interrupt ISR.
    void const          * p_context;   ///< Holder for user data.  Passed to callback in keymatrix_user_cb_data_t.
    void const          * p_extend;    ///< Extension parameter for hardware specific settings.
    uint8_t               irq_ipl;     ///< Interrupt priority level
} keymatrix_cfg_t;

/** Key Matrix driver structure. Key Matrix functions implemented at the HAL layer will use this API. */
typedef struct st_keymatrix_api
{
    /** Initial configuration.
     * @par Implemented as
     * - R_KINT_KEYMATRIX_Open()
     *
     * @param[out]  p_ctrl   Pointer to control block. Must be declared by user. Value set in this function.
     * @param[in]   p_cfg    Pointer to configuration structure. All elements of the structure must be set by user.
     */
    ssp_err_t (* open)(keymatrix_ctrl_t      * const p_ctrl,
                       keymatrix_cfg_t const * const p_cfg);

    /** Enable Key interrupt
     * @par Implemented as
     * - R_KINT_KEYMATRIX_Enable()
     *
     * @param[in]     p_ctrl      Control block pointer set in Open call for this Key interrupt.
     */
    ssp_err_t (* enable)(keymatrix_ctrl_t       * const p_ctrl);

    /** Disable Key interrupt.
     * @par Implemented as
     * - R_KINT_KEYMATRIX_Disable()
     *
     * @param[in]     p_ctrl      Control block pointer set in Open call for this Key interrupt.
     */
    ssp_err_t (* disable)(keymatrix_ctrl_t       * const p_ctrl);

    /** Set trigger for Key interrupt.
     * @par Implemented as
     * - R_KINT_KEYMATRIX_TriggerSet()
     *
     * @param[in]     p_ctrl       Control block pointer set in Open call for this Key interrupt.
     * @param[in]     trigger   Trigger source for key interrupt; defined in enumeration of @ref keymatrix_trigger_t.
     */
    ssp_err_t (* triggerSet)(keymatrix_ctrl_t       * const p_ctrl,
                             keymatrix_trigger_t      const trigger);

    /** Allow driver to be reconfigured. May reduce power consumption.
     * @par Implemented as
     * - R_KINT_KEYMATRIX_Close()
     *
     * @param[in]  p_ctrl      Control block pointer set in Open call for this Key interrupt.
     */
    ssp_err_t (* close)  (keymatrix_ctrl_t   * const p_ctrl);

    /** Get version and store it in provided pointer p_version.
     * @par Implemented as
     * - R_KINT_VersionGet()
     *
     * @param[out]  p_version  Code and API version used.
     */
    ssp_err_t (* versionGet) (ssp_version_t   * const p_version);

} keymatrix_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_keymatrix_instance
{
    keymatrix_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    keymatrix_cfg_t const * p_cfg;     ///< Pointer to the configuration structure for this instance
    keymatrix_api_t const * p_api;     ///< Pointer to the API structure for this instance
} keymatrix_instance_t;

/******************************************************************************************************************//**
 * @} (end addtogroup KEYMATRIX_API)
 *********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_KEYMATRIX_API_H */
