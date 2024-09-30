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
 * File Name    : r_pdc_api.h
 * Description  : PDC API. Allows for capture from a connected camera.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup PDC_API PDC Interface
 * @brief Interface for PDC functions.
 *
 * @section PDC_API_SUMMARY Summary
 * @brief The PDC interface provides the functionality for capturing an image from a camera. When a capture is complete
 * a transfer complete interrupt is triggered.
 *
 * @section PDC_API_INSTANCES Known Implementations
 * @see PDC
 *
 * Related SSP architecture topics:
 *  - What is an SSP Interface? @ref ssp-interfaces
 *  - What is a SSP Layer? @ref ssp-predefined-layers
 *  - How to use SSP Interfaces and Modules? @ref using-ssp-modules
 *
 * @{
 **********************************************************************************************************************/

#ifndef DRV_PDC_API_H
#define DRV_PDC_API_H

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
#define PDC_API_VERSION_MAJOR (2U)
#define PDC_API_VERSION_MINOR (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** Clock divider applied to PDC clock to provide PCKO output frequency  */
typedef enum e_pdc_clock_division
{
    PDC_CLOCK_DIVISION_2  = 0u,     ///< CLK / 2
    PDC_CLOCK_DIVISION_4  = 1u,     ///< CLK / 4
    PDC_CLOCK_DIVISION_6  = 2u,     ///< CLK / 6
    PDC_CLOCK_DIVISION_8  = 3u,     ///< CLK / 8
    PDC_CLOCK_DIVISION_10 = 4u,     ///< CLK / 10
    PDC_CLOCK_DIVISION_12 = 5u,     ///< CLK / 12
    PDC_CLOCK_DIVISION_14 = 6u,     ///< CLK / 14
    PDC_CLOCK_DIVISION_16 = 7u,     ///< CLK / 16
} pdc_clock_division_t;

/** Endian of captured data  */
typedef enum e_pdc_endian
{
    PDC_ENDIAN_LITTLE = 0u,         ///< Data is in little endian format
    PDC_ENDIAN_BIG    = 1u,         ///< Data is in big endian format
} pdc_endian_t;

/** Polarity of input HSYNC signal  */
typedef enum e_pdc_hsync_polarity
{
    PDC_HSYNC_POLARITY_HIGH = 0u,   ///< HSYNC signal is active high
    PDC_HSYNC_POLARITY_LOW  = 1u,   ///< HSYNC signal is active low
} pdc_hsync_polarity_t;

/** Polarity of input VSYNC signal  */
typedef enum e_pdc_vsync_polarity
{
    PDC_VSYNC_POLARITY_HIGH = 0u,   ///< VSYNC signal is active high
    PDC_VSYNC_POLARITY_LOW  = 1u,   ///< VSYNC signal is active low
} pdc_vsync_polarity_t;

/** PDC events */
typedef enum e_pdc_event
{
    PDC_EVENT_TRANSFER_COMPLETE = 0u,       ///< Complete frame transferred by DMAC/DTC
    PDC_EVENT_RX_DATA_READY     = 0x01u,    ///< Receive data ready interrupt
    PDC_EVENT_FRAME_END         = 0x02u,    ///< Frame end interrupt
    PDC_EVENT_ERR_OVERRUN       = 0x04u,    ///< Overrun interrupt
    PDC_EVENT_ERR_UNDERRUN      = 0x08u,    ///< Underrun interrupt
    PDC_EVENT_ERR_V_SET         = 0x10u,    ///< Vertical line setting error interrupt
    PDC_EVENT_ERR_H_SET         = 0x20u,    ///< Horizontal byte number setting error interrupt
} pdc_event_t;

/** VSYNC signal state */
typedef enum e_pdc_vsync_state
{
    PDC_VSYNC_STATE_LOW  = 0u,      ///< VSYNC signal is low
    PDC_VSYNC_STATE_HIGH = 1u,      ///< VSYNC signal is high
} pdc_vsync_state_t;

/** HSYNC signal state */
typedef enum e_pdc_hsync_state
{
    PDC_HSYNC_STATE_LOW  = 0u,      ///< HSYNC signal is low
    PDC_HSYNC_STATE_HIGH = 1u,      ///< HSYNC signal is high
} pdc_hsync_state_t;

/** PDC VSYNC/HSYNC state */
typedef struct st_pdc_state
{
    pdc_vsync_state_t  vsync;       ///< VSYNC signal state
    pdc_hsync_state_t  hsync;       ///< HSYNC signal state
} pdc_state_t;

/** Callback function parameter data */
typedef struct st_pdc_callback_args
{
    pdc_event_t  event;             ///< Event causing the callback
    uint8_t    * p_buffer;          ///< Pointer to buffer containing the captured data
    void const * p_context;         ///< Placeholder for user data.  Set in pdc_api_t::open function in ::pdc_cfg_t.
} pdc_callback_args_t;

/** PDC configuration parameters. */
typedef struct st_pdc_cfg
{
    uint16_t                           x_capture_start_pixel; ///< Horizontal position to start capture
    uint16_t                           x_capture_pixels;      ///< Number of horizontal pixels to capture
    uint16_t                           y_capture_start_pixel; ///< Vertical position to start capture
    uint16_t                           y_capture_pixels;      ///< Number of vertical lines/pixels to capture
    pdc_clock_division_t               clock_division;        ///< Clock divider
    pdc_endian_t                       endian;                ///< Endian of capture data
    pdc_hsync_polarity_t               hsync_polarity;        ///< Polarity of HSYNC input
    pdc_vsync_polarity_t               vsync_polarity;        ///< Polarity of VSYNC input
    uint8_t                          * p_buffer;              ///< Pointer to buffer to write image into
    uint8_t                            bytes_per_pixel;       ///< Number of bytes per pixel
    uint8_t                            frame_end_ipl;         ///< Frame end interrupt priority
    uint8_t                            irq_ipl;               ///< PDC interrupt priority
    transfer_instance_t const * const  p_lower_lvl_transfer;  ///< Pointer to the transfer instance the PDC should use
    void (* p_callback)(pdc_callback_args_t * p_args);        ///< Callback provided when a PDC transfer ISR occurs.
    /** Placeholder for user data.  Passed to the user callback in ::pdc_callback_args_t. */
    void const  * p_context;
    /** Extension parameter for hardware specific settings. */
    void const  * p_extend;
} pdc_cfg_t;

/** PDC control block.  Allocate an instance specific control block to pass into the PDC API calls.
 * @par Implemented as
 * - pdc_instance_ctrl_t
 */
typedef void pdc_ctrl_t;

/** PDC functions implemented at the HAL layer will follow this API. */
typedef struct st_pdc_api
{
    /** Initial configuration.
     * @par Implemented as
     * - R_PDC_Open()
     *
     * @note To reconfigure after calling this function, call pdc_api_t::close first.
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_cfg        Pointer to pin configuration structure.
     */
    ssp_err_t (* open)(pdc_ctrl_t * const p_ctrl, pdc_cfg_t const * const p_cfg);

    /** Closes the driver and allows reconfiguration. May reduce power consumption.
     * @par Implemented as
     * - R_PDC_Close()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    ssp_err_t (* close)(pdc_ctrl_t * const p_ctrl);

    /** Start a capture.
     * @par Implemented as
     * - R_PDC_CaptureStart()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_buffer     Pointer to store captured image data.
     */
    ssp_err_t (* captureStart)(pdc_ctrl_t * const p_ctrl, uint8_t * const p_buffer);

    /** Get the state of the VSYNC and HSYNC pins.
     * @par Implemented as
     * - R_PDC_StateGet()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_state      Pointer to store state data.
     */
    ssp_err_t (* stateGet)(pdc_ctrl_t * const p_ctrl, pdc_state_t * p_state);

    /** Return the version of the driver.
     * @par Implemented as
     * - R_PDC_VersionGet()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[out] p_data       Memory address to return version information to.
     */
    ssp_err_t (* versionGet)(ssp_version_t * const p_data);
} pdc_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_pdc_instance
{
    pdc_ctrl_t       * p_ctrl;  ///< Pointer to the control structure for this instance
    pdc_cfg_t const  * p_cfg;   ///< Pointer to the configuration structure for this instance
    pdc_api_t const  * p_api;   ///< Pointer to the API structure for this instance
} pdc_instance_t;

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_PDC_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup PDC_API)
 **********************************************************************************************************************/
