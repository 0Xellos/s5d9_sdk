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
 * File Name    : r_agt_input_capture.h
 * Description  : Prototypes of AGT functions used to implement the input capture interface.
 **********************************************************************************************************************/

#ifndef R_AGT_INPUT_CAPTURE_H
#define R_AGT_INPUT_CAPTURE_H

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup AGT_INPUT_CAPTURE AGT Input Capture
 * @brief Driver for the Asynchronous General-Purpose Timer (AGT) with Input Capture.
 *
 * @section AGT_INPUT_CAPTURE_SUMMARY Summary
 * Extends @ref INPUT_CAPTURE_API.
 *
 * This module implements the @ref INPUT_CAPTURE_API for the Asynchronous General-Purpose Timer (AGT) peripherals.
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_input_capture_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define AGT_INPUT_CAPTURE_CODE_VERSION_MAJOR (2U)
#define AGT_INPUT_CAPTURE_CODE_VERSION_MINOR (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** Count source */
typedef enum e_agt_input_capture_count_source
{
    AGT_INPUT_CAPTURE_CLOCK_PCLKB          = 0U, ///< Clock AGT_CLOCK_PCLKB.
    AGT_INPUT_CAPTURE_CLOCK_PCLKB_DIV_8    = 1U, ///< Superseded: See AGT_CLOCK_PCLKB.
    AGT_INPUT_CAPTURE_CLOCK_PCLKB_DIV_2    = 3U, ///< Superseded: See AGT_CLOCK_PCLKB.
    AGT_INPUT_CAPTURE_CLOCK_LOCO           = 4U, ///< Divided clock LOCO specified by bits CKS[2:0] in the AGTMR2 register
    AGT_CLOCK_INPUT_CAPTURE_FSUB           = 6U, ///< Divided clock fSUB specified by bits CKS[2:0] in the AGTMR2 register
} agt_input_capture_count_source_t;

/** AGT Input capture signal edge polarity for event counter mode */
typedef enum e_agt_input_capture_count_edges
{
    INPUT_CAPTURE_SIGNAL_SINGLE_EDGE,   ///< Counts only one edge of the pulse
    INPUT_CAPTURE_SIGNAL_BOTH_EDGE,     ///< Counts both edges of the pulse
} agt_input_capture_count_edges_t;

/** Input capture signal noise filter (debounce) setting. Only available for input signals in AGTIO pins.
 *  The noise filter samples the external signal at intervals of the PCLK divided by one of the values.
 *  When 3 consecutive samples are at the same level (high or low), then that level is passed on as
 *  the observed state of the signal. See "Noise Filter Function" in the hardware manual, AGT section.
 */
typedef enum e_agt_input_capture_signal_filter
{
    AGT_INPUT_CAPTURE_SIGNAL_FILTER_NONE,   ///< NO FILTER
    AGT_INPUT_CAPTURE_SIGNAL_FILTER_1,      ///< PCLK/1
    AGT_INPUT_CAPTURE_SIGNAL_FILTER_8,      ///< PCLK/8
    AGT_INPUT_CAPTURE_SIGNAL_FILTER_32,     ///< PCLK/32
} agt_input_capture_signal_filter_t;

/** AGT Input capture AGT LOCO or AGT FSUB divider. */
typedef enum e_agt_input_capture_clock_divider
{
    AGT_INPUT_CAPTURE_CLOCK_DIVIDER_1,      ///< / 1
    AGT_INPUT_CAPTURE_CLOCK_DIVIDER_2,      ///< / 2
    AGT_INPUT_CAPTURE_CLOCK_DIVIDER_4,      ///< / 4
    AGT_INPUT_CAPTURE_CLOCK_DIVIDER_8,      ///< / 8
    AGT_INPUT_CAPTURE_CLOCK_DIVIDER_16,     ///< / 16
    AGT_INPUT_CAPTURE_CLOCK_DIVIDER_32,     ///< / 32
    AGT_INPUT_CAPTURE_CLOCK_DIVIDER_64,     ///< / 64
    AGT_INPUT_CAPTURE_CLOCK_DIVIDER_128,    ///< / 128
} agt_input_capture_clock_divider_t;

/** AGT Input capture Event flags. */
typedef enum e_agt_input_capture_event_flag
{
    AGT_INPUT_CAPTURE_ACTIVE_EDGE_FLAG = 16U,   ///< Measurement event flag
    AGT_INPUT_CAPTURE_UNDERFLOW_FLAG = 32U,     ///< Underflow event flag
    AGT_INPUT_CAPTURE_COMPARE_A_FLAG = 64U,     ///< Compare match A event flag
    AGT_INPUT_CAPTURE_COMPARE_B_FLAG = 128U,    ///< Compare match B event flag
} agt_input_capture_event_flag_t;

/** AGT Input capture modes. */
typedef enum e_agt_input_capture_mode
{
    AGT_INPUT_CAPTURE_MODE_PULSE_WIDTH = 3U,    ///< Measure a signal pulse width.
    AGT_INPUT_CAPTURE_MODE_PERIOD = 4U,         ///< Measure a signal cycle period.
    AGT_INPUT_CAPTURE_MODE_PULSE_COUNT = 2U,    ///< Measure a signal event count.
} agt_input_capture_mode_t;

/** AGT Input capture AGTIO Pin Select. */
typedef enum e_agt_input_capture_pin_select
{
    AGT_INPUT_CAPTURE_PIN_AGTIO_A = 0U,     ///< Selects the pin AGTIO_A for input capture.
    AGT_INPUT_CAPTURE_PIN_AGTIO_B = 2U,     ///< Selects the pin AGTIO_B for input capture.
    AGT_INPUT_CAPTURE_PIN_AGTIO_C = 3U,     ///< Selects the pin AGTIO_C for input capture.
} agt_input_capture_pin_select_t;

/** @brief Extension configuration struct for AGT Input Capture.
 *
 * Pointed to by input_capture_cfg_t.p_extend
 */
typedef struct st_agt_input_capture_extend
{
    uint16_t                            pulse_count_value;  ///< Selects the pulse count value for pulse count capture.
    agt_input_capture_signal_filter_t   signal_filter;      ///< Selects the input signal filter
    agt_input_capture_count_source_t    count_source;       ///< Selects the input count clock source.
    agt_input_capture_clock_divider_t   clock_divider;      ///< Selects the AGT LOCO or AGT FSUB divider.
    agt_input_capture_count_edges_t     count_edge;         ///< Selects the count edge for pulse count capture.
    agt_input_capture_pin_select_t      pin_select;         ///< Selects the pin for agt input capture.
} agt_input_capture_extend_t;

/** Channel control block. DO NOT INITIALIZE.  Initialization occurs when input_capture_api_t::open is called. */
typedef struct st_agt_input_capture_instance_ctrl
{
    uint32_t                    open;               ///< Whether or not channel is open
    uint8_t                     channel;            ///< The channel in use.
    uint8_t                     flags;              ///< Input capture Event flags.
    input_capture_mode_t        mode;               ///< The mode of measurement being performed.
    input_capture_repetition_t  repetition;         ///< One-shot or periodic measurement.
    volatile uint32_t           capture_count;      ///< The value of the timer captured at the time of interrupt.
    volatile uint32_t           overflows_current;  ///< Running count of overflows in current measurement
    void (*p_callback) (input_capture_callback_args_t * p_args);    ///< Pointer to user callback
    void const                * p_context;          ///< Pointer to user's context data, to be passed to the callback function.
    void                      * p_reg;              ///< AGT base register for this channel
    IRQn_Type                   capture_irq;        ///< Capture IRQ number
    IRQn_Type                   overflow_irq;       ///< Overflow IRQ number
    volatile bool               pulse_period_first_edge;    ///< Whether the first edge in period has received
} agt_input_capture_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const input_capture_api_t g_input_capture_on_agt;
/** @endcond */

/*******************************************************************************************************************//**
 * @} (end defgroup AGT_INPUT_CAPTURE)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_AGT_INPUT_CAPTURE_H
