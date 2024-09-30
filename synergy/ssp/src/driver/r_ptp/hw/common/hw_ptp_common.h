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
 * File Name    : hw_ptp_common.h
 * Description  : PTP Module hardware common header file.
 **********************************************************************************************************************/

#ifndef HW_PTP_COMMON_H
#define HW_PTP_COMMON_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
Macro definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
Typedef definitions
 ***********************************************************************************************************************/

/** Defines the register settings for SYNFP filter 1 */
typedef enum e_ptp_synfp_filter1
{
    PTP_SYNFP_FILTER1_E2E_STOP                 = (0x00000001U),
    PTP_SYNFP_FILTER1_P2P_SLAVE_STOP           = (0x44400001U),
    PTP_SYNFP_FILTER1_E2E_MASTER_START         = (0x00004001U),
    PTP_SYNFP_FILTER1_P2P_MASTER_START         = (0x44400000U),
    PTP_SYNFP_FILTER1_E2E_SLAVE_START          = (0x00040441U),
    PTP_SYNFP_FILTER1_P2P_SLAVE_START          = (0x44400441U),
    PTP_SYNFP_FILTER1_LISTENING_START          = (0x44400001U),
    PTP_SYNFP_FILTER1_P2P_TC_START             = (0x44400222U),
    PTP_SYNFP_FILTER1_E2E_TC_START             = (0x22222222U),
} ptp_synfp_filter1_t;

/** Transmission enable register definitions */
typedef enum e_ptp_synfp_trans_enable
{
    PTP_SYNFP_TRANS_ENABLE_E2E_MASTER  = (0x00000011U),
    PTP_SYNFP_TRANS_ENABLE_P2P_MASTER  = (0x00001011U),
    PTP_SYNFP_TRANS_ENABLE_E2E_SLAVE   = (0x00000100U),
    PTP_SYNFP_TRANS_ENABLE_P2P         = (0x00001000U)
} ptp_synfp_trans_enable_t;

/** SYNFP status register definitions */
typedef enum e_ptp_sysr_status
{
    PTP_SYSR_STATUS_SLAVE_START          = (0x00000001U),
    PTP_SYSR_STATUS_E2E_MASTER_STOP      = (0x00030000U),
    PTP_SYSR_STATUS_P2P_MASTER_STOP      = (0x00020000U),
    PTP_SYSR_STATUS_E2E_SLAVE_STOP       = (0x00020000U),
    PTP_SYSR_STATUS_P2P_SLAVE_STOP       = (0x00030000U)
} ptp_sysr_status_t;

/** STCA operation mode definitions */
typedef enum e_ptp_stca_config
{
    PTP_STCA_CONFIG_MODE1              = (0x00000000U),
    PTP_STCA_CONFIG_MODE2_HW           = (0x00002000U),
    PTP_STCA_CONFIG_MODE2_SW           = (0x0000A000U)
} ptp_stca_config_t;

/** Transmission interval boundary definitions */
typedef enum e_ptp_transmission_interval
{
    PTP_TRANSMISSION_INTERVAL_MINIMUM    = (0xF9),
    PTP_TRANSMISSION_INTERVAL_MAXIMUM    = (0x06)
}ptp_transmission_interval_t;

/** PTP clock port definitions */
typedef enum e_ptp_ordinary_clock_port
{
    PTP_CLOCK_PORT_1    = (0x01),
    PTP_CLOCK_PORT_2    = (0x02)
}ptp_clock_port_t;


/***********************************************************************************************************************
Private function prototypes
 ***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables
 ***********************************************************************************************************************/

/***********************************************************************************************************************
Private Functions
 ***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Function to reset EPTPC.
 * @param[in]   p_reg_cfg    R_EPTPC_CFG_Type Base register
 * @param[in]   state        Enable or disable EPTPC reset
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_Reset(R_EPTPC_CFG_Type * p_reg_cfg, bool const state)
{
    /* Enable or disable PTP reset */
    p_reg_cfg->PTRSTR_b.RESET = state;
} /* End of function HW_PTP_Reset() */

/*******************************************************************************************************************//**
 * Function to select STCA Clock.
 * @param[in]   p_reg_cfg      R_EPTPC_CFG_Type Base register
 * @param[in]   clock_set      Sets the STCA clock select value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_STCAClockSelect(R_EPTPC_CFG_Type * p_reg_cfg, uint32_t clock_set)
{
    /* Set STCA Clock Select Register value */
    p_reg_cfg->STCSELR = clock_set;
} /* End of function HW_PTP_STCAClockSelect() */

/*******************************************************************************************************************//**
 * Function for selecting the time synchronization channel register.
 * @param[in]   p_reg_gen   R_EPTPC_GEN_Type Base register
 * @param[in]   channel     Sets the SYNFP channel (0 or 1)
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_TimeSynchChannelSelect(R_EPTPC_GEN_Type * p_reg_gen, uint32_t channel)
{
    /* Sets synchronization frame processing unit */
    p_reg_gen->STCHSELR_b.SYSEL = (uint32_t)(channel & 0x01);
} /* End of function HW_PTP_TimeSynchChannelSelect() */

/*******************************************************************************************************************//**
 * Function for setting the MINT IRQ register.
 * @param[in]   p_reg_gen   R_EPTPC_GEN_Type Base register
 * @param[in]   value       Sets the value of MINT Interrupt Request Enable Register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_MINTIrqRequestSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Sets MINT Interrupt Request Enable Register */
    p_reg_gen->MIEIPR = value;
} /* End of function HW_PTP_MINTIrqRequestSet() */

/*******************************************************************************************************************//**
 * Function for getting the MINT IRQ register.
 * @param[in]   p_reg_gen   R_EPTPC_GEN_Type Base register
 * @param[out]  p_value     Gets the value of MINT Interrupt Request Enable Register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_MINTIrqRequestGet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t * p_value)
{
    /* Gets MINT Interrupt Request Enable Register */
    *p_value= p_reg_gen->MIEIPR;
} /* End of function HW_PTP_MINTIrqRequestGet() */

/*******************************************************************************************************************//**
 * Function for setting the  IPLS IRQ permission register
 * @param[in]   p_reg_gen   R_EPTPC_GEN_Type Base register
 * @param[in]   value       Sets the value of IPLS Interrupt Request Enable Register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_IPLSIrqRequestSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Sets IPLS Interrupt Permission Register */
    p_reg_gen->ELIPPR = value;
} /* End of function HW_PTP_IPLSIrqRequestSet() */

/*******************************************************************************************************************//**
 * Function for getting the IPLS IRQ permission register.
 * @param[in]   p_reg_gen   R_EPTPC_GEN_Type Base register
 * @param[out]  p_value     Gets the value of IPLS Interrupt Request Enable Register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_IPLSIrqRequestGet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t * p_value)
{
    /* Gets IPLS Interrupt Permission Register */
    *p_value= p_reg_gen->ELIPPR;
} /* End of function HW_PTP_IPLSIrqRequestGet() */

/*******************************************************************************************************************//**
 * Function for setting the IPLS IRQ auto-clear register.
 * @param[in]   p_reg_gen   R_EPTPC_GEN_Type Base register
 * @param[in]   value       Sets the value of IPLS Interrupt auto-clear Register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_IPLSIrqAutoClearSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Sets IPLS Interrupt Auto-clear Register */
    p_reg_gen->ELIPACR = value;
} /* End of function HW_PTP_IPLSIrqAutoClearSet() */

/*******************************************************************************************************************//**
 * Function for getting the IPLS IRQ auto-clear register.
 * @param[in]   p_reg_gen   R_EPTPC_GEN_Type Base register
 * @param[out]  p_value     Gets the value of IPLS Interrupt auto-clear Register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_IPLSIrqAutoClearGet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t * p_value)
{
    /* Gets IPLS Interrupt Auto-clear Register */
    *p_value= p_reg_gen->ELIPACR;
} /* End of function HW_PTP_IPLSIrqAutoClearGet() */

/*******************************************************************************************************************//**
 * Function for setting the STCA Status notification register.
 * @param[in]   p_reg_gen   R_EPTPC_GEN_Type Base register
 * @param[in]   value       Sets the value of STCA Status Notification Enable Register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_STCAStatusNotificationSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Sets STCA Status Notification Enable Register */
    p_reg_gen->STIPR = value;
} /* End of function HW_PTP_STCAStatusNotificationSet() */

/*******************************************************************************************************************//**
 * Function for getting the STCA Status notification register.
 * @param[in]   p_reg_gen   R_EPTPC_GEN_Type Base register
 * @param[out]  p_value     Gets the value of STCA Status Notification Enable Register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_STCAStatusNotificationGet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t * p_value)
{
    /* Gets STCA Status Notification Enable Register */
    *p_value = p_reg_gen->STIPR;
} /* End of function HW_PTP_STCAStatusNotificationGet() */

/*******************************************************************************************************************//**
 * Function for setting PRCTC Status Notification register.
 * @param[in]   p_reg_gen   R_EPTPC_GEN_Type Base register
 * @param[in]   value       Sets the value of PRCTC Status Notification register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_PRCTCStatusNotificationSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Sets the value of PRCTC Status Notification register */
    p_reg_gen->PRIPR = value;
} /* End of function HW_PTP_PRCTCStatusNotificationSet() */

/*******************************************************************************************************************//**
 * Function for getting PRCTC Status Notification register.
 * @param[in]   p_reg_gen   R_EPTPC_GEN_Type Base register
 * @param[out]  p_value     Gets the value of PRCTC Status Notification register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_PRCTCStatusNotificationGet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t * p_value)
{
    /* Gets the value of PRCTC Status Notification register */
    *p_value = p_reg_gen->PRIPR;
} /* End of function HW_PTP_PRCTCStatusNotificationGet() */

/*******************************************************************************************************************//**
 * Function for setting MAC address.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   port         PRC-TC port number
 * @param[in]   addr_uppr    Sets the upper bits of MAC address
 * @param[in]   addr_lwr     Sets the lower bits of MAC address
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_MACAddressSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t port, uint32_t addr_uppr, uint32_t addr_lwr)
{
    /* Set the MAC address */
    p_reg_gen->PRMACRUCn[port].PRMACRUn = addr_uppr;
    p_reg_gen->PRMACRUCn[port].PRMACRLn = addr_lwr;
} /* End of function HW_PTP_MACAddressSet() */

/*******************************************************************************************************************//**
 * Function to control PTP packets transmission.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   value        Sets the value to transmit PTP packets to both port
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_PacketTransmissionControl(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Set PTP packets are transmitted to both ports */
    p_reg_gen->TRNDISR = value;
} /* End of function HW_PTP_PacketTransmissionControl() */

/*******************************************************************************************************************//**
 * Function to set STCA operation mode.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   mode         Sets the STCA operation mode register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SyncCorrectionMode(R_EPTPC_GEN_Type * p_reg_gen, uint32_t mode)
{
    /* Sets the STCA operation mode register */
    p_reg_gen->STMR = mode;
} /* End of function HW_PTP_SyncCorrectionMode() */

/*******************************************************************************************************************//**
 * Function to set Synchronization Loss Detection Threshold.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   value_uppr   Sets the higher order bits of loss detection threshold
 * @param[in]   value_lwr    Sets the lower order bits of loss detection threshold
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SyncLossDetection(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value_uppr, uint32_t value_lwr)
{
    /* Set higher and lower order bits of loss detection threshold */
    p_reg_gen->SYNTDARU = value_uppr;
    p_reg_gen->SYNTDARL = value_lwr;
} /* End of function HW_PTP_SyncLossDetection() */

/*******************************************************************************************************************//**
 * Function to set Synchronization Detection Threshold.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   value_uppr   Sets the higher order bits of Synchronization Detection Threshold
 * @param[in]   value_lwr    Sets the lower order bits of Synchronization Detection Threshold
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SyncDetection(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value_uppr, uint32_t value_lwr)
{
    /* Set higher and lower order bits of synchronization detection threshold */
    p_reg_gen->SYNTDBRU = value_uppr;
    p_reg_gen->SYNTDBRL = value_lwr;
} /* End of function HW_PTP_SyncDetection() */

/*******************************************************************************************************************//**
 * Function to set STCA clock frequency.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   freq         Sets the STCA clock frequency
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_STCAClockFrequencySet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t freq)
{
    /* Set STCA Clock Frequency value */
    p_reg_gen->STCFR_b.STCF = (uint32_t)(freq & 0x03);
} /* End of function HW_PTP_STCAClockFrequencySet() */

/*******************************************************************************************************************//**
 * Function to set Sync Message Reception Timeout.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   timeout      Sets the timeout
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SyncMsgReceptionTimeout(R_EPTPC_GEN_Type * p_reg_gen, uint32_t timeout)
{
    /* Set Sync message reception timeout value */
    p_reg_gen->SYNTOR = timeout;
} /* End of function HW_PTP_SyncMsgReceptionTimeout() */

/*******************************************************************************************************************//**
 * Function to set local time counter.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   bits_high    Sets the higher order bits of local clock counter
 * @param[in]   bits_mid     Sets the middle order bits of local clock counter
 * @param[in]   bits_low     Sets the lower order bits of local clock counter
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_LocalClockSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t bits_high,
        uint32_t bits_mid, uint32_t bits_low)
{
    /* Sets local time counter value */
    p_reg_gen->LCIVRU_b.LCIVRU = (uint16_t)bits_high;
    p_reg_gen->LCIVRM = bits_mid;
    p_reg_gen->LCIVRL = bits_low;
} /* End of function HW_PTP_LocalClockSet() */

/*******************************************************************************************************************//**
 * Function to set IPLS interrupt register.
 * @param[in]   p_reg_gen     R_EPTPC_GEN_Type Base register
 * @param[in]   value         Sets the IPLS Interrupt Request Timer Select Register value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_IPLSIrqTimerSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Set IPLS interrupt register value */
    p_reg_gen->IPTSELR = value;
} /* End of function HW_PTP_IPLSIrqTimerSet() */

/*******************************************************************************************************************//**
 * Function to set MINT interrupt timer select register.
 * @param[in]   p_reg_gen     R_EPTPC_GEN_Type Base register
 * @param[in]   value         Sets the MINT Interrupt Request Timer Select Register value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_MINTIrqTimerSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Set MINT interrupt register value */
    p_reg_gen->MITSELR = value;
} /* End of function HW_PTP_MINTIrqTimerSet() */

/*******************************************************************************************************************//**
 * Function to get MINT interrupt timer select register.
 * @param[in]   p_reg_gen     R_EPTPC_GEN_Type Base register
 * @param[out]  p_value       Gets the MINT Interrupt Request Timer Select Register value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_MINTIrqTimerGet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t * p_value)
{
    /* Get MINT interrupt register value */
    *p_value = p_reg_gen->MITSELR;
} /* End of function HW_PTP_MINTIrqTimerGet() */

/*******************************************************************************************************************//**
 * Function to set ELC interrupt register.
 * @param[in]   p_reg_gen      R_EPTPC_GEN_Type Base register
 * @param[in]   value          Sets the ELC Output Timer Select Register value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_ELCIrqTimerSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Set ELC interrupt register value */
    p_reg_gen->ELTSELR = value;
} /* End of function HW_PTP_ELCIrqTimerSet() */

/*******************************************************************************************************************//**
 * Function to set Slave Time Synchronization Control.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   value        Sets the Status Register value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SlaveTimeSyncControl(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Sets Status Register value */
    p_reg_gen->SYNSTARTR_b.STR = (uint32_t)(value & 0x01);
} /* End of function HW_PTP_SlaveTimeSyncControl() */

/*******************************************************************************************************************//**
 * Function to set Pulse output timer start time.
 * @param[in]   p_reg_gen            R_EPTPC_GEN_Type Base register
 * @param[in]   timer_channel        STCA pulse output timer channel
 * @param[in]   time_uppr            Sets the upper bits of timer start time
 * @param[in]   time_lwr             Sets the lower bits of timer start time
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_TimerStartTimeSet(R_EPTPC_GEN_Type * p_reg_gen, uint8_t timer_channel,
        uint32_t time_uppr, uint32_t time_lwr)
{
    /* Set Timer Start Time */
    p_reg_gen->TMRC0[timer_channel].TMSTTRUn = time_uppr;
    p_reg_gen->TMRC0[timer_channel].TMSTTRLn = time_lwr;
} /* End of function HW_PTP_TimerStartTimeSet() */

/*******************************************************************************************************************//**
 * Function to set Pulse output timer cycle.
 * @param[in]   p_reg_gen            R_EPTPC_GEN_Type Base register
 * @param[in]   timer_channel        STCA pulse output timer channel
 * @param[in]   cycle                Sets the timer cycle value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_TimerCycleSet(R_EPTPC_GEN_Type * p_reg_gen, uint8_t timer_channel, uint32_t cycle)
{
    /* Sets the timer cycle value */
    p_reg_gen->TMRC0[timer_channel].TMCYCRn = cycle;
} /* End of function HW_PTP_TimerCycleSet() */

/*******************************************************************************************************************//**
 * Function to set Pulse output timer Pulse Width.
 * @param[in]   p_reg_gen            R_EPTPC_GEN_Type Base register
 * @param[in]   timer_channel        STCA pulse output timer channel
 * @param[in]   pulse_width          Sets the pulse width value for timer
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_TimerPulseWidthSet(R_EPTPC_GEN_Type * p_reg_gen, uint8_t timer_channel, uint32_t pulse_width)
{
    /* set Timer Pulse Width */
    p_reg_gen->TMRC0[timer_channel].TMPLSRn = pulse_width;
} /* End of function HW_PTP_TimerPulseWidthSet() */

/*******************************************************************************************************************//**
 * Function to start Pulse output timer.
 * @param[in]   p_reg_gen      R_EPTPC_GEN_Type Base register
 * @param[in]   value          Sets the timer start register value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_TimerStartSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Set timer start register */
    p_reg_gen->TMSTARTR = value;
} /* End of function HW_PTP_TimerStartSet() */

/*******************************************************************************************************************//**
 * Function to get timer start register.
 * @param[in]   p_reg_gen      R_EPTPC_GEN_Type Base register
 * @param[out]  p_value        Gets the timer start register value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_TimerStartGet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t * p_value)
{
    /* Get timer start register */
    *p_value = p_reg_gen->TMSTARTR;
} /* End of function HW_PTP_TimerStartSet() */

/*******************************************************************************************************************//**
 * Function to set MINT interrupt status
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   value        Sets or clears MINT Interrupt Source Status Register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_MINTInterruptStatusSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Set or clear EPTPC MINT interrupt status Register */
    p_reg_gen->MIESR = value;
} /* End of function HW_PTP_MINTInterruptStatusSet() */

/*******************************************************************************************************************//**
 * Function to get MINT interrupt status
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @retval      value        Return MINT Interrupt Source Status Register
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_PTP_MINTInterruptStatusGet(R_EPTPC_GEN_Type * p_reg_gen)
{
    /* Get MINT Interrupt Source Status Register value */
    uint32_t value = p_reg_gen->MIESR;
    return value;
} /* End of function HW_PTP_MINTInterruptStatusGet() */

/*******************************************************************************************************************//**
 * Function to set STCA status.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   value        Sets STCA Status Register value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_STCAStatusSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Sets STCA Status Register */
    p_reg_gen->STSR = value;
} /* End of function HW_PTP_STCAStatusSet() */

/*******************************************************************************************************************//**
 * Function to get STCA status.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @retval      value        Return STCA Status Register value
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_PTP_STCAStatusGet(R_EPTPC_GEN_Type * p_reg_gen)
{
    /* Get STCA Status Register */
    uint32_t value = p_reg_gen->STSR;
    return value;
} /* End of function HW_PTP_STCAStatusGet() */

/*******************************************************************************************************************//**
 * Function to set PRC-TC status.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   value        Sets PRC-TC Status Register value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_PRCTCStatusSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t value)
{
    /* Set PRC-TC Status Register */
    p_reg_gen->PRSR = value;
} /* End of function HW_PTP_PRCTCStatusSet() */

/*******************************************************************************************************************//**
 * Function to get PRC-TC status.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @retval      value        Return PRC-TC Status Register value
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_PTP_PRCTCStatusGet(R_EPTPC_GEN_Type * p_reg_gen)
{
    /* Get PRC-TC Status Register */
    uint32_t value = p_reg_gen->PRSR;
    return value;
} /* End of function HW_PTP_PRCTCStatusGet() */

/*******************************************************************************************************************//**
 * Function to get current clock information.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[out]  p_info       Pointer to updated clock information
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_InfoGet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t * p_info)
{
    /* Get clock information */
    *p_info = p_reg_gen->GETINFOR;
} /* End of function HW_PTP_InfoGet() */

/*******************************************************************************************************************//**
 * Function to control information retention.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   value        Sets the value to control information retention
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_InfoControl(R_EPTPC_GEN_Type * p_reg_gen, const bool value)
{
    /* Enable or disable Information control retention register */
    p_reg_gen->GETINFOR_b.INFO = value;
} /* End of function HW_PTP_InfoControl() */

/*******************************************************************************************************************//**
 * Function to get local clock counter.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[out]  p_clock      Updated local counter value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_LocalClockGet(R_EPTPC_GEN_Type * p_reg_gen, ptp_timestamp_t * p_clock)
{
    /* Get local counter value */
    p_clock->nanosecondsField = p_reg_gen->LCCVRL;
    p_clock->secondsField.low = p_reg_gen->LCCVRM;
    p_clock->secondsField.high = p_reg_gen->LCCVRU_b.LCCVRU;
} /* End of function HW_PTP_LocalClockGet() */

/*******************************************************************************************************************//**
 * Function to load set value to local clock counter.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   value        Sets the value to load local clock counter
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_LoadSetValue(R_EPTPC_GEN_Type * p_reg_gen, bool value)
{
    /* Load set value to local clock counter */
    p_reg_gen->LCIVLDR_b.LOAD = value;
} /* End of function HW_PTP_LoadSetValue() */

/*******************************************************************************************************************//**
 * Function to set Worst 10 Acquisition Time
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   value        Sets the interval for acquisition of worst 10 values
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_W10AcquisitionTimeSet(R_EPTPC_GEN_Type * p_reg_gen, uint8_t value)
{
    /* Set Worst 10 Acquisition Time */
    p_reg_gen->STMR_b.WINT = value;
} /* End of function HW_PTP_W10AcquisitionTimeSet() */

/*******************************************************************************************************************//**
 * Function to request current worst 10 values acquired.
 * @param[in]   p_reg_gen    R_EPTPC_GEN_Type Base register
 * @param[in]   value        Start or stop acquisition of worst 10 values
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_AcquireW10Values(R_EPTPC_GEN_Type * p_reg_gen, bool value)
{
    /* Request current worst10 values acquired */
    p_reg_gen->GETW10R_b.GW10 = value;
} /* End of function HW_PTP_AcquireW10Values() */

/*******************************************************************************************************************//**
 * Function to get positive worst 10 values.
 * @param[in]   p_reg_gen           R_EPTPC_GEN_Type Base register
 * @param[out]  p_positive_w10      Gets the positive limit for worst 10 values
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_PositiveW10ValuesGet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t * p_positive_w10)
{
    /* Get positive gradient values */
    * p_positive_w10      = p_reg_gen->PW10VRU;
    *(p_positive_w10 + 1) = p_reg_gen->PW10VRM;
    *(p_positive_w10 + 2) = p_reg_gen->PW10VRL;
}/* End of function HW_PTP_PositiveW10ValuesGet() */

/*******************************************************************************************************************//**
 * Function to get negative worst 10 values.
 * @param[in]   p_reg_gen           R_EPTPC_GEN_Type Base register
 * @param[out]  p_negative_w10      Gets the negative limit for worst 10 values
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_NegativeW10ValuesGet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t * p_negative_w10)
{
    /* Get positive gradient values */
    * p_negative_w10      = p_reg_gen->MW10RU;
    *(p_negative_w10 + 1) = p_reg_gen->MW10RM;
    *(p_negative_w10 + 2) = p_reg_gen->MW10RL;
}/* End of function HW_PTP_NegativeW10ValuesGet() */

/*******************************************************************************************************************//**
 * Function to set positive worst 10 values.
 * @param[in]   p_reg_gen           R_EPTPC_GEN_Type Base register
 * @param[in]   p_positive_limit    Sets the positive limit for worst 10 values
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_PositiveW10ValuesSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t * p_positive_limit)
{
    p_reg_gen->PLIMITRU = *p_positive_limit;
    p_reg_gen->PLIMITRM = *(p_positive_limit + 1);
    p_reg_gen->PLIMITRL = *(p_positive_limit + 2);
}/* End of function HW_PTP_PositiveW10ValuesSet() */

/*******************************************************************************************************************//**
 * Function to set negative worst 10 values.
 * @param[in]   p_reg_gen           R_EPTPC_GEN_Type Base register
 * @param[in]   p_negative_limit    Sets the negative limit for worst 10 values
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_NegativeW10ValuesSet(R_EPTPC_GEN_Type * p_reg_gen, uint32_t * p_negative_limit)
{
    p_reg_gen->MLIMITRU = *p_negative_limit;
    p_reg_gen->MLIMITRM = *(p_negative_limit + 1);
    p_reg_gen->MLIMITRL = *(p_negative_limit + 2);
}/* End of function HW_PTP_NegativeW10ValuesSet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP Status Notification register.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the value of SYNFP Status Notification register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPStatusNotificationSet(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Initialize SYNFP */
    p_reg->SYIPR = value;
} /* End of function HW_PTP_SYNFPStatusNotificationSet() */

/*******************************************************************************************************************//**
 * Function for getting SYNFP Status Notification register.
 * @param[in]   p_reg     R_EPTPC0_Type Base register for this channel
 * @param[out]  p_value   Gets the value of SYNFP Status Notification register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPStatusNotificationGet(R_EPTPC0_Type * p_reg, uint32_t * p_value)
{
    /* Get SYNFP Status Notification register */
    *p_value = p_reg->SYIPR;
} /* End of function HW_PTP_SYNFPStatusNotificationGet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP MAC Address register.
 * @param[in]   p_reg      R_EPTPC0_Type Base register for this channel
 * @param[in]   mac_uppr   Sets the upper bits of MAC address
 * @param[in]   mac_lwr    Sets the lower bits of MAC address
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPMACAddressSet(R_EPTPC0_Type * p_reg, uint32_t mac_uppr, uint32_t mac_lwr)
{
    /* Set MAC address */
    p_reg->SYMACRU = mac_uppr;
    p_reg->SYMACRL = mac_lwr;
}/* End of function HW_PTP_SYNFPMACAddressSet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP local IP Address register.
 * @param[in]   p_reg     R_EPTPC0_Type Base register for this channel
 * @param[in]   ip_addr   Sets the IP address
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFP_IPAddressSet(R_EPTPC0_Type * p_reg, uint32_t ip_addr)
{
    /* Set IP address */
    p_reg->SYIPADDRR = ip_addr;
} /* End of function HW_PTP_SYNFP_IPAddressSet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP LLC-CTL value.
 * @param[in]   p_reg     R_EPTPC0_Type Base register for this channel
 * @param[in]   value     Sets control field in the LLC sub-layer when generating IEEE802.3 frames
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFP_LLC_CTLSet(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Set LLC-CTL value */
    p_reg->SYLLCCTLR = value;
} /* End of function HW_PTP_SYNFP_LLC_CTLSet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP Version register.
 * @param[in]   p_reg     R_EPTPC0_Type Base register for this channel
 * @param[in]   version   Sets the SYNFP version
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPVersionSet(R_EPTPC0_Type * p_reg, uint32_t version)
{
    /* Set SYNFP version field */
    p_reg->SYSPVRR_b.VER = (uint32_t)(version & 0x0F);
} /* End of function HW_PTP_SYNFPVersionSet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP Domain number register.
 * @param[in]   p_reg        R_EPTPC0_Type Base register for this channel
 * @param[in]   domain_num   Sets the SYNFP domain number
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPDomainNumberSet(R_EPTPC0_Type * p_reg, uint8_t domain_num)
{
    /* Set SYNFP Domain number */
    p_reg->SYDOMR_b.DNUM = domain_num;
} /* End of function HW_PTP_SYNFPDomainNumberSet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP Announce flag field register.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the SYNFP Announce flag field register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_AnnounceFlagFieldInitSet(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Set announce message flag field */
    p_reg->ANFR = value;
} /* End of function HW_PTP_AnnounceFlagFieldInitSet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP Announce flag bits register.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @param[in]   value    Sets the SYNFP Announce flag field register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_AnnounceFlagFieldSet(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Update PTP announce message flag fields */
    p_reg->ANFR = value;
} /* End of function HW_PTP_AnnounceFlagFieldSet() */

/*******************************************************************************************************************//**
 * Function for setting Sync message flag field register.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the sync message flag field register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SyncFlagFieldSet(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Set sync message flag field */
    p_reg->SYNFR = value;
} /* End of function HW_PTP_SyncFlagFieldSet() */

/*******************************************************************************************************************//**
 * Function for setting Delay request message flag field register.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the delay request message flag field register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_DelayReqFlagFieldSet(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Set delay request message flag field */
    p_reg->DYRQFR = value;
} /* End of function HW_PTP_DelayReqFlagFieldSet() */

/*******************************************************************************************************************//**
 * Function for setting Delay response message flag field register.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the delay response message flag field register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_DelayRespFlagFieldSet(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Set delay response message flag field */
    p_reg->DYRPFR = value;
} /* End of function HW_PTP_DelayRespFlagFieldSet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP local clock ID register.
 * @param[in]   p_reg     R_EPTPC0_Type Base register for this channel
 * @param[in]   id_uppr   Sets the upper bits of SYNFP local clock ID
 * @param[in]   id_lwr    Sets the lower bits of SYNFP local clock ID
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPLocalClkIDSet(R_EPTPC0_Type * p_reg, uint32_t id_uppr, uint32_t id_lwr)
{
    /* Set SYNFP local clock ID */
    p_reg->SYCIDRU = id_uppr;
    p_reg->SYCIDRL = id_lwr;
} /* End of function HW_PTP_SYNFPLocalClkIDSet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP Local port number register.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the value of SYNFP Local port number register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPPortNumSet(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Set SYNFP local port number */
    p_reg->SYPNUMR_b.PNUM = (uint16_t)value;
} /* End of function HW_PTP_SYNFPPortNumSet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP transmission enable register.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the value of SYNFP transmission enable register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPTransmissionEnableSet(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Set SYNFP transmission enable field */
    p_reg->SYTRENR = value;
} /* End of function HW_PTP_SYNFPTransmissionEnable() */

/*******************************************************************************************************************//**
 * Function for getting SYNFP transmission enable register.
 * @param[in]   p_reg     R_EPTPC0_Type Base register for this channel
 * @param[out]  p_value   Gets the value of SYNFP transmission enable register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPTransmissionEnableGet(R_EPTPC0_Type * p_reg, uint32_t * p_value)
{
    /* Get SYNFP transmission enable field */
    *p_value = p_reg->SYTRENR;
} /* End of function HW_PTP_SYNFPTransmissionEnable() */

/*******************************************************************************************************************//**
 * Function for setting Master clock ID register.
 * @param[in]   p_reg     R_EPTPC0_Type Base register for this channel
 * @param[in]   id_uppr   Sets the upper bits of master clock ID
 * @param[in]   id_lwr    Sets the lower bits of master clock ID
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_MasterClockIDSet(R_EPTPC0_Type * p_reg, uint32_t id_uppr, uint32_t id_lwr)
{
    /* Set master clock ID */
    p_reg->MTCIDU = id_uppr;
    p_reg->MTCIDL = id_lwr;
} /* End of function HW_PTP_MasterClockIDSet() */

/*******************************************************************************************************************//**
 * Function for setting Master clock port number register.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the value of master clock port number register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_MasterClkPortNumberSet(R_EPTPC0_Type * p_reg, uint16_t value)
{
    /* Set master clock port number */
    p_reg->MTPID_b.PNUM = value;
} /* End of function HW_PTP_MasterClkPortNumberSet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP Announce message Transmission Interval register.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the value of SYNFP Announce message Transmission Interval register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPAnnounceIntervalSet(R_EPTPC0_Type * p_reg, uint8_t value)
{
    /* Set SYNFP announce message Transmission Interval */
    p_reg->SYTLIR_b.ANCE = value;
} /* End of function HW_PTP_SYNFPAnnounceIntervalSet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP Sync message Transmission Interval register.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the value of SYNFP Sync message Transmission Interval register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPSyncIntervalSet(R_EPTPC0_Type * p_reg, uint8_t value)
{
    /* Set SYNFP sync message Transmission Interval */
    p_reg->SYTLIR_b.SYNC = value;
} /* End of function HW_PTP_SYNFPSyncIntervalSet() */

/*******************************************************************************************************************//**
 * Function for setting SYNFP Delay request Transmission Interval register.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the value of SYNFP Delay request Transmission Interval register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPDelayReqIntervalSet(R_EPTPC0_Type * p_reg, uint8_t value)
{
    /* Set SYNFP delay request transmission interval */
    p_reg->SYTLIR_b.DREQ = value;
} /* End of function HW_PTP_SYNFPDelayReqIntervalSet() */

/*******************************************************************************************************************//**
 * Function for getting SYNFP Delay request Transmission Interval register.
 * @param[in]   p_reg        R_EPTPC0_Type Base register for this channel
 * @param[out]  p_interval   Gets the value of SYNFP Delay request Transmission Interval register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPDelayReqIntervalGet(R_EPTPC0_Type * p_reg, int8_t * p_interval)
{
    /* Get SYNFP delay request transmission interval */
    *p_interval = (int8_t)p_reg->SYTLIR_b.DREQ;
} /* End of function HW_PTP_SYNFPDelayReqIntervalGet() */

/*******************************************************************************************************************//**
 * Function for getting SYNFP Delay response Transmission Interval register.
 * @param[in]   p_reg        R_EPTPC0_Type Base register for this channel
 * @param[out]  p_interval   Gets the value of SYNFP Delay response Transmission Interval register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPDelayRespIntervalGet(R_EPTPC0_Type * p_reg, int8_t * p_interval)
{
    /* Get SYNFP delay response transmission interval */
    *p_interval = (int8_t)p_reg->SYRLIR_b.DRESP;
} /* End of function HW_PTP_SYNFPDelayRespIntervalGet() */

/*******************************************************************************************************************//**
 * Function for setting Grand Master Priority 1 field register.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the value of Grand Master Priority 1 field register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_GrandMasterPriority1Set(R_EPTPC0_Type * p_reg, uint8_t value)
{
    /* Set Grand master priority 1 field */
    p_reg->GMPR_b.GMPR1 = value;
} /* End of function HW_PTP_GrandMasterPriority1Set() */

/*******************************************************************************************************************//**
 * Function for setting Grand Master Priority 2 field register.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the value of Grand Master Priority 2 field register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_GrandMasterPriority2Set(R_EPTPC0_Type * p_reg, uint8_t value)
{
    /* Set Grand master priority 2 field */
    p_reg->GMPR_b.GMPR2 = value;
} /* End of function HW_PTP_GrandMasterPriority2Set() */

/*******************************************************************************************************************//**
 * Function for setting grandmasterIdentity Field Setting Register.
 * @param[in]   p_reg     R_EPTPC0_Type Base register for this channel
 * @param[in]   id_uppr   Sets the upper bits of grandmasterIdentity Field Setting Register
 * @param[in]   id_lwr    Sets the lower bits of grandmasterIdentity Field Setting Register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_GrandMasterIdentityFieldSet(R_EPTPC0_Type * p_reg, uint32_t id_uppr, uint32_t id_lwr)
{
    /* Set Grand master identity field */
    p_reg->GMIDRU = id_uppr;
    p_reg->GMIDRL = id_lwr;
} /* End of function HW_PTP_GrandMasterIdentityFieldSet() */

/*******************************************************************************************************************//**
 * Function for setting grandmasterClockQuality.
 * @param[in]   p_reg   R_EPTPC0_Type Base register for this channel
 * @param[in]   value   Sets the value of grand master clock quality
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_GMClockQuality(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Set Grand master clock quality value */
    p_reg->GMCQR = value;
} /* End of function HW_PTP_GMClockQuality() */

/*******************************************************************************************************************//**
 * Function for setting current UTC Offset field.
 * @param[in]   p_reg        R_EPTPC0_Type Base register for this channel
 * @param[in]   utc_offset   Sets the UTC offset value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SetUtcOffset(R_EPTPC0_Type * p_reg, uint16_t utc_offset)
{
    /* Set current UtcOffset value */
    p_reg->CUOTSR_b.CUTO = utc_offset;
} /* End of function HW_PTP_SetUtcOffset() */

/*******************************************************************************************************************//**
 * Function for setting time Source field.
 * @param[in]   p_reg         R_EPTPC0_Type Base register for this channel
 * @param[in]   time_source   Sets the time source value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SetTimeSource(R_EPTPC0_Type * p_reg, uint8_t time_source)
{
    /* Set time Source value */
    p_reg->CUOTSR_b.TSRC = time_source;
} /* End of function HW_PTP_SetTimeSource() */

/*******************************************************************************************************************//**
 * Function for setting stepsRemoved field.
 * @param[in]   p_reg           R_EPTPC0_Type Base register for this channel
 * @param[in]   steps_removed   Sets the value of the stepsRemoved fields of Announce messages
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_StepsRemovedSet(R_EPTPC0_Type * p_reg, uint16_t steps_removed)
{
    /* Set steps removed field */
    p_reg->SRR_b.SRMV = steps_removed;
} /* End of function HW_PTP_StepsRemovedSet() */

/*******************************************************************************************************************//**
 * Function for setting PTP primary message MAC address.
 * @param[in]   p_reg         R_EPTPC0_Type Base register for this channel
 * @param[in]   mac_id_uppr   Sets the upper bits of MAC address
 * @param[in]   mac_id_lwr    Sets the lower bits of MAC address
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_PrimaryMessageMACaddressSet(R_EPTPC0_Type * p_reg, uint32_t mac_id_uppr, uint32_t mac_id_lwr)
{
    /* Set PTP message MAC address */
    p_reg->PPMACRU = mac_id_uppr;
    p_reg->PPMACRL = mac_id_lwr;
} /* End of function HW_PTP_PrimaryMessageMACaddressSet() */

/*******************************************************************************************************************//**
 * Function for setting PTP pDelay message MAC address.
 * @param[in]   p_reg         R_EPTPC0_Type Base register for this channel
 * @param[in]   mac_id_uppr   Sets the upper bits of MAC address
 * @param[in]   mac_id_lwr    Sets the lower bits of MAC address
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_pDelayMessageMACaddressSet(R_EPTPC0_Type * p_reg, uint32_t mac_id_uppr, uint32_t mac_id_lwr)
{
    /* Set PTP delay message MAC address */
    p_reg->PDMACRU = mac_id_uppr;
    p_reg->PDMACRL = mac_id_lwr;
} /* End of function HW_PTP_pDelayMessageMACaddressSet() */

/*******************************************************************************************************************//**
 * Function for setting PTP Ethernet type.
 * @param[in]   p_reg       R_EPTPC0_Type Base register for this channel
 * @param[in]   ether_type  Sets the Ethernet type for PTP message reception
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_Ethertype(R_EPTPC0_Type * p_reg, uint16_t ether_type)
{
    /* Set PTP Ethernet type */
    p_reg->PETYPER_b.PETYPER = ether_type;
} /* End of function HW_PTP_Ethertype() */

/*******************************************************************************************************************//**
 * Function for setting PTP primary message IP address.
 * @param[in]   p_reg     R_EPTPC0_Type Base register for this channel
 * @param[in]   ip_addr   Sets the primary message IP value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_PrimaryMessageIPaddressSet(R_EPTPC0_Type * p_reg, uint32_t ip_addr)
{
    /* Set PTP primary message IP address */
    p_reg->PPIPR = ip_addr;
} /* End of function HW_PTP_PrimaryMessageIPaddressSet() */

/*******************************************************************************************************************//**
 * Function for setting PTP pDelay message IP address.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @param[in]   ip_addr  Sets the p-delay message IP value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_pDelayMessageIPaddressSet(R_EPTPC0_Type * p_reg, uint32_t ip_addr)
{
    /* Set PTP pdelay message IP address */
    p_reg->PDIPR = ip_addr;
} /* End of function HW_PTP_pDelayMessageIPaddressSet() */

/*******************************************************************************************************************//**
 * Function for setting PTP event message's TOS field.
 * @param[in]   p_reg          R_EPTPC0_Type Base register for this channel
 * @param[in]   event_msg_tos  Sets the PTP event message's TOS field
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_EventMsgTOSField(R_EPTPC0_Type * p_reg, uint8_t event_msg_tos)
{
    /* Set PTP event message's TOS field */
    p_reg->PETOSR_b.EVTO = event_msg_tos;
} /* End of function HW_PTP_EventMsgTOSField() */

/*******************************************************************************************************************//**
 * Function for setting PTP general message's TOS field.
 * @param[in]   p_reg            R_EPTPC0_Type Base register for this channel
 * @param[in]   general_msg_tos  Sets the PTP general message's TOS field
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_GeneralMsgTOSField(R_EPTPC0_Type * p_reg, uint8_t general_msg_tos)
{
    /* Set PTP general message's TOS field */
    p_reg->PGTOSR_b.GETO = general_msg_tos;
} /* End of function HW_PTP_GeneralMsgTOSField() */

/*******************************************************************************************************************//**
 * Function for setting PTP-primary message's TTL field.
 * @param[in]   p_reg            R_EPTPC0_Type Base register for this channel
 * @param[in]   primary_msg_ttl  Sets the PTP-primary message's TTL field
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_PrimaryMsgTTLField(R_EPTPC0_Type * p_reg, uint8_t primary_msg_ttl)
{
    /* Set PTP-primary message's TTL field */
    p_reg->PPTTLR_b.PRTL = primary_msg_ttl;
} /* End of function HW_PTP_PrimaryMsgTTLField() */

/*******************************************************************************************************************//**
 * Function for setting PTP-pdelay message's TTL field.
 * @param[in]   p_reg             R_EPTPC0_Type Base register for this channel
 * @param[in]   pdelay_msg_ttl    Sets the PTP-pdelay message's TTL field
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_pDelayMsgTTLField(R_EPTPC0_Type * p_reg, uint8_t pdelay_msg_ttl)
{
    /* Set PTP-pdelay message's TTL field */
    p_reg->PDTTLR_b.PDTL = pdelay_msg_ttl;
} /* End of function HW_PTP_pDelayMsgTTLField() */

/*******************************************************************************************************************//**
 * Function for setting extended promiscuous mode.
 * @param[in]   p_reg       R_EPTPC0_Type Base register for this channel
 * @param[in]   value       Sets or clear extended promiscuous mode
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_ExtendedPromiscuousModeSet(R_EPTPC0_Type * p_reg, bool value)
{
    /* Sets or clear extended promiscuous mode */
    p_reg->FFLTR_b.EXTPRM = value;
} /* End of function HW_PTP_ExtendedPromiscuousModeSet() */

/*******************************************************************************************************************//**
 * Function for setting UDP port number.
 * @param[in]   p_reg          R_EPTPC0_Type Base register for this channel
 * @param[in]   event_msg      Sets the UDP port for event message
 * @param[in]   general_msg    Sets the UDP port for general message
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_UDPPortSet(R_EPTPC0_Type * p_reg, uint32_t event_msg, uint32_t general_msg)
{
    /* Set UDP port number for event and general message */
    p_reg->PEUDPR_b.EVUPT = (uint16_t)event_msg;
    p_reg->PGUDPR_b.GEUPT = (uint16_t)general_msg;
} /* End of function HW_PTP_UDPPortSet() */

/*******************************************************************************************************************//**
 * Function for setting asymmetric delay.
 * @param[in]   p_reg             R_EPTPC0_Type Base register for this channel
 * @param[in]   delay_uppr        Sets the value of delay higher order bits
 * @param[in]   delay_lwr         Sets the value of delay lower order bits
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SetAsymmetricDelay(R_EPTPC0_Type * p_reg, uint16_t delay_uppr, uint16_t delay_lwr)
{
    /* Set asymmetric delay */
    p_reg->DASYMRU_b.DASYMRU = delay_uppr;
    p_reg->DASYMRL = delay_lwr;
} /* End of function HW_PTP_SetAsymmetricDelay() */

/*******************************************************************************************************************//**
 * Function for setting timestamp latency of ingress and egress ports.
 * @param[in]   p_reg               R_EPTPC0_Type Base register for this channel
 * @param[in]   timestamp_latency   Sets the value of time stamp latency
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SetTimestampLatency(R_EPTPC0_Type * p_reg, uint32_t timestamp_latency)
{
    /* Set timestamp latency of ingress and egress ports */
    p_reg->TSLATR = timestamp_latency;
} /* End of function HW_PTP_SetTimestampLatency() */

/*******************************************************************************************************************//**
 * Function for setting frame format of PTP transmission message.
 * @param[in]   p_reg          R_EPTPC0_Type Base register for this channel
 * @param[in]   frame_format   Sets the value of PTP frame format
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_FrameFormat_ETH(R_EPTPC0_Type * p_reg, ptp_frame_format_t frame_format)
{
    /* Set frame format of PTP transmission message */
    p_reg->SYFORMR = frame_format;
} /* End of function HW_PTP_FrameFormat_ETH() */

/*******************************************************************************************************************//**
 * Function for setting reception timeout of PTP messages.
 * @param[in]   p_reg      R_EPTPC0_Type Base register for this channel
 * @param[in]   timeout    Sets the value of reception timeout for PTP messages
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_ReceptionTimeout(R_EPTPC0_Type * p_reg, uint32_t timeout)
{
    /* Set reception timeout value of PTP messages */
    p_reg->RSTOUTR = timeout;
} /* End of function HW_PTP_ReceptionTimeout() */

/*******************************************************************************************************************//**
 * Function to set SYNFP reception filter 1 register.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @param[in]   value    Sets the reception filter register 1
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPReceptionFilter1Set(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Set SYNFP Reception Filter Register 1 */
    p_reg->SYRFL1R = value;
} /* End of function HW_PTP_SYNFPReceptionFilter1Set() */

/*******************************************************************************************************************//**
 * Function to get SYNFP reception filter 1 register.
 * @param[in]   p_reg      R_EPTPC0_Type Base register for this channel
 * @param[out]  p_value    Gets the reception filter register 1
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPReceptionFilter1Get(R_EPTPC0_Type * p_reg, uint32_t * p_value)
{
    /* Get SYNFP Reception Filter Register 1 */
    *p_value = p_reg->SYRFL1R;
} /* End of function HW_PTP_SYNFPReceptionFilter1Get() */

/*******************************************************************************************************************//**
 * Function to set SYNFP reception filter 2 register.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @param[in]   value    Sets the reception filter register 2
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPReceptionFilter2Set(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Set SYNFP Reception Filter Register 2 */
    p_reg->SYRFL2R = value;
} /* End of function HW_PTP_SYNFPReceptionFilter2Set() */

/*******************************************************************************************************************//**
 * Function to get SYNFP reception filter 2 register.
 * @param[in]   p_reg      R_EPTPC0_Type Base register for this channel
 * @param[out]  p_value    Gets the reception filter register 2
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPReceptionFilter2Get(R_EPTPC0_Type * p_reg, uint32_t * p_value)
{
    /* Get SYNFP Reception Filter Register 2 */
    *p_value = p_reg->SYRFL2R;
} /* End of function HW_PTP_SYNFPReceptionFilter2Get() */

/*******************************************************************************************************************//**
 * Function to set SYNFP Operation Setting Register.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @param[in]   value    Sets the value of SYNFP Operation Setting Register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPOperationSet(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Set SYNFP Operation Setting Register value */
    p_reg->SYCONFR = value;
} /* End of function HW_PTP_SYNFPOperationSet() */

/*******************************************************************************************************************//**
 * Function to get SYNFP Operation Setting Register.
 * @param[in]   p_reg      R_EPTPC0_Type Base register for this channel
 * @param[out]  p_value    Gets the value of SYNFP Operation Setting Register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPOperationGet(R_EPTPC0_Type * p_reg, uint32_t * p_value)
{
    /* Get SYNFP Operation Setting Register value */
    *p_value = p_reg->SYCONFR;
} /* End of function HW_PTP_SYNFPOperationGet() */

/*******************************************************************************************************************//**
 * Function to set SYNFP Register Value Load Directive Register.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @param[in]   value    Sets the SYNFP register value load directive register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPLoadDirectiveSet(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Set SYNFP Register Value Load Directive Register */
    p_reg->SYRVLDR = value;
} /* End of function HW_PTP_SYNFPLoadDirectiveSet() */

/*******************************************************************************************************************//**
 * Function to set SYNFP Status Register.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @param[in]   value    Sets the SYNFP Status Register value
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPStatusSet(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Set SYNFP Status Register */
    p_reg->SYSR = value;
} /* End of function HW_PTP_SYNFPStatusSet() */

/*******************************************************************************************************************//**
 * Function to get SYNFP Status Register.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @retval      value    Return SYNFP Status Register value
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_PTP_SYNFPStatusGet(R_EPTPC0_Type * p_reg)
{
    /* Get SYNFP Status Register */
    uint32_t value = p_reg->SYSR;
    return value;
} /* End of function HW_PTP_SYNFPStatusGet() */

/*******************************************************************************************************************//**
 * Function to set INFABT Status bit.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @param[in]   value    Sets the INFABT Status Bit
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_INFABTStatusSet(R_EPTPC0_Type * p_reg, bool value)
{
    /* Set INFABT status bit */
    p_reg->SYSR_b.INFABT = value;
} /* End of function HW_PTP_INFABTStatusSet() */

/*******************************************************************************************************************//**
 * Function to get INFABT Status bit.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @retval      value    Return INFABT Status bit value
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_PTP_INFABTStatusGet(R_EPTPC0_Type * p_reg)
{
    /* Get INFABT Status */
    uint32_t value = p_reg->SYSR_b.INFABT;
    return value;
} /* End of function HW_PTP_INFABTStatusGet() */

/*******************************************************************************************************************//**
 * Function to get master clock ID.
 * @param[in]   p_reg      R_EPTPC0_Type Base register for this channel
 * @param[out]  p_clock    Pointer to master clock ID
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_MasterClockIDGet(R_EPTPC0_Type * p_reg, uint32_t * p_clock)
{
    /* Get master clock ID high and low field */
    *p_clock = p_reg->MTCIDU;
    *(p_clock+1) = p_reg->MTCIDL;
} /* End of function HW_PTP_MasterClockIDGet() */

/*******************************************************************************************************************//**
 * Function to get master port number field.
 * @param[in]   p_reg      R_EPTPC0_Type Base register for this channel
 * @param[out]  p_port     Pointer to master port number
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_MasterPortNumGet(R_EPTPC0_Type * p_reg, uint16_t * p_port)
{
    /* Get master port number field */
    *p_port = p_reg->MTPID_b.PNUM;
} /* End of function HW_PTP_MasterPortNumGet() */

/*******************************************************************************************************************//**
 * Function to update BMC values to SYNFP.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @param[in]   value    Sets the value to update best master clock
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_BMCUpdate(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Update BMC values to SYNFP */
    p_reg->SYRVLDR_b.BMUP = (uint32_t)(value & 0x01);
} /* End of function HW_PTP_ValidateSetValues() */

/*******************************************************************************************************************//**
 * Function to update announce message info to SYNFP.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @param[in]   value    Sets the value to update best master clock
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_AnnounceMsgInfoUpdate(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* Update announce message info to SYNFP */
    p_reg->SYRVLDR_b.ANUP = (uint32_t)(value & 0x01);
} /* End of function HW_PTP_AnnounceMsgInfoUpdate() */

/*******************************************************************************************************************//**
 * Function to get upper bits of offset from master.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @retval      value    Return upper bits of master offset
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_PTP_OFMUpperBitsGet(R_EPTPC0_Type * p_reg)
{
    /* Get upper bits of offset from master */
    uint32_t value = p_reg->OFMRU;
    return value;
} /* End of function HW_PTP_OFMUpperBitsGet() */

/*******************************************************************************************************************//**
 * Function to get lower bits of offset from master.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @retval      value    Return lower bits of master offset
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_PTP_OFMLowerBitsGet(R_EPTPC0_Type * p_reg)
{
    /* Get lower bits of offset from master */
    uint32_t value = p_reg->OFMRL;
    return value;
} /* End of function HW_PTP_OFMLowerBitsGet() */

/*******************************************************************************************************************//**
 * Function to get upper bits of mean path delay.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @retval      value    Return upper bits of mean path delay
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_PTP_MPDUpperBitsGet(R_EPTPC0_Type * p_reg)
{
    /* Get upper bits of mean path delay */
    uint32_t value = p_reg->MPDRU;
    return value;
} /* End of function HW_PTP_MPDUpperBitsGet() */

/*******************************************************************************************************************//**
 * Function to get lower bits of mean path delay.
 * @param[in]   p_reg    Base register for this channel
 * @retval      value    Returns lower bits of mean path delay
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_PTP_MPDLowerBitsGet(R_EPTPC0_Type * p_reg)
{
    /* Get lower bits of mean path delay */
    uint32_t value = p_reg->MPDRL;
    return value;
} /* End of function HW_PTP_MPDLowerBitsGet() */

/*******************************************************************************************************************//**
 * Function to update state of SYNFP register.
 * @param[in]   p_reg    R_EPTPC0_Type Base register for this channel
 * @param[in]   value    Sets the state of SYNFP register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_SYNFPStateUpdate(R_EPTPC0_Type * p_reg, uint32_t value)
{
    /* SYNFP state update */
    p_reg->SYRVLDR_b.STUP = (uint32_t)(value & 0x01);
} /* End of function HW_PTP_SYNFPStateUpdate() */

/*******************************************************************************************************************//**
 * Function for disabling bypass of EPTPC
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTP_BypassModule(void)
{
    /* Enable PTP and PTPEDMAC based on ETHERC and EDMAC Module Stop bit */
    if(R_MSTP->MSTPCRB_b.MSTPB14 == 0 )
    {
        /* Enable channel 1 */
        R_EPTPC_CFG->BYPASS_b.BYPASS1 = 0;
        R_ETHERC1->ECMR_b.PRM = 1;
    }
    if(R_MSTP->MSTPCRB_b.MSTPB15 == 0)
    {
        /* Enable channel 0 */
        R_EPTPC_CFG->BYPASS_b.BYPASS0 = 0;
        R_ETHERC0->ECMR_b.PRM = 1;
    }
}/*End of function HW_PTP_BypassModule()*/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file.*/
SSP_FOOTER

#endif /* HW_PTP_COMMON_H */
