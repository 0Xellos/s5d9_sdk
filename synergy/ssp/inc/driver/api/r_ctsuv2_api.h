/***********************************************************************************************************************
 * Copyright [2021-2024] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : r_ctsuv2_api.h
 * Description  : CTSU Interface
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup CTSU_V2_API CTSU v2 Interface
 * @brief Interface for Capacitive Touch Controllers.
 *
 * @section CTSU_V2_API_SUMMARY Summary
 * The CTSU v2 interface provides the functionality necessary to open, close, run and control the
 * CTSU depending upon the configuration passed as arguments.
 *
 * Implemented by:
 * @ref CTSU_V2
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * CTSU v2 Interface description: @ref HALCTSUV2Interface
 * @{
 **********************************************************************************************************************/

#ifndef R_CTSU_API_H
#define R_CTSU_API_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/* Register definitions, common services and error codes. */
#include "bsp_api.h"
#include "r_transfer_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define CTSU_API_VERSION_MAJOR              (2U)
#define CTSU_API_VERSION_MINOR              (0U)
#define CTSU_TARGET_VALUE_CONFIG_SUPPORT    (1)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** CTSU Events for callback function */
typedef enum e_ctsu_event
{
    CTSU_EVENT_SCAN_COMPLETE = 0x00U,  ///< Normal end
    CTSU_EVENT_OVERFLOW      = 0x01U,  ///< Sensor counter overflow (CTSUST.CTSUSOVF set)
    CTSU_EVENT_ICOMP         = 0x02U,  ///< Abnormal TSCAP voltage (CTSUERRS.CTSUICOMP set)
} ctsu_event_t;

/** CTSU Scan Start Trigger Select */
typedef enum e_ctsu_cap
{
    CTSU_CAP_SOFTWARE,                 ///< Scan start by software trigger
    CTSU_CAP_EXTERNAL                  ///< Scan start by external trigger
} ctsu_cap_t;

/** CTSU Power Supply Capacity Adjustment */
typedef enum e_ctsu_atune1
{
    CTSU_ATUNE1_NORMAL = 0U,           ///< Normal output (40uA)
    CTSU_ATUNE1_HIGH                   ///< High-current output (80uA)
} ctsu_atune1_t;

/** CTSU Measurement Mode Select */
typedef enum e_ctsu_mode
{
    CTSU_MODE_SELF_MULTI_SCAN  = 1U,   ///< Self-capacitance multi scan mode
    CTSU_MODE_MUTUAL_FULL_SCAN = 3U,   ///< Mutual capacitance full scan mode
    CTSU_MODE_DIAGNOSIS_SCAN   = 33    ///< Diagnosis scan mode
} ctsu_md_t;

/** CTSU Spectrum Diffusion Frequency Division Setting */
typedef enum e_ctsu_ssdiv
{
    CTSU_SSDIV_4000,                   ///< 4.00 <= Base clock frequency (MHz)
    CTSU_SSDIV_2000,                   ///< 2.00 <= Base clock frequency (MHz) < 4.00
    CTSU_SSDIV_1330,                   ///< 1.33 <= Base clock frequency (MHz) < 2.00
    CTSU_SSDIV_1000,                   ///< 1.00 <= Base clock frequency (MHz) < 1.33
    CTSU_SSDIV_0800,                   ///< 0.80 <= Base clock frequency (MHz) < 1.00
    CTSU_SSDIV_0670,                   ///< 0.67 <= Base clock frequency (MHz) < 0.80
    CTSU_SSDIV_0570,                   ///< 0.57 <= Base clock frequency (MHz) < 0.67
    CTSU_SSDIV_0500,                   ///< 0.50 <= Base clock frequency (MHz) < 0.57
    CTSU_SSDIV_0440,                   ///< 0.44 <= Base clock frequency (MHz) < 0.50
    CTSU_SSDIV_0400,                   ///< 0.40 <= Base clock frequency (MHz) < 0.44
    CTSU_SSDIV_0360,                   ///< 0.36 <= Base clock frequency (MHz) < 0.40
    CTSU_SSDIV_0330,                   ///< 0.33 <= Base clock frequency (MHz) < 0.36
    CTSU_SSDIV_0310,                   ///< 0.31 <= Base clock frequency (MHz) < 0.33
    CTSU_SSDIV_0290,                   ///< 0.29 <= Base clock frequency (MHz) < 0.31
    CTSU_SSDIV_0270,                   ///< 0.27 <= Base clock frequency (MHz) < 0.29
    CTSU_SSDIV_0000                    ///< 0.00 <= Base clock frequency (MHz) < 0.27
} ctsu_ssdiv_t;

/** Callback function parameter data */
typedef struct st_ctsu_callback_args
{
    ctsu_event_t event;                ///< The event can be used to identify what caused the callback.
    void const * p_context;            ///< Placeholder for user data. Set in ctsu_api_t::open function in ::ctsu_cfg_t.
} ctsu_callback_args_t;

/** CTSU Control block. Allocate an instance specific control block to pass into the API calls.
 * @par Implemented as
 * - ctsu_instance_ctrl_t
 */
typedef void ctsu_ctrl_t;

/** CTSU Configuration parameters. */
/** Element Configuration */
typedef struct st_ctsu_element
{
    ctsu_ssdiv_t ssdiv;                ///< CTSU Spectrum Diffusion Frequency Division Setting (CTSU Only)
    uint16_t     so;                   ///< CTSU Sensor Offset Adjustment
    uint8_t      snum;                 ///< CTSU Measurement Count Setting
    uint8_t      sdpa;                 ///< CTSU Base Clock Setting
} ctsu_element_cfg_t;

/** User configuration structure, used in open function */
typedef struct st_ctsu_cfg
{
    ctsu_cap_t                 cap;                     ///< CTSU Scan Start Trigger Select
    ctsu_atune1_t              atune1;                  ///< CTSU Power Supply Capacity Adjustment
    ctsu_md_t                  md;                      ///< CTSU Measurement Mode Select
    uint8_t                    ctsuchac0;               ///< TS00-TS07 enable mask
    uint8_t                    ctsuchac1;               ///< TS08-TS15 enable mask
    uint8_t                    ctsuchac2;               ///< TS16-TS23 enable mask
    uint8_t                    ctsuchac3;               ///< TS24-TS31 enable mask
    uint8_t                    ctsuchac4;               ///< TS32-TS39 enable mask
    uint8_t                    ctsuchtrc0;              ///< TS00-TS07 mutual-tx mask
    uint8_t                    ctsuchtrc1;              ///< TS08-TS15 mutual-tx mask
    uint8_t                    ctsuchtrc2;              ///< TS16-TS23 mutual-tx mask
    uint8_t                    ctsuchtrc3;              ///< TS24-TS31 mutual-tx mask
    uint8_t                    ctsuchtrc4;              ///< TS32-TS39 mutual-tx mask
    ctsu_element_cfg_t const * p_elements;              ///< Pointer to elements configuration array
    uint8_t                    num_rx;                  ///< Number of receive terminals
    uint8_t                    num_tx;                  ///< Number of transmit terminals
    uint16_t                   num_moving_average;      ///< Number of moving average for measurement data
    bool tunning_enable;                                ///< Initial offset tuning flag
    void (* p_callback)(ctsu_callback_args_t * p_args); ///< Callback provided when CTSUFN ISR occurs.
    transfer_instance_t const * p_transfer_tx;          ///< DTC instance for transmit at CTSUWR. Set to NULL if unused.
    transfer_instance_t const * p_transfer_rx;          ///< DTC instance for receive at CTSURD. Set to NULL if unused.
    IRQn_Type    write_irq;                             ///< CTSU_CTSUWR interrupt vector
    IRQn_Type    read_irq;                              ///< CTSU_CTSURD interrupt vector
    IRQn_Type    end_irq;                               ///< CTSU_CTSUFN interrupt vector
    void const * p_context;                             ///< User defined context passed into callback function.
    void const * p_extend;                              ///< Pointer to extended configuration by instance of interface.
    uint16_t     tuning_self_target_value;              ///< Target self value for initial offset tuning
    uint16_t     tuning_mutual_target_value;            ///< Target mutual value for initial offset tuning
} ctsu_cfg_t;

/** Functions implemented at the HAL layer will follow this API. */
typedef struct st_ctsu_api
{
    /** Open driver.
     * @par Implemented as
     * - @ref R_CTSU_Open()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_cfg        Pointer to pin configuration structure.
     */
    ssp_err_t (* open)(ctsu_ctrl_t * const p_ctrl, ctsu_cfg_t const * const p_cfg);

    /** Scan start.
     * @par Implemented as
     * - @ref R_CTSU_ScanStart()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    ssp_err_t (* scanStart)(ctsu_ctrl_t * const p_ctrl);

    /** Data get.
     * @par Implemented as
     * - @ref R_CTSU_DataGet()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[out] p_data       Pointer to get data array.
     */
    ssp_err_t (* dataGet)(ctsu_ctrl_t * const p_ctrl, uint16_t * p_data);

    /** Diagnosis.
     * @par Implemented as
     * - @ref R_CTSU_Diagnosis()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    ssp_err_t (* diagnosis)(ctsu_ctrl_t * const p_ctrl);

    /** Specify callback function and optional context pointer and working memory pointer.
     * @par Implemented as
     * - @ref R_CTSU_CallbackSet()
     *
     * @param[in]   p_ctrl                   Pointer to the CTSU control block.
     * @param[in]   p_callback               Callback function
     * @param[in]   p_context                Pointer to send to callback function
     * @param[in]   p_working_memory         Pointer to volatile memory where callback structure can be allocated.
     *                                       Callback arguments allocated here are only valid during the callback.
     */
    ssp_err_t (* callbackSet)(ctsu_ctrl_t * const p_api_ctrl, void (* p_callback)(ctsu_callback_args_t *),
                              void const * const p_context, ctsu_callback_args_t * const p_callback_memory);

    /** Close driver.
     * @par Implemented as
     * - @ref R_CTSU_Close()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    ssp_err_t (* close)(ctsu_ctrl_t * const p_ctrl);

    /** Return the version of the driver.
     * @par Implemented as
     * - @ref R_CTSU_VersionGet()
     *
     * @param[out] p_data       Memory address to return version information to.
     */
    ssp_err_t (* versionGet)(ssp_version_t * const p_data);
} ctsu_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_ctsu_instance
{
    ctsu_ctrl_t      * p_ctrl;         ///< Pointer to the control structure for this instance
    ctsu_cfg_t const * p_cfg;          ///< Pointer to the configuration structure for this instance
    ctsu_api_t const * p_api;          ///< Pointer to the API structure for this instance
} ctsu_instance_t;

/** Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif                                 /* R_CTSU_API_H */

/*******************************************************************************************************************//**
 * @} (end defgroup CTSU_V2_API)
 **********************************************************************************************************************/
