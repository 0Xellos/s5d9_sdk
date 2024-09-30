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
* File Name    : r_ssi.h
* Description  : Serial Sound Interface(SSI) driver module instance header file.
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup SSI SSI
 * @brief Driver for the Serial Sound Interface (SSI).
 *
 * @section SSI_SUMMARY Summary
 * Extends @ref I2S_API.
 * @{
 **********************************************************************************************************************/

#ifndef R_SSI_H
#define R_SSI_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_i2s_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define SSI_CODE_VERSION_MAJOR   (2U)
#define SSI_CODE_VERSION_MINOR   (0U)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/** Clock source. Selects GPT channel 1 or AUDIO_CLK input pin. */
typedef enum e_ssi_audio_clock
{
    SSI_AUDIO_CLOCK_EXTERNAL    = 0,      ///< Audio clock source is the AUDIO_CLK input pin
    SSI_AUDIO_CLOCK_GTIOC1A     = 1,      ///< Audio clock source is internal connection to GPT channel 1 output
} ssi_audio_clock_t;

/** Channel instance control block. DO NOT INITIALIZE.  Initialization occurs when i2s_api_t::open is called. */
typedef struct st_ssi_instance_ctrl
{
    /** Callback provided when an I2S ISR occurs.  NULL indicates no CPU interrupt. */
    void (* p_callback)(i2s_callback_args_t * p_args);

    /** Placeholder for user data.  Passed to the user callback in ::i2s_callback_args_t. */
    void const * p_context;
    R_SSI0_Type * p_reg;                       ///< Pointer to SSI register base address
    timer_instance_t const * p_timer;          ///< Timer used to generate audio clock
    transfer_instance_t const * p_transfer_tx; ///< Transfer used for hardware acceleration during write
    transfer_instance_t const * p_transfer_rx; ///< Transfer used for hardware acceleration during read

    /** Source buffer pointer used to fill hardware FIFO from transmit ISR. */
    uint32_t const * p_tx_src;

    /** Size of source buffer used to fill hardware FIFO from transmit ISR. */
    uint32_t tx_src_bytes;

    /** Destination buffer pointer used to fill from hardware FIFO in receive ISR. */
    uint32_t * p_rx_dest;

    /** Size of destination buffer used to fill from hardware FIFO in receive ISR. */
    uint32_t rx_dest_bytes;
    uint32_t sampling_freq_hz;                 ///< Sampling frequency in Hertz
    uint8_t  channel;                          ///< Channel number.
    uint8_t  fifo_access_bytes;                ///< Byte access to FIFO.
    bool     tx_in_use;                        ///< True if a transmission is in progress.
    bool     rx_in_use;                        ///< True if a reception is in progress.
    bool     zeros_written;                    ///< True if zeros have been transmitted.
    IRQn_Type  txi_irq;                        ///< Transmit IRQ number
    IRQn_Type  rxi_irq;                        ///< Receive IRQ number
    IRQn_Type  int_irq;                        ///< Idle/Error IRQ number
    uint32_t   open;                           ///< Whether or not this control block is initialized
} ssi_instance_ctrl_t;

/** SSI configuration extension. This extension is optional. */
typedef struct st_i2s_on_ssi_cfg
{
    ssi_audio_clock_t audio_clock;             ///< Audio clock source, default is SSI_AUDIO_CLOCK_EXTERNAL
} i2s_on_ssi_cfg_t;

/**********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
extern const i2s_api_t g_i2s_on_ssi;
/** @endcond */

/**********************************************************************************************************************
Function Prototypes
***********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_SSI_H

/*******************************************************************************************************************//**
 * @} (end defgroup SSI)
***********************************************************************************************************************/
