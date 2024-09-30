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
* File Name    : r_cac_api.h
* Description  : Clock Accuracy Circuit (CAC) Module API header file.
***********************************************************************************************************************/

#ifndef DRV_CAC_API_H
#define DRV_CAC_API_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
/** Register definitions, common services and error codes. */
#include "bsp_api.h"
#include "../ssp_common_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup CAC_API CAC Interface
 *
 * @brief Interface for clock frequency accuracy measurements.
 *
 * @section CAC_API_SUMMARY Summary
 * The interface for the clock frequency accuracy measurement circuit (CAC) peripheral is used to check a system
 * clock frequency with a reference clock signal by counting the number of pulses of the clock (system clock)
 * to be measured.
 *
  * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * CAC Interface description: @ref ModuleCAC
 *
 * @{
 **********************************************************************************************************************/

/**********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define CAC_API_VERSION_MAJOR   (2U)
#define CAC_API_VERSION_MINOR   (0U)

/** Event types returned by the ISR callback when used in CAC interrupt mode */
typedef enum e_cac_event
{
    CAC_EVENT_FREQUENCY_ERROR,          ///< Frequency error
    CAC_EVENT_MEASUREMENT_COMPLETE,     ///< Measurement complete
    CAC_EVENT_COUNTER_OVERFLOW          ///< Counter overflow
} cac_event_t;

/** CAC control block.  Allocate an instance specific control block to pass into the CAC API calls.
 * @par Implemented as
 * - cac_instance_ctrl_t
 */
typedef void cac_ctrl_t;

/** Enumeration of the two possible clocks. */
typedef enum e_cac_clock_type{
    CAC_CLOCK_MEASURED,     ///< Measurement clock
    CAC_CLOCK_REFERENCE     ///< Reference clock
} cac_clock_type_t;

/** Enumeration of the possible clock sources for both the reference and measurement clocks. */
typedef enum e_cac_clock_source{
    CAC_CLOCK_SOURCE_MAIN_OSC  = 0x00,  ///< Main clock oscillator
    CAC_CLOCK_SOURCE_SUBCLOCK  = 0x01,  ///< Sub-clock
    CAC_CLOCK_SOURCE_HOCO      = 0x02,  ///< HOCO (High speed on chip oscillator)
    CAC_CLOCK_SOURCE_MOCO      = 0x03,  ///< MOCO (Middle speed on chip oscillator)
    CAC_CLOCK_SOURCE_LOCO      = 0x04,  ///< LOCO (Middle speed on chip oscillator)
    CAC_CLOCK_SOURCE_PCLKB     = 0x05,  ///< PCLKB (Peripheral Clock B)
    CAC_CLOCK_SOURCE_IWDT      = 0x06,  ///< IWDT- Dedicated on-chip oscillator
    CAC_CLOCK_SOURCE_MEAS_MAX  = CAC_CLOCK_SOURCE_IWDT,   ///< Maximum measured clock
    CAC_CLOCK_SOURCE_EXTERNAL  = 0x07,  ///< Externally supplied measurement clock on CACREF pin
    CAC_CLOCK_SOURCE_REF_MAX   = CAC_CLOCK_SOURCE_EXTERNAL, ///< Maximum reference clock
} cac_clock_source_t;

/** Enumeration of available dividers for the reference clock. */
typedef enum e_cac_ref_divider{
    CAC_REF_DIV_32 =   0x00,    ///< Reference clock divided by 32
    CAC_REF_DIV_128 =  0x01,    ///< Reference clock divided by 128
    CAC_REF_DIV_1024 = 0x02,    ///< Reference clock divided by 1024
    CAC_REF_DIV_8192 = 0x03,    ///< Reference clock divided by 8192
} cac_ref_divider_t;

/** Enumeration of available digital filter settings for an external reference clock. */
typedef enum e_cac_ref_digfilter{
    CAC_REF_DIGITAL_FILTER_OFF =   0x00,      ///< No digital filter on the CACREF pin for reference clock
    CAC_REF_DIGITAL_FILTER_1 =     0x01,      ///< Sampling clock for digital filter = measuring frequency
    CAC_REF_DIGITAL_FILTER_4 =     0x02,      ///< Sampling clock for digital filter = measuring frequency/4
    CAC_REF_DIGITAL_FILTER_16 =    0x03,      ///< Sampling clock for digital filter = measuring frequency/16
} cac_ref_digfilter_t;

/** Enumeration of available edge detect settings for the reference clock. */
typedef enum e_cac_ref_edge{
    CAC_REF_EDGE_RISE =   0x00,     ///< Rising edge detect for the Reference clock
    CAC_REF_EDGE_FALL =   0x01,     ///< Falling edge detect for the Reference clock
    CAC_REF_EDGE_BOTH =   0x02      ///< Both Rising and Falling edges detect for the Reference clock
} cac_ref_edge_t;

/** Enumeration of available dividers for the measurement clock */
typedef enum e_cac_meas_divider{
    CAC_MEAS_DIV_1 =   0x00,    ///< Measurement clock divided by 1
    CAC_MEAS_DIV_4 =   0x01,    ///< Measurement clock divided by 4
    CAC_MEAS_DIV_8 =   0x02,    ///< Measurement clock divided by 8
    CAC_MEAS_DIV_32 =  0x03     ///< Measurement clock divided by 32
} cac_meas_divider_t;

/** Structure defining the settings that apply to reference clock configuration. */
typedef struct st_cac_ref_clock_config{
    cac_ref_divider_t   divider;        ///< Divider specification for the Reference clock
    cac_clock_source_t  clock;          ///< Clock source for the Reference clock
    cac_ref_digfilter_t digfilter;      ///< Digital filter selection for the CACREF ext clock
    cac_ref_edge_t      edge;           ///< Edge detection for the Reference clock
} cac_ref_clock_config_t;

/** Structure defining the settings that apply to measurement clock configuration. */
typedef struct st_cac_meas_clock_config{
    cac_meas_divider_t  divider;    ///< Divider specification for the Measurement clock
    cac_clock_source_t  clock;      ///< Clock source for the Measurement clock
} cac_meas_clock_config_t;

/** Callback function parameter data */
typedef struct st_cac_user_cb_data
{
    cac_event_t       event;        ///< The event can be used to identify what caused the callback (cac ready or error).
    void const      * p_context;    ///< Placeholder for user data.  Set in cac_api_t::open function in ::cac_cfg_t.
} cac_callback_args_t;

/** CAC Configuration */
typedef struct st_cac_cfg
{
    cac_ref_clock_config_t  cac_ref_clock;  ///< reference clock specific settings
    cac_meas_clock_config_t cac_meas_clock; ///< measurement clock specific settings
    uint16_t  cac_upper_limit;      ///< the upper limit counter threshold
    uint16_t  cac_lower_limit;      ///< the lower limit counter threshold
    bool    mei_interrupt_enabled;  ///< True if Measurement Complete interrupt is enabled
    bool    ovf_interrupt_enabled;  ///< True if Overflow interrupt is enabled
    bool    ferr_interrupt_enabled; ///< True if Frequency Error interrupt is enabled
    bool    continuous_mode;        ///< True if measurement continuously restarts after completing.
    uint8_t frequency_error_ipl;    ///< Frequency error interrupt priority
    uint8_t measurement_end_ipl;    ///< Measurement end interrupt priority
    uint8_t overflow_ipl;           ///< Overflow interrupt priority

    /* Configuration for CAC Event processing */
    void  (* p_callback)(cac_callback_args_t * p_args); ///< Callback provided when a CAC interrupt ISR occurs.

    /* Pointer to CAC peripheral specific configuration */
    void const * p_extend;             ///< CAC hardware dependent configuration
    void const * p_context;            ///< Placeholder for user data.  Passed to user callback in ::cac_callback_args_t.
} cac_cfg_t;

/** CAC functions implemented at the HAL layer API */
typedef struct st_cac_api
{
    /** Open function for CAC device.
     * @param[out]  p_ctrl     Pointer to CAC device control. Must be declared by user. Value set here.
     * @param[in]   cac_cfg_t  Pointer to CAC configuration structure. All elements of this structure must be set by user.
     */
    ssp_err_t (* open)(cac_ctrl_t  * const p_ctrl,  cac_cfg_t const * const p_cfg);


    /** Read function for CAC peripheral.
     * @param[in]   p_ctrl Control for the CAC device context.
     * @param[in]   p_status     Pointer to variable in which to store the current CASTR register contents.
     * @param[in]   p_counter    Pointer to variable in which to store the current CACNTBR register contents.
     */
    ssp_err_t (* read)(cac_ctrl_t  * const p_ctrl,  uint8_t  * const p_status,  uint16_t * const p_counter);


    /** Close function for CAC device.
     * @param[in]  p_ctrl        Pointer to CAC device control.
     */
    ssp_err_t (* close)(cac_ctrl_t  * const p_ctrl);

    /** End a measurement for the CAC peripheral.
     * @param[in]  p_ctrl        Pointer to CAC device control.
     */
    ssp_err_t (* stopMeasurement)(cac_ctrl_t  * const p_ctrl);

    /** Begin a measurement for the CAC peripheral.
     * @param[in]  p_ctrl        Pointer to CAC device control.
     */
    ssp_err_t (* startMeasurement)(cac_ctrl_t  * const p_ctrl);

    /** Reset function for CAC device.
     * @param[in]  p_ctrl        Pointer to CAC device control.
     */
    ssp_err_t (* reset)(cac_ctrl_t  * const p_ctrl);


    /** Get the CAC API and code version information.
     * @param[out]  p_version is value returned.
     */
    ssp_err_t (* versionGet)(ssp_version_t * p_version);
} cac_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_cac_instance
{
    cac_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    cac_cfg_t const * p_cfg;     ///< Pointer to the configuration structure for this instance
    cac_api_t const * p_api;     ///< Pointer to the API structure for this instance
} cac_instance_t;

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_CAC_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup CAC_API)
***********************************************************************************************************************/
