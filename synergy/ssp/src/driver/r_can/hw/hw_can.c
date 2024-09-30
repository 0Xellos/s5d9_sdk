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
 * File Name    : hw_can.c
 * Description  : Hardware related LLD functions for the CAN HAL
 **********************************************************************************************************************/

#include "bsp_api.h"
#include "hw_can_private.h"
#include "r_can_api.h"
#include "r_can_cfg.h"
#include "r_cgc.h"
#include "r_cgc_api.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define CAN_OPERATE_TIMEOUT (5000U)      ///< 5000 microseconds
#define CAN_HALT_TIMEOUT    (5000U)      ///< 5000 microseconds
#define CAN_SLEEP_TIMEOUT   (5000U)      ///< 5000 microseconds
#define CAN_RESET_TIMEOUT   (5000U)      ///< 5000 microseconds

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static bool HW_CAN_Halt_Mode(R_CAN0_Type* p_can_regs);

static bool HW_CAN_Operate_Mode(R_CAN0_Type* p_can_regs);

static bool HW_CAN_Sleep_Mode(R_CAN0_Type* p_can_regs);

static bool HW_CAN_Exit_Sleep_Mode(R_CAN0_Type* p_can_regs);

static bool HW_CAN_Reset_Mode(R_CAN0_Type* p_can_regs);

static bool HW_CAN_Listen_Mode(R_CAN0_Type* p_can_regs);

static bool HW_CAN_Loopback_Internal_Mode(R_CAN0_Type* p_can_regs);

static bool HW_CAN_Loopback_External_Mode(R_CAN0_Type* p_can_regs);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief      This function causes the CAN peripheral to go to halt mode.
 * @param[in]  p_can_regs
 * @retval     false if timed out, true if no timeout
 **********************************************************************************************************************/

static bool HW_CAN_Halt_Mode (R_CAN0_Type* p_can_regs)
{
    uint32_t timer = CAN_HALT_TIMEOUT;

    /** Ensure the device is not in sleep mode. */
    p_can_regs->CTLR_b.SLPM = CAN_SLEEP_AWAKEN;
    p_can_regs->CTLR_b.CANM = CAN_MODE_CONTROL_HALT;                        ///< Switch to HALT mode.
    while (((p_can_regs->STR & CAN_CHECK_MODE_MASK) != CAN_CHECK_MODE_HALT) && ((--timer) != 0U))                 ///< Wait for mode to switch.
    {
        R_BSP_SoftwareDelay((uint32_t) 1, BSP_DELAY_UNITS_MICROSECONDS);    ///< Delay 1 microsecond
    }

    /** Return true if halt mode is entered */
    return ((p_can_regs->STR & CAN_CHECK_MODE_MASK) == CAN_CHECK_MODE_HALT);
}

/*******************************************************************************************************************//**
 * @brief      This function causes the CAN peripheral to reset.
 * @param[in]  p_can_regs
 * @retval     false if timed out, true if no timeout
 **********************************************************************************************************************/
static bool HW_CAN_Reset_Mode (R_CAN0_Type* p_can_regs)
{
    uint32_t timer = CAN_RESET_TIMEOUT;

    /** Ensure the device is not in sleep mode. */
    p_can_regs->CTLR_b.SLPM = CAN_SLEEP_AWAKEN;
    p_can_regs->CTLR_b.CANM = CAN_MODE_CONTROL_RESET;                       ///< Enter Reset mode.
    /**  Wait for mode to switch. */
    while (((p_can_regs->STR & CAN_CHECK_MODE_MASK) != CAN_CHECK_MODE_RESET) && ((--timer) != 0U))
    {
        R_BSP_SoftwareDelay((uint32_t) 1, BSP_DELAY_UNITS_MICROSECONDS);    ///< Delay 1 microsecond
    }

    /** Return true if the device is in reset mode. */
    return ((p_can_regs->STR & CAN_CHECK_MODE_MASK) == CAN_CHECK_MODE_RESET);
}

/*******************************************************************************************************************//**
 * @brief      This function causes the CAN peripheral to go to normal
 *operation mode.
 * @param[in]  p_can_regs
 * @retval     false if timed out, true if no timeout
 **********************************************************************************************************************/
static bool HW_CAN_Operate_Mode (R_CAN0_Type* p_can_regs)
{
    uint32_t timer = CAN_OPERATE_TIMEOUT;

    /** Ensure the device is not in sleep mode. */
    p_can_regs->CTLR_b.SLPM = CAN_SLEEP_AWAKEN;
    p_can_regs->CTLR_b.CANM = CAN_MODE_CONTROL_NORMAL;                      ///< Switch to Normal operate mode.
    /** Wait for mode to switch. */
    while (((p_can_regs->STR & CAN_CHECK_MODE_MASK) != CAN_CHECK_MODE_NORMAL) && ((--timer) != 0U))
    {
        R_BSP_SoftwareDelay((uint32_t) 1, BSP_DELAY_UNITS_MICROSECONDS);    ///< Delay 1 microsecond
    }

    /** Return true if the device is not in reset, halt or sleep modes */
    return ((p_can_regs->STR & CAN_CHECK_MODE_MASK) == CAN_CHECK_MODE_NORMAL);
}

/*******************************************************************************************************************//**
 * @brief      This function causes the CAN peripheral to go to sleep mode.
 * @param[in]  p_can_regs
 * @retval     false if timed out, true if no timeout
 **********************************************************************************************************************/
static bool HW_CAN_Sleep_Mode (R_CAN0_Type* p_can_regs)
{
    uint32_t timer = CAN_SLEEP_TIMEOUT;

    /** Enter halt mode */
    HW_CAN_Halt_Mode(p_can_regs);

    p_can_regs->CTLR_b.SLPM = CAN_SLEEP_SLEEP;                              ///< Enter sleep mode.
    /** Wait for mode to switch. */
    while ((!p_can_regs->STR_b.SLPST) && ((--timer) != 0U))
    {
        R_BSP_SoftwareDelay((uint32_t) 1, BSP_DELAY_UNITS_MICROSECONDS);    ///< Delay 1 microsecond
    }

    /** Return true if device is in sleep mode */
    return (p_can_regs->STR_b.SLPST == 1U);
}

/*******************************************************************************************************************//**
 * @brief      This function causes the CAN peripheral to exit sleep mode.
 * @param[in]  p_can_regs
 * @retval     false if timed out, true if no timeout
 **********************************************************************************************************************/
static bool HW_CAN_Exit_Sleep_Mode (R_CAN0_Type* p_can_regs)
{
    uint32_t timer = CAN_SLEEP_TIMEOUT;

    p_can_regs->CTLR_b.SLPM = CAN_SLEEP_AWAKEN;                             ///< Exit sleep mode.
    /**  Wait for mode to switch. */
    while ((p_can_regs->STR_b.SLPST) && ((--timer) != 0U))
    {
        R_BSP_SoftwareDelay((uint32_t) 1, BSP_DELAY_UNITS_MICROSECONDS);    ///< Delay 1 microsecond
    }

    /** If the device is out of sleep mode return true if normal mode is entered otherwise return false. */
    if(p_can_regs->STR_b.SLPST == 0U)
    {
        return HW_CAN_Operate_Mode(p_can_regs);
    }
    else
    {
        return false;
    }
}

/*******************************************************************************************************************//**
 * @brief      This function causes the CAN peripheral to go to Listen mode.
 * @param[in]  p_can_regs
 * @retval     false if failed, true if succeeded
 **********************************************************************************************************************/
static bool HW_CAN_Listen_Mode (R_CAN0_Type* p_can_regs)
{
    bool ret_val = true;
    ret_val = HW_CAN_Halt_Mode(p_can_regs);                 ///< Switch to halt mode
    if (true == ret_val)
    {
        p_can_regs->TCR = CAN_TEST_LISTEN_ONLY;             ///< Enable listen only test mode
        ret_val = HW_CAN_Operate_Mode(p_can_regs);          ///< Switch to normal mode
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief      This function causes the CAN peripheral to go to Internal Loopback mode.
 * @param[in]  p_can_regs
 * @retval     false if failed, true if succeeded
 **********************************************************************************************************************/
static bool HW_CAN_Loopback_Internal_Mode (R_CAN0_Type* p_can_regs)
{
    bool ret_val = true;

    ret_val = HW_CAN_Halt_Mode(p_can_regs);                 ///< Switch to halt mode
    if (true == ret_val)
    {
        p_can_regs->TCR = CAN_TEST_LOOPBACK_INTERNAL;       ///< Enable internal loopback test mode
        ret_val = HW_CAN_Operate_Mode(p_can_regs);          ///< Switch to normal mode
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief      This function causes the CAN peripheral to go to External Loopback mode.
 * @param[in]  p_can_regs
 * @retval     false if failed, true if succeeded
 **********************************************************************************************************************/

static bool HW_CAN_Loopback_External_Mode (R_CAN0_Type* p_can_regs)
{
    bool ret_val = true;

    ret_val = HW_CAN_Halt_Mode(p_can_regs);                 ///< Switch to halt mode
    if (true == ret_val)
    {
        p_can_regs->TCR = CAN_TEST_LOOPBACK_EXTERNAL;       ///< Enable external loopback test mode
        ret_val = HW_CAN_Operate_Mode(p_can_regs);          ///< Switch to normal mode
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief      This function is used to switch the CAN peripheral operation and test modes.
 * @param[in]  p_can_regs
 * @param[in]  mode    - operation mode
 * @retval     SSP_SUCCESS                      CAN mode switched
 * @retval     SSP_ERR_CAN_MODE_SWITCH_FAILED   CAN mode switch failed
 **********************************************************************************************************************/

ssp_err_t HW_CAN_Control (R_CAN0_Type* p_can_regs, can_mode_t mode)
{
    bool ret_val;
    ssp_err_t err = SSP_SUCCESS;

    ret_val = true;
    switch (mode)
    {
        case CAN_MODE_NORMAL:
            p_can_regs->TCR = CAN_TEST_NORMAL;          ///< Disable test modes
            ret_val = HW_CAN_Operate_Mode(p_can_regs);  ///< Switch to Normal mode.
        break;

        case CAN_MODE_HALT:                             ///< Switch to HALT mode.
            ret_val = HW_CAN_Halt_Mode(p_can_regs);
        break;

        case CAN_MODE_SLEEP:                            ///< Switch to SLEEP mode.
            ret_val = HW_CAN_Sleep_Mode(p_can_regs);
        break;

        case CAN_MODE_EXIT_SLEEP:                       ///< Exit SLEEP mode.
            ret_val = HW_CAN_Exit_Sleep_Mode(p_can_regs);
        break;

        case CAN_MODE_RESET:                            ///< Switch to RESET mode.
            ret_val = HW_CAN_Reset_Mode(p_can_regs);
        break;

        case CAN_MODE_LISTEN:                           ///< Switch to LISTEN ONLY mode.
            ret_val = HW_CAN_Listen_Mode(p_can_regs);
        break;

        case CAN_MODE_LOOPBACK_INTERNAL:                ///< Switch to INTERNAL LOOPBACK mode.
            ret_val = HW_CAN_Loopback_Internal_Mode(p_can_regs);
        break;

        case CAN_MODE_LOOPBACK_EXTERNAL:                ///< Switch to EXTERNAL LOOPBACK mode.
            ret_val = HW_CAN_Loopback_External_Mode(p_can_regs);
        break;

        default:
            ret_val = false;                            ///< Invalid mode selected.
        break;
    }
    if (ret_val == false)
    {
        err = SSP_ERR_CAN_MODE_SWITCH_FAILED;
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief      This function sets the id for all receive mailboxes.
 * @param[in]  p_can_regs   CAN registers
 * @param[in]  count        Mailbox count
 * @param[in]  p_mailbox    List of mailboxes
 * @param[in]  id_mode      Standard or extended id mode
 **********************************************************************************************************************/
void HW_CAN_MailboxIdSet (R_CAN0_Type   *       p_can_regs, 
                          uint32_t              count, 
                          can_mailbox_t * const p_mailbox, 
                          can_id_mode_t         id_mode)
{
    uint32_t i;

    /** Set mailbox ids for all receive mailboxes. */
    for (i = 0U; i < count; i++)
    {
        if (CAN_MAILBOX_RECEIVE == (p_mailbox[i].mailbox_type))
        {
            /** Clear RX control register */
            p_can_regs->MCTLn_RX[i] = 0x00U;

            /** Set the receive ID depending on the configured mode type. */
            if (CAN_ID_MODE_STANDARD == id_mode)
            {
                p_can_regs->MBn[i].MBn_ID_b.SID = (p_mailbox[i].mailbox_id & CAN_SID_MASK);
            }
            else
            {
                p_can_regs->MBn[i].MBn_ID = (p_mailbox[i].mailbox_id & CAN_XID_MASK);
            }

            /** The IDE bit is enabled when the CTLR.IDFM[1:0] bits are 10b (mixed ID mode).
             * When the IDFM[1:0] bits are not 10b, only write 0 to IDE. It reads as 0. */
            p_can_regs->MBn[i].MBn_ID_b.IDE = 0U;

            /** Set receive mailbox for either Data or Remote frame type. */
            p_can_regs->MBn[i].MBn_ID_b.RTR = p_mailbox[i].frame_type;

            /** Clear NEWDATA, Mailbox configured for receive */
            p_can_regs->MCTLn_RX[i] = CAN_MAILBOX_RX;
        }
        if (CAN_MAILBOX_TRANSMIT == (p_mailbox[i].mailbox_type))
        {
            /** Clear NEWDATA, Mailbox not configured for receive */
            p_can_regs->MCTLn_TX[i] = 0x00U;
        }
    }
    /** Do not configure unused mailboxes for receive */
    for (i = count; i < CAN_MAX_NO_MAILBOXES; i++)
    {
        /** Clear NEWDATA, Mailbox not configured for receive */
        p_can_regs->MCTLn_TX[i] = 0x00U;
    }
}

/*******************************************************************************************************************//**
 * @brief      This function sets the mask for all receive mailboxes.
 * @param[in]  p_can_regs     CAN registers
 * @param[in]  count          Number of mailboxes to configure
 * @param[in]  p_mailbox_mask List of mailbox masks
 * @param[in]  id_mode        Standard or extended id mode
 **********************************************************************************************************************/
void HW_CAN_MailboxMaskSet (R_CAN0_Type   *       p_can_regs, 
                            uint32_t              count, 
                            uint32_t      * const p_mailbox_mask,
                            can_id_mode_t         id_mode)
{
    uint32_t i;
    uint32_t mask;
    uint32_t group_mask;
    uint32_t mask_enabled;

    /** Set all masks to disabled. */
    mask_enabled = 0U;

    /** Initialize group mask used to mark the groups mask as valid. */
    group_mask = CAN_GROUP_MASK;

    /** Set mask for each group of mailboxes the user is using */
    for (i = 0U; i < (count / CAN_MAILBOX_GROUP_SIZE); i++)
    {
        /** Read user configured mask for group. */
        mask = p_mailbox_mask[i];

        /**  If the user has defined a mask */
        if (mask < CAN_DEFAULT_MASK)
        {
            if (CAN_ID_MODE_STANDARD == id_mode)
            {
                /** Set standard ID mask. Set unused bits high */
                p_can_regs->MKRn[i] = CAN_DEFAULT_MASK;
                p_can_regs->MKRn_b[i].SID = (mask & CAN_SID_MASK);
            }
            else
            {
                /** Set extended ID mask. Set unused bits high */
                p_can_regs->MKRn[i] = (mask & CAN_XID_MASK);
            }
            /** Enable the mask for this group */
            mask_enabled |= group_mask;
        }

        /** Shift the group mask for marking the mailbox masks as enabled */
        group_mask = group_mask << CAN_MAILBOX_GROUP_SIZE;
    }

    /** Set the Mask as invalid for mailboxes that do not use the mask. */
    p_can_regs->MKIVLR = ~(mask_enabled);
}

/*******************************************************************************************************************//**
 * @brief      This function resets the timestamp.
 * @param[in]  p_can_regs   CAN registers
 * @retval     True if timestamp was reset, false if timed out.
 **********************************************************************************************************************/
bool HW_CAN_TimeStampReset (R_CAN0_Type* p_can_regs)
{
    uint32_t timer = CAN_RESET_TIMEOUT;

    p_can_regs->CTLR_b.TSRC = CAN_TIMESTAMP_RESET;                              ///< Set Timestamp counter reset command
    while ((p_can_regs->CTLR_b.TSRC == CAN_TIMESTAMP_RESET) && ((--timer) != 0U)) ///< Wait for timestamp to reset
    {
        R_BSP_SoftwareDelay((uint32_t) 1, BSP_DELAY_UNITS_MICROSECONDS);        ///< Delay 1 microsecond
    }

    return (p_can_regs->CTLR_b.TSRC == CAN_TIMESTAMP_RUNNING);
}

/*******************************************************************************************************************//**
 * @brief      This function returns the bit rate.
 * @param[in]  p_can_regs           CAN registers
 * @param[out] p_frequency          Bit rate of the CAN device
 * @retval     false if errors, true if no errors
 *         This function calls:
 *                                   * cgc_api_t::systemClockFreqGet
 **********************************************************************************************************************/
bool HW_CAN_BitRateGet (R_CAN0_Type* p_can_regs, uint32_t* const p_frequency)
{
    ssp_err_t err;
    uint32_t clock_frequency =
    { 0U };
    uint32_t time_segment;
    const cgc_api_t* pcgc = &g_cgc_on_cgc;
    err = SSP_SUCCESS;

    /** If PCLKB is being used */
    if (CAN_CLOCK_SOURCE_PCLKB == p_can_regs->BCR_b.CCLKS)
    {
        /** Get the clock frequency from cgc */
        err = pcgc->systemClockFreqGet(CGC_SYSTEM_CLOCKS_PCLKB, &clock_frequency);

        /** If getting the clock frequency failed return false */
        if (err != SSP_SUCCESS)
        {
            return false;
        }
    }
    else  ///< Clock source is Main Oscillator (S7 and S3 only).
    {
        /** Otherwise use the external clock */
        clock_frequency = BSP_CFG_XTAL_HZ;
    }

    /** Calculate the baudrate */
    time_segment = (p_can_regs->BCR_b.TSEG1 + 1U);
    time_segment += (p_can_regs->BCR_b.TSEG2 + 1U);
    time_segment += 1U;
    *p_frequency = (clock_frequency / (p_can_regs->BCR_b.BRP + 1U)) / time_segment;

    return true;
}

/*******************************************************************************************************************//**
 * @brief      This function clears all mailboxes.
 * @param[in]  p_can_regs       CAN registers
 * @retval     none
 **********************************************************************************************************************/
void HW_CAN_MailboxesClear (R_CAN0_Type* p_can_regs)
{
    uint32_t i;
    uint32_t j;
    for (i = 0U; i < CAN_MAX_NO_MAILBOXES; i++)
    {
        p_can_regs->MBn[i].MBn_ID = 0x00U;              ///< Clear ID.
        p_can_regs->MBn[i].MBn_DL_b.DLC = 0x0000U;      ///< Clear data length code.
        for (j = 0U; j < CAN_MAX_DATA_LENGTH; j++)
        {
            p_can_regs->MBn[i].MBn_D[j] = 0x00U;        ///< Clear data.
        }

        p_can_regs->MBn[i].MBn_TS = 0x0000U;        ///< Clear timestamp.
    }
}

/*******************************************************************************************************************//**
 * @brief      This function returns the received id from the designated
 *mailbox.
 * @param[in]  p_can_regs   CAN registers
 * @param[in]  id_mode      Standard or extended id mode
 * @param[in]  mailbox      Mailbox number
 * @retval     ID value of message
 **********************************************************************************************************************/
can_id_t HW_CAN_ReceiveIdGet (R_CAN0_Type* p_can_regs, can_id_mode_t id_mode, uint32_t mailbox)
{
    can_id_t id;

    if (CAN_ID_MODE_STANDARD == id_mode)                ///< Read standard id or extended id?
    {
        id = p_can_regs->MBn[mailbox].MBn_ID_b.SID;     ///< Read standard id.
    }
    else
    {
        id = p_can_regs->MBn[mailbox].MBn_ID;           ///< Read extended id.
    }

    return id;
}

/*******************************************************************************************************************//**
 * @brief      This function gets the type (send or receive) for the selected
 *mailbox.
 * @param[in]  p_can_regs   CAN registers
 * @param[in]  mailbox      Mailbox number
 * @retval     CAN_MAILBOX_RECEIVE if receive mailbox
 * @retval     CAN_MAILBOX_TRANSMIT if transmit mailbox.
 **********************************************************************************************************************/
can_mailbox_send_receive_t HW_CAN_MailboxTypeGet (R_CAN0_Type* p_can_regs, uint32_t mailbox)
{
    /** Check for setup as a receive mailbox. */
    if (CAN_MAILBOX_RX != (p_can_regs->MCTLn_TX[mailbox] & CAN_MAILBOX_RX))
    {
        return CAN_MAILBOX_TRANSMIT;    ///< Mailbox is a transmit mailbox.
    }
    else
    {
        return CAN_MAILBOX_RECEIVE;     ///< Mailbox is a receive mailbox.
    }
}

/*******************************************************************************************************************//**
 * @brief      This function returns the receive data count.
 * @param[in]  p_can_regs       CAN registers
 * @param[in]  mailbox          Mailbox number
 * @retval     data length
 **********************************************************************************************************************/
uint8_t HW_CAN_ReceiveDataCountGet (R_CAN0_Type* p_can_regs, uint32_t mailbox)
{
    uint8_t data_length_code = p_can_regs->MBn[mailbox].MBn_DL_b.DLC;

    /** If the Data Length Code is greater than 8 the lower 3 bits are don't care */
    if (data_length_code > CAN_MAX_DATA_LENGTH)
    {
        data_length_code = CAN_MAX_DATA_LENGTH;
    }
    return data_length_code;  ///< Return Data Length Count
}

/*******************************************************************************************************************//**
 * @brief      This function reads the receive mailbox data to the supplied
 *buffer pointer.
 * @param[in]  p_can_regs   CAN registers
 * @param[in]  mailbox      Mailbox number
 * @param[in]  id_mode      Standard or extended ID
 * @param[out] p_frame      Pointer to a can frame struct for populating
 * @retval     none
 **********************************************************************************************************************/
void HW_CAN_ReceiveDataGet (R_CAN0_Type   *       p_can_regs, 
                            uint32_t              mailbox, 
                            can_id_mode_t         id_mode,
                            can_frame_t   * const p_frame)
{
    uint32_t i;

    /** Get the frame type */
    p_frame->type = (can_frame_type_t) p_can_regs->MBn[mailbox].MBn_ID_b.RTR;
    /** Get the frame id */
    p_frame->id = HW_CAN_ReceiveIdGet(p_can_regs, id_mode, mailbox);
    /** Get the frame data length code */
    p_frame->data_length_code = HW_CAN_ReceiveDataCountGet(p_can_regs, mailbox);

    /** Be sure to check data_length_code in calling function */
    for (i = 0U; i < p_frame->data_length_code; i++)
    {
        p_frame->data[i] = p_can_regs->MBn[mailbox].MBn_D[i];  ///< Copy receive data to buffer.
    }

    /** Clear rx data flag */
    p_can_regs->MCTLn_RX[mailbox] = CAN_MAILBOX_RX;
}

/*******************************************************************************************************************//**
 * @brief      This function sends a mailbox frame.
 * @param[in]   p_can_regs      CAN registers
 * @param[in]   mailbox         Mailbox number
 * @param[in]   p_frame         Frame to transmit
 * @param[in]   id_mode         ID mode of the frame
 * @retval     none
 **********************************************************************************************************************/
void HW_CAN_TransmitFrameSend (R_CAN0_Type   *       p_can_regs, 
                               uint32_t              mailbox, 
                               can_frame_t   * const p_frame,
                               can_id_mode_t         id_mode)
{
    uint32_t i;

    /** Set the ID based on the ID mode */
    if (CAN_ID_MODE_STANDARD == id_mode)
    {
        p_can_regs->MBn[mailbox].MBn_ID_b.SID = (p_frame->id & CAN_SID_MASK);   ///< Set standard id.
    }
    else
    {
        p_can_regs->MBn[mailbox].MBn_ID = (p_frame->id & CAN_XID_MASK);         ///< Set extended id.
    }

    /** Put mailbox data length code */
    p_can_regs->MBn[mailbox].MBn_DL_b.DLC = (p_frame->data_length_code & 0x0fU);

    /** Put mailbox data. */
    for (i = 0U; i < p_frame->data_length_code; i++)
    {
        p_can_regs->MBn[mailbox].MBn_D[i] = p_frame->data[i];
    }

    /** Set frame type to Data or Remote frame type. */
    p_can_regs->MBn[mailbox].MBn_ID_b.RTR = p_frame->type;

    /** Clear SentData flag since we are about to send anew.
     * Do a byte-write to avoid read-modify-write with HW writing another bit in between.
     * TRMREQ/RECREQ already 0 after can_wait_tx_rx(). (No need to write twice).
     */
    p_can_regs->MCTLn_TX[mailbox] = 0U;
    p_can_regs->MCTLn_TX_b[mailbox].TRMREQ = 1U;                                ///< Send data
}

/*******************************************************************************************************************//**
 * @brief      This function returns the errors from the error interrupt.
 * @param[in]       p_can_regs      CAN registers
 * @param[out]      p_status        Interrupt status struct
 **********************************************************************************************************************/
void HW_CAN_InterruptErrorGet (R_CAN0_Type* p_can_regs, can_interrrupt_status_t* const p_status)
{
    p_status->status = (uint32_t) p_can_regs->EIFR;     ///< Read Error Interrupt Factor Judge register.
    /** *Clear errors if errors occurred. */
    if (0U != p_status->status)
    {
        p_can_regs->EIFR = 0x00U;                       ///< Clear Error Interrupt Factor Judge register.
    }
}

/*******************************************************************************************************************//**
 * @brief      This function returns the status of the CAN channel.
 * @param[in]   p_can_regs      CAN registers
 * @param[out]  p_info          CAN info struct
 * @retval     false if errors, true if no errors
 **********************************************************************************************************************/
bool HW_CAN_StatusGet (R_CAN0_Type* p_can_regs, can_info_t* const p_info)
{
    uint32_t bit_rate;
    bool ret_val;
    ret_val = true;
    bit_rate = 0U;

    /** Report Status register data. */
    p_info->status.status = (uint32_t) p_can_regs->STR;

    /** Report lowest mailbox with new data */
    p_info->status.status_b.new_data_mailbox = 0U;
    if (p_info->status.status_b.new_data)
    {
        /** Search for the lowest numbered mailbox with a message received. */
        p_can_regs->MSMR = CAN_RECEIVE_SEARCH;
        /** Get the mailbox number. */
        p_info->status.status_b.new_data_mailbox = p_can_regs->MSSR_b.MBNST;
    }

    /** Report lowest mailbox with message lost */
    p_info->status.status_b.msg_lost_mailbox = 0U;
    if (p_info->status.status_b.message_lost)
    {
        /** Search for the lowest numbered mailbox with a message lost */
        p_can_regs->MSMR = CAN_ERROR_SEARCH;
        /** Get the mailbox number */
        p_info->status.status_b.msg_lost_mailbox = p_can_regs->MSSR_b.MBNST;
    }

    /** Report test modes. */
    p_info->status.status_b.listen_only = (p_can_regs->TCR == CAN_TEST_LISTEN_ONLY) ? 1U : 0U;
    p_info->status.status_b.internal_loopback = (p_can_regs->TCR == CAN_TEST_LOOPBACK_INTERNAL) ? 1U : 0U;
    p_info->status.status_b.external_loopback = (p_can_regs->TCR == CAN_TEST_LOOPBACK_EXTERNAL) ? 1U : 0U;

    p_info->error_count_receive = p_can_regs->RECR;             ///< Report receive error count.
    p_info->error_count_transmit = p_can_regs->TECR;            ///< Report transmit error count.
    p_info->error_code.error = (uint32_t) p_can_regs->ECSR;     ///< Report error code.

    /* Since the error flags were read, we clear them to ensure that we won't read them again
     * Because register is volatile, we clear only the flags that we already read. Preserve the value
     * of EDPM bit. Clear error bits by writing 0 to bits that were found to be 1
     */
    p_can_regs->ECSR = (uint8_t)(p_info->error_code.error) ^ CAN_ERROR_MASK;

    if (true != HW_CAN_BitRateGet(p_can_regs, &bit_rate))
    {
        ret_val = false;                                        ///< Error occurred.
    }
    else
    {
        p_info->bit_rate = bit_rate;                            ///< Bit rate is correct value.
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief      This function returns CAN errors.
 * @param[in]  p_can_regs       CAN registers
 * @retval     false if no errors, true if errors
 **********************************************************************************************************************/
bool HW_CAN_ErrorsGet (R_CAN0_Type* p_can_regs)
{
    bool ret_val;
    ret_val = false;

    /** Check Error Status FLag */
    if (p_can_regs->STR_b.EST != 0U)
    {
        ret_val = true;
    }

    /** Clear Error Interrupt Factor Judge Register. */
    if (p_can_regs->EIFR != 0U)
    {
        ret_val = true;
        p_can_regs->EIFR = 0x0U;
    }

    /** Clear Error Code Store Register. */
    if ((p_can_regs->ECSR & CAN_ERROR_MASK) != 0U)
    {
        ret_val = true;
        p_can_regs->ECSR = 0x0U;
    }

    return ret_val;
}
