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
* File Name    : r_i2s_api.h
* Description  : I2S public API definitions.
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup I2S_API I2S Interface
 * @brief The I2S (Inter-IC Sound) interface provides APIs and definitions for I2S audio communication.
 *
 * @section I2S_API_SUMMARY Summary
 * @brief The I2S (Inter-IC Sound) interface provides APIs and definitions for I2S audio communication.
 *
 * @section I2S_API_INSTANCES Known Implementations
 * @ref SSI
 *
 * @section I2S_API_RELATED Related modules
 * See also: @ref SF_AUDIO_PLAYBACK_HW_I2S
 * @{
 **********************************************************************************************************************/

#ifndef R_I2S_API_H
#define R_I2S_API_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
/** Register definitions, common services and error codes. */
#include "bsp_api.h"
#include "r_timer_api.h"
#include "r_transfer_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define I2S_API_VERSION_MAJOR   (2U)
#define I2S_API_VERSION_MINOR   (0U)

/**********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/** Audio PCM width */
typedef enum e_i2s_pcm_width
{
    I2S_PCM_WIDTH_8_BITS  = 0,      ///< Using 8-bit PCM
    I2S_PCM_WIDTH_16_BITS = 1,      ///< Using 16-bit PCM
    I2S_PCM_WIDTH_18_BITS = 2,      ///< Using 18-bit PCM
    I2S_PCM_WIDTH_20_BITS = 3,      ///< Using 20-bit PCM
    I2S_PCM_WIDTH_22_BITS = 4,      ///< Using 22-bit PCM
    I2S_PCM_WIDTH_24_BITS = 5,      ///< Using 24-bit PCM
} i2s_pcm_width_t;

/** Audio system word length. */
typedef enum e_i2s_word_length
{
    I2S_WORD_LENGTH_8_BITS  = 0,    ///< Using 8-bit system word length
    I2S_WORD_LENGTH_16_BITS = 1,    ///< Using 16-bit system word length
    I2S_WORD_LENGTH_24_BITS = 2,    ///< Using 24-bit system word length
    I2S_WORD_LENGTH_32_BITS = 3,    ///< Using 32-bit system word length
} i2s_word_length_t;

/** Events that can trigger a callback function */
typedef enum e_i2s_event
{
    I2S_EVENT_IDLE,             ///< Communication is idle
    I2S_EVENT_TX_EMPTY,         ///< Transmit buffer is below FIFO trigger level
    I2S_EVENT_RX_FULL,          ///< Receive buffer is above FIFO trigger level
} i2s_event_t;

/** I2S communication direction */
typedef enum e_i2s_dir
{
    I2S_DIR_TX,                 ///< Transmit direction only
    I2S_DIR_RX,                 ///< Receive direction only
    I2S_DIR_TX_RX,              ///< Transmit and receive direction
} i2s_dir_t;

/** I2S communication mode */
typedef enum e_i2s_mode
{
    I2S_MODE_SLAVE  = 0,        ///< Slave mode
    I2S_MODE_MASTER = 1,        ///< Master mode
} i2s_mode_t;

/** Mute audio samples. */
typedef enum e_i2s_mute
{
    I2S_MUTE_ON  = 0,           ///< Enable mute
    I2S_MUTE_OFF = 1,           ///< Disable mute
} i2s_mute_t;

/** Whether to continue WS (word select line) transmission during idle state. */
typedef enum e_i2s_ws_continue
{
    I2S_WS_CONTINUE_ON  = 0,    ///< Enable WS continue mode
    I2S_WS_CONTINUE_OFF = 1,    ///< Disable WS continue mode
} i2s_ws_continue_t;

/** Possible status values returned by i2s_api_t::infoGet. */
typedef enum e_i2s_status
{
    I2S_STATUS_IN_USE,     ///< I2S is in use
    I2S_STATUS_STOPPED     ///< I2S is stopped
} i2s_status_t;

/** Callback function parameter data */
typedef struct st_i2s_callback_args
{
    /** Placeholder for user data.  Set in i2s_api_t::open function in ::i2s_cfg_t. */
    void const * p_context;
    i2s_event_t  event;         ///< The event can be used to identify what caused the callback (overflow or error).
} i2s_callback_args_t;

/** I2S control block.  Allocate an instance specific control block to pass into the I2S API calls.
 * @par Implemented as
 * - ssi_instance_ctrl_t
 */
typedef void i2s_ctrl_t;

/** Timer information structure to store various information for an I2S instance. */
typedef struct st_i2s_info
{
    i2s_status_t       status;
    uint32_t sampling_freq_hz;      ///< Sampling frequency in Hertz
} i2s_info_t;

/** User configuration structure, used in open function */
typedef struct st_i2s_cfg
{
    /** Select a channel corresponding to the channel number of the hardware. */
    uint8_t  channel;
    i2s_pcm_width_t pcm_width;      ///< Audio PCM data width
    i2s_word_length_t word_length;  ///< Audio word length, bits must be >= i2s_cfg_t::pcm_width bits
    i2s_ws_continue_t ws_continue;  ///< Whether to continue WS transmission during idle state.
    uint32_t sampling_freq_hz;      ///< Sampling frequency in Hertz
    i2s_mode_t operating_mode;      ///< Operating mode selection (i.e., Master/Slave mode)

    /** Audio clock frequency in Hertz.  Must be a multiple between 1 and 128 of
     * (16 * i2s_cfg_t::sampling_freq_hz * (i2s_cfg_t::word_length \<enum_value\> + 1) */
    uint32_t audio_clk_freq_hz;

    /** To generate audio clock with GPT, link a timer instance here.  Set to NULL if unused. */
    timer_instance_t const * p_timer;

    /** To use DTC during write, link a DTC instance here.  Set to NULL if unused. */
    transfer_instance_t const * p_transfer_tx;

    /** To use DTC during read, link a DTC instance here.  Set to NULL if unused. */
    transfer_instance_t const * p_transfer_rx;

    /** Callback provided when an I2S ISR occurs.  Set to NULL for no CPU interrupt. */
    void  (* p_callback)(i2s_callback_args_t * p_args);

    /** Placeholder for user data.  Passed to the user callback in ::i2s_callback_args_t. */
    void const  * p_context;
    void const  * p_extend;         ///< Extension parameter for hardware specific settings.
    uint8_t       rxi_ipl;          ///< Receive interrupt priority
    uint8_t       txi_ipl;          ///< Transmit interrupt priority
    uint8_t       idle_err_ipl;     ///< Idle/Error interrupt priority
} i2s_cfg_t;

/** I2S functions implemented at the HAL layer will follow this API. */
typedef struct st_i2s_api
{
    /** Initial configuration.
     * @par Implemented as
     * - R_SSI_Open()
     *
     * @pre Peripheral clocks and any required output pins should be configured prior to calling this function.
     * @note To reconfigure after calling this function, call i2s_api_t::close first.
     * @param[in]   p_ctrl     Pointer to control block. Must be declared by user. Elements set here.
     * @param[in]   p_cfg      Pointer to configuration structure. All elements of this structure must be set by user.
     */
    ssp_err_t (* open)(i2s_ctrl_t      * const p_ctrl,
                       i2s_cfg_t const * const p_cfg);

    /** Stop communication.  Transmission is stopped when callback is called with I2S_EVENT_IDLE.
     * Reception is stopped when callback is called with I2S_EVENT_RX_EMPTY.
     * @par Implemented as
     * - R_SSI_Stop()
     *
     * @param[in]   p_ctrl     Control block set in i2s_api_t::open call for this instance.
     * @param[in]   dir        Direction of communication to stop.
     */
    ssp_err_t (* stop)(i2s_ctrl_t      * const p_ctrl,
                       i2s_dir_t         const dir);

    /** Enable or disable mute.
     * @par Implemented as
     * - R_SSI_Mute()
     *
     * @param[in]   p_ctrl       Control block set in i2s_api_t::open call for this instance.
     * @param[in]   mute_enable  Whether to enable or disable mute.
     */
    ssp_err_t (* mute)(i2s_ctrl_t      * const p_ctrl,
                       i2s_mute_t        const mute_enable);

    /** Write I2S data.  All transmit data is queued when callback is called with I2S_EVENT_TX_EMPTY.
     * Transmission is complete when callback is called with I2S_EVENT_IDLE.
     * @par Implemented as
     * - R_SSI_Write()
     *
     * @param[in]   p_ctrl     Control block set in i2s_api_t::open call for this instance.
     * @param[in]   p_src      Buffer of PCM samples.  Must be 4 byte aligned.
     * @param[in]   bytes      Number of bytes in the buffer.  Recommended requesting a multiple of 8 bytes.  If not
     *                         a multiple of 8, padding 0s will be added to transmission to make it a multiple of 8.
     */
    ssp_err_t (* write) (i2s_ctrl_t    * const p_ctrl,
                         uint8_t const * const p_src,
                         uint16_t        const bytes);

    /** Read I2S data.  Reception is complete when callback is called with I2S_EVENT_RX_EMPTY.
     * @par Implemented as
     * - R_SSI_Read()
     *
     * @param[in]   p_ctrl     Control block set in i2s_api_t::open call for this instance.
     * @param[in]   p_dest     Buffer to store PCM samples.  Must be 4 byte aligned.
     * @param[in]   bytes      Number of bytes in the buffer.  Recommended requesting a multiple of 8 bytes.  If not
     *                         a multiple of 8, receive will stop at the multiple of 8 below requested bytes.
     */
    ssp_err_t (* read)  (i2s_ctrl_t    * const p_ctrl,
                         uint8_t       * const p_dest,
                         uint16_t        const bytes);

    /** Simultaneously write and read I2S data.  Transmission and reception are complete when
     * callback is called with I2S_EVENT_IDLE.
     * @par Implemented as
     * - R_SSI_WriteRead()
     *
     * @param[in]   p_ctrl     Control block set in i2s_api_t::open call for this instance.
     * @param[in]   p_src      Buffer of PCM samples.  Must be 4 byte aligned.
     * @param[in]   p_dest     Buffer to store PCM samples.  Must be 4 byte aligned.
     * @param[in]   bytes      Number of bytes in the buffers.  Recommended requesting a multiple of 8 bytes.  If not
     *                         a multiple of 8, padding 0s will be added to transmission to make it a multiple of 8,
     *                         and receive will stop at the multiple of 8 below requested bytes.
     */
    ssp_err_t (* writeRead) (i2s_ctrl_t    * const p_ctrl,
                             uint8_t const * const p_src,
                             uint8_t       * const p_dest,
                             uint16_t        const bytes);

    /** Get instance specific information and store it in provided pointer p_info.
     * @par Implemented as
     * - R_SSI_InfoGet()
     *
     * @param[in]   p_ctrl     Control block set in i2s_api_t::open call for this instance.
     * @param[out]  p_info     Collection of information for this instance.
     */
    ssp_err_t (* infoGet)(i2s_ctrl_t   * const p_ctrl,
                          i2s_info_t   * const p_info);

    /** Allows driver to be reconfigured and may reduce power consumption.
     * @par Implemented as
     * - R_SSI_Close()
     *
     * @param[in]   p_ctrl     Control block set in i2s_api_t::open call for this instance.
     */
    ssp_err_t (* close)(i2s_ctrl_t      * const p_ctrl);

    /** Get version and store it in provided pointer p_version.
     * @par Implemented as
     * - R_SSI_VersionGet()
     *
     * @param[out]  p_version  Code and API version used.
     */
    ssp_err_t (* versionGet)(ssp_version_t     * const p_version);
} i2s_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_i2s_instance
{
    i2s_ctrl_t       * p_ctrl;    ///< Pointer to the control structure for this instance
    i2s_cfg_t  const * p_cfg;     ///< Pointer to the configuration structure for this instance
    i2s_api_t  const * p_api;     ///< Pointer to the API structure for this instance
} i2s_instance_t;

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_I2S_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup I2S_API)
***********************************************************************************************************************/

