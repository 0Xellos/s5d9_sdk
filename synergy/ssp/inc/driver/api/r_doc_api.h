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
 * File Name    : r_doc_api.h
 * Description  : Data Operation Circuit (DOC) driver interface.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup DOC_API DOC Interface
 *
 * @brief Interface for the Data Operation Circuit.
 *
 * Defines the API and data structures for the DOC implementation of the Data Operation Circuit (DOC) interface.
 *
 * @section DOC_API_SUMMARY Summary
 * @brief This module implements the DOC_API using the Data Operation Circuit (DOC).
 *
 * Implemented by:
 * @ref DOC
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * DOC Interface description: @ref HALDOCInterface
 *
 * @{
 **********************************************************************************************************************/

#ifndef DRV_DOC_API_H
#define DRV_DOC_API_H


/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/** Register definitions, common services and error codes. */
#include "bsp_api.h"
#include "r_transfer_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define DOC_API_VERSION_MAJOR (2U)
#define DOC_API_VERSION_MINOR (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** Size of the comparison data supported by the Data Operation Circuit (DOC) */
typedef uint16_t doc_size_t;

/** Event that can trigger a callback function. */
typedef enum e_doc_event
{
    DOC_EVENT_COMPARISON_MISMATCH = 0x00,           ///< Comparison of data has resulted in a mismatch.
    DOC_EVENT_ADDITION            = 0x01,           ///< Addition of data has resulted in a value greater than H'FFFF.
    DOC_EVENT_SUBTRACTION         = 0x02,           ///< Subtraction of data has resulted in a value less than H'0000.
    DOC_EVENT_COMPARISON_MATCH    = 0x04,           ///< Comparison of data has resulted in a match.
} doc_event_t;

/** Status of the data comparison operation.  */
typedef enum e_doc_status
{
    DOC_STATUS_CONDITION_FALSE = 0,                 ///< Data comparison condition NOT met (match or mismatch),
                                                    ///< addition result NOT > H'FFFF, subtraction result NOT < H'0000.
    DOC_STATUS_CONDITION_TRUE  = 1,                 ///< Data comparison condition met (match or mismatch),
                                                    ///< addition result > H'FFFF, subtraction result < H'0000.
} doc_status_t;

/** Callback function parameter data. */
typedef struct st_doc_callback_args
{
    doc_event_t  event;                             ///< The event is used to identify what caused the callback.
    void const * p_context;                         ///< Placeholder for user data.
                                                    ///< Set in doc_api_t::open function in ::doc_cfg_t.
} doc_callback_args_t;

/** DOC control block.  Allocate an instance specific control block to pass into the DOC API calls.
 * @par Implemented as
 * - doc_instance_ctrl_t
 */
typedef void doc_ctrl_t;

/** Data to be written to DOC register for comparison/addition/subtraction. */
typedef struct st_doc_data
{
    doc_size_t  dodir;                              ///< Value to be written to the DOC DODIR.
    doc_size_t  dodsr;                              ///< Value to be written to the DOC DODSR.
} doc_data_t;

/** User configuration structure, used in the open function. */
typedef struct st_doc_cfg
{
    doc_event_t  event;                             ///< Select enumerated value from ::doc_event_t.
    uint8_t      irq_ipl;                           ///< DOC interrupt priority

    /** Callback provided when a DOC ISR occurs. */
    void (* p_callback)(doc_callback_args_t * p_args);

    /** Placeholder for user data. Passed to the user callback in ::doc_callback_args_t. */
    void const * p_context;
} doc_cfg_t;

/** Data Operation Circuit (DOC) API structure. DOC functions implemented at the HAL layer will follow this API. */
typedef struct st_doc_api
{
    /** Initial configuration.
     * @par Implemented as
     *  - R_DOC_Open()
     * @pre Peripheral clocks should be configured prior to calling this function.
     * @param[in]   p_ctrl		Pointer to control block. Must be declared by user. Elements set here.
     * @param[in]   p_cfg       Pointer to configuration structure. All elements of this structure must be set by user.
     */
    ssp_err_t (* open)(doc_ctrl_t       * const p_ctrl, doc_cfg_t const * const p_cfg);

    /**Allow the driver to be reconfigured. Will reduce power consumption.
     * @par Implemented as
     *  - R_DOC_Close()
     * @param[in]	p_ctrl		Control block set in doc_api_t::open call.
     */
    ssp_err_t (* close)(doc_ctrl_t      * const p_ctrl);

    /** Get the DOC status and stores it in the provided pointer p_status.
     * @par Implemented as
     *  - R_DOC_StatusGet()
     * @pre Call doc_api_t::open to configure the DOC before using this function.
     * @param[in]	p_ctrl		Control block set in doc_api_t::open call.
     * @param[out]	p_status	Indicates the status of the comparison/addition/subtraction operation.
     *                          Result will be one of ::doc_status_t. */
    ssp_err_t (* statusGet)(doc_ctrl_t  * const p_ctrl, doc_status_t * p_status);

    /** Clear DOPCF status flag.
     * @par Implemented as
     *  - R_DOC_StatusClear()
     * @pre Call doc_api_t::open to configure the DOC before using this function.
     * @param[in]	p_ctrl		Control block set in doc_api_t::open call.
     */
    ssp_err_t (* statusClear)(doc_ctrl_t    * const p_ctrl);

    /** Write to the DODIR and DODSR registers.
     * @par Implemented as
     *  - R_DOC_Write()
     *
     * @pre Call doc_api_t::open to configure the DOC before using this function.
     * @param[in]	p_ctrl		Control block set in doc_api_t::open call.
     * @param[in]	p_data		Pointer to data to be written to DOC DODIR and DODSR registers.
     */
    ssp_err_t (* write)(doc_ctrl_t  * const p_ctrl, doc_data_t * const p_data);

    /** Write to the DODIR register.
     * @par Implemented as
     *  - R_DOC_InputRegisterWrite()
     * @pre Call doc_api_t::open to configure the DOC before using this function.
     * @param[in]	p_ctrl		Control block set in doc_api_t::open call.
     * @param[in]	data		Data to be written to DOC DODIR register.
     */
    ssp_err_t (* inputRegisterWrite)(doc_ctrl_t * const p_ctrl, doc_size_t data);

    /** Get version and stores it in provided pointer p_version.
     * @par Implemented as
     *  - R_DOC_VersionGet()
     *
     * @param[out]  p_version  Code and API version used.
     */
    ssp_err_t (* versionGet)(ssp_version_t     * const p_version);
} doc_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_doc_instance
{
    doc_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    doc_cfg_t const * p_cfg;     ///< Pointer to the configuration structure for this instance
    doc_api_t const * p_api;     ///< Pointer to the API structure for this instance
} doc_instance_t;

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_DOC_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup DOC_API)
 **********************************************************************************************************************/
