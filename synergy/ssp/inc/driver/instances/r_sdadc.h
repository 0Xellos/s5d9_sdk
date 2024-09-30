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
* File Name    : r_sdadc.h
* @brief       : Functions for configuring and using the SDADC
***********************************************************************************************************************/

#ifndef R_SDADC_H
#define R_SDADC_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include <stdlib.h>
/* Fixed width integer support. */
#include <stdint.h>
/* bool support */
#include <stdbool.h>
#include "bsp_api.h"
#include "r_sdadc_cfg.h"
#include "r_adc_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup SDADC Sigma Delta ADC (SDADC)
 * @brief Driver for the 24-bit Sigma Delta A/D Converter (SDADC).
 *
 * This module supports the SDADC peripheral.  It implements
 * the following interfaces:
 *   - @ref ADC_API
 * @{
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/** Version of code that implements the API defined in this file */
#define SDADC_CODE_VERSION_MAJOR  (2U)
#define SDADC_CODE_VERSION_MINOR  (0U)

#define SDADC_MAX_NUM_CHANNELS    (5U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** Source of Vref. */
typedef enum e_sdadc_vref_src
{
    SDADC_VREF_SRC_INTERNAL = 0,              ///< Vref is internally sourced, can be output as SBIAS
    SDADC_VREF_SRC_EXTERNAL = 1,              ///< Vref is externally sourced from the VREFI pin
} sdadc_vref_src_t;

/** Voltage of Vref. */
typedef enum e_sdadc_vref_voltage
{
    SDADC_VREF_VOLTAGE_800_MV   = 0,          ///< Vref is 0.8 V
    SDADC_VREF_VOLTAGE_1000_MV  = 1,          ///< Vref is 1.0 V
    SDADC_VREF_VOLTAGE_1200_MV  = 2,          ///< Vref is 1.2 V
    SDADC_VREF_VOLTAGE_1400_MV  = 3,          ///< Vref is 1.4 V
    SDADC_VREF_VOLTAGE_1600_MV  = 4,          ///< Vref is 1.6 V
    SDADC_VREF_VOLTAGE_1800_MV  = 5,          ///< Vref is 1.8 V
    SDADC_VREF_VOLTAGE_2000_MV  = 6,          ///< Vref is 2.0 V
    SDADC_VREF_VOLTAGE_2200_MV  = 7,          ///< Vref is 2.2 V
    SDADC_VREF_VOLTAGE_2400_MV  = 15,         ///< Vref is 2.4 V (only valid for external Vref)
} sdadc_vref_voltage_t;

/** Per channel input mode. */
typedef enum e_sdadc_channel_input
{
    SDADC_CHANNEL_INPUT_DIFFERENTIAL = 0,         ///< Differential input
    SDADC_CHANNEL_INPUT_SINGLE_ENDED = 1,         ///< Single-ended input
} sdadc_channel_input_t;

/** Per channel stage 1 gain options. */
typedef enum e_sdadc_channel_stage_1_gain
{
    SDADC_CHANNEL_STAGE_1_GAIN_1    = 0,          ///< Gain of 1
    SDADC_CHANNEL_STAGE_1_GAIN_2    = 1,          ///< Gain of 2
    SDADC_CHANNEL_STAGE_1_GAIN_3    = 2,          ///< Gain of 3 (only valid for stage 1)
    SDADC_CHANNEL_STAGE_1_GAIN_4    = 3,          ///< Gain of 4
    SDADC_CHANNEL_STAGE_1_GAIN_8    = 4,          ///< Gain of 8
} sdadc_channel_stage_1_gain_t;

/** Per channel stage 2 gain options. */
typedef enum e_sdadc_channel_stage_2_gain
{
    SDADC_CHANNEL_STAGE_2_GAIN_1    = 0,          ///< Gain of 1
    SDADC_CHANNEL_STAGE_2_GAIN_2    = 1,          ///< Gain of 2
    SDADC_CHANNEL_STAGE_2_GAIN_4    = 2,          ///< Gain of 4
    SDADC_CHANNEL_STAGE_2_GAIN_8    = 3,          ///< Gain of 8
} sdadc_channel_stage_2_gain_t;

/** Per channel oversampling ratio. */
typedef enum e_sdadc_channel_oversampling
{
    SDADC_CHANNEL_OVERSAMPLING_64   = 0,          ///< Oversampling ratio of 64
    SDADC_CHANNEL_OVERSAMPLING_128  = 1,          ///< Oversampling ratio of 128
    SDADC_CHANNEL_OVERSAMPLING_256  = 2,          ///< Oversampling ratio of 256
    SDADC_CHANNEL_OVERSAMPLING_512  = 3,          ///< Oversampling ratio of 512
    SDADC_CHANNEL_OVERSAMPLING_1024 = 4,          ///< Oversampling ratio of 1024
    SDADC_CHANNEL_OVERSAMPLING_2048 = 5,          ///< Oversampling ratio of 2048
} sdadc_channel_oversampling_t;

/** Per channel polarity, valid for single-ended input only. */
typedef enum e_sdadc_channel_polarity
{
    SDADC_CHANNEL_POLARITY_POSITIVE = 0,          ///< Positive-side single-ended input
    SDADC_CHANNEL_POLARITY_NEGATIVE = 1,          ///< Negative-side single-ended input
} sdadc_channel_polarity_t;

/** Per channel number of conversions to average before conversion end callback. */
typedef enum e_sdadc_channel_average_t
{
    SDADC_CHANNEL_AVERAGE_NONE = 0,               ///< Do not average (callback for each conversion)
    SDADC_CHANNEL_AVERAGE_8    = 12,              ///< Average 8 samples for each conversion end callback
    SDADC_CHANNEL_AVERAGE_16   = 13,              ///< Average 16 samples for each conversion end callback
    SDADC_CHANNEL_AVERAGE_32   = 14,              ///< Average 32 samples for each conversion end callback
    SDADC_CHANNEL_AVERAGE_64   = 15,              ///< Average 64 samples for each conversion end callback
} sdadc_channel_average_t;

/** Per channel polarity, valid for negative-side single-ended input only. */
typedef enum e_sdadc_channel_inversion
{
    SDADC_CHANNEL_INVERSION_OFF = 0,              ///< Do not invert conversion result
    SDADC_CHANNEL_INVERSION_ON  = 1,              ///< Invert conversion result
} sdadc_channel_inversion_t;

/** Select a formula to specify the number of conversions. The following symbols are used in the formulas:
 *    * N: Number of conversions
 *    * n: sdadc_channel_cfg_t::coefficient_n, do not set to 0 if m is 0
 *    * m: sdadc_channel_cfg_t::coefficient_m, do not set to 0 if n is 0
 *
 * Either m or n must be non-zero.
 */
typedef enum e_sdadc_channel_count_formula
{
    SDADC_CHANNEL_COUNT_FORMULA_EXPONENTIAL = 0,  ///< N = 32 * (2 ^ n - 1) + m * 2 ^ n
    SDADC_CHANNEL_COUNT_FORMULA_LINEAR      = 1,  ///< N = (32 * n) + m
} sdadc_channel_count_formula_t;

/** Calibration mode. */
typedef enum e_sdadc_calibration
{
    SDADC_CALIBRATION_INTERNAL_GAIN_OFFSET = 0,   ///< Use internal reference to calibrate offset and gain
    SDADC_CALIBRATION_EXTERNAL_OFFSET      = 1,   ///< Use external reference to calibrate offset
    SDADC_CALIBRATION_EXTERNAL_GAIN        = 2,   ///< Use external reference to calibrate gain
} sdadc_calibration_t;

/** Structure to pass to the adc_api_t::calibrate p_extend argument. */
typedef struct st_sdadc_calibrate_args
{
    adc_register_t       channel;               ///< Which channel to calibrate
    sdadc_calibration_t  mode;                  ///< Calibration mode
} sdadc_calibrate_args_t;

/** SDADC per channel configuration. */
typedef struct st_sdadc_channel_cfg
{
    sdadc_channel_stage_2_gain_t  stage_2_gain  :2; ///< Gain of PGA stage 2, must be 1 for single-ended input
    sdadc_channel_stage_1_gain_t  stage_1_gain  :3; ///< Gain of PGA stage 1, must be 1 for single-ended input
    sdadc_channel_oversampling_t  oversampling  :3; ///< Oversampling ratio, must be 256 in single-ended input
    uint32_t                                    :6;
    sdadc_channel_polarity_t      polarity      :1; ///< Polarity, valid for single-ended mode only
    sdadc_channel_input_t         input         :1; ///< Single-ended or differential input
    uint32_t                      coefficient_m :5; ///< See ::sdadc_channel_count_formula_t
    uint32_t                      coefficient_n :3; ///< See ::sdadc_channel_count_formula_t
    sdadc_channel_average_t       average       :4; ///< Number of samples to average for each conversion result
    sdadc_channel_inversion_t     invert        :1; ///< Whether to invert negative single-ended input
    uint32_t                                    :2;
    sdadc_channel_count_formula_t count_formula :1; ///< Linear or exponential formula used for number of conversions
} sdadc_channel_cfg_t;

/** SDADC configuration extension. This extension is required and must be provided in adc_cfg_t::p_extend. */
typedef struct st_sdadc_on_adc_cfg
{
    uint8_t            calibration_end_ipl;       ///< Calibration end interrupt priority
    uint8_t            conversion_end_ipl;        ///< Conversion end interrupt priority
    bool               skip_internal_calibration; ///< Whether to skip internal calibration of of the PGA during open
    sdadc_vref_src_t   vref_src;                  ///< Source of Vref (internal or external)

    /** Voltage of Vref, required for both internal and external Vref.  If Vref is from an external source, the
     * voltage must match the specified voltage within 3%. */
    sdadc_vref_voltage_t vref_voltage;
    sdadc_channel_cfg_t const * p_channel_cfgs[SDADC_MAX_NUM_CHANNELS]; ///< Configuration for each channel, set to NULL if unused
} adc_on_sdadc_cfg_t;

/** ADC instance control block. DO NOT INITIALIZE.  Initialized in adc_api_t::open(). */
typedef struct
{
    adc_mode_t               mode;            ///< Single scan or continuous mode
    adc_resolution_t         resolution;      ///< 16 or 24 bit resolution
    adc_alignment_t          alignment;       ///< Left or right alignment
    void const             * p_context;       ///< Placeholder for user data
    R_SDADC0_Type          * p_reg;           ///< Base register for this unit
    void                  (* p_callback)(adc_callback_args_t *p_args); ///< User callback pointer
    adc_trigger_t            trigger;         ///< Software or hardware trigger
    uint32_t                 trigger_enabled; ///< If set, hardware trigger was enabled before calibration
    uint32_t                 opened;          ///< Boolean to verify that the Unit has been initialized
    uint32_t                 scan_mask;       ///< Scan mask of enabled channels.
    uint32_t                 scan_cfg_mask;   ///< Scan mask of configured channels.
    uint16_t                 unit;            ///< SDADC Unit in use
    volatile uint8_t         calib_status;    ///< Calibration in progress if set
    IRQn_Type                scan_end_irq;    ///< Scan end IRQ number
    IRQn_Type                calib_end_irq;   ///< Calibration end IRQ number
    IRQn_Type                conv_end_irq;    ///< Conversion end IRQ number

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    union
    {
        uint16_t results_16[SDADC_MAX_NUM_CHANNELS];
        uint32_t results_32[SDADC_MAX_NUM_CHANNELS];
    } results;
} sdadc_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Interface Structure for user access */
extern const adc_api_t g_adc_on_sdadc;
/** @endcond */


/*******************************************************************************************************************//**
 * @} (end defgroup ADC)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_SDADC_H */
