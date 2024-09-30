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
 * File Name    : r_ssi.c
 * Description  : Serial Sound Interface (SSI) driver.
 **********************************************************************************************************************/

/***********************************************************************************************************************
Includes
 **********************************************************************************************************************/
#include "r_i2s_api.h"
#include "r_ssi.h"
#include "r_ssi_cfg.h"
#include "r_ssi_private_api.h"
#include "hw/hw_ssi_private.h"

/***********************************************************************************************************************
Macro definitions
 **********************************************************************************************************************/
/** Macro for error logger. */
#ifndef SSI_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define SSI_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_version)
#endif

#define SSI_FIFO_SMALLEST_WRITE_WORDS  (2U)
#define SSI_FIFO_SMALLEST_READ_WORDS   (2U)
#define SSI_UTIL_BYTES_PER_WORD         (4U)
#define SSI_V1_FIFO_DEPTH               (8U)
#define SSI_FIFO_ACCESS_BYTES           (4U)

/* I2S protocol always has 2 channels (left and right). */
#define SSI_PRV_I2S_CHANNELS       (2U)
#define SSI_PRV_BITS_PER_BYTE      (8U)

/* "SSI" in ASCII, used to determine if driver is open. */
#define OPEN                       (0x535349U)

/***********************************************************************************************************************
Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
Private function prototypes
 **********************************************************************************************************************/
/* ISRs */
void ssi_txi_isr(void);
void ssi_rxi_isr(void);
void ssi_int_isr(void);

/* ISR subroutines */
static inline void ssi_idle_int_process  (ssi_instance_ctrl_t * p_ctrl);
static inline void ssi_tx_end_int_process(ssi_instance_ctrl_t * p_ctrl);

/* FIFO subroutines */
static uint32_t ssi_fifo_write(ssi_instance_ctrl_t * p_ctrl);
static void ssi_fifo_read(ssi_instance_ctrl_t * p_ctrl);

/* Open subroutines */
#if SSI_CFG_PARAM_CHECKING_ENABLE
static ssp_err_t ssi_open_param_check (ssi_instance_ctrl_t * const p_ctrl,
                                       i2s_cfg_t     const * const p_cfg);
#endif

static void ssi_interrupts_configure (ssi_instance_ctrl_t * const p_ctrl,
                                      i2s_cfg_t     const * const p_cfg,
                                      ssp_feature_t       * const p_feature);

static ssp_err_t ssi_transfer_configure (i2s_cfg_t           const * const p_cfg,
                                         ssp_feature_t             * const p_feature,
                                         transfer_instance_t const *       p_transfer,
                                         ssp_signal_t                      ssp_signal);

static ssp_err_t ssi_dependent_drivers_configure (ssi_instance_ctrl_t * const p_ctrl,
                                                  i2s_cfg_t     const * const p_cfg,
                                                  ssp_feature_t       * const p_feature);

static void ssi_master_mode_configure (ssi_instance_ctrl_t * const p_ctrl,
                                      i2s_cfg_t      const * const p_cfg);


static void ssi_audio_clock_configure (ssi_instance_ctrl_t * const p_ctrl,
                                       i2s_cfg_t     const * const p_cfg);

/* Stop subroutines */
static void ssi_stop_tx (ssi_instance_ctrl_t * const p_ctrl);
static void ssi_stop_rx (ssi_instance_ctrl_t * const p_ctrl);
static void ssi_process_complete(ssi_instance_ctrl_t * p_ctrl);

/* Start subroutines */
static ssp_err_t ssi_start (ssi_instance_ctrl_t * const p_ctrl, i2s_dir_t dir);

/* Read and write subroutines. */
ssp_err_t ssi_tx_load_fifo(ssi_instance_ctrl_t * const p_ctrl,
                           uint8_t       const * const p_src,
                           uint16_t              const bytes);
ssp_err_t ssi_tx_start(ssi_instance_ctrl_t     * const p_ctrl);
ssp_err_t ssi_rx_unload_fifo(ssi_instance_ctrl_t * const p_ctrl,
                             uint8_t             * const p_dest,
                             uint16_t              const bytes);


/***********************************************************************************************************************
Private global variables
 **********************************************************************************************************************/
#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug. This pragma suppresses the warnings in this
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_version =
{
    .api_version_minor  = I2S_API_VERSION_MINOR,
    .api_version_major  = I2S_API_VERSION_MAJOR,
    .code_version_major = SSI_CODE_VERSION_MAJOR,
    .code_version_minor = SSI_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

#if (0 != BSP_CFG_ERROR_LOG)
/** Name of module used by error logger macro */
static const char g_module_name[] = "ssi";
#endif

/** Lookup table for audio clock divisor.  Must be sorted in ascending order by the first parameter. */
static const uint8_t g_audio_clock_div_lookup[SSI_CLOCK_DIV_MAX][2] =
{
    {1U, 0U},
    {2U, 1U},
    {4U, 2U},
    {6U, 8U},
    {8U, 3U},
    {12U, 9U},
    {16U, 4U},
    {24U, 10U},
    {32U, 5U},
    {48U, 11U},
    {64U, 6U},
    {96U, 12U},
    {128U, 7U},
};

/** Stores which version of SSI this is. */
static ssi_version_t g_ssi_version = SSI_VERSION_SSI;

/** Stores SSI number of FIFO stages. */
static uint8_t g_ssi_fifo_num_stages = SSI_V1_FIFO_DEPTH;

/** Maps i2s_dir_t to SSICR bits. */
static const uint8_t g_ren_ten_lookup[] =
{
    SSI_SSICR_TEN_BIT,                     // I2S_DIR_TX, TEN is bit 1
    SSI_SSICR_REN_BIT,                     // I2S_DIR_RX, REN is bit 0
    SSI_SSICR_TEN_BIT | SSI_SSICR_REN_BIT, // I2S_DIR_TX_RX, TEN and REN
};

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/

/** SSI Implementation of I2S interface.  */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const i2s_api_t g_i2s_on_ssi =
{
    .open            = R_SSI_Open,
    .stop            = R_SSI_Stop,
    .write           = R_SSI_Write,
    .read            = R_SSI_Read,
    .writeRead       = R_SSI_WriteRead,
    .mute            = R_SSI_Mute,
    .infoGet         = R_SSI_InfoGet,
    .close           = R_SSI_Close,
    .versionGet      = R_SSI_VersionGet
};

/******************************************************************************
Macro definitions
 ******************************************************************************/

/******************************************************************************
Typedef definitions
 ******************************************************************************/

/******************************************************************************
Exported global variables
 ******************************************************************************/

/******************************************************************************
Private global variables and functions
 ******************************************************************************/

/*******************************************************************************************************************//**
 * @addtogroup SSI
 * @{
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Opens the SSI. Implements i2s_api_t::open.
 *
 * This function calculates the clock divisor based on the input audio clock frequency and the requested sampling
 * frequency.  It sets this clock divisor and the configurations specified in i2s_cfg_t.  It also opens the timer and
 * transfer interfaces if they are provided.
 *
 * @retval SSP_SUCCESS           Ready for I2S communication.
 * @retval SSP_ERR_ASSERTION     The pointer to p_ctrl or p_cfg is null.
 * @retval SSP_ERR_IN_USE        The requested channel has already been opened or hardware has been locked.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * fmi_api_t::productFeatureGet
 *                                   * fmi_api_t::eventInfoGet
 *                                   * transfer_api_t::open
 *                                   * timer_api_t::open
 **********************************************************************************************************************/
ssp_err_t R_SSI_Open (i2s_ctrl_t      * const p_api_ctrl,
                      i2s_cfg_t const * const p_cfg)
{
    ssi_instance_ctrl_t * p_ctrl = (ssi_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err;

#if SSI_CFG_PARAM_CHECKING_ENABLE
    err = ssi_open_param_check(p_ctrl, p_cfg);
    SSI_ERROR_RETURN((SSP_SUCCESS == err), err);
#endif

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_cfg->channel;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_SSI;
    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    SSI_ERROR_RETURN(SSP_SUCCESS == err, err);
    g_ssi_version = (ssi_version_t) info.version_major;
    p_ctrl->p_reg = (R_SSI0_Type *) info.ptr;

    bsp_feature_ssi_t ssi_feature = {0U};
    R_BSP_FeatureSsiGet(&ssi_feature);
    g_ssi_fifo_num_stages = ssi_feature.fifo_num_stages;

    err  = R_BSP_HardwareLock(&ssp_feature);
    SSI_ERROR_RETURN((SSP_SUCCESS == err), err);

    /** Configure dependent timer and transfer drivers. */
    err = ssi_dependent_drivers_configure(p_ctrl, p_cfg, &ssp_feature);
    if (SSP_SUCCESS != err)
    {
        R_BSP_HardwareUnlock(&ssp_feature);
    }
    SSI_ERROR_RETURN((SSP_SUCCESS == err), err);

    /** Configure interrupts. */
    ssi_interrupts_configure(p_ctrl, p_cfg, &ssp_feature);

    R_BSP_ModuleStart(&ssp_feature);
    HW_SSI_RegisterReset(p_ctrl->p_reg);
    HW_SSI_Reset(p_ctrl->p_reg);
    p_ctrl->fifo_access_bytes = SSI_FIFO_ACCESS_BYTES;
    if (SSI_VERSION_SSIE == g_ssi_version)
    {
        if (I2S_PCM_WIDTH_8_BITS == p_cfg->pcm_width)
        {
            p_ctrl->fifo_access_bytes = 1U;
        }
        if (I2S_PCM_WIDTH_16_BITS == p_cfg->pcm_width)
        {
            p_ctrl->fifo_access_bytes = 2U;
        }

        /* Set SSIE FIFO watermark to half the FIFO size. */
        uint8_t fifo_setting = (uint8_t) ((g_ssi_fifo_num_stages / 2U) - 1U);
        HW_SSIE_FifoInit(p_ctrl->p_reg, (ssie_fifo_t) fifo_setting);
    }
    else
    {
        HW_SSI_FifoInit(p_ctrl->p_reg, SSI_FIFO_2_EMPTY);
    }

    HW_SSI_WordLengthSet(p_ctrl->p_reg, p_cfg->word_length);
    HW_SSI_PcmWidthSet(p_ctrl->p_reg, p_cfg->pcm_width);
    HW_SSI_MasterSlaveSet(p_ctrl->p_reg, p_cfg->operating_mode, g_ssi_version);
    if (I2S_MODE_MASTER == p_cfg->operating_mode)
    {
        ssi_master_mode_configure(p_ctrl, p_cfg);
    }

    p_ctrl->p_transfer_tx     = p_cfg->p_transfer_tx;
    p_ctrl->p_transfer_rx     = p_cfg->p_transfer_rx;
    p_ctrl->zeros_written     = false;
    p_ctrl->rx_in_use         = false;
    p_ctrl->tx_in_use         = false;
    p_ctrl->sampling_freq_hz  = p_cfg->sampling_freq_hz;
    p_ctrl->channel    = p_cfg->channel;
    p_ctrl->p_callback = p_cfg->p_callback;
    p_ctrl->p_context  = p_cfg->p_context;
    p_ctrl->p_timer    = p_cfg->p_timer;

    /** Mark driver as open by initializing it to "SSI" in its ASCII equivalent. */
    p_ctrl->open       = OPEN;

    return SSP_SUCCESS;
} /* End of function R_SSI_Open */

/*******************************************************************************************************************//**
 * @brief  Stops SSI. Implements i2s_api_t::stop.
 *
 * This function disables the transfer if the transfer interface is used, or sends a stop signal to stop writing data
 * in the ISR if interrupt driven mode is used.
 *
 * @retval SSP_SUCCESS           I2S communication stop request issued.
 * @retval SSP_ERR_ASSERTION     The pointer to p_ctrl null.
 * @retval SSP_ERR_NOT_OPEN      The channel is not opened.
 * @return                       See @ref Common_Error_Codes or lower level drivers for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t R_SSI_Stop(i2s_ctrl_t * const p_api_ctrl,
                     i2s_dir_t    const dir)
{
    ssi_instance_ctrl_t * p_ctrl = (ssi_instance_ctrl_t *) p_api_ctrl;

#if SSI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSI_ERROR_RETURN(OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** Stop is complete after an I2S_EVENT_IDLE interrupt. */
    if ((I2S_DIR_TX == dir) || (I2S_DIR_TX_RX == dir))
    {
        ssi_stop_tx(p_ctrl);
    }
    if ((I2S_DIR_RX == dir) || (I2S_DIR_TX_RX == dir))
    {
        ssi_stop_rx(p_ctrl);
    }

    return SSP_SUCCESS;
} /* End of function R_SSI_Stop */

/*******************************************************************************************************************//**
 * @brief  Closes SSI. Implements i2s_api_t::close.
 *
 * This function powers down the SSI and closes the lower level timer and transfer drivers if they are used.
 *
 * @retval SSP_SUCCESS           Device closed successfully.
 * @retval SSP_ERR_ASSERTION     The pointer to p_ctrl null.
 * @retval SSP_ERR_NOT_OPEN      The channel is not opened.
 **********************************************************************************************************************/
ssp_err_t R_SSI_Close(i2s_ctrl_t * const p_api_ctrl)
{
    ssi_instance_ctrl_t * p_ctrl = (ssi_instance_ctrl_t *) p_api_ctrl;
#if SSI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSI_ERROR_RETURN(OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    p_ctrl->open = 0U;

    /** Stop feeding clock to SSI peripheral to deactivate it. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_ctrl->channel;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_SSI;
    R_BSP_ModuleStop(&ssp_feature);

    /** If a timer instance is provided, close the timer instance. */
    if (NULL != p_ctrl->p_timer)
    {
        p_ctrl->p_timer->p_api->close(p_ctrl->p_timer->p_ctrl);
    }

    /** If a transfer instance is provided for write, close the transfer instance. */
    if (NULL != p_ctrl->p_transfer_tx)
    {
        p_ctrl->p_transfer_tx->p_api->close(p_ctrl->p_transfer_tx->p_ctrl);
    }

    /** If a transfer instance is provided for read, close the transfer instance. */
    if (NULL != p_ctrl->p_transfer_rx)
    {
        p_ctrl->p_transfer_rx->p_api->close(p_ctrl->p_transfer_rx->p_ctrl);
    }

    /** Release HW lock. */
    R_BSP_HardwareUnlock(&ssp_feature);

    return SSP_SUCCESS;
} /* End of function R_SSI_Close */

/*******************************************************************************************************************//**
 * @brief  Writes data buffer to SSI. Implements i2s_api_t::write.
 *
 * This function resets the transfer if the transfer interface is used, or writes the length of data that fits in the
 * FIFO then stores the remaining write buffer in the control block to be written in the ISR.
 *
 * Write() cannot be called if another write(), read() or writeRead() operation is in progress.  Write can be called
 * when the SSI is idle, or after the I2S_EVENT_TX_EMPTY event.
 *
 * @retval SSP_SUCCESS           Write initiated successfully.
 * @retval SSP_ERR_ASSERTION     The pointer to p_ctrl or p_src was null, or bytes requested was 0.
 * @retval SSP_ERR_IN_USE        Another transfer is in progress, data was not written.
 * @retval SSP_ERR_NOT_OPEN      The channel is not opened.
 * @retval SSP_ERR_UNDERFLOW     The transmit FIFO underflowed before it was reloaded.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * transfer_api_t::reset
 *                                   * timer_api_t::start
 **********************************************************************************************************************/
ssp_err_t R_SSI_Write(i2s_ctrl_t    * const p_api_ctrl,
                      uint8_t const * const p_src,
                      uint16_t        const bytes)
{
    ssi_instance_ctrl_t * p_ctrl = (ssi_instance_ctrl_t *) p_api_ctrl;

#if SSI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSI_ERROR_RETURN(OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    SSP_ASSERT(NULL != p_src);
    SSP_ASSERT(bytes > 0U);
#endif

    /** If a transfer instance is provided for write, reset the transfer. Otherwise load the transmit FIFO. */
    ssp_err_t err = ssi_tx_load_fifo(p_ctrl, p_src, bytes);
    SSI_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Make sure transmission is enabled. */
    err = ssi_start(p_ctrl, I2S_DIR_TX);
    SSI_ERROR_RETURN(SSP_SUCCESS == err, err);

    err = ssi_tx_start(p_ctrl);
    SSI_ERROR_RETURN(SSP_SUCCESS == err, err);

    return SSP_SUCCESS;
} /* End of function R_SSI_Write */

/*******************************************************************************************************************//**
 * @brief  Reads data into provided buffer. Implements i2s_api_t::read.
 *
 * This function resets the transfer if the transfer interface is used, or reads the length of data available in the
 * FIFO then stores the remaining read buffer in the control block to be filled in the ISR.
 *
 * Read() cannot be called if another write(), read() or writeRead() operation is in progress.  Read can be called
 * when the SSI is idle, or after the I2S_EVENT_RX_FULL event.
 *
 * @retval SSP_SUCCESS           Read initiated successfully.
 * @retval SSP_ERR_ASSERTION     The pointer to p_ctrl or p_dest was null, or bytes requested was 0.
 * @retval SSP_ERR_NOT_OPEN      The channel is not opened.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * transfer_api_t::reset
 *                                   * timer_api_t::start
 **********************************************************************************************************************/
ssp_err_t R_SSI_Read(i2s_ctrl_t * const p_api_ctrl,
                     uint8_t    * const p_dest,
                     uint16_t     const bytes)
{
    ssi_instance_ctrl_t * p_ctrl = (ssi_instance_ctrl_t *) p_api_ctrl;

#if SSI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSI_ERROR_RETURN(OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    SSP_ASSERT(NULL != p_dest);
    SSP_ASSERT(bytes > 0U);
#endif

    /** If a transfer instance is provided for read, reset the transfer. Otherwise unload the receive FIFO. */
    ssp_err_t err = ssi_rx_unload_fifo(p_ctrl, p_dest, bytes);
    SSI_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Make sure reception is enabled. */
    err = ssi_start(p_ctrl, I2S_DIR_RX);
    SSI_ERROR_RETURN(SSP_SUCCESS == err, err);

    return SSP_SUCCESS;
} /* End of function R_SSI_Read */

/*******************************************************************************************************************//**
 * @brief  Writes from source buffer and reads data into destination buffer. Implements i2s_api_t::writeRead.
 *
 * This function calls R_SSI_Write and R_SSI_Read.
 *
 * writeRead() cannot be called if another write(), read() or writeRead() operation is in progress.  writeRead() can be
 * called when the SSI is idle, or after the I2S_EVENT_RX_FULL event.
 *
 * @retval SSP_SUCCESS           Write and read initiated successfully.
 * @retval SSP_ERR_ASSERTION     The pointer to p_ctrl, p_src, or p_dest was null, or bytes requested was 0.
 * @retval SSP_ERR_NOT_OPEN      The channel is not opened.
 * @retval SSP_ERR_UNDERFLOW     The transmit FIFO underflowed before it was reloaded.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * transfer_api_t::reset
 *                                   * timer_api_t::start
 **********************************************************************************************************************/
ssp_err_t R_SSI_WriteRead (i2s_ctrl_t    * const p_api_ctrl,
                           uint8_t const * const p_src,
                           uint8_t       * const p_dest,
                           uint16_t        const bytes)
{
    ssi_instance_ctrl_t * p_ctrl = (ssi_instance_ctrl_t *) p_api_ctrl;

#if SSI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_src);
    SSP_ASSERT(NULL != p_dest);
    SSP_ASSERT(bytes > 0U);
    SSI_ERROR_RETURN(OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** If a transfer instance is provided for read, reset the transfer. */
    ssp_err_t err = ssi_rx_unload_fifo(p_ctrl, p_dest, bytes);
    SSI_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** If a transfer instance is provided for write, reset the transfer. */
    err = ssi_tx_load_fifo(p_ctrl, p_src, bytes);
    SSI_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Make sure transmission is enabled. */
    err = ssi_start(p_ctrl, I2S_DIR_TX_RX);
    SSI_ERROR_RETURN(SSP_SUCCESS == err, err);

    err = ssi_tx_start(p_ctrl);
    SSI_ERROR_RETURN(SSP_SUCCESS == err, err);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Mutes SSI. Implements i2s_api_t::mute.
 *
 * Data is still written while mute is enabled, but the transmit line outputs zeros.
 *
 * @retval SSP_SUCCESS           Transmission is muted.
 * @retval SSP_ERR_ASSERTION     The pointer to p_ctrl was null.
 * @retval SSP_ERR_NOT_OPEN      The channel is not opened.
 **********************************************************************************************************************/
ssp_err_t R_SSI_Mute(i2s_ctrl_t * const p_api_ctrl,
                     i2s_mute_t   const mute_enable)
{
    ssi_instance_ctrl_t * p_ctrl = (ssi_instance_ctrl_t *) p_api_ctrl;

#if SSI_CFG_PARAM_CHECKING_ENABLE
    /** Make sure parameters are valid. */
    SSP_ASSERT(NULL != p_ctrl);
    SSI_ERROR_RETURN(OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif
    /** Enables the mute if MUTE_ON is set. Otherwise, disables the mute. */
    if (I2S_MUTE_ON == mute_enable)
    {
        HW_SSI_MuteOn(p_ctrl->p_reg);
    }
    else
    {
        HW_SSI_MuteOff(p_ctrl->p_reg);
    }

    return SSP_SUCCESS;
} /* End of function R_SSI_Mute */

/*******************************************************************************************************************//**
 * @brief  Get I2S information and store it in provided pointer p_info. Implements i2s_api_t::infoGet.
 *
 * @retval SSP_SUCCESS           Information stored successfully.
 * @retval SSP_ERR_ASSERTION     The p_ctrl or p_info parameter was null.
 * @retval SSP_ERR_NOT_OPEN      The channel is not opened.
 **********************************************************************************************************************/
ssp_err_t R_SSI_InfoGet (i2s_ctrl_t * const p_api_ctrl,
                         i2s_info_t * const p_info)
{
    ssi_instance_ctrl_t * p_ctrl = (ssi_instance_ctrl_t *) p_api_ctrl;

#if SSI_CFG_PARAM_CHECKING_ENABLE
    /** Make sure parameters are valid. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_info);
    SSI_ERROR_RETURN(OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** Get the SSI hardware status information. */
    bool idle = HW_SSI_StatusGet(p_ctrl->p_reg);

    /** If SSI hardware status is idle, set status to stopped. Otherwise, set status to in use. */
    if (idle)
    {
        p_info->status = I2S_STATUS_STOPPED;
    }
    else
    {
        p_info->status = I2S_STATUS_IN_USE;
    }

    /** Get the sampling frequency information. */
    p_info->sampling_freq_hz = p_ctrl->sampling_freq_hz;

    return SSP_SUCCESS;
} /* End of function R_SSI_InfoGet */

/*******************************************************************************************************************//**
 * @brief      Sets driver version based on compile time macros.
 *
 * @retval     SSP_SUCCESS          Successful close.
 * @retval     SSP_ERR_ASSERTION    The parameter p_version is NULL.
 **********************************************************************************************************************/
ssp_err_t R_SSI_VersionGet (ssp_version_t * const p_version)
{
#if SSI_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid */
    SSP_ASSERT(NULL != p_version);
#endif

    p_version->version_id = g_version.version_id;

    return SSP_SUCCESS;
} /* End of function R_SSI_VersionGet */

/*******************************************************************************************************************//**
 * @} (end addtogroup R_SSI)
 **********************************************************************************************************************/

/***********************************************************************************************************************
Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Performs parameter checking for R_SSI_Open().
 *
 * @param[in] p_ctrl  Pointer to the control block.
 * @param[in] p_cfg   Pointer to the configuration structure.
 *
 * @retval SSP_SUCCESS           Ready for I2S communication.
 * @retval SSP_ERR_ASSERTION     The pointer to p_ctrl or p_cfg was null, or an invalid channel number was specified.
 * @retval SSP_ERR_IN_USE        The requested channel has already been opened.
 **********************************************************************************************************************/
#if SSI_CFG_PARAM_CHECKING_ENABLE
static ssp_err_t ssi_open_param_check (ssi_instance_ctrl_t * const p_ctrl,
                                       i2s_cfg_t     const * const p_cfg)
{
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_cfg);
    SSI_ERROR_RETURN(OPEN != p_ctrl->open, SSP_ERR_IN_USE);

    return SSP_SUCCESS;
}
#endif

/*******************************************************************************************************************//**
 * @brief Gets the interrupt vector number for the transmit, receive, and idle interrupts.
 * For each interrupt, if it is linked into the the vector table, sets the interrupt priority
 * based on user configuration and stores the control block in the vector information array.
 *
 * @param[in] p_ctrl     Pointer to the control block.
 * @param[in] p_cfg      Pointer to the configuration structure.
 * @param[in] p_feature  Pointer to the SSI feature for this instance.
 **********************************************************************************************************************/
static void ssi_interrupts_configure (ssi_instance_ctrl_t * const p_ctrl,
                                      i2s_cfg_t     const * const p_cfg,
                                      ssp_feature_t       * const p_feature)
{
    ssp_vector_info_t * p_vector_info;
    /* Initialize event_info variable. */
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    /* Set interrupt priority based on user configuration of interrupt number for SSP_SIGNAL_SSI_INT signal */
    g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_SSI_INT, &event_info);
    p_ctrl->int_irq = event_info.irq;
    if (SSP_INVALID_VECTOR != p_ctrl->int_irq)
    {
        R_SSP_VectorInfoGet(p_ctrl->int_irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->int_irq, p_cfg->idle_err_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }
    /* Set interrupt priority based on user configuration of interrupt number for SSP_SIGNAL_SSI_RXI
     * or SSP_SIGNAL_SSI_TXI_RXI signals */
    g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_SSI_RXI, &event_info);
    p_ctrl->rxi_irq = event_info.irq;
    if (SSP_INVALID_VECTOR == p_ctrl->rxi_irq)
    {
        g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_SSI_TXI_RXI, &event_info);
        p_ctrl->rxi_irq = event_info.irq;
    }
    if (SSP_INVALID_VECTOR != p_ctrl->rxi_irq)
    {
        R_SSP_VectorInfoGet(p_ctrl->rxi_irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->rxi_irq, p_cfg->rxi_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }
    /* Set interrupt priority based on user configuration of interrupt number for SSP_SIGNAL_SSI_TXI
     * or SSP_SIGNAL_SSI_TXI_RXI signals */
    g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_SSI_TXI, &event_info);
    p_ctrl->txi_irq = event_info.irq;
    if (SSP_INVALID_VECTOR == p_ctrl->txi_irq)
    {
        g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_SSI_TXI_RXI, &event_info);
        p_ctrl->txi_irq = event_info.irq;
    }
    if (SSP_INVALID_VECTOR != p_ctrl->txi_irq)
    {
        R_SSP_VectorInfoGet(p_ctrl->txi_irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->txi_irq, p_cfg->txi_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }
}

/*******************************************************************************************************************//**
 * Configures dependent transfer driver.
 *
 * @param[in] p_cfg           Pointer to the configuration structure.
 * @param[in] p_feature       Pointer to the SSI feature for this instance.
 * @param[in] p_transfer      Pointer to the transfer instance.
 * @param[in] ssp_signal      Activation source signal for this transfer
 *
 * @retval SSP_SUCCESS           Transfer driver configured successfully.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * transfer_api_t::open
 **********************************************************************************************************************/
static ssp_err_t ssi_transfer_configure (i2s_cfg_t           const * const p_cfg,
                                         ssp_feature_t             * const p_feature,
                                         transfer_instance_t const *       p_transfer,
                                         ssp_signal_t                      ssp_signal)
{
    /* Assign instance of transfer configuration structure. */
    transfer_cfg_t transfer_cfg = *(p_transfer->p_cfg);
    /* Initialize event_info variable. */
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    ssp_err_t fmi_err = g_fmi_on_fmi.eventInfoGet(p_feature, ssp_signal, &event_info);
    if (SSP_SUCCESS != fmi_err)
    {
        g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_SSI_TXI_RXI, &event_info);
    }
    transfer_cfg.activation_source = event_info.event;
    transfer_size_t size = TRANSFER_SIZE_4_BYTE;
    if (SSI_VERSION_SSIE == g_ssi_version)
    {
        if (I2S_PCM_WIDTH_8_BITS == p_cfg->pcm_width)
        {
            size = TRANSFER_SIZE_1_BYTE;
        }
        if (I2S_PCM_WIDTH_16_BITS == p_cfg->pcm_width)
        {
            size = TRANSFER_SIZE_2_BYTE;
        }
    }
    p_transfer->p_cfg->p_info->size = size;
    p_transfer->p_cfg->p_info->length = 0U;
    ssp_err_t err = p_transfer->p_api->open(p_transfer->p_ctrl, &transfer_cfg);
    SSI_ERROR_RETURN((SSP_SUCCESS == err), err);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Configures any dependent drivers selected by the user, including transfer and timer drivers.
 *
 * @param[in] p_ctrl     Pointer to the control block.
 * @param[in] p_cfg      Pointer to the configuration structure.
 * @param[in] p_feature  Pointer to the SSI feature for this instance.
 *
 * @retval SSP_SUCCESS           Dependent drivers configured successfully.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * transfer_api_t::open
 *                                   * timer_api_t::open
 *                                   * timer_api_t::start
 **********************************************************************************************************************/
static ssp_err_t ssi_dependent_drivers_configure (ssi_instance_ctrl_t * const p_ctrl,
                                                  i2s_cfg_t     const * const p_cfg,
                                                  ssp_feature_t       * const p_feature)
{
    /** If a timer instance is provided, open the timer instance. */
    ssp_err_t err;
    if (NULL != p_cfg->p_timer)
    {
        err = p_cfg->p_timer->p_api->open(p_cfg->p_timer->p_ctrl, p_cfg->p_timer->p_cfg);
        SSI_ERROR_RETURN((SSP_SUCCESS == err), err);

        /** If a timer instance is provided and WS continue mode is enabled, start the timer. */
        if (I2S_WS_CONTINUE_ON == p_cfg->ws_continue)
        {
            err = p_cfg->p_timer->p_api->start(p_cfg->p_timer->p_ctrl);
            SSI_ERROR_RETURN((SSP_SUCCESS == err), err);
        }
    }

    /** If a transfer instance is provided for write, open the transfer instance. */
    if (NULL != p_cfg->p_transfer_tx)
    {
        p_cfg->p_transfer_tx->p_cfg->p_info->p_dest = (void *) HW_SSI_TxAddrGet(p_ctrl->p_reg);
        err = ssi_transfer_configure(p_cfg, p_feature, p_cfg->p_transfer_tx, SSP_SIGNAL_SSI_TXI);
        SSI_ERROR_RETURN((SSP_SUCCESS == err), err);
    }

    /** If a transfer instance is provided for read, open the transfer instance. */
    if (NULL != p_cfg->p_transfer_rx)
    {
        p_cfg->p_transfer_rx->p_cfg->p_info->p_src = (void *) HW_SSI_RxAddrGet(p_ctrl->p_reg);
        err = ssi_transfer_configure(p_cfg, p_feature, p_cfg->p_transfer_rx, SSP_SIGNAL_SSI_RXI);
        SSI_ERROR_RETURN((SSP_SUCCESS == err), err);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Configures the master mode settings.
 *
 * @param[in] p_ctrl  Pointer to the control block.
 * @param[in] p_cfg   Pointer to the configuration structure.
 **********************************************************************************************************************/
static void ssi_master_mode_configure (ssi_instance_ctrl_t * const p_ctrl,
                                      i2s_cfg_t      const * const p_cfg)
{
    /** Configure the audio clock. */
    ssi_audio_clock_configure(p_ctrl, p_cfg);

    if (I2S_WS_CONTINUE_ON == p_cfg->ws_continue)
    {
        HW_SSI_WsContinueEnable(p_ctrl->p_reg);
    }
}

/*******************************************************************************************************************//**
 * Configures the audio clock.
 *
 * @param[in] p_ctrl  Pointer to the control block.
 * @param[in] p_cfg   Pointer to the configuration structure.
 **********************************************************************************************************************/
static void ssi_audio_clock_configure (ssi_instance_ctrl_t * const p_ctrl,
                                       i2s_cfg_t     const * const p_cfg)
{
    /* The target bit clock (Hz) should be the number of channels * word length (bits) * sampling frequency (Hz). */
    uint32_t target_bit_clock_hz = SSI_PRV_I2S_CHANNELS * (SSI_PRV_BITS_PER_BYTE * (p_cfg->word_length + 1U)) *
            p_cfg->sampling_freq_hz;
    uint32_t audio_clock_div = p_cfg->audio_clk_freq_hz / target_bit_clock_hz;
    ssi_clock_div_t audio_clock_div_bits = SSI_CLOCK_DIV_1;
    for (uint32_t i = 0U; i < SSI_CLOCK_DIV_MAX; i++)
    {
        if (audio_clock_div <= g_audio_clock_div_lookup[i][0])
        {
            audio_clock_div_bits = (ssi_clock_div_t) g_audio_clock_div_lookup[i][1];
            break;
        }
    }
    HW_SSI_ClockDivBitsSet(p_ctrl->p_reg, audio_clock_div_bits);
    if (NULL != p_cfg->p_extend)
    {
        i2s_on_ssi_cfg_t * p_extend = (i2s_on_ssi_cfg_t *) p_cfg->p_extend;
        HW_SSI_AudioClockSet(p_ctrl->p_reg, p_extend->audio_clock);
    }
}

/*******************************************************************************************************************//**
 * Disables SSI transmission if peripheral is idle or sets a flag for transmission to be disabled in ISR.
 *
 * @param[in] p_ctrl  Pointer to the control block.
 **********************************************************************************************************************/
static void ssi_stop_tx (ssi_instance_ctrl_t * const p_ctrl)
{
    /** If transfer is used, disable transfer when stop is requested. */
    p_ctrl->tx_in_use = false;
    HW_SSI_TxInterruptDisable(p_ctrl->p_reg, p_ctrl->txi_irq);
    if (HW_SSI_IsIdle(p_ctrl->p_reg))
    {
        ssi_process_complete(p_ctrl);
    }
    p_ctrl->p_tx_src = NULL;
    p_ctrl->tx_src_bytes = 0U;
    if (NULL != p_ctrl->p_transfer_tx)
    {
        p_ctrl->p_transfer_tx->p_api->disable(p_ctrl->p_transfer_tx->p_ctrl);
    }
}

/*******************************************************************************************************************//**
 * Disables SSI reception if peripheral is idle or sets a flag for reception to be disabled in ISR.
 *
 * @param[in] p_ctrl  Pointer to the control block.
 **********************************************************************************************************************/
static void ssi_stop_rx (ssi_instance_ctrl_t * const p_ctrl)
{
    /** If transfer is used, disable transfer when stop is requested. */
    HW_SSI_RxInterruptDisable(p_ctrl->p_reg, p_ctrl->rxi_irq);
    p_ctrl->rx_in_use = false;
    p_ctrl->p_rx_dest = NULL;
    p_ctrl->rx_dest_bytes = 0U;
    ssi_process_complete(p_ctrl);
    if (NULL != p_ctrl->p_transfer_rx)
    {
        p_ctrl->p_transfer_rx->p_api->disable(p_ctrl->p_transfer_rx->p_ctrl);
    }
}

/*******************************************************************************************************************//**
 * Configures the transmit FIFO to be loaded by DTC or stores the source data to be loaded into the FIFO in the transmit
 * interrupt.
 *
 * @param[in] p_ctrl     Pointer to the control block.
 * @param[in] p_src      Pointer to the source buffer.
 * @param[in] bytes      Length of source buffer.
 *
 * @retval SSP_SUCCESS           Transmit FIFO successfully loaded.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * transfer_api_t::reset
 **********************************************************************************************************************/
ssp_err_t ssi_tx_load_fifo(ssi_instance_ctrl_t * const p_ctrl,
                           uint8_t       const * const p_src,
                           uint16_t              const bytes)
{
    /** If a transfer instance is provided for write, reset the transfer. */
    p_ctrl->zeros_written = false;
    if (NULL != p_ctrl->p_transfer_tx)
    {
        ssp_err_t err = p_ctrl->p_transfer_tx->p_api->reset(p_ctrl->p_transfer_tx->p_ctrl, p_src, NULL,
                (uint16_t) (bytes / p_ctrl->fifo_access_bytes));
        SSI_ERROR_RETURN(SSP_SUCCESS == err, err);
    }
    else
    {
        /** Otherwise, bytes are written in the ISR. */
        p_ctrl->p_tx_src = (uint32_t *) p_src;
        p_ctrl->tx_src_bytes = bytes;
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Begins transmission and checks for synchronization errors.
 *
 * @param[in] p_ctrl     Pointer to the control block.
 *
 * @retval SSP_SUCCESS           Transmission is started.
 * @retval SSP_ERR_UNDERFLOW     The transmit FIFO underflowed before it was reloaded.
 **********************************************************************************************************************/
ssp_err_t ssi_tx_start(ssi_instance_ctrl_t * const p_ctrl)
{
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    /** Clear transmit FIFO empty flag.  If the TDE condition is still valid, the flag will reset on the next PCLK. */
    R_BSP_IrqStatusClear(p_ctrl->txi_irq);
    NVIC_ClearPendingIRQ(p_ctrl->txi_irq);
    HW_SSI_TxFifoEmptyFlagClear(p_ctrl->p_reg);

    uint32_t current_ssicr_ren_ten = HW_SSI_RxTxEnabledGet(p_ctrl->p_reg);

    ssi_irq_events ssi_irq_events_info = {0};
    ssi_irq_events_info.SSISR = HW_SSI_EventsGet(p_ctrl->p_reg);

    SSP_CRITICAL_SECTION_EXIT;

    /** Transmission is disabled when a transmit underflow occurs.  Return an error if this happened before the
     * transmit FIFO was reloaded.
     */
    uint32_t desired_ssicr_ren_ten = g_ren_ten_lookup[I2S_DIR_TX];
    SSI_ERROR_RETURN((desired_ssicr_ren_ten & current_ssicr_ren_ten) > 0U, SSP_ERR_UNDERFLOW);
    SSI_ERROR_RETURN(1U != ssi_irq_events_info.SSISR_b.TUIRQ, SSP_ERR_UNDERFLOW);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Configures the receive FIFO to be unloaded by DTC or stores the destination buffer for data to be unloaded in the
 * receive interrupt.
 *
 * @param[in] p_ctrl     Pointer to the control block.
 * @param[in] p_dest     Pointer to the destination buffer.
 * @param[in] bytes      Length of destination buffer.
 *
 * @retval SSP_SUCCESS           Receive FIFO successfully unloaded.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * transfer_api_t::reset
 **********************************************************************************************************************/
ssp_err_t ssi_rx_unload_fifo(ssi_instance_ctrl_t * const p_ctrl,
                             uint8_t             * const p_dest,
                             uint16_t              const bytes)
{
    /** If a transfer instance is provided for read, reset the transfer. */
    if (NULL != p_ctrl->p_transfer_rx)
    {
        ssp_err_t err = p_ctrl->p_transfer_rx->p_api->reset(p_ctrl->p_transfer_rx->p_ctrl, NULL, p_dest,
                (uint16_t) (bytes / p_ctrl->fifo_access_bytes));
        SSI_ERROR_RETURN((SSP_SUCCESS == err), err);
    }
    else
    {
        /** Otherwise, read from the FIFO in the interrupt. */
        p_ctrl->p_rx_dest = (uint32_t *) p_dest;
        p_ctrl->rx_dest_bytes = bytes;
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Enables SSI transmission and/or reception.
 *
 * @param[in] p_ctrl  Pointer to the control block.
 * @param[in] dir     Start transmit, receive, or both.
 *
 * @retval SSP_SUCCESS           Ready for I2S transmission.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * timer_api_t::start
 **********************************************************************************************************************/
static ssp_err_t ssi_start (ssi_instance_ctrl_t * const p_ctrl, i2s_dir_t dir)
{
    uint32_t current_ssicr_ren_ten = HW_SSI_RxTxEnabledGet(p_ctrl->p_reg);
    uint32_t desired_ssicr_ren_ten = g_ren_ten_lookup[dir];

    /** If the peripheral is not already in the correct mode, attempt to start it. */
    if (desired_ssicr_ren_ten != (current_ssicr_ren_ten & desired_ssicr_ren_ten))
    {
        /** If the peripheral is in the wrong mode or not idle, return an error. */
        SSI_ERROR_RETURN(0U == current_ssicr_ren_ten, SSP_ERR_IN_USE);
        SSI_ERROR_RETURN(HW_SSI_IsIdle(p_ctrl->p_reg), SSP_ERR_IN_USE);

        if (!HW_SSI_WsContinueIsEnabled(p_ctrl->p_reg))
        {
            /** If starting transmission or reception and WS continue mode is disabled, start the timer if used. */
            if (NULL != p_ctrl->p_timer)
            {
                ssp_err_t err = p_ctrl->p_timer->p_api->start(p_ctrl->p_timer->p_ctrl);
                SSI_ERROR_RETURN((SSP_SUCCESS == err), err);
            }

            if (SSI_VERSION_SSI == g_ssi_version)
            {
                /** If WS continue mode is disabled, SSI must be reset after going idle. */
                HW_SSI_ResetRestore(p_ctrl->p_reg);
            }
        }

        /** Reset the FIFOs before starting the SSI. */
        HW_SSI_TxReset(p_ctrl->p_reg);
        HW_SSI_TxFifoRegisterReset(p_ctrl->p_reg);
        HW_SSI_RxReset(p_ctrl->p_reg);
        HW_SSI_RxFifoRegisterReset(p_ctrl->p_reg);

        /** If starting transmission, enable transmit interrupts. */
        if ((desired_ssicr_ren_ten & SSI_SSICR_TEN_BIT) > 0U)
        {
            p_ctrl->tx_in_use = true;
            HW_SSI_TxInterruptEnable(p_ctrl->p_reg, p_ctrl->txi_irq);
        }

        /** If starting reception, enable receive interrupts. */
        if ((desired_ssicr_ren_ten & SSI_SSICR_REN_BIT) > 0U)
        {
            p_ctrl->rx_in_use = true;
            HW_SSI_RxInterruptEnable(p_ctrl->p_reg, p_ctrl->rxi_irq);
        }

        p_ctrl->p_reg->SSICR |= desired_ssicr_ren_ten;
    }

    /** Enable error/idle interrupts. */
    HW_SSI_ErrorInterruptEnable(p_ctrl->int_irq);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 *  Writes data to FIFO.
 *
 * @param[in] p_ctrl  Pointer to the control block.
 *
 * @return The number of stages written
 **********************************************************************************************************************/
static uint32_t ssi_fifo_write(ssi_instance_ctrl_t * p_ctrl)
{
    /** If the data pointer is invalid, return immediately. */
    if (NULL == p_ctrl->p_tx_src)
    {
        return 0;
    }

    /** Calculate the number of free spaces in transmit FIFO. */
    uint32_t fifo_free_stages = (uint32_t) g_ssi_fifo_num_stages - HW_SSI_TxFifoFullGet(p_ctrl->p_reg);

    /** Calculate the number of words to write, limited by the space available in the FIFO. */
    uint32_t stages_to_write = p_ctrl->tx_src_bytes / p_ctrl->fifo_access_bytes;
    if (fifo_free_stages < stages_to_write)
    {
        stages_to_write = fifo_free_stages;
    }
    uint32_t stages_written = stages_to_write;

    /** If remaining data is less than 2 words, add padding zeros and write 2 words of data. */
    if (stages_to_write < (SSI_FIFO_SMALLEST_WRITE_WORDS * (sizeof(uint32_t) / p_ctrl->fifo_access_bytes)))
    {
        uint32_t p_src_32[SSI_FIFO_SMALLEST_WRITE_WORDS] = {0U, 0U};
        uint8_t * p_src = (uint8_t *) &p_src_32[0];
        uint8_t * p_tx_src_8 = (uint8_t *) p_ctrl->p_tx_src;
        for (uint32_t i = 0U; i < (SSI_FIFO_SMALLEST_WRITE_WORDS * p_ctrl->fifo_access_bytes); i++)
        {
            if (i < p_ctrl->tx_src_bytes)
            {
                p_src[i] = p_tx_src_8[i];
                p_ctrl->tx_src_bytes--;
            }
            else
            {
                p_src[i] = 0U;
            }
        }
        HW_SSI_TxFifoWrite(p_ctrl->p_reg, &p_src_32[0], p_ctrl->fifo_access_bytes);
        HW_SSI_TxFifoWrite(p_ctrl->p_reg, &p_src_32[1], p_ctrl->fifo_access_bytes);

        stages_written = 2U;
    }
    else
    {
        /** If remaining data is 2 words or more, write 2 words at a time to the transmit FIFO. */
        while (stages_to_write >= (SSI_FIFO_SMALLEST_WRITE_WORDS * (sizeof(uint32_t) / p_ctrl->fifo_access_bytes)))
        {
            HW_SSI_TxFifoWrite(p_ctrl->p_reg, p_ctrl->p_tx_src, p_ctrl->fifo_access_bytes);
            p_ctrl->p_tx_src = p_ctrl->p_tx_src + 1U;
            HW_SSI_TxFifoWrite(p_ctrl->p_reg, p_ctrl->p_tx_src, p_ctrl->fifo_access_bytes);
            p_ctrl->p_tx_src = p_ctrl->p_tx_src + 1U;
            uint32_t bytes_written = SSI_FIFO_SMALLEST_WRITE_WORDS * sizeof(uint32_t);
            stages_to_write -= (bytes_written / p_ctrl->fifo_access_bytes);
            p_ctrl->tx_src_bytes -= bytes_written;
        }
    }

    /** If transmission is complete, clear transmit buffer to NULL. */
    if (0U == p_ctrl->tx_src_bytes)
    {
        p_ctrl->p_tx_src = NULL;
    }

    return stages_written;
} /* End of function ssi_fifo_write */

/*******************************************************************************************************************//**
 *  Reads data from FIFO.
 *
 * @param[in] p_ctrl  Pointer to the control block.
 **********************************************************************************************************************/
static void ssi_fifo_read(ssi_instance_ctrl_t * p_ctrl)
{
    /** If the destination buffer pointer is invalid, return immediately. */
    if (NULL == p_ctrl->p_rx_dest)
    {
        return;
    }

    /** Calculate the number of available bytes of data in receive FIFO. */
    uint32_t fifo_filled_stages = HW_SSI_RxFifoFullGet(p_ctrl->p_reg);
    if (fifo_filled_stages < SSI_FIFO_SMALLEST_READ_WORDS)
    {
        return;
    }

    /** Calculate the number of FIFO stages requested to read. */
    uint32_t stages_to_read = 0U;
    stages_to_read = p_ctrl->rx_dest_bytes / p_ctrl->fifo_access_bytes;
    if (fifo_filled_stages < stages_to_read)
    {
        stages_to_read = fifo_filled_stages;
    }

    /** If remaining read request is less than 2 words, read 2 words and discard extra data. */
    if (stages_to_read < (SSI_FIFO_SMALLEST_READ_WORDS * (sizeof(uint32_t) / p_ctrl->fifo_access_bytes)))
    {
        uint32_t p_dest_32[SSI_FIFO_SMALLEST_READ_WORDS] = {0U, 0U};
        p_dest_32[0] = HW_SSI_RxFifoReadLimit(p_ctrl->p_reg, p_ctrl->fifo_access_bytes, fifo_filled_stages);
        p_dest_32[1] = HW_SSI_RxFifoReadLimit(p_ctrl->p_reg, p_ctrl->fifo_access_bytes, fifo_filled_stages);
        uint8_t * p_dest = (uint8_t *) p_dest_32;
        uint8_t * p_rx_dest_8 = (uint8_t *) p_ctrl->p_rx_dest;
        for (uint32_t i = 0U; i < (SSI_FIFO_SMALLEST_READ_WORDS * sizeof(uint32_t)); i++)
        {
            if (p_ctrl->rx_dest_bytes > 0U)
            {
                p_rx_dest_8[i] = p_dest[i];
                p_ctrl->rx_dest_bytes--;
            }
        }
    }
    else
    {
        /** If remaining data is 2 words or more, read 2 words at a time from the receive FIFO. */
        while (stages_to_read >= (SSI_FIFO_SMALLEST_READ_WORDS * (sizeof(uint32_t) / p_ctrl->fifo_access_bytes)))
        {
            *(p_ctrl->p_rx_dest) = HW_SSI_RxFifoRead(p_ctrl->p_reg, p_ctrl->fifo_access_bytes);
            p_ctrl->p_rx_dest = (p_ctrl->p_rx_dest) + 1u;
            *(p_ctrl->p_rx_dest) = HW_SSI_RxFifoRead(p_ctrl->p_reg, p_ctrl->fifo_access_bytes);
            p_ctrl->p_rx_dest = (p_ctrl->p_rx_dest) + 1u;
            uint32_t bytes_read = SSI_FIFO_SMALLEST_READ_WORDS * sizeof(uint32_t);
            stages_to_read -= (bytes_read / p_ctrl->fifo_access_bytes);
            p_ctrl->rx_dest_bytes -= bytes_read;
        }
    }

    /** Clear receive FIFO full flag */
    HW_SSI_RxFifoFullFlagClear(p_ctrl->p_reg);

    /** If reception is complete, clear receive buffer to NULL. */
    if (0U == p_ctrl->rx_dest_bytes)
    {
        p_ctrl->p_rx_dest = NULL;
    }
} /* End of function ssi_fifo_read */

/*******************************************************************************************************************//**
 * Process Idle interrupt event.  Calls callback when idle interrupt occurs.
 *
 * @param[in] p_ctrl  Control block of instance generating interrupt.
 **********************************************************************************************************************/
static inline void ssi_idle_int_process(ssi_instance_ctrl_t * p_ctrl)
{
    HW_SSI_IdleInterruptDisable(p_ctrl->p_reg, p_ctrl->int_irq);

    /** If peripheral is idle and the timer is used and WS continue is disabled, stop the timer to save power. */
    if (!HW_SSI_WsContinueIsEnabled(p_ctrl->p_reg))
    {
        if (p_ctrl->p_timer)
        {
            p_ctrl->p_timer->p_api->stop(p_ctrl->p_timer->p_ctrl);
        }
    }

    /** If peripheral is idle, call idle callback. */
    if (NULL != p_ctrl->p_callback)
    {
        i2s_callback_args_t args;
        args.event = I2S_EVENT_IDLE;
        args.p_context = p_ctrl->p_context;
        p_ctrl->p_callback(&args);
    }
}

/*******************************************************************************************************************//**
 * Process transmit underflow interrupt event.  If transfer is used for transmission, disable the transfer interface if
 * stop is requested.  If necessary, write zeros to ensure the total write length is a multiple of 8 bytes.
 *
 * @param[in] p_ctrl  Control block of instance generating interrupt.
 **********************************************************************************************************************/
static inline void ssi_tx_end_int_process(ssi_instance_ctrl_t * p_ctrl)
{
    /** SSI and SSIE must be stopped after a transmit underflow. */
    ssi_stop_tx(p_ctrl);

    /** After transmit underflow, write 0s if TSWNO is not 0. */
    uint32_t zero_data = 0U;
    if ((SSI_VERSION_SSI == g_ssi_version) && (!p_ctrl->zeros_written))
    {
        HW_SSI_TxFifoWrite(p_ctrl->p_reg, &zero_data, p_ctrl->fifo_access_bytes);
        HW_SSI_TxFifoWrite(p_ctrl->p_reg, &zero_data, p_ctrl->fifo_access_bytes);
        if (HW_SSI_TxWordNumberGet(p_ctrl->p_reg, g_ssi_version))
        {
            HW_SSI_TxFifoWrite(p_ctrl->p_reg, &zero_data, p_ctrl->fifo_access_bytes);
        }
        p_ctrl->zeros_written = true;
    }
    else
    {
        if (HW_SSI_TxWordNumberGet(p_ctrl->p_reg, g_ssi_version))
        {
            HW_SSI_TxFifoWrite(p_ctrl->p_reg, &zero_data, p_ctrl->fifo_access_bytes);
            return;
        }

        /** After transmit underflow if TSWNO is 0, disable transmission, disable error interrupts, and enable
         * the idle interrupt. */
        HW_SSI_TxUnderflowDisable(p_ctrl->p_reg);
        ssi_process_complete(p_ctrl);
    }
}

/*******************************************************************************************************************//**
 * If both transmit and receive are no longer in use, stop the SSI.
 *
 * @param[in] p_ctrl  Control block of instance generating interrupt.
 **********************************************************************************************************************/
static void ssi_process_complete(ssi_instance_ctrl_t * p_ctrl)
{
    if ((!p_ctrl->tx_in_use) && (!p_ctrl->rx_in_use))
    {
        if (!HW_SSI_IsIdle(p_ctrl->p_reg))
        {
            HW_SSI_IdleInterruptEnable(p_ctrl->p_reg);
        }
        HW_SSI_Stop(p_ctrl->p_reg);
    }
}

/*******************************************************************************************************************//**
 * Transmit ISR. Calls callback when transmission is complete.  Fills FIFO if transfer interface is not used.
 **********************************************************************************************************************/
void ssi_txi_isr(void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    ssi_instance_ctrl_t * p_ctrl = (ssi_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /* Clear the IR flag in the ICU */
    R_BSP_IrqStatusClear (R_SSP_CurrentIrqGet());

    if (NULL != p_ctrl)
    {
        i2s_callback_args_t args;
        if (NULL == p_ctrl->p_transfer_tx)
        {
            /** If transfer is not used, write data. */
            uint32_t stages_written = ssi_fifo_write(p_ctrl);
            if (stages_written > 0U)
            {
                HW_SSI_TxFifoEmptyFlagClear(p_ctrl->p_reg);
            }
        }

        /** If transmit transfer or data transmission is complete, call transmit complete callback. */
        if ((p_ctrl->tx_src_bytes < (SSI_FIFO_SMALLEST_WRITE_WORDS * sizeof(uint32_t)))
                || (NULL != p_ctrl->p_transfer_tx))
        {
            if (NULL != p_ctrl->p_callback)
            {
                args.event = I2S_EVENT_TX_EMPTY;
                args.p_context = p_ctrl->p_context;
                p_ctrl->p_callback(&args);
            }
        }
    }

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

/*******************************************************************************************************************//**
 * Receive ISR.  Calls callback when reception is complete.  Empties FIFO if transfer interface is not used.
 **********************************************************************************************************************/
void ssi_rxi_isr(void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    ssi_instance_ctrl_t * p_ctrl = (ssi_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /* Clear the IR flag in the ICU */
    R_BSP_IrqStatusClear (R_SSP_CurrentIrqGet());

    if (NULL != p_ctrl)
    {
        i2s_callback_args_t args;
        if (NULL == p_ctrl->p_transfer_rx)
        {
            /** If transfer is not used, read data into the destination buffer. */
            ssi_fifo_read(p_ctrl);
            if (0U == p_ctrl->rx_dest_bytes)
            {
                p_ctrl->rx_in_use = false;
            }
        }

        /** If receive transfer or data reception is complete, call receive complete callback. */
        if (((NULL != p_ctrl->p_transfer_rx) && ((uint16_t) 0U == p_ctrl->p_transfer_rx->p_cfg->p_info->length))
                || (0U == p_ctrl->rx_dest_bytes))
        {
            if (NULL != p_ctrl->p_callback)
            {
                args.event = I2S_EVENT_RX_FULL;
                args.p_context = p_ctrl->p_context;
                p_ctrl->p_callback(&args);
            }
        }
    }

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

/*******************************************************************************************************************//**
 * Error and idle ISR.
 *
 * Handles transmit underflow errors and calls callback when idle interrupt occurs.
 **********************************************************************************************************************/
void ssi_int_isr(void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    ssi_instance_ctrl_t * p_ctrl = (ssi_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    if (NULL != p_ctrl)
    {
        ssi_irq_events ssi_irq_events_info = {0};
        ssi_irq_events_info.SSISR = HW_SSI_EventsGet(p_ctrl->p_reg);
        /* Events that are being serviced are being cleared. During this time if another error occurs then this
         * interrupt will fire again */
        HW_SSI_EventsClear(p_ctrl->p_reg, (uint32_t)ssi_irq_events_info.SSISR);

        if (1U == ssi_irq_events_info.SSISR_b.IIRQ)
        {
            ssi_idle_int_process(p_ctrl);
        }
        else
        {
        	/** SSI and SSIE must go idle after a transmit underflow or receive overflow error. */
            if (1U == ssi_irq_events_info.SSISR_b.TUIRQ)
            {
                ssi_tx_end_int_process(p_ctrl);
            }
            if (1U == ssi_irq_events_info.SSISR_b.ROIRQ)
            {
            	/** After receive error, disable reception, disable error interrupts, and enable
            	 * the idle interrupt. */
                ssi_stop_rx(p_ctrl);
            }
        }
    }

    /* Clear the IR flag in the ICU */
    R_BSP_IrqStatusClear (R_SSP_CurrentIrqGet());

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

