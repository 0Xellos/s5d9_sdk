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
* File Name    : r_analog_connect_api.h
* Description  : Analog internal connection interface file.
***********************************************************************************************************************/

#ifndef R_ANALOG_CONNECT_API_H
#define R_ANALOG_CONNECT_API_H

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup ANALOG_CONNECT_API Analog Connect Interface
 * @brief Interface for analog connections.
 *
 * @section ANALOG_CONNECT_API_SUMMARY Summary
 * The analog connection interface allows the user to configure internal analog connections.
 *
 * Implemented by:
 * @ref ANALOG_CONNECT
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * Analog Connect Interface description: @ref HALAnalogConnectModule
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
#define ANALOG_CONNECT_API_VERSION_MAJOR   (2U)
#define ANALOG_CONNECT_API_VERSION_MINOR   (0U)

/*****************************************************************************
Typedef definitions
******************************************************************************/
/** User configuration structure, used in init function */
typedef struct st_analog_connect_cfg
{
    uint8_t unused;             ///< Placeholder for future use
} analog_connect_cfg_t;

/** Table of connections. */
typedef struct st_analog_connect_table
{
    uint32_t                 number_of_connections; ///< Number of connections in the table
    analog_connect_t const * p_connection_table;    ///< List of connections
} analog_connect_table_t;

/** Comparator functions implemented at the HAL layer will follow this API. */
typedef struct st_analog_connect_api
{
    /** Initialize the analog connect module.
     *
     * @par Implemented as
     *  - R_ANALOG_CONNECT_Init()
     *
     * @param[in]  p_cfg   Pointer to configuration
     */
    ssp_err_t (* init)(analog_connect_cfg_t const * const p_cfg);

    /** Make one internal analog connection.
     *
     * @par Implemented as
     *  - R_ANALOG_CONNECT_Connect()
     *
     * @param[in]  connection    Internal analog connection to make
     */
    ssp_err_t (* connect)(analog_connect_t const connection);

    /** Make multiple internal analog connections.  Connections are made in the order they are listed in the table.
     * This API is most efficient when all connections for the same module/channel combination are grouped together.
     *
     * @par Implemented as
     *  - R_ANALOG_CONNECT_ConnectMultiple()
     *
     * @param[in]  p_table    Pointer to table of internal analog connection to make
     */
    ssp_err_t (* connectMultiple)(analog_connect_table_t const * const p_table);

    /** Retrieve the API version.
     *
     * @par Implemented as
     *  - R_ANALOG_CONNECT_VersionGet()
     *
     * @param[in]  p_version   Pointer to version structure
     */
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);

} analog_connect_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_analog_connect_instance
{
    analog_connect_cfg_t         const * p_cfg;          ///< Pointer to the configuration structure for this instance
    analog_connect_api_t         const * p_api;          ///< Pointer to the API structure for this instance
} analog_connect_instance_t;

/*******************************************************************************************************************//**
 * @} (end defgroup ANALOG_CONNECT_API)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_ANALOG_CONNECT_API_H */
