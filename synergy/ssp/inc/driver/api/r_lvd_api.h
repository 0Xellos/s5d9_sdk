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
 * File Name    : r_lvd_api.h
 * Description  : LVD Low Voltage Detection interface
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup LVD_API Low Voltage Detection Interface
 * @brief This section defines the API for the LVD (Low Voltage Detection) Driver.
 *
 *
 * The LVD driver provides functions for configuring the LVD hardware peripheral.
 *
 * The process of configuring and enabling a Low Voltage Detection monitor has very specific
 * timing constraints and register write ordering. Because of these constraints, the entire
 * process of configuring and enabling a voltage monitor is most effectively performed by a
 * single function. The API function configure performs configuration and enables the
 * monitor in order to properly enforce the timing and register write ordering constraints.
 *
 * The LVD driver configures all of the settings of the available configurable LVD monitors.
 *
 * The settings include:
 *
 * - voltage_threshold: Determines the voltage detection threshold (i.e. 2.99 Volts).
 *
 * - sample_clock_divisor: Determines the sample clock rate of the digital filter,
 *                         based on division of the LOCO clock. 
 *                         Also disables or enables the digital filter if available on the MCU.
 *
 * - detection_response: Determines which event will occur, reset, interrupt, non-maskable interrupt, or no response,
 *                       when the voltage threshold is crossed
 *
 * - voltage_slope: Choose either rising or falling voltage as the trigger for a
 *                  voltage detection interrupt.
 *
 * - negation_delay: Determine whether timing of the negation of the voltage
 *                   event is based upon the reset event or based on the voltage
 *                   event itself.
 *
 * - p_callback: Address of user defined function to be called when the
 *                         voltage event interrupt occurs.
 *
 * @note Low Voltage Monitor 0 (LVD0) is not configurable at runtime but can be
 *        configured by changing the OFS1 register value on the BSP Properties tab of
 *        the Synergy Project Configurator in the e2 studio ISDE.
 *
 * @note Digital filter is not to be used with standby modes. If software standby or
 *        deep standby mode is to be used, the digital filter should be disabled.
 *
 * For details about the implementation of the driver functions see section @ref LVD.
 *
 * @{
 **********************************************************************************************************************/
#ifndef DRV_LVD_API_H
#define DRV_LVD_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/** Register definitions, common services and error codes. */
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define LVD_API_VERSION_MAJOR (2U)
#define LVD_API_VERSION_MINOR (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** Voltage detection level */
/** The thresholds supported by each MCU is in the MCU User's Manual as well as
 *  in the r_lvd module description on the threads tab of the Synergy project. */
typedef enum e_lvd_threshold
{
    LVD_THRESHOLD_MONITOR_1_LEVEL_0   = 0x0UL,    ///< 4.29V (Vdet1_0)
    LVD_THRESHOLD_MONITOR_1_LEVEL_1   = 0x1UL,    ///< 4.14V (Vdet1_1)
    LVD_THRESHOLD_MONITOR_1_LEVEL_2   = 0x2UL,    ///< 4.02V (Vdet1_2)
    LVD_THRESHOLD_MONITOR_1_LEVEL_3   = 0x3UL,    ///< 3.84V (Vdet1_3)
    LVD_THRESHOLD_MONITOR_1_LEVEL_4   = 0x4UL,    ///< 3.10V (Vdet1_4)
    LVD_THRESHOLD_MONITOR_1_LEVEL_5   = 0x5UL,    ///< 3.00V (Vdet1_5)
    LVD_THRESHOLD_MONITOR_1_LEVEL_6   = 0x6UL,    ///< 2.90V (Vdet1_6)
    LVD_THRESHOLD_MONITOR_1_LEVEL_7   = 0x7UL,    ///< 2.79V (Vdet1_7)
    LVD_THRESHOLD_MONITOR_1_LEVEL_8   = 0x8UL,    ///< 2.68V (Vdet1_8)
    LVD_THRESHOLD_MONITOR_1_LEVEL_9   = 0x9UL,    ///< 2.58V (Vdet1_9)
    LVD_THRESHOLD_MONITOR_1_LEVEL_A   = 0xAUL,    ///< 2.48V (Vdet1_A)
    LVD_THRESHOLD_MONITOR_1_LEVEL_B   = 0xBUL,    ///< 2.20V (Vdet1_B)
    LVD_THRESHOLD_MONITOR_1_LEVEL_C   = 0xCUL,    ///< 1.96V (Vdet1_C)
    LVD_THRESHOLD_MONITOR_1_LEVEL_D   = 0xDUL,    ///< 1.86V (Vdet1_D)
    LVD_THRESHOLD_MONITOR_1_LEVEL_E   = 0xEUL,    ///< 1.75V (Vdet1_E)
    LVD_THRESHOLD_MONITOR_1_LEVEL_F   = 0xFUL,    ///< 1.65V (Vdet1_F)
    LVD_THRESHOLD_MONITOR_1_LEVEL_11  = 0x11UL,   ///< 2.99V (Vdet1_11)
    LVD_THRESHOLD_MONITOR_1_LEVEL_12  = 0x12UL,   ///< 2.92V (Vdet1_12)
    LVD_THRESHOLD_MONITOR_1_LEVEL_13  = 0x13UL,   ///< 2.85V (Vdet1_13)
    LVD_THRESHOLD_MONITOR_2_LEVEL_0   = 0x0UL,    ///< 4.29V (Vdet2_0)
    LVD_THRESHOLD_MONITOR_2_LEVEL_1   = 0x1UL,    ///< 4.14V (Vdet2_1)
    LVD_THRESHOLD_MONITOR_2_LEVEL_2   = 0x2UL,    ///< 4.02V (Vdet2_2)
    LVD_THRESHOLD_MONITOR_2_LEVEL_3   = 0x3UL,    ///< 3.84V (Vdet2_3)
    LVD_THRESHOLD_MONITOR_2_LEVEL_5   = 0x5UL,    ///< 2.99V (Vdet2_5)
    LVD_THRESHOLD_MONITOR_2_LEVEL_6   = 0x6UL,    ///< 2.92V (Vdet2_6)
    LVD_THRESHOLD_MONITOR_2_LEVEL_7   = 0x7UL,    ///< 2.85V (Vdet2_7)
} lvd_threshold_t;

/** Response types to a threshold crossing event, interrupt, reset, NMI... */
typedef enum e_lvd_response
{
    LVD_RESPONSE_NMI,           ///< Non-maskable interrupt
    LVD_RESPONSE_INTERRUPT,     ///< Maskable interrupt
    LVD_RESPONSE_RESET,         ///< Reset
    LVD_RESPONSE_NONE,          ///< No response, status must be requested via statusGet function
} lvd_response_t;

/** Voltage slope, rising, falling, or both */
typedef enum e_lvd_voltage_slope
{
    LVD_VOLTAGE_SLOPE_RISING    = 0,    ///< When VCC >= Vdet2 (rise) is detected
    LVD_VOLTAGE_SLOPE_FALLING   = 1,    ///< When VCC < Vdet2 (drop) is detected
    LVD_VOLTAGE_SLOPE_BOTH      = 2,    ///< When drop and rise are detected
} lvd_voltage_slope_t;

/** Threshold crossing detection (latched) */
typedef enum e_lvd_threshold_crossing
{
    LVD_THRESHOLD_CROSSING_NOT_DETECTED     = 0,    ///< Threshold crossing has not been detected.
    LVD_THRESHOLD_CROSSING_DETECTED         = 1,    ///< Threshold crossing has been detected.
} lvd_threshold_crossing_t;

/** Instantaneous status of VCC (above or below threshold) */
typedef enum e_lvd_current_state
{
    LVD_CURRENT_STATE_BELOW_THRESHOLD   = 0,    ///< VCC <  threshold
    LVD_CURRENT_STATE_ABOVE_THRESHOLD   = 1,    ///< VCC >= threshold or monitor is disabled
} lvd_current_state_t;

/**
 * Voltage monitor status structure, used with statusGet function and p_callback to provide
 * current state of the monitor, (threshold crossing detected, vcc currently within range).
 */
typedef struct st_lvd_status
{
    /** Threshold crossing detection (latched) */
    lvd_threshold_crossing_t    crossing_detected;
    /** Instantaneous status of monitored voltage (above or below threshold) */
    lvd_current_state_t         current_state;
} lvd_status_t;

/** LVD callback parameter definition */
typedef struct st_lvd_callback_args
{
    uint32_t       monitor_number;  ///< Monitor number
    lvd_status_t   status;          ///< Status of monitor
    void const   * p_context;       ///< Placeholder for user data
} lvd_callback_args_t;

/** LVD configuration structure */
typedef struct st_lvd_cfg
{
    /** Monitor number, 1, 2, ... */
    const uint32_t              monitor_number;
    /** Threshold for out of range voltage detection */
    lvd_threshold_t             voltage_threshold;
    /** Response on detecting a threshold crossing */
    lvd_response_t              detection_response;
    /** Rising or falling voltage is to be detected */
    lvd_voltage_slope_t         voltage_slope;
    /** Interrupt priority level. */
    uint8_t                     monitor_ipl;
    /** User function to be called from interrupt */
    void                     (* p_callback)(lvd_callback_args_t * p_args);
    /** Placeholder for user data. Passed to the user callback in  */
    void const                * p_context;
    /** Extension parameter for hardware specific settings */
    void const                * p_extend;
} lvd_cfg_t;

/** LVD control block.  Allocate an instance specific control block to pass into the LVD API calls.
 * @par Implemented as
 * - lvd_instance_ctrl_t
 */
typedef void lvd_ctrl_t;

/** LVD driver API structure.
 *  LVD driver functions implemented at the HAL layer will adhere to this API.
 */
typedef struct st_lvd_api
{
    /** Initializes a low voltage detection driver according to the passed in configuration structure.
     * Enables an LVD peripheral based on configuration structure.
     * @par Implemented as
     *  - R_LVD_Open()
     * @param[in]       p_ctrl      Pointer to monitor control structure for the driver instance
     * @param[in]       p_cfg       Pointer to the configuration structure for the driver instance
     **/
    ssp_err_t (* open)(lvd_ctrl_t * const p_ctrl, lvd_cfg_t const * const p_cfg);

    /** Get the current state of the monitor, (threshold crossing detected, voltage currently within range)
     * Can be used to poll the state of the LVD monitor at any time.
     * Must be used if the peripheral was initialized with lvd_response_t set to LVD_RESPONSE_NONE.
     * @par Implemented as
     *  - R_LVD_StatusGet()
     * @param[in]       p_ctrl          Pointer to the control structure for the driver instance
     * @param[in,out]   p_lvd_status    Pointer to an lvd_status_t instance
     **/
    ssp_err_t (* statusGet)(lvd_ctrl_t * const p_ctrl, lvd_status_t * p_lvd_status);

    /** Clears the latched status of the monitor.
     * Must be used if the peripheral was initialized with lvd_response_t set to LVD_RESPONSE_NONE.
     * @par Implemented as
     *  - R_LVD_StatusClear()
     * @param[in]       p_ctrl      Pointer to the control structure for the driver instance
     **/
    ssp_err_t (* statusClear)(lvd_ctrl_t * const p_ctrl);

    /** Disables the LVD peripheral.
     * Closes the driver instance.
     * @par Implemented as
     *  - R_LVD_Close()
     * @param[in]   p_ctrl      Pointer to the control structure for the driver instance
     **/
    ssp_err_t (* close)(lvd_ctrl_t * const p_ctrl);

    /** Returns the LVD driver version based on compile time macros.
     * @par Implemented as
     *  - R_LVD_VersionGet()
     * @param[in,out]   p_version   Pointer to version structure
     **/
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);
} lvd_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_lvd_instance
{
    lvd_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    lvd_cfg_t const * p_cfg;     ///< Pointer to the configuration structure for this interface instance
    lvd_api_t const * p_api;     ///< Pointer to the API structure for this interface instance
} lvd_instance_t;

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_LVD_API_H */

/*******************************************************************************************************************//**
 * @} (end defgroup LVD_API)
 **********************************************************************************************************************/
