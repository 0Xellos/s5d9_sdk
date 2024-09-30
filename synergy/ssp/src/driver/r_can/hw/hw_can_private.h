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
 * File Name    : hw_can_private.h
 * Description  : Private CAN register access functions
 **********************************************************************************************************************/

#ifndef HW_CAN_PRIVATE_H
#define HW_CAN_PRIVATE_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_can.h"
#include "../r_can_private_api.h"
#include "common/hw_can_common.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define CAN_MAX_DATA_LENGTH                 (8U)
#define CAN_MAX_NO_MAILBOXES                (32U)
#define CAN_TIME_SEGMENT1_MIN               CAN_TIME_SEGMENT1_TQ4
#define CAN_TIME_SEGMENT1_MAX               CAN_TIME_SEGMENT1_TQ16
#define CAN_TIME_SEGMENT2_MIN               CAN_TIME_SEGMENT2_TQ2
#define CAN_TIME_SEGMENT2_MAX               CAN_TIME_SEGMENT2_TQ8
#define CAN_SYNC_JUMP_WIDTH_MIN             CAN_SYNC_JUMP_WIDTH_TQ1
#define CAN_SYNC_JUMP_WIDTH_MAX             CAN_SYNC_JUMP_WIDTH_TQ4
#define CAN_BAUD_RATE_PRESCALER_MIN         (1U)
#define CAN_BAUD_RATE_PRESCALER_MAX         (1024U)

#define CAN_SID_MASK                        (0x000007FFU)   ///< Maximum Standard ID
#define CAN_XID_MASK                        (0x1FFFFFFFU)   ///< Maximum Extended ID
#define CAN_INVALID_MASK                    (0xFFFFFFFFU)   ///< Mask used to determine invalid masks in MKRk
#define CAN_GROUP_MASK                      (0x0000000FU)
#define CAN_MAILBOX_GROUP_SIZE              (4U)
#define CAN_DEFAULT_MASK                    (0x1FFFFFFFU)
#define CAN_MAILBOX_IDLE                    (0x00U)
#define CAN_MAILBOX_RX                      (0x40U)
#define CAN_ERROR_MASK                      (0x7FU)     ///< Ignore Error Display Mode Select Bit
#define CAN_ACCUMULATED_ERROR_CODE          (0x80U)     ///< Set Error Display Mode to Accumulated Error Code
#define CAN_ERROR_SEARCH                    (2U)
#define CAN_RECEIVE_SEARCH                  (0U)
#define CAN_TRANSMIT_SEARCH                 (1U)
#define CAN_RECOVERY_NORMAL                 (0U)
#define CAN_MAILBOX_MODE_NORMAL             (0U)
#define CAN_ID_PRIORITY_TRANSMIT            (0U)
#define CAN_TIMESTAMP_RESET                 (1U)
#define CAN_TIMESTAMP_RUNNING               (0U)

#define CAN_CHECK_MODE_MASK                       (0x0700U)
#define CAN_CHECK_MODE_RESET                      (0x0100U)
#define CAN_CHECK_MODE_HALT                       (0x0200U)
#define CAN_CHECK_MODE_NORMAL                     (0x0000U)

#define CAN_PCLKB_RATIO                     (2U)        ///< The ratio between PCLKA or ICLK and PCLKB. From hardware manual.

#define CAN_TRANSMIT_CLEAR                  (0U)
#define CAN_TX_RX_INTERRUPTS_ENABLE         (0xFFFFFFFFU)
#define CAN_TX_RX_INTERRUPTS_DISABLE        (0x00000000U)

#define CAN_BAUD_RATE_PRESCALER_MASK        (0x3FFU)

#define CAN_TIMESTAMP_PRESCALER_1BITTIME    (0x0U)
#define CAN_TIMESTAMP_PRESCALER_2BITTIME    (0x1U)
#define CAN_TIMESTAMP_PRESCALER_4BITTIME    (0x2U)
#define CAN_TIMESTAMP_PRESCALER_8BITTIME    (0x3U)

/** Error-Warning | Error-Passive | Bus-Off Entry | Bus-Off Recovery | Overrun */
#define CAN_ERROR_INTERRUPTS_ENABLE         (0x3EU)
#define CAN_ERROR_INTERRUPTS_DISABLE        (0x00U)

/** CAN1 Control Register (CTLR) b9, b8 CANM[1:0] CAN Operating Mode Select. */
typedef enum e_can_mode_control
{
    CAN_MODE_CONTROL_NORMAL,             ///< CAN operation mode
    CAN_MODE_CONTROL_RESET,              ///< CAN reset mode
    CAN_MODE_CONTROL_HALT,               ///< CAN halt mode
    CAN_MODE_CONTROL_RESET_FORCED        ///< CAN reset mode (forced transition)
} can_mode_control_t;
/** Local sleep mode for CAN module */

typedef enum e_can_sleep
{
    CAN_SLEEP_AWAKEN, CAN_SLEEP_SLEEP,
} can_sleep_t;

#define    CAN_TEST_NORMAL            (0U)
#define    CAN_TEST_LISTEN_ONLY       (3U)
#define    CAN_TEST_LOOPBACK_EXTERNAL (5U)
#define    CAN_TEST_LOOPBACK_INTERNAL (7U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Prototypes
 **********************************************************************************************************************/
ssp_err_t HW_CAN_Control(R_CAN0_Type * p_can_regs, can_mode_t mode);

void HW_CAN_MailboxIdSet(R_CAN0_Type * p_can_regs, uint32_t count, can_mailbox_t * const p_mailbox,
        can_id_mode_t id_mode);

void HW_CAN_MailboxMaskSet(R_CAN0_Type * p_can_regs, uint32_t count, uint32_t * const p_mailbox_mask,
        can_id_mode_t id_mode);

bool HW_CAN_TimeStampReset(R_CAN0_Type * p_can_regs);

bool HW_CAN_BitRateGet(R_CAN0_Type * p_can_regs, uint32_t * const p_frequency);

void HW_CAN_MailboxesClear(R_CAN0_Type * p_can_regs);

can_id_t HW_CAN_ReceiveIdGet(R_CAN0_Type * p_can_regs, can_id_mode_t id_mode, uint32_t mailbox);

can_mailbox_send_receive_t HW_CAN_MailboxTypeGet(R_CAN0_Type * p_can_regs, uint32_t mailbox);

uint8_t HW_CAN_ReceiveDataCountGet(R_CAN0_Type * p_can_regs, uint32_t mailbox);

void HW_CAN_ReceiveDataGet(R_CAN0_Type * p_can_regs, uint32_t mailbox, can_id_mode_t id_mode,
        can_frame_t * const p_frame);

void HW_CAN_TransmitFrameSend(R_CAN0_Type * p_can_regs, uint32_t mailbox, can_frame_t * const p_frame,
        can_id_mode_t id_mode);

void HW_CAN_InterruptErrorGet(R_CAN0_Type * p_can_regs, can_interrrupt_status_t * const p_status);

bool HW_CAN_StatusGet(R_CAN0_Type * p_can_regs, can_info_t * const p_info);

bool HW_CAN_ErrorsGet(R_CAN0_Type * p_can_regs);

void HW_CAN_PowerOn(R_CAN0_Type * p_can_regs);

void HW_CAN_PowerOff(R_CAN0_Type * p_can_regs);

/***********************************************************************************************************************
 Private Functions
 ***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief      This function sets the bus off recovery mode to CAN spec.
 * @param[in]  channel
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_BusRecoveryModeSet (R_CAN0_Type * p_can_regs)
{
    /** BOM:    Bus Off recovery mode acc. to IEC11898-1 */
    p_can_regs->CTLR_b.BOM = CAN_RECOVERY_NORMAL;
}

/*******************************************************************************************************************//**
 * @brief      This function sets the mode for all mailboxes to normal mailbox (not FIFO mode).
 * @param[in]  channel
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_MailboxModeSet (R_CAN0_Type * p_can_regs)
{
    /** MBM: Select normal mailbox mode. */
    p_can_regs->CTLR_b.MBM = CAN_MAILBOX_MODE_NORMAL;
}

/*******************************************************************************************************************//**
 * @brief      This function sets the priority to ID, the CAN standard.
 * @param[in]  channel
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_PriorityModeSet (R_CAN0_Type * p_can_regs)
{
    /** TPM: ID priority mode. */
    p_can_regs->CTLR_b.TPM = CAN_ID_PRIORITY_TRANSMIT;
}

/*******************************************************************************************************************//**
 * @brief      This function sets the timestamp update period.
 * @param[in]  channel
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_TimeStampSet (R_CAN0_Type * p_can_regs)
{
    /** Time Stamp Pre Scaler: Update every 8 bit times */
    p_can_regs->CTLR_b.TSPS = CAN_TIMESTAMP_PRESCALER_8BITTIME;
}

/*******************************************************************************************************************//**
 * @brief      This function sets the mask enable default (off) for all receive mailboxes.
 * @param[in]  channel
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_MaskEnableDefautSet (R_CAN0_Type * p_can_regs)
{
    /** Mask invalid for all mailboxes by default. */
    p_can_regs->MKIVLR = CAN_INVALID_MASK;
}

/*******************************************************************************************************************//**
 * @brief      This function clears the transmit ready flag.
 * @param[in]  channel
 * @param[in]  mailbox - mailbox number
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_TransmitDataClear (R_CAN0_Type * p_can_regs, uint32_t mailbox)
{
    /** Clear SENTDATA.
     * Do a byte-write to avoid read-modify-write with HW writing another bit in between.
     * TRMREQ must be set to 0 (or will send again).
     * Do it twice since "Bits SENTDATA and TRMREQ cannot be set to 0 simultaneously." */
    p_can_regs->MCTLn_TX[mailbox] = CAN_TRANSMIT_CLEAR;
    p_can_regs->MCTLn_TX[mailbox] = CAN_TRANSMIT_CLEAR;
}

/*******************************************************************************************************************//**
 * @brief      This function finds the lowest numbered mailbox with a message lost.
 * @param[in]  channel
 * @param[in]  mailbox - mailbox number
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_ErrorMailboxGet (R_CAN0_Type * p_can_regs, uint32_t * mailbox)
{
    uint8_t saved_msmr = p_can_regs->MSMR;              ///< Save the current MSMR value
    p_can_regs->MSMR = CAN_ERROR_SEARCH;                ///< search for lowest numbered mailbox with message lost
    *mailbox = p_can_regs->MSSR_b.MBNST;                ///< get mailbox number
    p_can_regs->MSMR = saved_msmr;                      ///< Restore the previous MSMR value
}

/*******************************************************************************************************************//**
 * @brief      This function finds the lowest numbered mailbox with a message received.
 * @param[in]  channel
 * @param[in]  mailbox - mailbox number
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_ReceiveMailboxGet (R_CAN0_Type * p_can_regs, uint32_t * mailbox)
{
    uint8_t saved_msmr = p_can_regs->MSMR;              ///< Save the current MSMR value
    p_can_regs->MSMR = CAN_RECEIVE_SEARCH;              ///< search for lowest numbered mailbox with message received
    *mailbox = p_can_regs->MSSR_b.MBNST;                ///< get mailbox number
    p_can_regs->MSMR = saved_msmr;                      ///< Restore the previous MSMR value
}

/*******************************************************************************************************************//**
 * @brief      This function finds the lowest numbered mailbox with a message transmitted.
 * @param[in]  channel
 * @param[in]  mailbox - mailbox number
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_TransmitMailboxGet (R_CAN0_Type * p_can_regs, uint32_t * mailbox)
{
    uint8_t saved_msmr = p_can_regs->MSMR;              ///< Save the current MSMR value
    p_can_regs->MSMR = CAN_TRANSMIT_SEARCH;             ///< search for lowest numbered mailbox with message transmitted
    *mailbox = p_can_regs->MSSR_b.MBNST;                ///< get mailbox number
    p_can_regs->MSMR = saved_msmr;                      ///< Restore the previous MSMR value
}

/*******************************************************************************************************************//**
 * @brief      This function enables the error interrupts.
 * @param[in]  channel
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_ErrorInterruptEnable (R_CAN0_Type * p_can_regs)
{
    /** Error-Warning | Error-Passive | Bus-Off Entry | Bus-Off Recovery | Overrun */
    p_can_regs->EIER = CAN_ERROR_INTERRUPTS_ENABLE;
}

/*******************************************************************************************************************//**
 * @brief      This function disables the error interrupts.
 * @param[in]  channel
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_ErrorInterruptDisable (R_CAN0_Type * p_can_regs)
{
    p_can_regs->EIER = CAN_ERROR_INTERRUPTS_DISABLE;
}

/*******************************************************************************************************************//**
 * @brief      This function enables interrupts for all mailboxes, transmit and receive.
 * @param[in]  channel
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_SendReceiveInterruptsEnable (R_CAN0_Type * p_can_regs)
{
    p_can_regs->MIER = CAN_TX_RX_INTERRUPTS_ENABLE;     ///< Enable interrupts for all mailboxes, transmit and receive.
}

/*******************************************************************************************************************//**
 * @brief      This function disables interrupts for all mailboxes, transmit and receive.
 * @param[in]  channel
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_SendReceiveInterruptsDisable (R_CAN0_Type * p_can_regs)
{
    p_can_regs->MIER = CAN_TX_RX_INTERRUPTS_DISABLE;    ///< Disable interrupts for all mailboxes, transmit and receive.
}

/*******************************************************************************************************************//**
 * @brief      This function sets the error display mode.
 * @param[in]  channel
 * @retval     None.
 **********************************************************************************************************************/

__STATIC_INLINE void HW_CAN_ErrorModeSet (R_CAN0_Type * p_can_regs)
{
    p_can_regs->ECSR = CAN_ACCUMULATED_ERROR_CODE;          ///< Set error display mode.
}

/*******************************************************************************************************************//**
 * @brief      This function returns the transmit data ready flag.
 * @param[in]  channel
 * @param[in]  mailbox - mailbox number
 * @retval     false if transmit not ready, true if transmit ready.
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_CAN_TransmitDataReady (R_CAN0_Type * p_can_regs, uint32_t mailbox)
{
    /** Check for send enabled and Check for data send complete. */
    return !((1U == p_can_regs->MCTLn_TX_b[mailbox].TRMREQ) && (0U == p_can_regs->MCTLn_TX_b[mailbox].SENTDATA));
}

/*******************************************************************************************************************//**
 * @brief      This function reports whether receive message is lost or not.
 * @param[in]  channel
 * @param[in]  mailbox      - mailbox number
 * @retval     false if no loss, true if data lost
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_CAN_MessageLostGet (R_CAN0_Type * p_can_regs, uint32_t mailbox)
{
    /** Return affirmative value when message is lost. */
    return (bool) (p_can_regs->MCTLn_RX_b[mailbox].MSGLOST == 1U);
}

__STATIC_INLINE bool HW_CAN_NewDataStatusFlag (R_CAN0_Type * p_can_regs)
{
    return (p_can_regs->STR_b.NDST);
}

__STATIC_INLINE bool HW_CAN_SentDataStatusFlag (R_CAN0_Type * p_can_regs)
{
    return (p_can_regs->STR_b.SDST);
}

__STATIC_INLINE bool HW_CAN_MailboxMessageLost (R_CAN0_Type * p_can_regs)
{
    return (p_can_regs->STR_b.NMLST);
}

__STATIC_INLINE void HW_CAN_ClearNewData (R_CAN0_Type* p_can_regs, uint32_t mailbox)
{
    /** Clear rx data flag */
    p_can_regs->MCTLn_RX[mailbox] = CAN_MAILBOX_RX;
}

/*******************************************************************************************************************//**
 * @brief      This function reports whether receive data is available or not.
 * @param[in]  channel
 * @param[in]  mailbox      - mailbox number
 * @retval     false if no data, true if data available
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_CAN_ReceiveDataAvailable (R_CAN0_Type * p_can_regs, uint32_t mailbox)
{
    /** Return affirmative value when data is available. */
    return ((p_can_regs->MCTLn_RX_b[mailbox].NEWDATA) && (!(p_can_regs->MCTLn_RX_b[mailbox].INVALDATA)));
}

/*******************************************************************************************************************//**
 * @brief      This function sets the id mode for all mailboxes.
 * @param[in]  channel
 * @param[in]  id_mode      - standard or extended id mode
 * @retval     false if invalid id mode, true if no errors
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_IdModeSet (R_CAN0_Type * p_can_regs, can_id_mode_t id_mode)
{
    p_can_regs->CTLR_b.IDFM = id_mode;                  ///< Set ID mode.
}

/*******************************************************************************************************************//**
 * @brief      This function sets the mailbox message mode to overrun or overwrite mode.
 * @param[in]  channel
 * @param[in]  message_mode      - overrun or overwrite mode
 * @retval     false if invalid message mode, true if no errors
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_MessageModeSet (R_CAN0_Type * p_can_regs, can_message_mode_t message_mode)
{
    p_can_regs->CTLR_b.MLM = message_mode;              ///< Set message mode.
}

/*******************************************************************************************************************//**
 * @brief      This function sets CAN bit timing.
 * @param[in]  channel
 * @retval     false if cfg values out of range, true if no errors
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_BitTimingSet (R_CAN0_Type * p_can_regs, can_bit_timing_cfg_t * const p_timing)
{
    p_can_regs->BCR_b.BRP = ((p_timing->baud_rate_prescaler - 1) & CAN_BAUD_RATE_PRESCALER_MASK);
    p_can_regs->BCR_b.TSEG1 = p_timing->time_segment_1;
    p_can_regs->BCR_b.TSEG2 = p_timing->time_segment_2;
    p_can_regs->BCR_b.SJW = p_timing->synchronization_jump_width;
}

/*******************************************************************************************************************//**
 * @brief      This function sets the CAN source clock.
 * @param[in]  p_can_regs   CAN registers
 * @param[in]  p_cfg        CAN configuration
 * @retval     SSP_SUCCESS              Clock is configured properly
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAN_ClockSet (R_CAN0_Type* p_can_regs, can_clock_source_t clock_source)
{
    /** Set the clock source */
    p_can_regs->BCR_b.CCLKS = clock_source;
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_CAN_PRIVATE_H */
