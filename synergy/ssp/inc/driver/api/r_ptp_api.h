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
 * File Name    : r_ptp_api.h
 * Description  : Interface file for PTP HAL API
 **********************************************************************************************************************/

#ifndef R_PTP_API_H
#define R_PTP_API_H

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup PTP_API PTP driver Interface
 *
 * @brief Interface for PTP functions.
 *
 * @section PTP_API_SUMMARY Summary
 * The PTP interface provides time synchronization functionality.
 *
 * The PTP interface can be implemented by:
 * - @ref PTP
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * PTP Interface description: @ref HALPTPInterface
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/* Includes board and MCU related header files. */
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/* Version Number of API. */
#define PTP_API_VERSION_MAJOR (2U)
#define PTP_API_VERSION_MINOR (0U)

/* Sync message reception timeout value */
/* No timeout detection */
#define PTP_SYNC_MSG_TIMEOUT   (0x00000000) /* 1024 nanoseconds unit */

/* Local clock counter initial value */
/* PTP time: Annex B.3 */
#define PTP_CURRENT_UTC_OFFSET (8U) /* approx. 1588 spec AnnexB B.2 */
#define PTP_NTP_OFFSET         (2208988800U) /* 2,208,988,800 seconds */
#define PTP_NTP_SEC            (3668572800U) /* NTP(Network Time Protocol) second, 2016/03/31 08:00:00, 3,668,572,800 seconds */
#define PTP_SEC                ((PTP_NTP_SEC - PTP_NTP_OFFSET) + PTP_CURRENT_UTC_OFFSET) /* PTP second */

#define PTP_LCCLK_SEC_HIGH     (0x0000U)     /* seconds order high */
#define PTP_LCCLK_SEC_LOW      (PTP_SEC)    /* seconds order low  */
#define PTP_LCCLK_NANO         (0x12345678U) /* nanoseconds order (305,419,896 nanoseconds) */

/* RMII timestamp latency setting */
#define PTP_TIMESTAMP_LATENCY (0x04240302U) /* RMII, 100Mbps and STCA = 20MHz */

/* PTP data sets and messages related parameter */
/* LLC-CTL filed of IEEE802.3 format */
#define PTP_LLC_CTL           (3U)

/* PTP version field */
#define PTP_VER_NUM           (0x02U) /* IEEE1588-2008 (version2) */

/* domainNumber field */
#define PTP_DOMAIN_NUM        (0U) /* 0: Default domain, 1 to 3: Alternate domain, 4 to 127: User defined */

/* Announce message's flag field */
/* PTP profile Specific 2(b14) = false, PTP profile Specific 1(b13) = false,
unicastFlag(b10) = false, alternateMasterFlag(b8) = false,
frequencyTraceable(b5) = false, timeTraceable(b4) = false, ptpTimescale(b3) = false,
currentUtcOffsetValid(b2) = false, leap59(b1) = false, leap61(b0) = false */
#define PTP_ANNOUNCE_FLAG_FIELD (0x0000U)

/* Sync message's flag field */
/* PTP profile Specific 2(b14) = false, PTP profile Specific 1(b13) = false,
unicastFlag(b10) = false, twoStepFlag(b9) = false , alternateMasterFlag(b8) = false */
#define PTP_SYNC_FLAG_FIELD (0x0000U)

/* Delay_Req/Pdelay_Req message's flag field */
/* PTP profile Specific 2(b14) = false, PTP profile Specific 1(b13) = false,
unicastFlag(b10) = false */
#define PTP_DELAY_REQ_FLAG_FIELD (0x0000U)

/* Delay_Resp/Pdelay_Resp message's flag field */
/* PTP profile Specific 2(b14) = false, PTP profile Specific 1(b13) = false,
unicastFlag(b10) = false,
twoStepFlag(b9) = false */
#define PTP_DELAY_RESP_FLAG_FIELD (0x0000U)

/* Master clock-ID: In general, equal to parentDS.parentPortIdentity.clockIdentity field */
#define PTP_MTCID_U (0x00000000U) /* Clock-ID high */
#define PTP_MTCID_L (0x00000000U) /* Clock-ID low */

/* Master port number: In general, equal to parentDS.parentPortIdentity.portNumber field */
#define PTP_MTPID (0x0000U)

/* grandmasterPriority1: Equal to parentDS.grandmasterPriority1 field */
/* From 0 to 255 can be set and lower value has higher priority */
#define PTP_GM_PRIORITY1 (0x00U)

/* grandmasterPriority2: Equal to parentDS.grandmasterPriority2 field */
/* From 0 to 255 can be set and lower value has higher priority */
#define PTP_GM_PRIORITY2 (0x00U)

/* grandmasterClockQuality: Equal to parentDS.grandmasterClockQuality field */
/* b31 to b24: clockClass, default value(=248, = 0xF8), 255 is slave only clock */
/* b23 to b16: clockAccuracy, default value(=0x21)is within 100 nsec, 0x20 to 0x31 */
/* b15 to b0: offsetScaledLogVariance, default value(=0xFFFF) is not calculated yet */
#define PTP_GM_CLK_QUALITY (0xF821FFFFU)
/* grandmasterIdentity: Equal to parentDS.grandmasterIdentity field */
#define PTP_GM_CLK_ID_U (0x00000000U) /* Clock-ID high*/
#define PTP_GM_CLK_ID_L (0x00000000U) /* Clock-ID low */

/* timeSource: Equal to timePropertiesDS.timeSource field */
#define PTP_TIME_SOURCE (0xA0U) /* Time source is internal oscillator */

/* stepsRemoved: Equal to currentDS.stepsRemoved field */
#define PTP_STEPS_REMOVED (0x0000U) /* No traversed */

/* PTP event message's TOS field */
#define PTP_EVENT_TOS (0x00U) /* 0: Best effort */

/* PTP general message's TOS field */
#define PTP_GENERAL_TOS (0x00U) /* 0: Best effort */

/* PTP-primary message's TTL field */
#define PTP_PRIMARY_TTL (0x80U) /* 128*/

/* PTP-pdelay message's TTL field */
#define PTP_PDELAY_TTL (0x01U) /* 1 */

/* PTP message transmission interval   */
#define PTP_LOG_ANNOUNCE_INTERVAL              (0x01U) /* 0x01: 2sec */
#define PTP_LOG_SYNC_INTERVAL                  (0x00U) /* 0x00: 1sec */
#define PTP_LOG_MIN_DELAY_REQ_INTERVAL         (0x00U) /* 0x00: 1sec */
#define PTP_LOG_MIN_PDELAY_REQ_INTERVAL        (0x00U) /* 0x00: 1sec */

/* PTP message transmission interval limit  */
#define PTP_MINIMUM_TRANSMISSION_INTERVAL      (-0x07)
#define PTP_MAXIMUM_TRANSMISSION_INTERVAL      (0x07)

#define PTP_MASK_BIT14                         (0x4000)
#define PTP_MASK_BIT13                         (0x2000)
#define PTP_MASK_BIT10                         (0x0400)
#define PTP_MASK_BIT8                          (0x0100)
#define PTP_MASK_BIT5                          (0x0020)
#define PTP_MASK_BIT4                          (0x0010)
#define PTP_MASK_BIT3                          (0x0008)
#define PTP_MASK_BIT2                          (0x0004)
#define PTP_MASK_BIT1                          (0x0002)
#define PTP_MASK_BIT0                          (0x0001)

#define PTP_TIMER_EDGE_FALLING                 (0x08U)

#define PTP_ENABLE_SYNFP0_INTERRUPT            (0x0002U)
#define PTP_ENABLE_SYNFP1_INTERRUPT            (0x0004U)
#define PTP_ENABLE_INFABT_NOTIFICATION         (0x4000U)

#define PTP_SYNFP_OC_BC_FILTER2_SET            (0x00000011U)
#define PTP_SYNFP_TC_FILTER2_SET               (0x20000033U)
#define PTP_SYNFP_OPERATION_SET                (0x00000028U)
#define PTP_SYNFP_TC_OPERATION_SET             (0x00100028U)
#define PTP_SYNFP_LOAD_REGISTER                (0x00000007U)
#define PTP_PRIMARY_MESSAGE_MAC_ADDRESS        (0x00011B19U)
#define PTP_PDELAY_MESSAGE_MAC_ADDRESS_UPPER   (0x000180C2U)
#define PTP_PDELAY_MESSAGE_MAC_ADDRESS_LOWER   (0x0000000EU)
#define PTP_PRIMARY_MESSAGE_IP_ADDRESS         (0xE0000181U)
#define PTP_PDELAY_MESSAGE_IP_ADDRESS          (0xE000006BU)
#define PTP_ETHERNET_TYPE                      (0x000088F7U)
#define PTP_EVENT_MESSAGE_UDP_PORT             (0x0000013FU)
#define PTP_GENERAL_MESSAGE_UDP_PORT           (0x00000140U)
#define PTP_MESSAGE_RECEPTION_TIMEOUT          (0x00200000U)
#define PTP_MINT_IRQ_STATUS                    (0x003F000FU)
#define PTP_STCA_STATUS                        (0x0000001BU)
#define PTP_PRC_TC_STATUS                      (0x3000010FU)
#define PTP_SYNFP_STATUS                       (0x00035277U)
#define PTP_STCA_W10_STATUS                    (0x00000010U)
#define PTP_STCA_CLOCK_DIVIDER                 (0x00000006U)
#define PTP_SYNC_LOSS_DETECTION_THRESHOLD      (0x00000001U)     // H'0000_0001_0000_0000 nsec, approx 4 sec
#define PTP_SYNC_DETECTION_THRESHOLD           (0x00000002U)     // H'0000_0002_0000_0000 nsec, approx 8 sec
#define PTP_CHECK_MSB                          (0x80000000U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** PTP Clock type definitions */
typedef enum 
{
    PTP_DEVICE_DISABLED        = 0xFFU,    ///< Unsupported
    PTP_DEVICE_ORDINARY_CLOCK0 = 0U,       ///< Ordinary Clock Port 0
    PTP_DEVICE_ORDINARY_CLOCK1,            ///< Ordinary Clock Port 1
    PTP_DEVICE_BOUNDARY_CLOCK,             ///< Boundary Clock
    PTP_DEVICE_TRANSPARENT_CLOCK,          ///< Transparent Clock
} ptp_device_t;

/** PTP Delay correction mechanism definitions */
typedef enum 
{
    PTP_DELAY_MECHANISM_DISABLED   = 0xFFU,       ///< Unsupported
    PTP_DELAY_MECHANISM_P2P        = 0U,          ///< Peer to peer delay mechanism
    PTP_DELAY_MECHANISM_E2E,                      ///< End to end delay mechanism
} ptp_delay_mechanism_t;

/** PTP clock state definitions*/
typedef enum 
{
    PTP_STATE_DISABLED = 0xFFU,          ///< Unsupported
    PTP_STATE_MASTER   = 0U,             ///< Master state
    PTP_STATE_SLAVE,                     ///< Slave state
    PTP_STATE_LISTENING                  ///< Listening state
} ptp_state_t;

/** STCA mode and gradient setting definitions */
typedef enum 
{
    PTP_STCA_MODE_1    = 0x00U,         ///< Mode1 (not use STCA)
    PTP_STCA_MODE_2_HW = 0x02U,         ///< Mode2 (use STCA) and HW gradient setting
    PTP_STCA_MODE_2_SW = 0x03U          ///< Mode2 (use STCA) and SW gradient setting
} ptp_stca_mode_t;

/** PTP message frame format definitions */
typedef enum
{
    PTP_FRAME_FORMAT_DISABLED     = 0xFFU,      ///< Unsupported
    PTP_FRAME_FORMAT_ETH          = 0x00U,      ///< Ethernet II frame format
    PTP_FRAME_FORMAT_ETH_8023,                  ///< Ethernet 802.3 frame format
    PTP_FRAME_FORMAT_UDP4,                      ///< Ethernet II over UDP4 frame format
    PTP_FRAME_FORMAT_UDP4_8023                  ///< Ethernet 802.3 over UDP4 frame format
} ptp_frame_format_t;

/** STCA pulse output timer channel definitions */
typedef enum
{
    PTP_STCA_TIMER_CHANNEL_0 = 0x01U,     ///< STCA pulse output timer 0
    PTP_STCA_TIMER_CHANNEL_1 = 0x02U,     ///< STCA pulse output timer 1
    PTP_STCA_TIMER_CHANNEL_2 = 0x04U,     ///< STCA pulse output timer 2
    PTP_STCA_TIMER_CHANNEL_3 = 0x08U,     ///< STCA pulse output timer 3
    PTP_STCA_TIMER_CHANNEL_4 = 0x10U,     ///< STCA pulse output timer 4
    PTP_STCA_TIMER_CHANNEL_5 = 0x20U      ///< STCA pulse output timer 5
}ptp_stca_timer_channel_t;

/** STCA pulse output timer edge definitions */
typedef enum
{
    PTP_STCA_TIMER_PULSE_EDGE_RISING = 0U,      ///< Rising edge
    PTP_STCA_TIMER_PULSE_EDGE_FALLING           ///< Falling edge
}ptp_stca_timer_pulse_edge_t;

/** MINT interrupt register definitions */
typedef enum
{
    PTP_EVENT_TIMER = 0U, ///< Interrupt from Timer
    PTP_EVENT_STCA,       ///< Interrupt from STCA
    PTP_EVENT_PRCTC,      ///< Interrupt from PRC-TC
    PTP_EVENT_SYNFP0,     ///< Interrupt from SYNFP0
    PTP_EVENT_SYNFP1,     ///< Interrupt from SYNFP1
} ptp_event_t;

/** PTP data type structure */
/* 64 bit unsigned integer */
typedef struct
{
    uint32_t high;
    uint32_t low;
} UInt64_t;

/* 48 bit unsigned integer */
typedef struct
{
    uint16_t high;
    uint32_t low;
} UInt48_t;

/* 64 bit scaled nanosecond unit expression */
typedef struct
{
    int32_t scaledNanoseconds_high;
    uint32_t scaledNanoseconds_low;
} ptp_timeInterval_t;

/** PTP message timestamp structure */
typedef struct
{
    UInt48_t secondsField;
    uint32_t nanosecondsField;
} ptp_timestamp_t;

/** PTP channel related structure (MAC address, IP address) */
typedef struct 
{
    uint8_t mac_addr[6];
    uint8_t ip_addr[4];
} ptp_address_t;

/** Announce flagField type structure */
typedef union
{
    uint32_t LONG;
    struct {
        uint32_t Rsv1:17;
        uint32_t profileSpec2:1;
        uint32_t profileSpec1:1;
        uint32_t Rsv2:2;
        uint32_t unicastFlag:1;
        uint32_t Rsv3:1;
        uint32_t alternateMasterFlag:1;
        uint32_t Rsv4:2;
        uint32_t frequencyTraceable:1;
        uint32_t timeTraceable:1;
        uint32_t ptpTimescale:1;
        uint32_t currentUtcOffsetValid:1;
        uint32_t leap59:1;
        uint32_t leap61:1;
    } bit;
} ptp_announce_flag_t;

/** PTP clock quality structure */
typedef struct 
{
    uint8_t clockClass;
    uint8_t clockAccuracy;
    uint16_t offsetScaledLogVariance;
} ptp_clock_quality_t;

/** Announce message field type structure */
typedef struct
{
    uint8_t             grandmasterPriority1;
    uint8_t             grandmasterPriority2;
    ptp_clock_quality_t grandmasterClockQuality;
    int8_t            * p_grandmasterIdentity;
} ptp_announce_message_t;

/** PTP message reception configuration field structure */
typedef struct
{
    uint32_t reception_filter1;
    uint32_t reception_filter2;
    uint32_t transmission_enable;
    uint32_t operation_control;
} ptp_message_reception_t;

/** PTP callback arguments definition */
typedef struct st_ptp_callback_arg
{
    void const                  * p_context;     ///< Context provided to user during callback
    ptp_event_t                   event;         ///< The event can be used to identify what caused the callback
    ptp_stca_timer_channel_t      timer_channel; ///< STCA pulse output timer channel
} ptp_callback_args_t;

/** PTP user configuration structure */
typedef struct st_ptp_cfg
{
    /* PTP MINT interrupt callback configuration */
    ssp_err_t (* p_callback)(ptp_callback_args_t * p_args); ///< Pointer to interrupt callback function
    void const * p_context;                                 ///< User defined context passed into callback function
    uint8_t      irq_ipl;                                   ///< MINT interrupt IRQ number

    /* PTP generic configuration */
    ptp_device_t                device;               ///< PTP clock type
    ptp_state_t                 state[2];             ///< PTP clock state
    ptp_delay_mechanism_t       delay[2];             ///< Delay correction mechanism
    ptp_frame_format_t          frame_format[2];      ///< PTP message frame format
    ptp_stca_mode_t             stca_sync_mode;       ///< STCA synchronous mode
    void const                * p_extend;             ///< Extension parameter for hardware specific settings
} ptp_cfg_t;

/** PTP control block.  Allocate an instance specific control block to pass into the PTP API calls.
 * @par Implemented as
 * - ptp_instance_ctrl_t
 */
typedef void ptp_ctrl_t;

/** PTP functions implemented at the HAL layer follow this API. */
typedef struct st_ptp_api
{
    /** Open the PTP driver module.
     * @par Implemented as
     * - R_PTP_Open()
     *
     * @param[in] p_ctrl                     Pointer to the control structure
     * @param[in] p_cfg                      Pointer to a configuration structure
     **/
    ssp_err_t (* open)(ptp_ctrl_t * const p_ctrl,
            ptp_cfg_t const * const p_cfg);

    /** Close the PTP driver module.
     * @par Implemented as
     * - R_PTP_Close()
     *
     * @param[in] p_ctrl                     Pointer to the control structure
     **/
    ssp_err_t (* close)(ptp_ctrl_t * const p_ctrl);

    /** Configures the PTP driver module.
     * @par Implemented as
     * - R_PTP_Configure()
     *
     * @param[in] p_ctrl                         Pointer to the control structure
     * @param[in] p_ip_address                   Pointer to the IP address
     * @param[in] p_physical_address_msw         Pointer to the higher bits of physical address
     * @param[in] p_physical_address_lsw         Pointer to the lower bits of physical address
     **/
    ssp_err_t (* configure)(ptp_ctrl_t * const p_ctrl,
            uint32_t * p_ip_address,
            uint32_t * p_physical_address_msw,
            uint32_t * p_physical_address_lsw);

    /** Sets or clears the extended promiscuous mode
     * @par Implemented as
     * - R_PTP_SetExtPromiscuous()
     *
     * @param[in] p_ctrl                  Pointer to the control structure
     * @param[in] ptp_channel             EPTPC channel
     * @param[in] is_set                  Set/clear extended promiscuous mode
     **/
    ssp_err_t (* setExtPromiscuous)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, bool is_set);

    /** Sets local clock counter
     * @par Implemented as
     * - R_PTP_SetLocalClock()
     *
     * @param[in] p_ctrl                   Pointer to the control structure
     * @param[in] p_clock                  Pointer to local clock counter
     **/
    ssp_err_t (* setLocalClock)(ptp_ctrl_t * const p_ctrl, ptp_timestamp_t * p_clock);

    /** Gets local clock counter
     * @par Implemented as
     * - R_PTP_GetLocalClock()
     *
     * @param[in] p_ctrl                   Pointer to the control structure
     * @param[in] wait_option              Time out interval
     * @param[out] p_clock                 Pointer to local clock counter
     **/
    ssp_err_t (* getLocalClock)(ptp_ctrl_t * const p_ctrl, ptp_timestamp_t * p_clock, uint32_t wait_option);

    /** Gets master port ID
     * @par Implemented as
     * - R_PTP_GetMasterPortID()
     *
     * @param[in] p_ctrl                  Pointer to the control structure
     * @param[in] ptp_channel             EPTPC channel
     * @param[out] p_clock                Pointer to local clock counter
     * @param[out] p_port                 Pointer to master port
     **/
    ssp_err_t (* getMasterPortID)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, uint32_t * p_clock, uint16_t * p_port);

    /** Sets master port ID
     * @par Implemented as
     * - R_PTP_SetMasterPortID()
     *
     * @param[in] p_ctrl                  Pointer to the control structure
     * @param[in] ptp_channel             EPTPC channel
     * @param[in] p_clock                 Pointer to local clock counter
     * @param[in] p_port                  Pointer to master port
     **/
    ssp_err_t (* setMasterPortID)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, uint32_t * p_clock, uint16_t * p_port);

    /**  Get current offsetFromMaster and meanPathDelay.
     * @par Implemented as
     * - R_PTP_GetSyncInfo()
     *
     * @param[in] p_ctrl                  Pointer to the control structure
     * @param[in] ptp_channel             EPTPC channel
     * @param[out] p_master_offset        Returns the offset from master
     * @param[out] p_path_delay           Returns the mean path delay
     **/
    ssp_err_t (* getSyncInfo)(ptp_ctrl_t  * const p_ctrl, uint8_t ptp_channel, ptp_timeInterval_t * p_master_offset, ptp_timeInterval_t * p_path_delay);

    /** Starts the time synchronization.
     * @par Implemented as
     * - R_PTP_Start()
     *
     * @param[in] p_ctrl                  Pointer to the control structure
     * @param[in] wait_option             Timeout interval
     **/
    ssp_err_t (* start)(ptp_ctrl_t * const p_ctrl, uint32_t wait_option);

    /** Stops the time synchronization.
     * @par Implemented as
     * - R_PTP_Stop()
     *
     * @param[in] p_ctrl                  Pointer to the control structure
     * @param[in] wait_option             Timeout interval
     **/
    ssp_err_t (* stop)(ptp_ctrl_t * const p_ctrl, uint32_t wait_option);

    /** Sets the time interval for collecting worst 10 values
     * @par Implemented as
     * - R_PTP_SetWorst10Values()
     *
     * @param[in] p_ctrl                  Pointer to the control structure
     * @param[in] interval                Time interval to collect worst 10 values
     *
     **/
    ssp_err_t (* setWorst10Values)(ptp_ctrl_t * const p_ctrl, uint8_t interval);

    /** Checks worst 10 values by hardware and set as gradient limits.
     * @par Implemented as
     * - R_PTP_CheckWorst10Values()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] wait_option            Timeout interval
     **/
    ssp_err_t (* checkWorst10Values)(ptp_ctrl_t * const p_ctrl, uint32_t wait_option);

    /** Get worst 10 values by software.
     * @par Implemented as
     * - R_PTP_GetWorst10Values()
     *
     * @param[in] p_ctrl                  Pointer to the control structure
     * @param[in] wait_option             Timeout interval
     * @param[out] p_positive_worst10     Returns the positive worst 10 values
     * @param[out] p_negative_worst10     Returns the negative worst 10 values
     **/
    ssp_err_t (* getWorst10Values)(ptp_ctrl_t * const p_ctrl,
            uint32_t * p_positive_worst10,
            uint32_t * p_negative_worst10,
            uint32_t wait_option);

    /** Set the gradient limits for positive and negative worst 10 values.
     * @par Implemented as
     * - R_PTP_SetGradientLimit()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] p_positive_limit       Positive limit of worst 10 values
     * @param[in] p_negative_limit       Negative limit of worst 10 values
     **/
    ssp_err_t (* setGradientLimit)(ptp_ctrl_t * const p_ctrl, uint32_t * p_positive_limit, uint32_t * p_negative_limit);

    /** Update the clock identity field.
     * @par Implemented as
     * - R_PTP_UpdateClockID()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] ptp_channel            EPTPC channel
     * @param[in] p_clock_id             Pointer to clock ID
     **/
    ssp_err_t (* updateClockID)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, int8_t * p_clock_id);

    /** Update the domain number field in the message header.
     * @par Implemented as
     * - R_PTP_UpdateDomainNumber()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] ptp_channel            EPTPC channel
     * @param[in] domain_num             Domain number
     **/
    ssp_err_t (* updateDomainNumber)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, uint8_t domain_num);

    /** Update the announce message's flag field.
     * @par Implemented as
     * - R_PTP_UpdateAnnounceFlags()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] ptp_channel            EPTPC channel
     * @param[in] p_flag                 Pointer to announce message flag field
     **/
    ssp_err_t (* updateAnnounceFlags)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, ptp_announce_flag_t * p_flag);

    /** Update the announce message's message field.
     * @par Implemented as
     * - R_PTP_UpdateAnnounceMsgs()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] ptp_channel            EPTPC channel
     * @param[in] p_message              Pointer to announce message field
     **/
    ssp_err_t (* updateAnnounceMsgs)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, ptp_announce_message_t * p_message);

    /** Update transmission interval and logMessageInterval of Sync and Announce messages.
     * @par Implemented as
     * - R_PTP_UpdateSyncAnnounceMsgInterval()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] ptp_channel            EPTPC channel
     * @param[in] p_sync_interval        Pointer to sync message interval
     * @param[in] p_announce_interval    Pointer to announce message interval
     **/
    ssp_err_t (* updateSyncAnnounceMsgInterval)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, int8_t * p_sync_interval, int8_t * p_announce_interval);

    /** Update transmission interval, logMessageInterval and timeout values of Delay messages.
     * @par Implemented as
     * - R_PTP_UpdateDelayMsgInterval()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] ptp_channel            EPTPC channel
     * @param[in] p_interval             Case of master: Delay_Resp logMessageInterval
                                         Case of slave: Delay_Req/Pdelay_Req transmission interval
     * @param[in] p_timeout              Delay_Resp/Pdelay_Resp receiving timeout
     **/
    ssp_err_t (* updateDelayMsgInterval)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, int8_t * p_interval, uint32_t * p_timeout);

    /** Get message reception configuration
     * @par Implemented as
     * - R_PTP_GetMessageReceptionConfig()
     *
     * @param[in] p_ctrl                     Pointer to the control structure
     * @param[in] ptp_channel                EPTPC channel
     * @param[out] p_ptp_message_reception   Pointer to SYNFP message reception configuration structure
     **/
    ssp_err_t (* getMessageReceptionConfig)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, ptp_message_reception_t * p_ptp_message_reception);

    /** Set message reception configuration.
     * @par Implemented as
     * - R_PTP_SetMessageReceptionConfig()
     *
     * @param[in] p_ctrl                    Pointer to the control structure
     * @param[in] ptp_channel               EPTPC channel
     * @param[in] p_ptp_message_reception   Pointer to SYNFP message reception config structure
     **/
    ssp_err_t (* setMessageReceptionConfig)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, ptp_message_reception_t * p_ptp_message_reception);

    /** Disable timer event interrupt.
     * @par Implemented as
     * - R_PTP_DisableTimer()
     *
     * @param[in] p_ctrl                  Pointer to the control structure
     * @param[in] timer_channel          STCA pulse output timer channel
     **/
    ssp_err_t (* disableTimer)(ptp_ctrl_t * const p_ctrl, ptp_stca_timer_channel_t timer_channel);

    /** Set/clear interrupt indication to ELC output on generation of pulse produced by pulse output timer.
     * @par Implemented as
     * - R_PTP_IndicateEvent()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] timer_channel          STCA pulse output timer channel
     * @param[in] timer_pulse_edge       STCA pulse output timer pulse edge
     * @param[in] is_set                 Set or clear indication of
     **/
    ssp_err_t (* indicateEvent)(ptp_ctrl_t * const p_ctrl,
            ptp_stca_timer_channel_t timer_channel,
            ptp_stca_timer_pulse_edge_t timer_pulse_edge,
            bool is_set);

    /** Set/clear auto clear mode for enabling one time output of ELC event.
     * @par Implemented as
     * - R_PTP_AutoClearEvent()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] timer_channel          STCA pulse output timer channel
     * @param[in] timer_pulse_edge       STCA pulse output timer edge
     * @param[in] is_set                 Enable or disable automatic clearing of event
     **/
    ssp_err_t (* autoClearEvent)(ptp_ctrl_t * const p_ctrl,
            ptp_stca_timer_channel_t timer_channel,
            ptp_stca_timer_pulse_edge_t timer_pulse_edge,
            bool is_set);

    /** Sets start time, pulse period and pulse width for the pulse output timer.
     * @par Implemented as
     * - R_PTP_SetTimer()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] timer_channel          STCA pulse output timer channel
     * @param[in] event                  Timer event start time
     * @param[in] cycle                  Pulse output cycle interval
     * @param[in] pulse_width            Width of the high level of the pulse signal
     **/
    ssp_err_t (* setTimer)(ptp_ctrl_t * const p_ctrl,
            uint8_t timer_channel,
            UInt64_t event,
            uint32_t cycle,
            uint32_t pulse_width);

    /** Set MINT interrupt event to enable notification for change in state of modules.
     * @par Implemented as
     * - R_PTP_SetMINTevent()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] ptp_reg                MINT interrupt register
     * @param[in] event                  Interrupt element
     * @param[in] is_set                 Set or clear MINT event
     **/
    ssp_err_t (* setMINTevent)(ptp_ctrl_t * const p_ctrl, ptp_event_t ptp_reg, uint32_t event, bool is_set);

    /** Enable EPTPC INFABT notification
     * @par Implemented as
     * - R_PTP_EnableINFABTnotification()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] ptp_channel            EPTPC channel
     **/
    ssp_err_t (* enableINFABTnotification)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel);

    /** Disable EPTPC INFABT notification
     * @par Implemented as
     * - R_PTP_DisableINFABTnotification()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] ptp_channel            EPTPC channel
     **/
    ssp_err_t (* disableINFABTnotification)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel);

    /** Checks the status of INFABT flag
     * @par Implemented as
     * - R_PTP_CheckINFABTstatus()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] ptp_channel            EPTPC channel
     * @param[out] p_status              Returns status of INFABT flag
     **/
    ssp_err_t (* checkINFABTstatus)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, uint8_t * p_status);

    /** Clear INFABT interrupt occurrence flag.
     * @par Implemented as
     * - R_PTP_ClearINFABTstatus()
     *
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[in] ptp_channel            EPTPC channel
     **/
    ssp_err_t (* clearINFABTstatus)(ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel);

    /** Get the driver version based on compile time macros.
     * @par Implemented as
     * - R_PTP_VersionGet()
     * @param[out]  p_version  Code and API version used.
     **/
    ssp_err_t (* versionGet) (ssp_version_t * const p_version);
} ptp_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_ptp_instance
{
    ptp_ctrl_t       * p_ctrl;    ///< Pointer to the control structure for this instance
    ptp_cfg_t  const * p_cfg;     ///< Pointer to the configuration structure for this instance
    ptp_api_t  const * p_api;     ///< Pointer to the API structure for this instance
} ptp_instance_t;


/** @} (end defgroup PTP_API) */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_PTP_API_H */
