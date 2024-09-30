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
 * File Name    : r_can_api.h
 * Description  : CAN Shared Interface definition
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup CAN_API CAN Interface
 * @brief Interface for CAN peripheral
 *
 * @section CAN_INTERFACE_SUMMARY Summary
 * The CAN interface provides common APIs for CAN HAL drivers. CAN interface supports following features.
 * - Full-duplex CAN communication
 * - Generic CAN parameter setting
 * - Interrupt driven transmit/receive processing
 * - Callback function support with returning event code
 * - Hardware resource locking during a transaction
 * @{
 **********************************************************************************************************************/

#ifndef DRV_CAN_API_H
#define DRV_CAN_API_H

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
#define CAN_API_VERSION_MAJOR (2U)
#define CAN_API_VERSION_MINOR (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** CAN event codes */
typedef enum e_can_event
{
    CAN_EVENT_RX_COMPLETE,                      ///< Receive complete event.
    CAN_EVENT_TX_COMPLETE,                      ///< Transmit complete event.
    CAN_EVENT_ERR_BUS_OFF,                      ///< Bus Off event.
    CAN_EVENT_BUS_RECOVERY,                     ///< Bus Off Recovery event.
    CAN_EVENT_ERR_PASSIVE,                      ///< Error Passive event.
    CAN_EVENT_ERR_WARNING,                      ///< Error Warning event.
    CAN_EVENT_MAILBOX_OVERWRITE_OVERRUN = 6,    ///< DEPRECATED, Mailbox has been overrun. This event is not used when the mailbox is overwritten.
    CAN_EVENT_MAILBOX_OVERRUN = 6,              ///< Mailbox has been overrun.
} can_event_t;

/** CAN Status */
typedef union
{
    uint32_t  status;
    struct st_status_b
    {
        uint32_t  new_data          : 1;    ///< One or more receive mailboxes have new data available.
        uint32_t  data_sent         : 1;    ///< One or more transmit mailboxes have sent data.
        uint32_t  reserved_1        : 2;    ///< Reserved bits.
        uint32_t  message_lost      : 1;    ///< One or more mailboxes have lost data.
        uint32_t  reserved_2        : 1;    ///< Reserved bit.
        uint32_t  transmit_abort    : 1;    ///< One or more mailboxes have aborted transmit.
        uint32_t  error             : 1;    ///< Error occurred.
        uint32_t  reset_mode        : 1;    ///< Module in CAN reset mode.
        uint32_t  halt_mode         : 1;    ///< Module in CAN halt mode.
        uint32_t  sleep_mode        : 1;    ///< Module in CAN sleep mode.
        uint32_t  error_passive     : 1;    ///< Module in CAN error passive state.
        uint32_t  bus_off           : 1;    ///< Module in CAN bus off state.
        uint32_t  transmit          : 1;    ///< Transmission in progress.
        uint32_t  receive           : 1;    ///< Reception in progress.
        uint32_t  reserved_3        : 1;    ///< Reserved bit.
        uint32_t  listen_only       : 1;    ///< Listen only test m
        uint32_t  external_loopback : 1;    ///< External loopback test mode.
        uint32_t  internal_loopback : 1;    ///< Internal loopback test mode.
        uint32_t  new_data_mailbox  : 5;    ///< Lowest mailbox number with new data. 0 if none.
        uint32_t  msg_lost_mailbox  : 5;    ///< Lowest mailbox number with msg lost. 0 if none.
        uint32_t  reserved_4        : 3;    ///< Reserved bits.
    }  status_b;
} can_status_t;

/** CAN Error Code */
typedef union
{
    uint32_t  error;
    struct st_error_b
    {
        uint32_t  stuff_error           : 1;    ///< Stuff error detected if 1.
        uint32_t  form_error            : 1;    ///< Form error detected if 1.
        uint32_t  ack_error             : 1;    ///< ACK error detected if 1.
        uint32_t  crc_error             : 1;    ///< CRC error detected if 1.
        uint32_t  bit_recessive_error   : 1;    ///< Recessive Bit error detected if 1.
        uint32_t  bit_dominant_error    : 1;    ///< Dominant Bit error detected if 1.
        uint32_t  ack_delimiter_error   : 1;    ///< ACK delimiter error detected if 1.
        uint32_t  reserved_9            : 25;   ///< reserved bits.

    } error_b;
} can_error_t;
/** CAN Operation modes */
typedef enum e_can_mode
{
    CAN_MODE_NORMAL,                        ///< CAN Normal Mode.
    CAN_MODE_HALT,                          ///< CAN Halt Mode.
    CAN_MODE_SLEEP,                         ///< CAN SLEEP Mode.
    CAN_MODE_EXIT_SLEEP,                    ///< CAN Exit SLEEP Mode.
    CAN_MODE_RESET,                         ///< CAN Reset Mode.
    CAN_MODE_LISTEN,                        ///< CAN Listen Mode.
    CAN_MODE_LOOPBACK_INTERNAL,             ///< CAN Internal Loopback Mode.
    CAN_MODE_LOOPBACK_EXTERNAL,             ///< CAN External Loopback Mode.
} can_mode_t;

typedef struct st_can_info
{
    can_mode_t    operation_mode;           ///< Can operation mode.
    can_status_t  status;                   ///< CAN status.
    uint32_t      bit_rate;                 ///< CAN bit rate.
    uint8_t       error_count_transmit;     ///< Transmit error count.
    uint8_t       error_count_receive;      ///< Receive error count.
    can_error_t   error_code;               ///< Error code, cleared after reading.
} can_info_t;

/** CAN callback parameter definition */
typedef struct st_can_callback_arg
{
    uint32_t     channel;                   ///< Device channel number.
    can_event_t  event;                     ///< Event code.
    uint32_t     mailbox;                   ///< Mailbox number of interrupt source.
    void const * p_context;                 ///< Context provided to user during callback.
} can_callback_args_t;

/** CAN ID modes */
typedef enum e_can_id_mode
{
    CAN_ID_MODE_STANDARD,           ///< Standard IDs of 11 bits used.
    CAN_ID_MODE_EXTENDED,           ///< Extended IDs of 29 bits used.
} can_id_mode_t;

/** CAN frame types */
typedef enum e_can_frame_type
{
    CAN_FRAME_TYPE_DATA,            ///< Data frame type.
    CAN_FRAME_TYPE_REMOTE,          ///< Remote frame type.
} can_frame_type_t;

/** CAN Message Modes */
typedef enum e_can_message_mode
{
    CAN_MESSAGE_MODE_OVERWRITE,     ///< Receive data will be overwritten if not read before the next frame.
    CAN_MESSAGE_MODE_OVERRUN,       ///< Receive data will be retained until it is read.
} can_message_mode_t;

/** CAN Source Clock */
typedef enum e_can_clock_source
{
    CAN_CLOCK_SOURCE_PCLKB,         ///< PCLB is the source of the CAN Clock
    CAN_CLOCK_SOURCE_CANMCLK,       ///< CANMCLK is the source of the CAN Clock
} can_clock_source_t;

/** CAN Time Segment 1 Time Quanta */
typedef enum e_can_time_segment1
{
    CAN_TIME_SEGMENT1_TQ4 = 3,      ///< Time Segment 1 setting for  4 Time Quanta
    CAN_TIME_SEGMENT1_TQ5,          ///< Time Segment 1 setting for  5 Time Quanta
    CAN_TIME_SEGMENT1_TQ6,          ///< Time Segment 1 setting for  6 Time Quanta
    CAN_TIME_SEGMENT1_TQ7,          ///< Time Segment 1 setting for  7 Time Quanta
    CAN_TIME_SEGMENT1_TQ8,          ///< Time Segment 1 setting for  8 Time Quanta
    CAN_TIME_SEGMENT1_TQ9,          ///< Time Segment 1 setting for  9 Time Quanta
    CAN_TIME_SEGMENT1_TQ10,         ///< Time Segment 1 setting for 10 Time Quanta
    CAN_TIME_SEGMENT1_TQ11,         ///< Time Segment 1 setting for 11 Time Quanta
    CAN_TIME_SEGMENT1_TQ12,         ///< Time Segment 1 setting for 12 Time Quanta
    CAN_TIME_SEGMENT1_TQ13,         ///< Time Segment 1 setting for 13 Time Quanta
    CAN_TIME_SEGMENT1_TQ14,         ///< Time Segment 1 setting for 14 Time Quanta
    CAN_TIME_SEGMENT1_TQ15,         ///< Time Segment 1 setting for 15 Time Quanta
    CAN_TIME_SEGMENT1_TQ16,         ///< Time Segment 1 setting for 16 Time Quanta
} can_time_segment1_t;

/** CAN Time Segment 2 Time Quanta */
typedef enum e_can_time_segment2
{
    CAN_TIME_SEGMENT2_TQ2 = 1,      ///< Time Segment 2 setting for 2 Time Quanta
    CAN_TIME_SEGMENT2_TQ3,          ///< Time Segment 2 setting for 3 Time Quanta
    CAN_TIME_SEGMENT2_TQ4,          ///< Time Segment 2 setting for 4 Time Quanta
    CAN_TIME_SEGMENT2_TQ5,          ///< Time Segment 2 setting for 5 Time Quanta
    CAN_TIME_SEGMENT2_TQ6,          ///< Time Segment 2 setting for 6 Time Quanta
    CAN_TIME_SEGMENT2_TQ7,          ///< Time Segment 2 setting for 7 Time Quanta
    CAN_TIME_SEGMENT2_TQ8,          ///< Time Segment 2 setting for 8 Time Quanta
} can_time_segment2_t;

/** CAN Synchronization Jump Width Time Quanta */
typedef enum e_can_sync_jump_width
{
    CAN_SYNC_JUMP_WIDTH_TQ1 = 0,    ///< Synchronization Jump Width setting for 1 Time Quanta
    CAN_SYNC_JUMP_WIDTH_TQ2,        ///< Synchronization Jump Width setting for 2 Time Quanta
    CAN_SYNC_JUMP_WIDTH_TQ3,        ///< Synchronization Jump Width setting for 3 Time Quanta
    CAN_SYNC_JUMP_WIDTH_TQ4,        ///< Synchronization Jump Width setting for 4 Time Quanta
} can_sync_jump_width_t;

/** CAN bit rate configuration. */
typedef struct st_can_bit_timing_cfg
{
    uint32_t                baud_rate_prescaler;             ///< Baud rate prescaler. Valid values: 1 - 1024.
    can_time_segment1_t     time_segment_1;                  ///< Time segment 1 control.
    can_time_segment2_t     time_segment_2;                  ///< Time segment 2 control.
    can_sync_jump_width_t   synchronization_jump_width;      ///< Synchronization jump width.
} can_bit_timing_cfg_t;

/** CAN Id */
typedef uint32_t can_id_t;

/** CAN data Frame */
typedef struct st_can_frame
{
    can_id_t          id;                                   ///< CAN id.
    uint8_t           data_length_code;                     ///< CAN Data Length code, number of bytes in the message.
    uint8_t           data[8];                              ///< CAN data, up to 8 bytes.
    can_frame_type_t  type;                                 ///< Frame type, data or remote frame.
} can_frame_t;

/** CAN  Mailbox type */
typedef enum e_can_mailbox_send_receive
{
    CAN_MAILBOX_RECEIVE,                                    ///< Mailbox is for receiving.
    CAN_MAILBOX_TRANSMIT,                                   ///< Mailbox is for sending.
} can_mailbox_send_receive_t;

/** CAN Mailbox */
typedef struct st_can_mailbox
{
    can_id_t                    mailbox_id;                 ///< Mailbox ID.
    can_mailbox_send_receive_t  mailbox_type;               ///< Receive or Transmit mailbox type.
    can_frame_type_t            frame_type;                 ///< Frame type for receive mailbox.
} can_mailbox_t;

/** CAN control commands. */
typedef enum e_can_command
{
    CAN_COMMAND_MODE_SWITCH = 1,                            ///< Switch CAN operating mode..
} can_command_t;

/** CAN Configuration */
typedef struct st_can_cfg
{
    /* CAN generic configuration */
    uint32_t                channel;                                        ///< CAN channel.
    can_bit_timing_cfg_t  * p_bit_timing;                                   ///< CAN bit timing.
    can_id_mode_t           id_mode;                                        ///< Standard or Extended ID mode.
    uint32_t                mailbox_count;                                  ///< Number of mailboxes.
    can_mailbox_t         * p_mailbox;                                      ///< Pointer to mailboxes.
    can_message_mode_t      message_mode;                                   ///< Overwrite message or overrun.
    /* Configuration for CAN Event processing */
    void                 (* p_callback)(can_callback_args_t * p_args);      ///< Pointer to callback function
    void const            * p_context;                                      ///< User defined callback context.

    /* Pointer to CAN peripheral specific configuration */
    void const            * p_extend;                                       ///< CAN hardware dependent configuration
    uint8_t                 error_ipl;                                      ///< Error interrupt priority
    uint8_t                 mailbox_rx_ipl;                                 ///< Receive interrupt priority
    uint8_t                 mailbox_tx_ipl;                                 ///< Transmit interrupt priority
} can_cfg_t;

/** CAN control block.  Allocate an instance specific control block to pass into the CAN API calls.
 * @par Implemented as
 * - can_instance_ctrl_t
 */
typedef void can_ctrl_t;

/** Shared Interface definition for CAN */
typedef struct st_can_api
{
    /** Open function for CAN device
     * @par Implemented as
     * - R_CAN_Open()
     *
     * @param[in,out]  p_ctrl     Pointer to the CAN control block Must be declared by user. Value set here.
     * @param[in]      can_cfg_t  Pointer to CAN configuration structure. All elements of this structure must be set by
     *                            user.
     */
    ssp_err_t (* open)(can_ctrl_t      * const p_ctrl,
                       can_cfg_t const * const p_cfg);

    /** Read function for CAN device, non-Blocking.
     * @par Implemented as
     * - R_CAN_Read()
     * @param[in]   p_ctrl          Pointer to the CAN control block for the channel.
     * @param[in]   mailbox         Mailbox to read from.
     * @param[out]  p_frame         Pointer for frame of CAN ID, DLC, data and frame type.
     */
    ssp_err_t (* read)(can_ctrl_t  * const  p_ctrl,
                       uint32_t             mailbox,
                       can_frame_t * const  p_frame);

    /** Write function for CAN device
     * @par Implemented as
     * - R_CAN_Write()
     * @param[in]   p_ctrl          Pointer to the CAN control block.
     * @param[in]   mailbox         Mailbox to write to.
     * @param[in]   p_frame         Pointer for frame of CAN ID, DLC, data and frame type to write.
     */
    ssp_err_t (* write)(can_ctrl_t  * const p_ctrl,
                        uint32_t            mailbox,
                        can_frame_t * const p_frame);

    /** Close function for CAN device
     * @par Implemented as
     * - R_CAN_Close()
     * @param[in]   p_ctrl     Pointer to the CAN control block.
     */
    ssp_err_t (* close)(can_ctrl_t * const p_ctrl);

    /** Control function for CAN device
     * @par Implemented as
     * - R_CAN_Control()
     * @param[in]   p_ctrl     Pointer to the CAN control block.
     * @param[in]   command    Command type.
     * @param[in]   p_data     Command data.
     */
    ssp_err_t (* control)(can_ctrl_t * const p_ctrl, can_command_t const command, void * p_data);

    /** Get CAN channel info.
     * @par Implemented as
     * - R_CAN_InfoGet()
     *
     * @param[in]   p_ctrl  Handle for channel (pointer to channel control block)
     * @param[out]  p_info  Memory address to return channel specific data to.
     */
    ssp_err_t (* infoGet)(can_ctrl_t * const p_ctrl, can_info_t * const p_info);

    /** Version get function for CAN device
     * @par Implemented as
     * - R_CAN_VersionGet()
     * @param[in]   p_version  Pointer to the memory to store the version information
     */
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);
} can_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_can_instance
{
    can_ctrl_t       * p_ctrl;   ///< Pointer to the control structure for this instance
    can_cfg_t const  * p_cfg;    ///< Pointer to the configuration structure for this instance
    can_api_t const  * p_api;    ///< Pointer to the API structure for this instance
} can_instance_t;

/** @} (end defgroup CAN_API) */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_CAN_API_H */
