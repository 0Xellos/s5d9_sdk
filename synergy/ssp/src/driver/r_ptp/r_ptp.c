/***********************************************************************************************************************
 * Copyright [2019-2024] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : r_ptp.c
 * Description  : HAL API for PTP driver
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_ptp.h"
#include "hw/hw_ptp_private.h"
#include "r_ptp_private_api.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#ifndef PTP_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define PTP_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_version)
#endif

/** "PTP" in ASCII. Used to determine if the channel is open. */
#define PTP_OPEN                  (0x00505450U)
#define MIN16_S                   (0xFFFF8000U)
#define MAX16_S                   (0x00007FFFU)

/* Scaled time interval maximum and minimum value (2^(16) nano second unit)  */
#define SCALED_TIME_MAX_U         (0x7FFFFFFFU)
#define SCALED_TIME_MAX_L         (0xFFFFFFFFU)
#define SCALED_TIME_MIN_U         (0x7FFFFFFFU)
#define SCALED_TIME_MIN_L         (0x00000000U)
/* Timeout to wait for register to be written */
#define MAX_REGISTER_WAIT_COUNT   (0xFFFFFFFFUL)

/**********************************************************************************************************************
Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

static void r_ptp_interrupt_request_initialize(ptp_instance_ctrl_t * const p_ctrl);
static void r_ptp_synfp_state_initialize(ptp_instance_ctrl_t * const p_ctrl);
static void r_ptp_clock_state_initialize(ptp_instance_ctrl_t * const p_ctrl, uint8_t ptp_channel);
static void r_ptp_transparent_clock_initialize(ptp_instance_ctrl_t * const p_api_ctrl, uint8_t ptp_channel);
static void r_ptp_init_synfp(ptp_instance_ctrl_t * const p_ctrl, uint8_t ptp_channel);
static void r_ptp_init_prc_stca(ptp_instance_ctrl_t * const p_ctrl, ptp_stca_mode_t stca_mode, uint8_t ptp_channel);
static void r_ptp_time_info_get(uint32_t reg_upper, uint32_t reg_lower, ptp_timeInterval_t * p_info);
static void r_ptp_start_transparent_clock (ptp_instance_ctrl_t * const p_ctrl, uint8_t ptp_channel);
static void r_ptp_ip_mac_address_configure(ptp_instance_ctrl_t * const p_ctrl, uint32_t * p_ip_address,
        uint32_t * p_physical_address_msw,
        uint32_t * p_physical_address_lsw);

static ssp_err_t r_ptp_base_address_get(ptp_instance_ctrl_t * const p_ctrl, ptp_cfg_t const * const p_cfg, ssp_feature_t * const ssp_feature);
static ssp_err_t r_ptp_event_configure(ptp_instance_ctrl_t * const p_ctrl, ssp_feature_t * const ssp_feature);
static ssp_err_t r_ptp_wait(volatile uint32_t * reg, uint32_t event, uint32_t wait_option);
static ssp_err_t r_ptp_synfp_wait(R_EPTPC0_Type * p_ptp_reg, uint32_t event, uint32_t wait_option);
static ssp_err_t r_ptp_info_check(R_EPTPC_GEN_Type * p_ptp_reg_gen, uint32_t wait_option);
static ssp_err_t r_ptp_start_clock_mode (ptp_instance_ctrl_t * const p_ctrl, uint8_t ptp_channel, uint32_t wait_option);
static ssp_err_t r_ptp_stop_clock_mode (ptp_instance_ctrl_t * const p_ctrl, uint8_t ptp_channel, uint32_t wait_option);

/***********************************************************************************************************************
 * ISR function prototypes
 **********************************************************************************************************************/
void eptpc_mint_isr(void);

/***********************************************************************************************************************
Exported global variables (to be accessed by other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "r_ptp";
#endif

#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug. This pragma suppresses the warnings in this
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_version =
{
    .api_version_major  = PTP_API_VERSION_MAJOR,
    .api_version_minor  = PTP_API_VERSION_MINOR,
    .code_version_major = PTP_CODE_VERSION_MAJOR,
    .code_version_minor = PTP_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/*********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const ptp_api_t g_ptp_on_ptp =
{
    .open                           = R_PTP_Open,
    .close                          = R_PTP_Close,
    .configure                      = R_PTP_Configure,
    .setExtPromiscuous              = R_PTP_SetExtPromiscuous,
    .setLocalClock                  = R_PTP_SetLocalClock,
    .getLocalClock                  = R_PTP_GetLocalClock,
    .getMasterPortID                = R_PTP_GetMasterPortID,
    .setMasterPortID                = R_PTP_SetMasterPortID,
    .getSyncInfo                    = R_PTP_GetSyncInfo,
    .start                          = R_PTP_Start,
    .stop                           = R_PTP_Stop,
    .setWorst10Values               = R_PTP_SetWorst10Values,
    .checkWorst10Values             = R_PTP_CheckWorst10Values,
    .getWorst10Values               = R_PTP_GetWorst10Values,
    .setGradientLimit               = R_PTP_SetGradientLimit,
    .updateClockID                  = R_PTP_UpdateClockID,
    .updateDomainNumber             = R_PTP_UpdateDomainNumber,
    .updateAnnounceFlags            = R_PTP_UpdateAnnounceFlags,
    .updateAnnounceMsgs             = R_PTP_UpdateAnnounceMsgs,
    .updateSyncAnnounceMsgInterval  = R_PTP_UpdateSyncAnnounceMsgInterval,
    .updateDelayMsgInterval         = R_PTP_UpdateDelayMsgInterval,
    .getMessageReceptionConfig      = R_PTP_GetMessageReceptionConfig,
    .setMessageReceptionConfig      = R_PTP_SetMessageReceptionConfig,
    .disableTimer                   = R_PTP_DisableTimer,
    .indicateEvent                  = R_PTP_IndicateEvent,
    .autoClearEvent                 = R_PTP_AutoClearEvent,
    .setTimer                       = R_PTP_SetTimer,
    .setMINTevent                   = R_PTP_SetMINTevent,
    .enableINFABTnotification       = R_PTP_EnableINFABTnotification,
    .disableINFABTnotification      = R_PTP_DisableINFABTnotification,
    .checkINFABTstatus              = R_PTP_CheckINFABTstatus,
    .clearINFABTstatus              = R_PTP_ClearINFABTstatus,
    .versionGet                     = R_PTP_VersionGet
};

/*******************************************************************************************************************//**
 * @addtogroup PTP
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Open the PTP driver, handles required initialization described in hardware manual.
 * Implements ptp_api_t::open
 *
 * @retval SSP_SUCCESS                PTP driver has opened successfully and initialization was successful.
 * @retval SSP_ERR_IN_USE             The channel specified has already been opened.
 * @retval SSP_ERR_ASSERTION          The p_ctrl or p_cfg parameter was NULL.
 * @retval  SSP_ERR_IRQ_BSP_DISABLED  A required interrupt does not exist in the vector table.
 * @return                             See @ref Common_Error_Codes or functions called by this function for other possible
 *                                     return codes. This function calls:
 *                                         * fmi_api_t::productFeatureGet
 *                                         * fmi_api_t::eventInfoGet
 **********************************************************************************************************************/
ssp_err_t R_PTP_Open (ptp_ctrl_t * const p_api_ctrl, ptp_cfg_t const * const p_cfg)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if PTP_CFG_PARAM_CHECKING_ENABLE
    /** Check parameters */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_cfg);
#endif

    ssp_err_t err = SSP_SUCCESS;

    /** Verify if this unit has not already been initialized */
    PTP_ERROR_RETURN(PTP_OPEN != p_ctrl->open, SSP_ERR_IN_USE);

    /** Make sure the peripheral exists */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    err  = r_ptp_base_address_get(p_ctrl, p_cfg, &ssp_feature);
    PTP_ERROR_RETURN(SSP_SUCCESS == err, err);

    R_EPTPC_CFG_Type * p_ptp_reg_cfg = (R_EPTPC_CFG_Type *) p_ctrl->p_reg_cfg;
    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    ssp_feature.channel  = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_EPTPC;

    /** Lock EPTPC channel */
    err = R_BSP_HardwareLock(&ssp_feature);
    PTP_ERROR_RETURN(SSP_SUCCESS == err, SSP_ERR_IN_USE);

    /** Power on the PTP module. */
    R_BSP_ModuleStart(&ssp_feature);

    /** Configure MINT interrupt */
    r_ptp_event_configure(p_ctrl, &ssp_feature);

    /** Reset PTP driver */
    HW_PTP_Reset(p_ptp_reg_cfg, true);

    /** Wait at least 64 cycles of PCLKA to reset the PTPEDMAC and EPTPC.
     * PCLKA must be at least 12.5 MHz to use Ethernet, so wait at least 5.12 us. */
    R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MILLISECONDS);

    /* Release reset EPTPC */
    HW_PTP_Reset(p_ptp_reg_cfg, false);

    /** Initialize the channel state information */
    p_ctrl->device              = p_cfg->device;
    p_ctrl->delay[0]            = p_cfg->delay[0];
    p_ctrl->state[0]            = p_cfg->state[0];
    p_ctrl->frame_format[0]     = p_cfg->frame_format[0];
    p_ctrl->delay[1]            = p_cfg->delay[1];
    p_ctrl->state[1]            = p_cfg->state[1];
    p_ctrl->frame_format[1]     = p_cfg->frame_format[1];
    p_ctrl->stca_mode           = p_cfg->stca_sync_mode;
    p_ctrl->p_callback          = p_cfg->p_callback;
    p_ctrl->p_context           = p_cfg->p_context;

    /* Initialize the EPTPC channel flag */
    p_ctrl->eptpc_flag[0] = 0U;
    p_ctrl->eptpc_flag[1] = 0U;

    /** Select synchronization frame processing unit (SYNFP0 or SYNFP1) */
    if ((PTP_DEVICE_ORDINARY_CLOCK1 == p_ctrl->device) ||
            ((PTP_DEVICE_BOUNDARY_CLOCK == p_ctrl->device) &&
                    (PTP_STATE_SLAVE == p_ctrl->state[1])))
    {
        /* OC port1 or BC port1 slave (BC port0 master) */
        HW_PTP_TimeSynchChannelSelect(p_ptp_reg_gen, 1U);
    }
    else
    {
        /* OC port0, BC master-master, BC port0 slave (BC port1 master) and TC */
        HW_PTP_TimeSynchChannelSelect(p_ptp_reg_gen, 0U);
    }

    /** Initialize EPTPC MINT interrupt requests */
    r_ptp_interrupt_request_initialize(p_ctrl);

    /** Initialize SYNFP reception status */
    r_ptp_synfp_state_initialize(p_ctrl);

    /** Enable MINT interrupt */
    if(SSP_INVALID_VECTOR != p_ctrl->mint_irq)
    {
        NVIC_EnableIRQ(p_ctrl->mint_irq);
    }
    /** Disable bypassing of EPTPC module */
    HW_PTP_BypassModule();

    /** Mark driver as opened by initializing it to "PTP" in its ASCII equivalent for this unit */
    p_ctrl->open = PTP_OPEN;

    return SSP_SUCCESS;
} /* End of function R_PTP_Open() */

/*******************************************************************************************************************//**
 * @brief Closes the PTP driver.
 * Implements ptp_api_t::close
 *
 * @retval SSP_SUCCESS           The PTP driver is successfully closed.
 * @retval SSP_ERR_NOT_OPEN      The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION     Pointer to the control block is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTP_Close (ptp_ctrl_t * const p_api_ctrl)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if PTP_CFG_PARAM_CHECKING_ENABLE
    /** Check parameters */
    SSP_ASSERT(p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_EPTPC;

    /** Disable PTP MINT interrupt */
    if(SSP_INVALID_VECTOR != p_ctrl->mint_irq)
    {
        NVIC_DisableIRQ(p_ctrl->mint_irq);
    }

    /** Release hardware lock */
    R_BSP_HardwareUnlock(&ssp_feature);

    /* Clear the EPTPC channel flag */
    p_ctrl->eptpc_flag[0] = 0U;
    p_ctrl->eptpc_flag[1] = 0U;

    /** Mark driver as closed */
    p_ctrl->open = 0U;

    return SSP_SUCCESS;
}/* End of function R_PTP_Close() */

/*******************************************************************************************************************//**
 * @brief Configures the PTP driver with IP and MAC address.
 * Implements ptp_api_t::configure
 *
 * @retval SSP_SUCCESS           The PTP driver is successfully configured with IP and MAC address.
 * @retval SSP_ERR_NOT_OPEN      The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION     Pointer to the control block or passed argument is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTP_Configure (ptp_ctrl_t  * const p_api_ctrl,
        uint32_t * p_ip_address,
        uint32_t * p_physical_address_msw,
        uint32_t * p_physical_address_lsw)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_ip_address);
    SSP_ASSERT(NULL != p_physical_address_msw);
    SSP_ASSERT(NULL != p_physical_address_lsw);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    r_ptp_ip_mac_address_configure(p_ctrl, p_ip_address, p_physical_address_msw, p_physical_address_lsw);

    /** Initialize Synchronization Frame Processing unit (SYNFP) */
    /** Initialize Packet Relation Controller (PRC-TC) and Statistical Time Correction Algorithm (STCA) units */
    switch(p_ctrl->device)
    {
        case PTP_DEVICE_ORDINARY_CLOCK0:
        {
            r_ptp_init_synfp(p_ctrl, 0U);
            r_ptp_init_prc_stca(p_ctrl, p_ctrl->stca_mode, 0U);
            break;
        }

        case PTP_DEVICE_ORDINARY_CLOCK1:
        {
            r_ptp_init_synfp(p_ctrl, 1U);
            r_ptp_init_prc_stca(p_ctrl, p_ctrl->stca_mode, 1U);
            break;
        }

        case PTP_DEVICE_BOUNDARY_CLOCK:
        case PTP_DEVICE_TRANSPARENT_CLOCK:
        {
            r_ptp_init_synfp(p_ctrl, 0U);
            r_ptp_init_synfp(p_ctrl, 1U);
            r_ptp_init_prc_stca(p_ctrl, p_ctrl->stca_mode, 0U);
            r_ptp_init_prc_stca(p_ctrl, p_ctrl->stca_mode, 1U);
            break;
        }

        default:
        {
            break;
        }
    }

    return SSP_SUCCESS;
}/* End of function R_PTP_Configure() */

/*******************************************************************************************************************//**
 * @brief Sets or clear the extended promiscuous mode of the specified PTP channel.
 * Implements ptp_api_t::setExtPromiscuous
 *
 * @retval SSP_SUCCESS                    Extended promiscuous mode is set/cleared successfully.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_SetExtPromiscuous(ptp_ctrl_t  * const p_api_ctrl, uint8_t ptp_channel, bool is_set)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    /** Set or clear extended promiscuous mode of the specified PTP channel */
    HW_PTP_ExtendedPromiscuousModeSet(p_ptp_reg, is_set);

    return SSP_SUCCESS;
} /* End of function R_PTP_SetExtPromiscuous() */

/*******************************************************************************************************************//**
 * @brief Gets the current local clock counter value in Timestamp format(UNIX time) when configured as slave.
 * Implements ptp_api_t::getLocalClock
 *
 * @retval SSP_SUCCESS              Local clock counter get is successful.
 * @retval SSP_ERR_NOT_OPEN         The PTP driver is not opened.
 * @retval SSP_ERR_TIMEOUT          Clock info is not acquired before timeout.
 * @retval SSP_ERR_ASSERTION        Pointer to the control block or p_clock parameter is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTP_GetLocalClock(ptp_ctrl_t  * const p_api_ctrl, ptp_timestamp_t * p_clock, uint32_t wait_option)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_clock);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    /** Request the current local clock counter information */
    HW_PTP_InfoControl(p_ptp_reg_gen, true);

    /** Wait till local clock counter value is loaded with current local time */
    ssp_err_t err = r_ptp_info_check(p_ptp_reg_gen, wait_option);
    PTP_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Save current local clock counter value */
    HW_PTP_LocalClockGet(p_ptp_reg_gen, p_clock);

    return SSP_SUCCESS;
} /* End of function R_PTP_GetLocalClock() */

/*******************************************************************************************************************//**
 * @brief Sets local clock counter value with Timestamp (UNIX time). Master clock set the master time.
 * Implements ptp_api_t::setLocalClock
 *
 * @retval SSP_SUCCESS                    Local clock counter set is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block or p_clock parameter is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTP_SetLocalClock(ptp_ctrl_t  * const p_api_ctrl, ptp_timestamp_t * p_clock)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_clock);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    /** Set local clock counter value with the specified time information */
    HW_PTP_LocalClockSet(p_ptp_reg_gen, p_clock->secondsField.high, p_clock->secondsField.low, p_clock->nanosecondsField);

    /* Load set value to local clock counter */
    HW_PTP_LoadSetValue(p_ptp_reg_gen, true);

    return SSP_SUCCESS;
} /* End of function R_PTP_SetLocalClock() */

/*******************************************************************************************************************//**
 * @brief Gets master clock ID and master port number fields of the specified PTP channel.
 * Note: If the argument (p_clock or p_port) is NULL pointer, the value will not be acquired.
 * Implements ptp_api_t::getMasterPortID
 *
 * @retval SSP_SUCCESS                    Master port ID get is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block, p_clock or p_port parameter is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_GetMasterPortID(ptp_ctrl_t  * const p_api_ctrl, uint8_t ptp_channel, uint32_t * p_clock, uint16_t * p_port)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    if (NULL != p_clock)
    {
        /** Get master clock ID high and low fields */
        HW_PTP_MasterClockIDGet(p_ptp_reg, p_clock);
    }
    if (NULL != p_port)
    {
        /** Get master port number field */
        HW_PTP_MasterPortNumGet(p_ptp_reg, p_port);
    }

    return SSP_SUCCESS;
} /* End of function R_PTP_GetMasterPortID() */

/*******************************************************************************************************************//**
 * @brief Sets master clock ID and master port number fields of the specified PTP channel.
 * Note: If the argument (p_clock or p_port) is NULL pointer, the value will not be updated.
 * Implements ptp_api_t::setMasterPortID
 *
 * @retval SSP_SUCCESS                    Master port ID is updated successfully.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block, p_clock or p_port is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_SetMasterPortID(ptp_ctrl_t  * const p_api_ctrl, uint8_t ptp_channel, uint32_t * p_clock, uint16_t * p_port)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    if (NULL != p_clock)
    {
        /** Set master clock ID high and low fields */
        HW_PTP_MasterClockIDSet(p_ptp_reg, (uint32_t)*p_clock, (uint32_t)*(p_clock+1));
    }
    if (NULL != p_port)
    {
        /** Set master port number field */
        HW_PTP_MasterClkPortNumberSet(p_ptp_reg, *p_port);

    }
    if ((NULL != p_clock) || (NULL != p_port))
    {
        /* Validate set values to SYNFP */
        HW_PTP_BMCUpdate(p_ptp_reg, 1U);
    }

    return SSP_SUCCESS;
} /* End of function R_PTP_SetMasterPortID() */

/*******************************************************************************************************************//**
 * @brief Gets the current offset from master and mean path delay of the specified PTP channel when configured as slave.
 * Note: If the argument (p_master_offset or p_path_delay) is NULL pointer, the value will not be acquired.
 *  Implements ptp_api_t::getSyncInfo
 *
 * @retval SSP_SUCCESS                    Offset from master and mean path delay get is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block, p_master_offset or p_path_delay is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_GetSyncInfo(ptp_ctrl_t  * const p_api_ctrl,
        uint8_t ptp_channel,
        ptp_timeInterval_t * p_master_offset,
        ptp_timeInterval_t * p_path_delay)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    uint32_t reg_u;     // upper register absolute value
    uint32_t reg_l;     // lower register absolute value

    if (NULL != p_master_offset)
    {
        /** Request offset from master from the specified PTP channel */
        reg_u = (uint32_t)HW_PTP_OFMUpperBitsGet(p_ptp_reg);
        reg_l = (uint32_t)HW_PTP_OFMLowerBitsGet(p_ptp_reg);

        /** Get the offset from master in nano-seconds format */
        r_ptp_time_info_get(reg_u, reg_l, p_master_offset);
    }
    if (NULL != p_path_delay)
    {
        /** Request mean path delay from the specified PTP channel */
        reg_u = (uint32_t)HW_PTP_MPDUpperBitsGet(p_ptp_reg);
        reg_l = (uint32_t)HW_PTP_MPDLowerBitsGet(p_ptp_reg);

        /** Get the mean path delay in nano-seconds format */
        r_ptp_time_info_get(reg_u, reg_l, p_path_delay);
    }

    return SSP_SUCCESS;
} /* End of function R_PTP_GetSyncInfo() */

/*******************************************************************************************************************//**
 * @brief Starts time synchronization.
 *  Implements ptp_api_t::start
 *
 * @retval SSP_SUCCESS                    Time synchronization started successfully.
 * @retval SSP_ERR_TIMEOUT                Time synchronization did not start before timeout.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_MODE           Invalid PTP mode.
 **********************************************************************************************************************/
ssp_err_t R_PTP_Start(ptp_ctrl_t  * const p_api_ctrl, uint32_t wait_option)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    ssp_err_t err = SSP_SUCCESS;
    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    /** Starts time synchronization */
    switch(p_ctrl->device)
    {
        case PTP_DEVICE_ORDINARY_CLOCK0:
        {
            err = r_ptp_start_clock_mode(p_ctrl, 0U, wait_option);
            PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
            break;
        }

        case PTP_DEVICE_ORDINARY_CLOCK1:
        {
            err = r_ptp_start_clock_mode(p_ctrl, 1U, wait_option);
            PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
            break;
        }

        case PTP_DEVICE_BOUNDARY_CLOCK:
        {
            err = r_ptp_start_clock_mode(p_ctrl, 0U, wait_option);
            PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
            err = r_ptp_start_clock_mode(p_ctrl, 1U, wait_option);
            PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
            break;
        }

        case PTP_DEVICE_TRANSPARENT_CLOCK:
        {
            r_ptp_start_transparent_clock(p_ctrl, 0U);
            r_ptp_start_transparent_clock(p_ctrl, 1U);
            break;
        }
        default:
        {
            break;
        }
    }

    if ((PTP_STATE_SLAVE == p_ctrl->state[0]) || (PTP_STATE_SLAVE == p_ctrl->state[1]))
    {
        HW_PTP_SlaveTimeSyncControl(p_ptp_reg_gen, 1U);
    }

    return err;
} /* End of function R_PTP_Start() */

/*******************************************************************************************************************//**
 * @brief Stops time synchronization.
 * Implements ptp_api_t::stop
 *
 * @retval SSP_SUCCESS                    Time synchronization stopped successfully.
 * @retval SSP_ERR_TIMEOUT                Time synchronization did not stop before timeout.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_MODE           Invalid PTP mode.
 **********************************************************************************************************************/
ssp_err_t R_PTP_Stop(ptp_ctrl_t  * const p_api_ctrl, uint32_t wait_option)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    ssp_err_t err = SSP_SUCCESS;

    /** Stops the time synchronization */
    switch(p_ctrl->device)
    {
        case PTP_DEVICE_ORDINARY_CLOCK0:
        {
            err = r_ptp_stop_clock_mode(p_ctrl, 0U, wait_option);
            PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
            break;
        }

        case PTP_DEVICE_ORDINARY_CLOCK1:
        {
            err = r_ptp_stop_clock_mode(p_ctrl, 1U, wait_option);
            PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
            break;
        }

        case PTP_DEVICE_BOUNDARY_CLOCK:
        {
            err = r_ptp_stop_clock_mode(p_ctrl, 0U, wait_option);
            PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
            err = r_ptp_stop_clock_mode(p_ctrl, 1U, wait_option);
            PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
            break;
        }

        case PTP_DEVICE_TRANSPARENT_CLOCK:
        {
            /* no operation */
            break;
        }
        default:
        {
            break;
        }
    }
    return err;
} /* End of function R_PTP_Stop() */

/*******************************************************************************************************************//**
 * @brief Sets the interval for collecting worst 10 values.
 *  Implements ptp_api_t::setWorst10Values
 *
 * @retval SSP_SUCCESS                    Worst 10 values are recorded successfully.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTP_SetWorst10Values(ptp_ctrl_t * const p_api_ctrl, uint8_t interval)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    /** Sets interval to get worst 10 values */
    HW_PTP_W10AcquisitionTimeSet(p_ptp_reg_gen, interval);

    return SSP_SUCCESS;
} /* End of function R_PTP_SetWorst10Values() */

/*******************************************************************************************************************//**
 * @brief Checks if worst 10 values are acquired and set as gradient limits when configured as slave.
 * Implements ptp_api_t::checkWorst10Values
 *
 * @retval SSP_SUCCESS             Gradient values are set based on worst 10 values successfully.
 * @retval SSP_ERR_TIMEOUT         Gradient values are not set before timeout.
 * @retval SSP_ERR_NOT_OPEN        The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION       Pointer to the control block is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTP_CheckWorst10Values(ptp_ctrl_t * const p_api_ctrl, uint32_t wait_option)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    ssp_err_t err = SSP_SUCCESS;
    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    /** Wait till worst10 values are acquired by hardware and set as gradient limits */
    err = r_ptp_wait((volatile uint32_t *)(HW_PTP_STCAStatusGet(p_ptp_reg_gen)), PTP_STCA_W10_STATUS, wait_option);

    return err;
} /* End of function R_PTP_CheckWorst10Values() */

/*******************************************************************************************************************//**
 * @brief Gets positive and negative worst 10 values by software.
 * Note: If the argument (p_positive_worst10 or p_negative_worst10) is NULL pointer, the value will not be acquired.
 *  Implements ptp_api_t::getWorst10Values
 *
 * @retval SSP_SUCCESS                    Positive and negative worst 10 values get is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTP_GetWorst10Values(ptp_ctrl_t * const p_api_ctrl,
        uint32_t * p_positive_worst10,
        uint32_t * p_negative_worst10,
        uint32_t wait_option)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    ssp_err_t err = SSP_SUCCESS;
    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    /** Request current worst10 values acquired */
    HW_PTP_AcquireW10Values(p_ptp_reg_gen, true);

    /** Wait till gradient worst10 values are acquired by software */
    err = r_ptp_wait((volatile uint32_t *)(HW_PTP_STCAStatusGet(p_ptp_reg_gen)), PTP_STCA_W10_STATUS, wait_option);
    PTP_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* Request worst10 gradient values */
    HW_PTP_InfoControl(p_ptp_reg_gen, true);

    /* Wait till worst 10 gradient values are loaded */
    err = r_ptp_info_check(p_ptp_reg_gen, wait_option);
    PTP_ERROR_RETURN(SSP_SUCCESS == err, err);

    if (NULL != p_positive_worst10)
    {
        /** Get positive gradient values */
        HW_PTP_PositiveW10ValuesGet(p_ptp_reg_gen, p_positive_worst10);
    }

    if (NULL != p_negative_worst10)
    {
        /** Get negative gradient values */
        HW_PTP_NegativeW10ValuesGet(p_ptp_reg_gen, p_negative_worst10);
    }

    return err;
} /* End of function R_PTP_GetWorst10Values() */

/*******************************************************************************************************************//**
 * @brief Sets the gradient limits for positive and negative worst 10 values.
 * Note: If the argument (p_positive_limit or p_negative_limit) is NULL pointer, the value will not be acquired.
 * Implements ptp_api_t::setGradientLimit
 *
 * @retval SSP_SUCCESS                    Positive and negative gradient values set is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTP_SetGradientLimit(ptp_ctrl_t * const p_api_ctrl, uint32_t * p_positive_limit, uint32_t * p_negative_limit)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    if (NULL != p_positive_limit)
    {
        /** Set positive gradient values */
        HW_PTP_PositiveW10ValuesSet(p_ptp_reg_gen, p_positive_limit);
    }

    if (NULL != p_negative_limit)
    {
        /** Set negative gradient values */
        HW_PTP_NegativeW10ValuesSet(p_ptp_reg_gen, p_negative_limit);
    }

    return SSP_SUCCESS;
} /* End of function R_PTP_SetGradientLimit() */

/*******************************************************************************************************************//**
 * @brief Updates clock identity field.
 * Implements ptp_api_t::updateClockID
 *
 * @retval SSP_SUCCESS                    Clock identity field set is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_UpdateClockID(ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, int8_t * p_clock_id)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_clock_id);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

   /* Update local clock ID upper and lower fields */
    uint32_t clock_id_upper = (uint32_t)((*p_clock_id << 24U) | ((*(p_clock_id + 1) << 16U) & 0xFF0000) | ((*(p_clock_id + 2) << 8U) & 0xFF00) | (*(p_clock_id + 3) & 0xFF));
    uint32_t clock_id_lower = (uint32_t)((*(p_clock_id + 4) << 24U) | ((*(p_clock_id + 5) << 16U) & 0xFF0000) | ((*(p_clock_id + 6) << 8U) & 0xFF00) | (*(p_clock_id + 7) & 0xFF));

    /** Update local clockIdentity field of the specified EPTPC port */
    HW_PTP_SYNFPLocalClkIDSet(p_ptp_reg, clock_id_upper, clock_id_lower);

    return SSP_SUCCESS;
} /* End of function R_PTP_UpdateClockID() */

/*******************************************************************************************************************//**
 * @brief Updates domain number field in the message header.
 * Implements ptp_api_t::updateDomainNumber
 *
 * @retval SSP_SUCCESS                    Domain number field set is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_UpdateDomainNumber(ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, uint8_t domain_num)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    /** Update domainNumber field of the PTP message header */
    HW_PTP_SYNFPDomainNumberSet(p_ptp_reg, domain_num);

    return SSP_SUCCESS;
} /* End of function R_PTP_UpdateDomainNumber() */

/*******************************************************************************************************************//**
 * @brief Updates announce message's flag field.
 * Implements ptp_api_t::updateAnnounceFlags
 *
 * @retval SSP_SUCCESS                    Announce message flag field set is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_UpdateAnnounceFlags(ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, ptp_announce_flag_t * p_flag)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_flag);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    /** Update announce message's flag field */
    uint32_t flag_value = (uint32_t)((p_flag->bit.profileSpec2 << 14U) & PTP_MASK_BIT14) |
            ((p_flag->bit.profileSpec1 << 13U) & PTP_MASK_BIT13) |
            ((p_flag->bit.unicastFlag << 10U) & PTP_MASK_BIT10) |
            ((p_flag->bit.alternateMasterFlag << 8U) & PTP_MASK_BIT8) |
            ((p_flag->bit.frequencyTraceable << 5U) & PTP_MASK_BIT5) |
            ((p_flag->bit.timeTraceable << 4U) & PTP_MASK_BIT4) |
            ((p_flag->bit.ptpTimescale << 3U) & PTP_MASK_BIT3) |
            ((p_flag->bit.currentUtcOffsetValid << 2U) & PTP_MASK_BIT2) |
            ((p_flag->bit.leap59 << 1U) & PTP_MASK_BIT1) |
            ((p_flag->bit.leap61) & PTP_MASK_BIT0);

    HW_PTP_AnnounceFlagFieldSet(p_ptp_reg, flag_value);

    /* Update Announce Message Generation Information */
    HW_PTP_AnnounceMsgInfoUpdate(p_ptp_reg, 1U);

    return SSP_SUCCESS;
} /* End of function R_PTP_UpdateAnnounceFlags() */

/*******************************************************************************************************************//**
 * @brief Updates announce message's message field.
 * Implements ptp_api_t::updateAnnounceMsgs
 *
 * @retval SSP_SUCCESS                    Announce message field set is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_UpdateAnnounceMsgs(ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, ptp_announce_message_t * p_message)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_message);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    int8_t * p_id;
    uint32_t clock_quality;
    uint32_t clock_class = (uint32_t)p_message->grandmasterClockQuality.clockClass;
    uint32_t clock_accuracy = (uint32_t)p_message->grandmasterClockQuality.clockAccuracy;
    uint32_t log_msg_interval = (uint32_t)p_message->grandmasterClockQuality.offsetScaledLogVariance;

    /** Update grandmasterPriority1 and grandmasterPriority2 fields */
    HW_PTP_GrandMasterPriority1Set(p_ptp_reg, p_message->grandmasterPriority1);
    HW_PTP_GrandMasterPriority2Set(p_ptp_reg, p_message->grandmasterPriority2);

    /** Update grandmasterClockQuality fields */
    clock_quality = (clock_class << 24U) | ((clock_accuracy << 16U) & 0xFF0000) | (log_msg_interval & 0xFFFF);
    HW_PTP_GMClockQuality(p_ptp_reg, clock_quality);

    /* Update grandmasterIdentity fields */
    p_id = p_message->p_grandmasterIdentity;
    uint32_t id_upper = (uint32_t)((*p_id << 24U) | ((*(p_id + 1) << 16U) & 0xFF0000) | ((*(p_id + 2) << 8U) & 0xFF00) | (*(p_id + 3) & 0xFF));
    uint32_t id_lower = (uint32_t)((*(p_id + 4) << 24U) | ((*(p_id + 5) << 16U) & 0xFF0000) | ((*(p_id + 6) << 8U) & 0xFF00) | (*(p_id + 7) & 0xFF));

    /** Update grandmasterIdentity fields of Announce messages */
    HW_PTP_GrandMasterIdentityFieldSet(p_ptp_reg, id_upper, id_lower);

    /* Update Announce Message Generation Information */
    HW_PTP_AnnounceMsgInfoUpdate(p_ptp_reg, 1U);

    return SSP_SUCCESS;
} /* End of function R_PTP_UpdateAnnounceMsgs() */

/*******************************************************************************************************************//**
 * @brief Updates transmission interval and logMessageInterval of Sync and Announce messages.
 * Note: If the argument (p_sync_interval or p_announce_interval) is NULL pointer, the value will not be acquired.
 * Implements ptp_api_t::updateSyncAnnounceMsgInterval
 *
 * @retval SSP_SUCCESS                    Transmission interval and logMessage interval set is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_UpdateSyncAnnounceMsgInterval(ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, int8_t * p_sync_interval, int8_t * p_announce_interval)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    if (NULL != p_sync_interval)
    {
        /** Update SYNFP Sync message transmission interval */
        /* Check if the interval is less than -7 (2^(-7) seconds) */
        if ((int8_t)PTP_MINIMUM_TRANSMISSION_INTERVAL > (*p_sync_interval))
        {
            /* If interval is less than 2^(-7), set to 7.8125 msec */
            HW_PTP_SYNFPSyncIntervalSet(p_ptp_reg, PTP_TRANSMISSION_INTERVAL_MINIMUM);
        }
        else if ((int8_t)PTP_MAXIMUM_TRANSMISSION_INTERVAL <= (*p_sync_interval))
        {
            /* If interval is more than 2^(6), set to 64 sec */
            HW_PTP_SYNFPSyncIntervalSet(p_ptp_reg, PTP_TRANSMISSION_INTERVAL_MAXIMUM);
        }
        else
        {
            HW_PTP_SYNFPSyncIntervalSet(p_ptp_reg, (uint8_t)(*p_sync_interval));
        }

        /* Update SYNFP state */
        HW_PTP_SYNFPStateUpdate(p_ptp_reg, 1U);
    }

    if (NULL != p_announce_interval)
    {
        /** Update SYNFP announce message Transmission Interval */
        /* Check if the interval is less than -7 (2^(-7) seconds) */
        if ((int8_t)PTP_MINIMUM_TRANSMISSION_INTERVAL > (*p_announce_interval))
        {
            /* If interval is less than 2^(-7), set to 7.8125 msec */
            HW_PTP_SYNFPAnnounceIntervalSet(p_ptp_reg, PTP_TRANSMISSION_INTERVAL_MINIMUM);

        }
        else if ((int8_t)PTP_MAXIMUM_TRANSMISSION_INTERVAL <= (*p_announce_interval))
        {
            /* If interval is more than 2^(6), set to 64 sec */
            HW_PTP_SYNFPAnnounceIntervalSet(p_ptp_reg, PTP_TRANSMISSION_INTERVAL_MAXIMUM);
        }
        else
        {
            HW_PTP_SYNFPAnnounceIntervalSet(p_ptp_reg, (uint8_t)(*p_announce_interval));
        }

        /* Update Announce Message Generation Information */
        HW_PTP_AnnounceMsgInfoUpdate(p_ptp_reg, 1U);
    }

    return SSP_SUCCESS;
} /* End of function R_PTP_UpdateSyncAnnounceMsgInterval() */

/*******************************************************************************************************************//**
 * @brief Updates transmission interval, logMessageInterval and timeout values of Delay message.
 * Note: If the argument (p_interval or p_timeout) is NULL pointer, the value will not be updated.
 * Implements ptp_api_t::updateDelayMsgInterval
 *
 * @retval SSP_SUCCESS                    Transmission interval, logMessage interval and timeout set is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_UpdateDelayMsgInterval(ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, int8_t * p_interval, uint32_t * p_timeout)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    int8_t delay_resp;

    if (NULL != p_interval)
    {
        if (PTP_STATE_MASTER == p_ctrl->state[ptp_channel])
        {
            /** If clock state is master, update Delay_Response logMessageInterval value */
            /* Check if the interval is less than -7 (2^(-7) seconds) */
            if ((int8_t)PTP_MINIMUM_TRANSMISSION_INTERVAL > (*p_interval))
            {
                /* If interval is less than 2^(-7), set to 7.8125 msec */
                HW_PTP_SYNFPDelayReqIntervalSet(p_ptp_reg, PTP_TRANSMISSION_INTERVAL_MINIMUM);
            }
            else if ((int8_t)PTP_MAXIMUM_TRANSMISSION_INTERVAL <= (*p_interval))
            {
                /* If interval is more than 2^(6), set to 64 sec */
                HW_PTP_SYNFPDelayReqIntervalSet(p_ptp_reg, PTP_TRANSMISSION_INTERVAL_MAXIMUM);
            }
            else
            {
                HW_PTP_SYNFPDelayReqIntervalSet(p_ptp_reg, (uint8_t)(*p_interval));
            }
        }
        else
        {
            /** If clock state is slave, update Delay_Request /Pdelay_Request transmission interval value */
            HW_PTP_SYNFPDelayRespIntervalGet(p_ptp_reg, &delay_resp);

            /* Check if the interval is less than -7 (2^(-7) seconds) */
            if ((int8_t)PTP_MINIMUM_TRANSMISSION_INTERVAL > delay_resp)
            {
                /* If interval is less than 2^(-7), set to 7.8125 msec */
                HW_PTP_SYNFPDelayReqIntervalSet(p_ptp_reg, PTP_TRANSMISSION_INTERVAL_MINIMUM);
            }
            else if ((int8_t)PTP_MAXIMUM_TRANSMISSION_INTERVAL <= delay_resp)
            {
                /* If interval is more than 2^(6), set to 64 sec */
                HW_PTP_SYNFPDelayReqIntervalSet(p_ptp_reg, PTP_TRANSMISSION_INTERVAL_MAXIMUM);
            }
            else
            {
                /* Set Delay_Resp/Pdelay_Resp field to Delay_Req/Pdelay_Req field */
                HW_PTP_SYNFPDelayReqIntervalSet(p_ptp_reg, (uint8_t)delay_resp);
            }
            HW_PTP_SYNFPDelayReqIntervalGet(p_ptp_reg, p_interval);
        }
    }

    if (NULL != p_timeout)
    {
        /** Update Delay_Response/Pdelay_Response receiving timeout value */
        HW_PTP_ReceptionTimeout(p_ptp_reg, * p_timeout);
    }

    if ((NULL != p_interval) || (NULL != p_timeout))
    {
        /* Update SYNFP state */
        HW_PTP_SYNFPStateUpdate(p_ptp_reg, 1U);
    }

    return SSP_SUCCESS;
} /* End of function R_PTP_UpdateDelayMsgInterval() */

/*******************************************************************************************************************//**
 * @brief Gets PTP message reception synchronous configuration.
 * Implements ptp_api_t::getMessageReceptionConfig
 *
 * @retval SSP_SUCCESS                    Synchronous configuration get is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_GetMessageReceptionConfig(ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, ptp_message_reception_t * p_ptp_message_reception)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_ptp_message_reception);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    /** Get SYNFP Reception Filter Register 1 */
    HW_PTP_SYNFPReceptionFilter1Get(p_ptp_reg, &p_ptp_message_reception->reception_filter1);

    /** Get SYNFP Reception Filter Register 2 */
    HW_PTP_SYNFPReceptionFilter2Get(p_ptp_reg, &p_ptp_message_reception->reception_filter2);

    /** Get SYNFP Transmission Enable Register */
    HW_PTP_SYNFPTransmissionEnableGet(p_ptp_reg, &p_ptp_message_reception->transmission_enable);

    /** Get SYNFP Operation Setting Register */
    HW_PTP_SYNFPOperationGet(p_ptp_reg, &p_ptp_message_reception->operation_control);

    return SSP_SUCCESS;
} /* End of function R_PTP_GetMessageReceptionConfig() */

/*******************************************************************************************************************//**
 * @brief Sets PTP message reception synchronous configuration.
 * Implements ptp_api_t::setMessageReceptionConfig
 *
 * @retval SSP_SUCCESS                    Synchronous configuration set is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_SetMessageReceptionConfig(ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, ptp_message_reception_t * p_ptp_message_reception)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_ptp_message_reception);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    /** Set SYNFP Reception Filter Register 1 */
    HW_PTP_SYNFPReceptionFilter1Set(p_ptp_reg, p_ptp_message_reception->reception_filter1);

    /** Set SYNFP Reception Filter Register 2 */
    HW_PTP_SYNFPReceptionFilter2Set(p_ptp_reg, p_ptp_message_reception->reception_filter2);

    /** Set SYNFP Transmission Enable Register */
    HW_PTP_SYNFPTransmissionEnableSet(p_ptp_reg, p_ptp_message_reception->transmission_enable);

    /** Set SYNFP Operation Setting Register */
    HW_PTP_SYNFPOperationSet(p_ptp_reg, p_ptp_message_reception->operation_control);

    /* Update SYNFP state */
    HW_PTP_SYNFPStateUpdate(p_ptp_reg, 1U);

    return SSP_SUCCESS;
} /* End of function R_PTP_SetMessageReceptionConfig() */

/*******************************************************************************************************************//**
 * @brief Disables the specified timer event interrupt.
 * Implements ptp_api_t::disableTimer
 *
 * @retval SSP_SUCCESS                    Timer event interrupt disable is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTP_DisableTimer(ptp_ctrl_t * const p_api_ctrl, ptp_stca_timer_channel_t timer_channel)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    uint32_t mieipr;

    /** Get MINT interrupt request status */
    HW_PTP_MINTIrqRequestGet(p_ptp_reg_gen, &mieipr);
    mieipr &= ~((uint32_t)timer_channel << 16U);

    /** Disable the timer event of specified pulse output channel */
    HW_PTP_MINTIrqRequestSet(p_ptp_reg_gen, mieipr);

    return SSP_SUCCESS;
} /* End of function R_PTP_DisableTimer() */

/*******************************************************************************************************************//**
 * @brief Set/clear interrupt indication to ELC output on generation of pulse produced by pulse output timer.
 * Implements ptp_api_t::indicateEvent
 *
 * @retval SSP_SUCCESS                    ELC interrupt indication set or clear is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTP_IndicateEvent(ptp_ctrl_t * const p_api_ctrl,
        ptp_stca_timer_channel_t timer_channel,
        ptp_stca_timer_pulse_edge_t timer_pulse_edge,
        bool is_set)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    uint32_t elc_output_status;
    uint32_t channel = timer_channel;

    /** Get IPLS Interrupt Permission Register status */
    HW_PTP_IPLSIrqRequestGet(p_ptp_reg_gen, &elc_output_status);

    /* Check if timer pulse edge is falling */
    if (1U == timer_pulse_edge)
    {
        channel <<= PTP_TIMER_EDGE_FALLING;
    }

    if (true == is_set)
    {
        /** If set, enable output from pulse output timer channel */
        elc_output_status |= channel;
    }

    else
    {
        /* Disable output from pulse output timer channel */
        elc_output_status &= ~channel;
    }

    /** Set IPLS Interrupt Permission Register status */
    HW_PTP_IPLSIrqRequestSet(p_ptp_reg_gen, elc_output_status);

    return SSP_SUCCESS;
} /* End of function R_PTP_IndicateEvent() */

/*******************************************************************************************************************//**
 * @brief Set/clear auto clear mode for enabling one time output of ELC event.
 * Implements ptp_api_t::autoClearEvent
 *
 * @retval SSP_SUCCESS                    ELC interrupt auto clear mode set or clear is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTP_AutoClearEvent(ptp_ctrl_t * const p_api_ctrl,
        ptp_stca_timer_channel_t timer_channel,
        ptp_stca_timer_pulse_edge_t timer_pulse_edge,
        bool is_set)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    uint32_t auto_clear_status;
    uint32_t channel = timer_channel;

    /** Get IPLS Interrupt Permission Automatic clearing register status */
    HW_PTP_IPLSIrqAutoClearGet(p_ptp_reg_gen, &auto_clear_status);

    /* Check if timer pulse edge is falling */
    if (1U == timer_pulse_edge)
    {
        channel <<= PTP_TIMER_EDGE_FALLING;
    }

    /** If set, enable automatic clearing for specified pulse output timer channel */
    if (true == is_set)
    {
        auto_clear_status |= channel;
    }

    else
    {
        /* Disable automatic clearing for specified pulse output timer channel */
        auto_clear_status &= ~ channel;
    }

    /** Set IPLS Interrupt Permission Automatic clearing register status */
    HW_PTP_IPLSIrqAutoClearSet(p_ptp_reg_gen, auto_clear_status);

    return SSP_SUCCESS;
} /* End of function R_PTP_AutoClearEvent() */

/*******************************************************************************************************************//**
 * @brief Sets start time, pulse period and pulse width for the pulse output timer.
 * Implements ptp_api_t::setTimer
 *
 * @retval SSP_SUCCESS                    Timer event set is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTP_SetTimer(ptp_ctrl_t * const p_api_ctrl,
        uint8_t timer_channel,
        UInt64_t event_time,
        uint32_t cycle,
        uint32_t pulse_width)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if (PTP_CFG_PARAM_CHECKING_ENABLE)
    /** Check parameters */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(5U >= timer_channel);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    uint32_t temp = 0U;
    ptp_stca_timer_channel_t ptp_timer_channel = (ptp_stca_timer_channel_t)(1U << timer_channel);

    HW_PTP_MINTIrqTimerGet(p_ptp_reg_gen, &temp);
    uint32_t timer_select = temp | (uint32_t)ptp_timer_channel;

    /* Set the pulse output timer */
    HW_PTP_MINTIrqTimerSet(p_ptp_reg_gen, timer_select);

    /** Set event time, PWM cycle interval and PWM pulse width to specified timer channel */
    HW_PTP_TimerStartTimeSet(p_ptp_reg_gen, timer_channel, event_time.high, event_time.low);
    HW_PTP_TimerCycleSet(p_ptp_reg_gen, timer_channel, cycle);
    HW_PTP_TimerPulseWidthSet(p_ptp_reg_gen, timer_channel, pulse_width);

    /** Start the specified pulse output timer */
    HW_PTP_TimerStartGet(p_ptp_reg_gen, &temp);
    uint32_t tmstartr = temp | (uint32_t)ptp_timer_channel;
    HW_PTP_TimerStartSet(p_ptp_reg_gen, tmstartr);

    return SSP_SUCCESS;
} /* End of function R_PTP_SetTimer() */

/*******************************************************************************************************************//**
 * @brief Sets MINT interrupt event to enable notification for change in state of modules
 * Implements ptp_api_t::setMINTevent
 *
 * @retval SSP_SUCCESS                    MINT interrupt event set is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTP_SetMINTevent (ptp_ctrl_t * const p_api_ctrl, ptp_event_t ptp_reg, uint32_t event, bool is_set)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if PTP_CFG_PARAM_CHECKING_ENABLE
    /** Check parameters */
    SSP_ASSERT(p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    uint32_t value = 0U;

    switch(ptp_reg)
    {
        case PTP_EVENT_STCA:
        {
            HW_PTP_STCAStatusNotificationGet(p_ctrl->p_reg_gen, &value);

            if(true == is_set)
            {
                value |= event;
            }
            else
            {
                value &= ~event;
            }
            /** Enable notification of STCA status */
            HW_PTP_STCAStatusNotificationSet(p_ctrl->p_reg_gen, value);
            break;
        }

        case PTP_EVENT_PRCTC:
        {
            HW_PTP_PRCTCStatusNotificationGet(p_ctrl->p_reg_gen, &value);

            if(true == is_set)
            {
                value |= event;
            }
            else
            {
                value &= ~event;
            }
            /** Enable notification of PRC-TC status */
            HW_PTP_PRCTCStatusNotificationSet(p_ctrl->p_reg_gen, value);
            break;
        }

        case PTP_EVENT_SYNFP0:
        {
            HW_PTP_SYNFPStatusNotificationGet(p_ctrl->p_reg[0], &value);

            if(true == is_set)
            {
                value |= event;
            }
            else
            {
                value &= ~event;
            }
            /** Enable notification of SYNFP0 status */
            HW_PTP_SYNFPStatusNotificationSet(p_ctrl->p_reg[0], value);
            break;
        }

        case PTP_EVENT_SYNFP1:
        {
            HW_PTP_SYNFPStatusNotificationGet(p_ctrl->p_reg[1], &value);

            if(true == is_set)
            {
                value |= event;
            }
            else
            {
                value &= ~event;
            }
            /** Enable notification of SYNFP1 status */
            HW_PTP_SYNFPStatusNotificationSet(p_ctrl->p_reg[1], value);
            break;
        }

        default:
        {
            break;
        }
    }

    return SSP_SUCCESS;
}/* End of function R_PTP_SetMINTevent() */

/*******************************************************************************************************************//**
 * @brief Enables EPTPC INFABT notification of the specified PTP channel.
 * Implements ptp_api_t::enableINFABTnotification
 *
 * @retval SSP_SUCCESS                    INFABT notification enable is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 *@retval SSP_ERR_INVALID_CHANNEL         Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_EnableINFABTnotification (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if PTP_CFG_PARAM_CHECKING_ENABLE
    /** Check parameters */
    SSP_ASSERT(p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    uint32_t mint_irq_enable = 0U;
    uint32_t synfp_status = 0U;

    /** Initially clear the status of INFABT detection flag of the specified PTP channel */
    p_ctrl->infabt_flag[ptp_channel] = false;

    HW_PTP_MINTIrqRequestGet(p_ptp_reg_gen, &mint_irq_enable);

    if (0U == ptp_channel)
    {
        /** Enable generation of ETHER_MINT interrupt by SYNFP0 status flag */
        mint_irq_enable |= PTP_ENABLE_SYNFP0_INTERRUPT;
    }

    else
    {
        /** Enable generation of ETHER_MINT interrupt by SYNFP1 status flag */
        mint_irq_enable |= PTP_ENABLE_SYNFP1_INTERRUPT;
    }

    HW_PTP_SYNFPStatusNotificationGet(p_ctrl->p_reg[ptp_channel], &synfp_status);

    /** Enable the INFABT detection flag of specified PTP channel */
    synfp_status |= PTP_ENABLE_INFABT_NOTIFICATION;

    HW_PTP_SYNFPStatusNotificationSet(p_ctrl->p_reg[ptp_channel], synfp_status);

    HW_PTP_MINTIrqRequestSet(p_ptp_reg_gen, mint_irq_enable);

    return SSP_SUCCESS;
}/* End of function R_PTP_EnableINFABTnotification() */

/*******************************************************************************************************************//**
 * @brief Disables EPTPC INFABT notification of the specified PTP channel.
 * Implements ptp_api_t::disableINFABTnotification
 *
 * @retval SSP_SUCCESS                    INFABT notification disable is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_DisableINFABTnotification (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if PTP_CFG_PARAM_CHECKING_ENABLE
    /** Check parameters */
    SSP_ASSERT(p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    uint32_t synfp_status = 0U;

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    HW_PTP_SYNFPStatusNotificationGet(p_ptp_reg, &synfp_status);

    /** Disable INFABT detection flag of the specified PTP channel */
    synfp_status &= ~ PTP_ENABLE_INFABT_NOTIFICATION;

    HW_PTP_SYNFPStatusNotificationSet(p_ptp_reg, synfp_status);

    return SSP_SUCCESS;
}/* End of function R_PTP_DisableINFABTnotification() */

/*******************************************************************************************************************//**
 * @brief Checks the status of INFABT flag of the specified PTP channel.
 * Implements ptp_api_t::checkINFABTstatus
 *
 * @retval SSP_SUCCESS                    INFABT flag status check is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_CheckINFABTstatus (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, uint8_t * p_status)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if PTP_CFG_PARAM_CHECKING_ENABLE
    /** Check parameters */
    SSP_ASSERT(p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    /** Gets the current status of INFABT notification flag of the specified PTP channel */
    *p_status = p_ctrl->infabt_flag[ptp_channel];

    return SSP_SUCCESS;
} /* End of function R_PTP_CheckINFABTstatus() */

/*******************************************************************************************************************//**
 * @brief Clears INFABT interrupt occurrence flag of the specified PTP channel.
 * Implements ptp_api_t::clearINFABTstatus
 *
 * @retval SSP_SUCCESS                    INFABT status flag clear is successful.
 * @retval SSP_ERR_NOT_OPEN               The PTP driver is not opened.
 * @retval SSP_ERR_ASSERTION              Pointer to the control block is NULL.
 * @retval SSP_ERR_INVALID_CHANNEL        Invalid EPTPC channel.
 **********************************************************************************************************************/
ssp_err_t R_PTP_ClearINFABTstatus (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel)
{
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *) p_api_ctrl;

#if PTP_CFG_PARAM_CHECKING_ENABLE
    /** Check parameters */
    SSP_ASSERT(p_ctrl);
    PTP_ERROR_RETURN(PTP_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTP_ERROR_RETURN(((0U == ptp_channel) || (1U == ptp_channel)) && (1U == p_ctrl->eptpc_flag[ptp_channel]), SSP_ERR_INVALID_CHANNEL);
#endif

    /** Clear the status of INFABT notification flag of the specified PTP channel */
    p_ctrl->infabt_flag[ptp_channel] = false;

    return SSP_SUCCESS;
} /* End of function R_PTP_CheckINFABTstatus() */

/*******************************************************************************************************************//**
 * @brief   Gets version information and stores it in the provided version struct.
 * Implements ptp_api_t::versionGet
 *
 * @retval SSP_SUCCESS           Version returned successfully.
 * @retval SSP_ERR_ASSERTION     Parameter p_version was NULL.
 ***********************************************************************************************************************/
ssp_err_t R_PTP_VersionGet(ssp_version_t * const p_version)
{
#if PTP_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid */
    SSP_ASSERT(NULL != p_version);
#endif

    /** Return the version number */
    p_version->version_id = g_version.version_id;

    return SSP_SUCCESS;
} /* End of function R_PTP_GetVersion() */

/*******************************************************************************************************************//**
 * @} (end addtogroup PTP)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Initializes EPTPC MINT interrupt status.
 *
 * @param[in] p_ctrl              Pointer to PTP instance block
 *
 * @retval      void
 **********************************************************************************************************************/
static void r_ptp_interrupt_request_initialize(ptp_instance_ctrl_t * const p_ctrl)
{

    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;

    /* Set EPTPC interrupt mask */
    HW_PTP_MINTIrqRequestSet(p_ptp_reg_gen, 0U);
    HW_PTP_STCAStatusNotificationSet(p_ptp_reg_gen, 0U);
    HW_PTP_PRCTCStatusNotificationSet(p_ptp_reg_gen, 0U);

    /* Clear EPTPC interrupt status */
    HW_PTP_MINTInterruptStatusSet(p_ptp_reg_gen, PTP_MINT_IRQ_STATUS);
    HW_PTP_STCAStatusSet(p_ptp_reg_gen, PTP_STCA_STATUS);
    HW_PTP_PRCTCStatusSet(p_ptp_reg_gen, PTP_PRC_TC_STATUS);

    HW_PTP_MINTIrqRequestSet(p_ptp_reg_gen, PTP_MINT_IRQ_STATUS);
}/* End of function r_ptp_interrupt_request_initialize() */

/*******************************************************************************************************************//**
 * @brief Initializes SYNFP reception status.
 *
 * @param[in] p_ctrl              Pointer to PTP instance block
 *
 * @retval      void
 **********************************************************************************************************************/
static void r_ptp_synfp_state_initialize(ptp_instance_ctrl_t * const p_ctrl)
{

    switch(p_ctrl->device)
    {
         case PTP_DEVICE_ORDINARY_CLOCK0:
         {
             /* Initialize the SYNFP reception filters for ordinary clock 0 */
             r_ptp_clock_state_initialize(p_ctrl, 0U);
             p_ctrl->eptpc_flag[0] = 1U;
             break;
         }

         case PTP_DEVICE_ORDINARY_CLOCK1:
         {
             /* Initialize the SYNFP reception filters for ordinary clock 1 */
             r_ptp_clock_state_initialize(p_ctrl, 1U);
             p_ctrl->eptpc_flag[1] = 1U;
             break;
         }

         case PTP_DEVICE_BOUNDARY_CLOCK:
         {
             /* Initialize the SYNFP reception filters for boundary clock */
             r_ptp_clock_state_initialize(p_ctrl, 0U);
             r_ptp_clock_state_initialize(p_ctrl, 1U);
             /* Set the EPTPC flag */
             p_ctrl->eptpc_flag[0] = 1U;
             p_ctrl->eptpc_flag[1] = 1U;
             break;
         }

         case PTP_DEVICE_TRANSPARENT_CLOCK:
         {
             /* Initialize the SYNFP reception filters for transparent clock */
             r_ptp_transparent_clock_initialize(p_ctrl, 0U);
             r_ptp_transparent_clock_initialize(p_ctrl, 1U);
             /* Set the EPTPC flag */
             p_ctrl->eptpc_flag[0] = 1U;
             p_ctrl->eptpc_flag[1] = 1U;
             break;
         }
         default:
         {
             break;
         }
    }
} /* End of function r_ptp_synfp_state_initialize() */

/*******************************************************************************************************************//**
 * @brief Initializes SYNFP registers when clock mode is configured as ordinary or boundary clock.
 *
 * @param[in] p_ctrl              Pointer to PTP instance block
 * @param[in] ptp_channel         EPTPC channel
 *
 * @retval    void
 **********************************************************************************************************************/
static void r_ptp_clock_state_initialize(ptp_instance_ctrl_t * const p_ctrl, uint8_t ptp_channel)
{

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    if (PTP_DELAY_MECHANISM_P2P == p_ctrl->delay[ptp_channel])
    {
        /* Set PTP reception filters based on the device type */
        if (PTP_STATE_MASTER == p_ctrl->state[ptp_channel])
        {
            HW_PTP_SYNFPReceptionFilter1Set(p_ptp_reg, PTP_SYNFP_FILTER1_P2P_MASTER_START);
            HW_PTP_SYNFPReceptionFilter2Set(p_ptp_reg, PTP_SYNFP_OC_BC_FILTER2_SET);
            HW_PTP_SYNFPOperationSet(p_ptp_reg, PTP_SYNFP_OPERATION_SET);
        }
        else if (PTP_STATE_SLAVE == p_ctrl->state[ptp_channel])
        {
            HW_PTP_SYNFPReceptionFilter1Set(p_ptp_reg, PTP_SYNFP_FILTER1_P2P_SLAVE_START);
            HW_PTP_SYNFPReceptionFilter2Set(p_ptp_reg, PTP_SYNFP_OC_BC_FILTER2_SET);
            HW_PTP_SYNFPOperationSet(p_ptp_reg, PTP_SYNFP_OPERATION_SET);
        }
        else if (PTP_STATE_LISTENING == p_ctrl->state[ptp_channel])
        {
            /* PTP listening state */
            HW_PTP_SYNFPReceptionFilter1Set(p_ptp_reg, PTP_SYNFP_FILTER1_LISTENING_START);
            HW_PTP_SYNFPReceptionFilter2Set(p_ptp_reg, PTP_SYNFP_OC_BC_FILTER2_SET);
            HW_PTP_SYNFPOperationSet(p_ptp_reg, PTP_SYNFP_OPERATION_SET);
        }
        else
        {
            /* No operation */
        }
    }
    else
    {
        if (PTP_STATE_MASTER == p_ctrl->state[ptp_channel])
        {
            HW_PTP_SYNFPReceptionFilter1Set(p_ptp_reg, PTP_SYNFP_FILTER1_E2E_MASTER_START);
            HW_PTP_SYNFPReceptionFilter2Set(p_ptp_reg, PTP_SYNFP_OC_BC_FILTER2_SET);
            HW_PTP_SYNFPOperationSet(p_ptp_reg, PTP_SYNFP_OPERATION_SET);
        }
        else if (PTP_STATE_SLAVE == p_ctrl->state[ptp_channel])
        {
            HW_PTP_SYNFPReceptionFilter1Set(p_ptp_reg, PTP_SYNFP_FILTER1_E2E_SLAVE_START);
            HW_PTP_SYNFPReceptionFilter2Set(p_ptp_reg, PTP_SYNFP_OC_BC_FILTER2_SET);
            HW_PTP_SYNFPOperationSet(p_ptp_reg, PTP_SYNFP_OPERATION_SET);
        }
        else if (PTP_STATE_LISTENING == p_ctrl->state[ptp_channel])
        {
            /* PTP listening state */
            HW_PTP_SYNFPReceptionFilter1Set(p_ptp_reg, PTP_SYNFP_FILTER1_LISTENING_START);
            HW_PTP_SYNFPReceptionFilter2Set(p_ptp_reg, PTP_SYNFP_OC_BC_FILTER2_SET);
            HW_PTP_SYNFPOperationSet(p_ptp_reg, PTP_SYNFP_OPERATION_SET);
        }
        else
        {
            /* No operation */
        }
    }

    /* Set SYNFP Register Value Load Directive Register */
    HW_PTP_SYNFPLoadDirectiveSet(p_ptp_reg, PTP_SYNFP_LOAD_REGISTER);

    HW_PTP_SYNFPStatusSet(p_ptp_reg, PTP_SYNFP_STATUS);

    HW_PTP_SYNFPStatusNotificationSet(p_ptp_reg, 0U);
}/* End of function r_ptp_clock_state_initialize() */

/*******************************************************************************************************************//**
 * @brief Initializes SYNFP registers when clock mode is configured as transparent clock.
 *
 * @param[in] p_ctrl              Pointer to PTP instance block
 * @param[in] ptp_channel         EPTPC channel
 *
 * @retval    void
 **********************************************************************************************************************/
static void r_ptp_transparent_clock_initialize(ptp_instance_ctrl_t * const p_ctrl, uint8_t ptp_channel)
{

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    /* Set PTP reception filters for transparent clock */
    if (PTP_DELAY_MECHANISM_P2P == p_ctrl->delay[0])
    {
        HW_PTP_SYNFPReceptionFilter1Set(p_ptp_reg, PTP_SYNFP_FILTER1_P2P_TC_START);
        HW_PTP_SYNFPReceptionFilter2Set(p_ptp_reg, PTP_SYNFP_TC_FILTER2_SET);
        HW_PTP_SYNFPOperationSet(p_ptp_reg, PTP_SYNFP_TC_OPERATION_SET);
    }
    else
    {
        HW_PTP_SYNFPReceptionFilter1Set(p_ptp_reg, PTP_SYNFP_FILTER1_E2E_TC_START);
        HW_PTP_SYNFPReceptionFilter2Set(p_ptp_reg, PTP_SYNFP_TC_FILTER2_SET);
        HW_PTP_SYNFPOperationSet(p_ptp_reg, PTP_SYNFP_OPERATION_SET);
    }

    /* Set SYNFP Register Value Load Directive Register */
    HW_PTP_SYNFPLoadDirectiveSet(p_ptp_reg, PTP_SYNFP_LOAD_REGISTER);

    HW_PTP_SYNFPStatusSet(p_ptp_reg, PTP_SYNFP_STATUS);

    HW_PTP_SYNFPStatusNotificationSet(p_ptp_reg, 0U);
}/* End of function r_ptp_transparent_clock_initialize() */

/*******************************************************************************************************************//**
 * @brief Initializes Synchronization Frame Processing unit(SYNFP) of the specified PTP channel.
 *
 * @param[in] p_ctrl                 Pointer to PTP instance block
 * @param[in] ptp_channel            EPTPC channel
 *
 * @retval    void
 **********************************************************************************************************************/
static void r_ptp_init_synfp(ptp_instance_ctrl_t * const p_ctrl, uint8_t ptp_channel)
{

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    typedef union
    {
        uint32_t All ;
        /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
        struct
        {
            uint8_t b0 ;
            uint8_t b1 ;
            uint8_t b2 ;
            uint8_t b3 ;
        } bit ;
    } LDATA_t ;
    LDATA_t LDATA;
    /** Set MAC address */
    LDATA.bit.b3 = 0U;
    LDATA.bit.b2 = p_ctrl->address[ptp_channel].mac_addr[0];
    LDATA.bit.b1 = p_ctrl->address[ptp_channel].mac_addr[1];
    LDATA.bit.b0 = p_ctrl->address[ptp_channel].mac_addr[2];
    uint32_t mac_uppr = LDATA.All;

    LDATA.bit.b3 = 0U;
    LDATA.bit.b2 = p_ctrl->address[ptp_channel].mac_addr[3];
    LDATA.bit.b1 = p_ctrl->address[ptp_channel].mac_addr[4];
    LDATA.bit.b0 = p_ctrl->address[ptp_channel].mac_addr[5];
    uint32_t mac_lwr = LDATA.All;

    HW_PTP_SYNFPMACAddressSet(p_ptp_reg, mac_uppr, mac_lwr);

    /** Set IP address */
    LDATA.bit.b3 = p_ctrl->address[ptp_channel].ip_addr[0];
    LDATA.bit.b2 = p_ctrl->address[ptp_channel].ip_addr[1];
    LDATA.bit.b1 = p_ctrl->address[ptp_channel].ip_addr[2];
    LDATA.bit.b0 = p_ctrl->address[ptp_channel].ip_addr[3];
    uint32_t ip_addr = LDATA.All;
    HW_PTP_SYNFP_IPAddressSet(p_ptp_reg, ip_addr);

    HW_PTP_SYNFP_LLC_CTLSet(p_ptp_reg, PTP_LLC_CTL);

    /* Set PTP version */
    HW_PTP_SYNFPVersionSet(p_ptp_reg, PTP_VER_NUM);
    /* Set domain number */
    HW_PTP_SYNFPDomainNumberSet(p_ptp_reg, PTP_DOMAIN_NUM);

    /** Initialize PTP message flag field */
    HW_PTP_AnnounceFlagFieldInitSet(p_ptp_reg, PTP_ANNOUNCE_FLAG_FIELD); // Announce message
    HW_PTP_SyncFlagFieldSet(p_ptp_reg, PTP_SYNC_FLAG_FIELD); // Sync message
    HW_PTP_DelayReqFlagFieldSet(p_ptp_reg, PTP_DELAY_REQ_FLAG_FIELD); // Delay_Req/Pdelay_Req message
    HW_PTP_DelayRespFlagFieldSet(p_ptp_reg, PTP_DELAY_RESP_FLAG_FIELD); // Delay_Resp/Pdelay_Resp message

    /** Initialize clock identity */
    /* Create clock-ID from MAC address */
    /* Vendor ID (3byte) - FF - FE - Product ID (3byte) */
    LDATA.bit.b3 = p_ctrl->address[ptp_channel].mac_addr[0];
    LDATA.bit.b2 = p_ctrl->address[ptp_channel].mac_addr[1];
    LDATA.bit.b1 = p_ctrl->address[ptp_channel].mac_addr[2];
    LDATA.bit.b0 = 0xFF;
    uint32_t id_uppr = LDATA.All;

    LDATA.bit.b3 = 0xFE;
    LDATA.bit.b2 = p_ctrl->address[ptp_channel].mac_addr[3];
    LDATA.bit.b1 = p_ctrl->address[ptp_channel].mac_addr[4];
    LDATA.bit.b0 = p_ctrl->address[ptp_channel].mac_addr[5];
    uint32_t id_lwr = LDATA.All;

    HW_PTP_SYNFPLocalClkIDSet(p_ptp_reg, id_uppr, id_lwr);

    /* Set the SYNFP port number */
    if(((PTP_DEVICE_BOUNDARY_CLOCK == p_ctrl->device) || (PTP_DEVICE_TRANSPARENT_CLOCK == p_ctrl->device)) && (1U == ptp_channel))
    {
        HW_PTP_SYNFPPortNumSet(p_ptp_reg, PTP_CLOCK_PORT_2);
    }
    else
    {
        HW_PTP_SYNFPPortNumSet(p_ptp_reg, PTP_CLOCK_PORT_1);
    }

    HW_PTP_SYNFPTransmissionEnableSet(p_ptp_reg, 0U);

    /* Initialize synchronize master PortIdentity */
    HW_PTP_MasterClockIDSet(p_ptp_reg, PTP_MTCID_U, PTP_MTCID_L);
    HW_PTP_MasterClkPortNumberSet(p_ptp_reg, (uint16_t)PTP_MTPID);

    /** Initialize transmission interval */
    HW_PTP_SYNFPAnnounceIntervalSet(p_ptp_reg, (uint8_t)PTP_LOG_ANNOUNCE_INTERVAL);
    HW_PTP_SYNFPSyncIntervalSet(p_ptp_reg, PTP_LOG_SYNC_INTERVAL);

    if (PTP_DELAY_MECHANISM_P2P == p_ctrl->delay[ptp_channel])
    {
        /* Peer to peer delay correction mechanism */
        HW_PTP_SYNFPDelayReqIntervalSet(p_ptp_reg, PTP_LOG_MIN_PDELAY_REQ_INTERVAL);
    }
    else
    {
        /* End to end delay correction mechanism */
        HW_PTP_SYNFPDelayReqIntervalSet(p_ptp_reg, PTP_LOG_MIN_DELAY_REQ_INTERVAL);
    }

    /* Set grandmasterPriority 1/2 */
    HW_PTP_GrandMasterPriority1Set(p_ptp_reg, PTP_GM_PRIORITY1);
    HW_PTP_GrandMasterPriority2Set(p_ptp_reg, PTP_GM_PRIORITY2);

    /* Set grandmasterClockQuality */
    HW_PTP_GMClockQuality(p_ptp_reg, PTP_GM_CLK_QUALITY);

    /* Set grandmasterIdentity */
    HW_PTP_GrandMasterIdentityFieldSet(p_ptp_reg, PTP_GM_CLK_ID_U, PTP_GM_CLK_ID_L);

    /* Set currentUtcOffset and timeSource */
    HW_PTP_SetUtcOffset(p_ptp_reg, (uint16_t)PTP_CURRENT_UTC_OFFSET);
    HW_PTP_SetTimeSource(p_ptp_reg, (uint8_t)PTP_TIME_SOURCE);

    /* Set steps removed field */
    HW_PTP_StepsRemovedSet(p_ptp_reg, PTP_STEPS_REMOVED);

    /* Set PTP message MAC address */
    /* PTP-primary messages: 01-1B-19-00-00-00 */
    HW_PTP_PrimaryMessageMACaddressSet(p_ptp_reg, PTP_PRIMARY_MESSAGE_MAC_ADDRESS, 0U);

    /* PTP-pdelay messages: 01-80-C2-00-00-0E */
    HW_PTP_pDelayMessageMACaddressSet(p_ptp_reg, PTP_PDELAY_MESSAGE_MAC_ADDRESS_UPPER, PTP_PDELAY_MESSAGE_MAC_ADDRESS_LOWER);

    /* Set PTP Ethernet type */
    HW_PTP_Ethertype(p_ptp_reg, PTP_ETHERNET_TYPE);

    /** Set PTP primary/pdelay message IP address setting */
    /* PTP-primary messages: E0-00-01-81 (224.0.1.129) */
    HW_PTP_PrimaryMessageIPaddressSet(p_ptp_reg, PTP_PRIMARY_MESSAGE_IP_ADDRESS);
    /* PTP-pdelay messages: E0-00-00-6B (224.0.0.107) */
    HW_PTP_pDelayMessageIPaddressSet(p_ptp_reg, PTP_PDELAY_MESSAGE_IP_ADDRESS);

    /* Set PTP message TOS and TTL fields */
    HW_PTP_EventMsgTOSField(p_ptp_reg, PTP_EVENT_TOS);
    HW_PTP_GeneralMsgTOSField(p_ptp_reg, PTP_GENERAL_TOS);
    HW_PTP_PrimaryMsgTTLField(p_ptp_reg, (uint8_t)PTP_PRIMARY_TTL);
    HW_PTP_pDelayMsgTTLField(p_ptp_reg, (uint8_t)PTP_PDELAY_TTL);

    /* Clear extended promiscuous mode */
    HW_PTP_ExtendedPromiscuousModeSet(p_ptp_reg, false);

    /* Set UDP port number */
    HW_PTP_UDPPortSet(p_ptp_reg, PTP_EVENT_MESSAGE_UDP_PORT, PTP_GENERAL_MESSAGE_UDP_PORT);

    /* Set asymmetric delay, prohibit but for 0 (always symmetric) */
    HW_PTP_SetAsymmetricDelay(p_ptp_reg, 0x0000, 0x00000000);

    /* Set timestamp latency of ingress and egress ports*/
    HW_PTP_SetTimestampLatency(p_ptp_reg, PTP_TIMESTAMP_LATENCY);

    /** Set frame format for generation of PTP message */
    HW_PTP_FrameFormat_ETH(p_ptp_reg, p_ctrl->frame_format[ptp_channel]);

    /* Initialize reception timeout of PTP messages, 1024 nsec unit */
    HW_PTP_ReceptionTimeout(p_ptp_reg, PTP_MESSAGE_RECEPTION_TIMEOUT);

    /* Validate set values to SYNFP */
    HW_PTP_SYNFPLoadDirectiveSet(p_ptp_reg, PTP_SYNFP_LOAD_REGISTER);

} /* End of function r_ptp_init_synfp() */

/*******************************************************************************************************************//**
 * @brief Initializes Packet relation controller(PRC) and Statistical time correction algorithm(STCA) units.
 *
 * @param[in] p_ctrl              Pointer to PTP instance block
 * @param[in] stca_mode           STCA operation mode
 * @param[in] ptp_channel         EPTPC channel
 *
 * @retval    void
 **********************************************************************************************************************/
static void r_ptp_init_prc_stca(ptp_instance_ctrl_t  * const p_ctrl, ptp_stca_mode_t stca_mode, uint8_t ptp_channel)
{
    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;
    R_EPTPC_CFG_Type * p_ptp_reg_cfg = (R_EPTPC_CFG_Type *) p_ctrl->p_reg_cfg;

    typedef union
    {
        uint32_t All ;
        /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
        struct
        {
            uint8_t b0 ;
            uint8_t b1 ;
            uint8_t b2 ;
            uint8_t b3 ;
        } bit ;
    } LDATA_t ;
    LDATA_t LDATA;

    /* Initialize packet relation controller unit (PRC-TC) */
    /* Set MAC address */
    LDATA.bit.b3 = 0U;
    LDATA.bit.b2 = p_ctrl->address[ptp_channel].mac_addr[0];
    LDATA.bit.b1 = p_ctrl->address[ptp_channel].mac_addr[1];
    LDATA.bit.b0 = p_ctrl->address[ptp_channel].mac_addr[2];
    uint32_t addr_uppr = LDATA.All;

    LDATA.bit.b3 = 0U;
    LDATA.bit.b2 = p_ctrl->address[ptp_channel].mac_addr[3];
    LDATA.bit.b1 = p_ctrl->address[ptp_channel].mac_addr[4];
    LDATA.bit.b0 = p_ctrl->address[ptp_channel].mac_addr[5];
    uint32_t addr_lwr = LDATA.All;
    HW_PTP_MACAddressSet(p_ptp_reg_gen, ptp_channel, addr_uppr, addr_lwr);

    /* Set PTP packets are transmitted to both ports */
    HW_PTP_PacketTransmissionControl(p_ptp_reg_gen, 0U);

    /** Set STCA operation mode */
    /* Mode 1 - Gradient collection is not applicable */
    if(PTP_STCA_MODE_1 == stca_mode)
    {
        HW_PTP_SyncCorrectionMode(p_ptp_reg_gen, PTP_STCA_CONFIG_MODE1);
    }
    /* Mode 2 - Gradient collection by hardware (Use STCA) */
    else if(PTP_STCA_MODE_2_HW == stca_mode)
    {
        HW_PTP_SyncCorrectionMode(p_ptp_reg_gen, PTP_STCA_CONFIG_MODE2_HW);
    }
    /* Mode 2 - Gradient collection by software (Use STCA) */
    else
    {
        HW_PTP_SyncCorrectionMode(p_ptp_reg_gen, PTP_STCA_CONFIG_MODE2_SW);
    }

    HW_PTP_SyncLossDetection(p_ptp_reg_gen, PTP_SYNC_LOSS_DETECTION_THRESHOLD, 0U);

    HW_PTP_SyncDetection(p_ptp_reg_gen, PTP_SYNC_DETECTION_THRESHOLD, 0U);

    /** Set STCA clock source as PCLKA and set frequency division to 6 */
    HW_PTP_STCAClockSelect(p_ptp_reg_cfg, PTP_STCA_CLOCK_DIVIDER);

    /* Set STCA clock frequency as 20 MHz */
    HW_PTP_STCAClockFrequencySet(p_ptp_reg_gen, 0);

    if (PTP_STATE_SLAVE == p_ctrl->state[ptp_channel])
    {
        HW_PTP_SyncMsgReceptionTimeout(p_ptp_reg_gen, PTP_SYNC_MSG_TIMEOUT);
    }
    else
    {
        HW_PTP_LocalClockSet(p_ptp_reg_gen, PTP_LCCLK_SEC_HIGH, (uint32_t)PTP_LCCLK_SEC_LOW, (uint32_t)PTP_LCCLK_NANO);
        HW_PTP_LoadSetValue(p_ptp_reg_gen, 1U);
    }

    /* Initialize MINT IRQ select registers */
    HW_PTP_IPLSIrqTimerSet(p_ptp_reg_gen, 0U);
    HW_PTP_MINTIrqTimerSet(p_ptp_reg_gen, 0U);
    HW_PTP_ELCIrqTimerSet(p_ptp_reg_gen, 0U);

    HW_PTP_SlaveTimeSyncControl(p_ptp_reg_gen, 0);

    /** Initialize pulse output timer m (m = 0 to 5) */
    for (uint8_t channel = 0U; channel <= 5U; channel++)
    {
        HW_PTP_TimerStartTimeSet(p_ptp_reg_gen, channel, 0U, 0U);
        HW_PTP_TimerCycleSet(p_ptp_reg_gen, channel, 0U);
        HW_PTP_TimerPulseWidthSet(p_ptp_reg_gen, channel, 0U);
    }

} /* End of function r_ptp_init_prc_stca() */

/*******************************************************************************************************************//**
 * @brief Gets PTP time information in scaled nanoseconds format.
 *
 * @param[in] reg_upper              Upper register value
 * @param[in] reg_lower              Lower register value
 * @param[in] p_info                 Pointer to PTP time information
 *
 * @retval    void
 **********************************************************************************************************************/
static void r_ptp_time_info_get(uint32_t reg_upper, uint32_t reg_lower, ptp_timeInterval_t * p_info)
{
    ptp_timeInterval_t max_p = {(int32_t)SCALED_TIME_MAX_U, (uint32_t)SCALED_TIME_MAX_L}; /* largest positive value */
    ptp_timeInterval_t min_n = {(int32_t)SCALED_TIME_MIN_U, (uint32_t)SCALED_TIME_MIN_L}; /* smallest minimum value */
    /* Check MSB if mean path delay is positive or negative */
    if (PTP_CHECK_MSB == (reg_upper & PTP_CHECK_MSB))
    {
        /* Register contains negative value */
        if (MIN16_S > reg_upper)
        {
            /* In case of under flow, change it to smallest minimum value */
            p_info->scaledNanoseconds_high = (int32_t)min_n.scaledNanoseconds_high;
            p_info->scaledNanoseconds_low = (uint32_t)min_n.scaledNanoseconds_low;
        }
        else
        {
            /* Conversion to scaled nanosecond expression */
            p_info->scaledNanoseconds_high = (int32_t)((reg_upper << 16U) | ((reg_lower >> 16U) & 0xFFFF));
            p_info->scaledNanoseconds_low = (uint32_t)(reg_lower << 16U);
        }
    }
    else
    {
        /* Register contains positive value */
        if (MAX16_S < reg_upper)
        {
            /* In case of over flow, change it to largest maximum value */
            p_info->scaledNanoseconds_high = (int32_t)max_p.scaledNanoseconds_high;
            p_info->scaledNanoseconds_low = (uint32_t)max_p.scaledNanoseconds_low;
        }
        else
        {
            /* Conversion to scaled nanosecond expression */
            p_info->scaledNanoseconds_high = (int32_t)((reg_upper << 16U) | ((reg_lower >> 16U) & 0xFFFF));
            p_info->scaledNanoseconds_low = (uint32_t)(reg_lower << 16U);
        }
    }
}/* End of function r_ptp_time_info_get() */

/*******************************************************************************************************************//**
 * @brief Get the base addresses of EPTPC
 *
 * @param[in] p_ctrl               Pointer to PTP instance block
 * @param[in] p_cfg                Pointer to configuration structure
 * @param[in] ssp_feature          Pointer to PTP feature
 *
 * @return                        See @ref Common_Error_Codes or functions called by this function for other possible
 *                                return codes. This function calls:
 *                                   * fmi_api_t::productFeatureGet
 **********************************************************************************************************************/
static ssp_err_t r_ptp_base_address_get(ptp_instance_ctrl_t * const p_ctrl, ptp_cfg_t const * const p_cfg, ssp_feature_t * const ssp_feature)
{
    ssp_err_t err = SSP_SUCCESS;

    ssp_feature->id         = SSP_IP_EPTPC;
    fmi_feature_info_t info = {0};
    ssp_feature->unit       = 1U;
    ssp_feature->channel    = 0U;

    err = g_fmi_on_fmi.productFeatureGet(ssp_feature, &info);
    PTP_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* Get the base address of R_EPTPC_CFG_Type register */
    p_ctrl->p_reg_cfg = info.ptr;

    ssp_feature->unit = 2U;
    err = g_fmi_on_fmi.productFeatureGet(ssp_feature, &info);
    PTP_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* Get the base address of R_EPTPC_GEN_Type register */
    p_ctrl->p_reg_gen = info.ptr;

    ssp_feature->unit = 3U;
    err = g_fmi_on_fmi.productFeatureGet(ssp_feature, &info);
    PTP_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* Get the base address of R_EPTPC0_Type register (R_EPTPC0) */
    p_ctrl->p_reg[0] = info.ptr;

    if(PTP_DEVICE_ORDINARY_CLOCK0 != p_cfg->device)
    {
        /* Get the base address of R_EPTPC0_Type register (R_EPTPC1) */
        ssp_feature->channel = 1U;
        ssp_feature->unit = 3U;
        err = g_fmi_on_fmi.productFeatureGet(ssp_feature, &info);
        PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
        p_ctrl->p_reg[1] = info.ptr;
    }

    return err;
}/* End of function r_ptp_base_address_get() */

/*******************************************************************************************************************//**
 * @brief Initializes EPTPC MINT interrupt.
 *
 * @param[in] p_ctrl            Pointer to PTP instance block
 * @param[in] p_feature         Pointer to PTP feature
 * @return                      See @ref Common_Error_Codes or functions called by this function for other possible
 *                                return codes. This function calls:
 *                                        * fmi_api_t::eventInfoGet
***********************************************************************************************************************/
static ssp_err_t r_ptp_event_configure(ptp_instance_ctrl_t * const p_ctrl, ssp_feature_t * const p_feature)
{
    ssp_err_t err = SSP_SUCCESS;

    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    ssp_vector_info_t * p_vector_info;
    err = g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_EPTPC_MINT, &event_info);
    p_ctrl->mint_irq = event_info.irq;
    /** If interrupt is registered in the vector table, disable interrupts, set priority, and store control block in
     * the vector information so it can be accessed from the callback. */
    if (SSP_SUCCESS == err)
    {
        NVIC_DisableIRQ(p_ctrl->mint_irq);
        R_BSP_IrqStatusClear(p_ctrl->mint_irq);
        NVIC_SetPriority(p_ctrl->mint_irq, p_ctrl->mint_irq);
        R_SSP_VectorInfoGet(p_ctrl->mint_irq, &p_vector_info);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }

    return err;
}/* End of function r_ptp_ip_mac_address_configure() */

/*******************************************************************************************************************//**
 * @brief Configure IP and MAC addresses.
 *
 * @param[in] p_ctrl                   Pointer to PTP instance block
 * @param[in] p_ip_address             Pointer to IP address
 * @param[in] p_physical_address_msw   Pointer to MAC address high bits
 * @param[in] p_physical_address_lsw   Pointer to MAC address low bits
 *
 * @retval    void
 **********************************************************************************************************************/
static void r_ptp_ip_mac_address_configure(ptp_instance_ctrl_t * const p_ctrl, uint32_t * p_ip_address,
        uint32_t * p_physical_address_msw,
        uint32_t * p_physical_address_lsw)
{
    /** Set IP and MAC address to control structure */
    for(uint8_t i = 0U; i < 2U; i++)
    {
        p_ctrl->address[i].mac_addr[0] = (uint8_t)((p_physical_address_msw[i] >> 8) & 0x00ff);
        p_ctrl->address[i].mac_addr[1] = (uint8_t)((p_physical_address_msw[i]) & 0x00ff);
        p_ctrl->address[i].mac_addr[2] = (uint8_t)((p_physical_address_lsw[i] >> 24) & 0x00ff);
        p_ctrl->address[i].mac_addr[3] = (uint8_t)((p_physical_address_lsw[i] >> 16) & 0x00ff);
        p_ctrl->address[i].mac_addr[4] = (uint8_t)((p_physical_address_lsw[i] >> 8) & 0x00ff);
        p_ctrl->address[i].mac_addr[5] = (uint8_t)((p_physical_address_lsw[i]) & 0x00ff);
        p_ctrl->address[i].ip_addr[0]  = (uint8_t)((p_ip_address[i] >> 24) & 0x00ff);
        p_ctrl->address[i].ip_addr[1]  = (uint8_t)((p_ip_address[i] >> 16) & 0x00ff);
        p_ctrl->address[i].ip_addr[2]  = (uint8_t)((p_ip_address[i] >> 8) & 0x00ff);
        p_ctrl->address[i].ip_addr[3]  = (uint8_t)((p_ip_address[i]) & 0x00ff);
    }

}/* End of function r_ptp_ip_mac_address_configure() */

/*******************************************************************************************************************//**
 * @brief Wait till SYNFP status register holds the same value as event before timeout.
 *
 * @param[in] p_ptp_reg              Pointer to R_EPTPC0_Type register
 * @param[in] event                  Expected register value
 * @param[in] wait_option            Timeout
 *
 * @retval SSP_SUCCESS                 PTP event operation performed successfully.
 * @retval SSP_ERR_TIMEOUT             PTP event not completed before timeout.
 **********************************************************************************************************************/
static ssp_err_t r_ptp_synfp_wait(R_EPTPC0_Type * p_ptp_reg, uint32_t event, uint32_t wait_option)
{
    do
    {
        /** Wait till SYNFP status register holds the same value as event */
        if((HW_PTP_SYNFPStatusGet(p_ptp_reg) & event) == event)
        {
            return SSP_SUCCESS;
        }
        if(0x00U < wait_option)
        {
            R_BSP_SoftwareDelay(10U, BSP_DELAY_UNITS_MICROSECONDS);
            wait_option--;
        }
    }while (0x00U != wait_option);

    return SSP_ERR_TIMEOUT;
} /* End of function r_ptp_synfp_wait() */

/*******************************************************************************************************************//**
 * @brief Waits for the PTP event to complete before timeout.
 *
 * @param[in] p_reg                  EPTPC register
 * @param[in] event                  PTP Event
 * @param[in] wait_option            Timeout
 *
 * @retval SSP_SUCCESS               PTP event operation performed successfully.
 * @retval SSP_ERR_TIMEOUT           PTP event not completed before timeout
 **********************************************************************************************************************/
static ssp_err_t r_ptp_wait(volatile uint32_t * p_reg, uint32_t event, uint32_t wait_option)
{
    volatile uint32_t tmp;

    do
    {
        /** Wait till p_reg holds the same value as event */
        tmp = *p_reg;
        if ((tmp & event) == event)
        {
            *p_reg = tmp;
            return SSP_SUCCESS;
        }

        if(0x00U < wait_option)
        {
            R_BSP_SoftwareDelay(10U, BSP_DELAY_UNITS_MICROSECONDS);
            wait_option--;
        }
    } while(0x00U != wait_option);

    return SSP_ERR_TIMEOUT;
} /* End of function r_ptp_wait() */

/*******************************************************************************************************************//**
 * @brief Checks clock information.
 *
 * @param[in] p_ptp_reg_gen              Pointer to R_EPTPC_GEN_type register
 * @param[in] wait_option                Timeout
 *
 * @retval SSP_SUCCESS                   Clock information captured successfully.
 * @retval SSP_ERR_TIMEOUT               Clock information not acquired before timeout.
 **********************************************************************************************************************/
static ssp_err_t r_ptp_info_check(R_EPTPC_GEN_Type * p_ptp_reg_gen, uint32_t wait_option)
{
    uint32_t info;
    /** Get current clock information */
    HW_PTP_InfoGet(p_ptp_reg_gen, &info);
    do
    {
        /** Wait till information fetching is completed */
        if (0U == (info & 1U))
        {
            return SSP_SUCCESS;
        }

        if(0x00U < wait_option)
        {
            R_BSP_SoftwareDelay(10U, BSP_DELAY_UNITS_MICROSECONDS);
            wait_option--;
        }
    } while(0x00U != wait_option);

    return SSP_ERR_TIMEOUT;
} /* End of function r_ptp_info_check() */

/*******************************************************************************************************************//**
 * @brief Starts the time synchronization when clock mode is configured as transparent clock
 *
 * @param[in] p_ctrl              Pointer to PTP instance block
 * @param[in] ptp_channel         EPTPC channel
 *
 * @retval SSP_SUCCESS            Transparent clock started successfully.
 **********************************************************************************************************************/
static void r_ptp_start_transparent_clock (ptp_instance_ctrl_t * const p_ctrl, uint8_t ptp_channel)
{

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    if (PTP_DELAY_MECHANISM_P2P == p_ctrl->delay[0])
    {
        HW_PTP_SYNFPTransmissionEnableSet(p_ptp_reg, PTP_SYNFP_TRANS_ENABLE_P2P);
    }
    else
    {
        HW_PTP_SYNFPTransmissionEnableSet(p_ptp_reg, 0U);
    }

    /* Update SYNFP state */
    HW_PTP_SYNFPStateUpdate(p_ptp_reg, 1U);

}/* End of function r_ptp_start_transparent_clock() */

/*******************************************************************************************************************//**
 * @brief Starts the time synchronization when clock mode is configured as ordinary or boundary clock
 *
 * @param[in] p_ctrl              Pointer to PTP instance block
 * @param[in] ptp_channel         EPTPC channel
 * @param[in] wait_option         Timeout
 *
 * @retval SSP_SUCCESS            Configured clock mode started successfully.
 * @retval SSP_ERR_INVALID_MODE   Invalid PTP mode.
 **********************************************************************************************************************/
static ssp_err_t r_ptp_start_clock_mode (ptp_instance_ctrl_t * const p_ctrl, uint8_t ptp_channel, uint32_t wait_option)
{
    ssp_err_t err = SSP_SUCCESS;

    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    if (PTP_DELAY_MECHANISM_P2P == p_ctrl->delay[ptp_channel])
    {
        if (PTP_STATE_MASTER == p_ctrl->state[ptp_channel])
        {
            HW_PTP_SYNFPTransmissionEnableSet(p_ptp_reg, PTP_SYNFP_TRANS_ENABLE_P2P_MASTER);
        }
        else if (PTP_STATE_SLAVE == p_ctrl->state[ptp_channel])
        {
            HW_PTP_SYNFPTransmissionEnableSet(p_ptp_reg, PTP_SYNFP_TRANS_ENABLE_P2P);
            /** Check if offset from master value update flag is set */
            err = r_ptp_synfp_wait(p_ptp_reg, (uint32_t)PTP_SYSR_STATUS_SLAVE_START, wait_option);
            PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
        }
        else if (PTP_STATE_LISTENING == p_ctrl->state[ptp_channel])
        {
            err = SSP_ERR_INVALID_MODE;
        }
    }
    else
    {
        if (PTP_STATE_MASTER == p_ctrl->state[ptp_channel])
        {
            HW_PTP_SYNFPTransmissionEnableSet(p_ptp_reg, PTP_SYNFP_TRANS_ENABLE_E2E_MASTER);
        }
        else if (PTP_STATE_SLAVE == p_ctrl->state[ptp_channel])
        {
            HW_PTP_SYNFPTransmissionEnableSet(p_ptp_reg, PTP_SYNFP_TRANS_ENABLE_E2E_SLAVE);
            /** Check if offset from master value update flag is set */
            err = r_ptp_synfp_wait(p_ptp_reg, (uint32_t)PTP_SYSR_STATUS_SLAVE_START, wait_option);
            PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
        }
        else if (PTP_STATE_LISTENING == p_ctrl->state[ptp_channel])
        {
            err = SSP_ERR_INVALID_MODE;
        }
    }
    /* Update SYNFP state */
    HW_PTP_SYNFPStateUpdate(p_ptp_reg, 1U);

    return err;
}/* End of function r_ptp_start_clock_mode() */

/*******************************************************************************************************************//**
 * @brief Stops the time synchronization when clock mode is configured as ordinary or boundary clock
 *
 * @param[in] p_ctrl              Pointer to PTP instance block
 * @param[in] ptp_channel         EPTPC channel
 * @param[in] wait_option         Timeout
 *
 * @retval SSP_SUCCESS            Configured clock mode stopped successfully.
 * @retval SSP_ERR_INVALID_MODE   Invalid PTP mode.
 **********************************************************************************************************************/
static ssp_err_t r_ptp_stop_clock_mode (ptp_instance_ctrl_t * const p_ctrl, uint8_t ptp_channel, uint32_t wait_option)
{
    ssp_err_t err = SSP_SUCCESS;

    R_EPTPC_GEN_Type * p_ptp_reg_gen = (R_EPTPC_GEN_Type *) p_ctrl->p_reg_gen;
    R_EPTPC0_Type * p_ptp_reg = (R_EPTPC0_Type *) p_ctrl->p_reg[ptp_channel];

    if (PTP_DELAY_MECHANISM_P2P == p_ctrl->delay[ptp_channel])
    {
        if (PTP_STATE_MASTER == p_ctrl->state[ptp_channel])
        {
            HW_PTP_SYNFPTransmissionEnableSet(p_ptp_reg, PTP_SYNFP_TRANS_ENABLE_P2P);
            HW_PTP_SYNFPStateUpdate(p_ptp_reg, 1U);
            HW_PTP_LoadSetValue(p_ptp_reg_gen, 0U);

            /** Check if Response and generation stop completion detection flag are set */
            err = r_ptp_synfp_wait(p_ptp_reg, (uint32_t)PTP_SYSR_STATUS_P2P_MASTER_STOP, wait_option);
            PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
        }
        else if (PTP_STATE_SLAVE == p_ctrl->state[ptp_channel])
        {
            HW_PTP_SlaveTimeSyncControl(p_ptp_reg_gen, 0U);
            HW_PTP_SYNFPReceptionFilter1Set(p_ptp_reg, PTP_SYNFP_FILTER1_P2P_SLAVE_STOP);
            HW_PTP_SYNFPStateUpdate(p_ptp_reg, 1U);
        }
        else
        {
            return SSP_ERR_INVALID_MODE;
        }

        HW_PTP_SYNFPReceptionFilter1Set(p_ptp_reg, 1U);
        HW_PTP_SYNFPTransmissionEnableSet(p_ptp_reg, 0U);
        HW_PTP_SYNFPStateUpdate(p_ptp_reg, 1U);

        /** Check if Generation stop completion detection flag is set */
        err = r_ptp_synfp_wait(p_ptp_reg, (uint32_t)PTP_SYSR_STATUS_P2P_SLAVE_STOP, wait_option);
        PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
    }
    else
    {
        if (PTP_STATE_MASTER == p_ctrl->state[ptp_channel])
        {
            HW_PTP_SYNFPReceptionFilter1Set(p_ptp_reg, PTP_SYNFP_FILTER1_E2E_STOP);
            HW_PTP_SYNFPTransmissionEnableSet(p_ptp_reg, 0U);
            HW_PTP_SYNFPStateUpdate(p_ptp_reg, 1U);
            HW_PTP_LoadSetValue(p_ptp_reg_gen, 0U);

            /** Check if Response and generation stop completion detection flag are set */
            err = r_ptp_synfp_wait(p_ptp_reg, (uint32_t)PTP_SYSR_STATUS_E2E_MASTER_STOP, wait_option);
            PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
        }
        else if (PTP_STATE_SLAVE == p_ctrl->state[ptp_channel])
        {
            HW_PTP_SlaveTimeSyncControl(p_ptp_reg_gen, 0U);
            HW_PTP_SYNFPReceptionFilter1Set(p_ptp_reg, PTP_SYNFP_FILTER1_E2E_STOP);
            HW_PTP_SYNFPTransmissionEnableSet(p_ptp_reg, 0U);
            HW_PTP_SYNFPStateUpdate(p_ptp_reg, 1U);

            /** Check if Generation stop completion detection flag is set */
            err = r_ptp_synfp_wait(p_ptp_reg, (uint32_t)PTP_SYSR_STATUS_E2E_SLAVE_STOP, wait_option);
            PTP_ERROR_RETURN(SSP_SUCCESS == err, err);
        }
        else
        {
            err = SSP_ERR_INVALID_MODE;
        }
    }
    return err;
}/* End of function r_ptp_stop_clock_mode() */

/*******************************************************************************************************************//**
 * @brief PTP MINT interrupt handler
 *
 *  Saves context if RTOS is used, clears the interrupt flag, calls callback if one was provided in the open function,
 *  and restores context if RTOS is used.
 ***********************************************************************************************************************/
void eptpc_mint_isr(void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    ptp_instance_ctrl_t * p_ctrl = (ptp_instance_ctrl_t *)*(p_vector_info->pp_ctrl);

    ptp_callback_args_t args;

    /** Clear pending IRQ to make sure it doesn't fire again after exiting */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    uint32_t miesr;
    uint32_t sysr;
    uint8_t interrupt_ch;

    miesr = HW_PTP_MINTInterruptStatusGet(p_ctrl->p_reg_gen);

    /** Invoke the callback function if it is set. */
    if (NULL != p_ctrl->p_callback)
    {
        interrupt_ch = (uint8_t)((miesr >> 16) & 0x3F);
        if(0U != interrupt_ch)
        {
            args.timer_channel = (ptp_stca_timer_channel_t)interrupt_ch;
            args.event = PTP_EVENT_TIMER;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
        /** Setup parameters for the user-supplied callback function. */
        if ((miesr & 0x0001U) != 0x0000U)
        {
            args.event = PTP_EVENT_STCA;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }

        if ((miesr & 0x0002U) != 0x0000U)
        {
            sysr = HW_PTP_SYNFPStatusGet(p_ctrl->p_reg[0]);
            if ((sysr & 0x4000) == 0x4000)
            {
                p_ctrl->infabt_flag[0] = true;
            }
            args.event = PTP_EVENT_SYNFP0;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }

        if ((miesr & 0x0004U) != 0x0000U)
        {
            sysr = HW_PTP_SYNFPStatusGet(p_ctrl->p_reg[1]);
            if ((sysr & 0x4000) == 0x4000)
            {
                p_ctrl->infabt_flag[1] = true;
            }
            args.event = PTP_EVENT_SYNFP1;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }

        if ((miesr & 0x0008U) != 0x0000U)
        {
            args.event = PTP_EVENT_PRCTC;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
    }

    HW_PTP_MINTInterruptStatusSet(p_ctrl->p_reg_gen, miesr);

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
} /* End of function eptpc_mint_isr() */
