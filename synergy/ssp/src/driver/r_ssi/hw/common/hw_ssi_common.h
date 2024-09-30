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
* File Name    : hw_ssi_common.h
* Description  : Serial Sound Interface (SSI) lower level driver
**********************************************************************************************************************/

#ifndef HW_SSI_COMMON_H
#define HW_SSI_COMMON_H

/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define SSI_FIFO_ACCESS_BYTE       (1U)
#define SSI_FIFO_ACCESS_HALFWORD   (2U)
#define SSI_FIFO_ACCESS_WORD       (4U)
#define SSI_REN_TEN_MASK           (3U)
#define SSI_SSICR_TEN_BIT          (2U)
#define SSI_SSICR_REN_BIT          (1U)


/** 416 usec time-out count
   How to decide "time-out count".
     Functions R_SSI_Stop() & R_SSI_Mute() wait 2 PCM data in TX FIFO &
       1 data in shift register to be sent. So totally at least "1/Fs [uSec] x 3"
       period of time is needed.
     And when Sampling frequency is 8kHz, the time to finish sending all data
       is maximum. it is
       1/(8kHz) x 3 = 375[uSec].
     So waiting time must be larger than 375[uSec].
     The largest supported CPU clock is 240MHz.
       1/(240MHz) x 90000 = 375[uSec]
     Adding about 10% margin(10000), total times for loop is
       1/(240MHz) x (90000 + 10000) = 416[uSec]
     Then, here timeout loop times is configured "100000".
*/
#define SSI_TIMEOUT_416USEC         (100000) /* time-out count */

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
typedef enum e_ssi_data_delay
{
    SSI_DATA_DELAY_I2S_MODE         = 0,
    SSI_DATA_DELAY_MSB_OR_LSB_FIRST = 1,
} ssi_data_delay_t;

typedef enum e_ssi_fifo
{
    SSI_FIFO_2_EMPTY = 1,
    SSI_FIFO_4_EMPTY = 2,
    SSI_FIFO_6_EMPTY = 3,
} ssi_fifo_t;

typedef enum e_ssie_fifo
{
    SSIE_FIFO_1_EMPTY  = 0,
    SSIE_FIFO_2_EMPTY  = 1,
    SSIE_FIFO_3_EMPTY  = 2,
    SSIE_FIFO_4_EMPTY  = 3,
    SSIE_FIFO_5_EMPTY  = 4,
    SSIE_FIFO_6_EMPTY  = 5,
    SSIE_FIFO_7_EMPTY  = 6,
    SSIE_FIFO_8_EMPTY  = 7,
    SSIE_FIFO_9_EMPTY  = 8,
    SSIE_FIFO_10_EMPTY = 9,
    SSIE_FIFO_11_EMPTY = 10,
    SSIE_FIFO_12_EMPTY = 11,
    SSIE_FIFO_13_EMPTY = 12,
    SSIE_FIFO_14_EMPTY = 13,
    SSIE_FIFO_15_EMPTY = 14,
    SSIE_FIFO_16_EMPTY = 15,
    SSIE_FIFO_17_EMPTY = 16,
    SSIE_FIFO_18_EMPTY = 17,
    SSIE_FIFO_19_EMPTY = 18,
    SSIE_FIFO_20_EMPTY = 19,
    SSIE_FIFO_21_EMPTY = 20,
    SSIE_FIFO_22_EMPTY = 21,
    SSIE_FIFO_23_EMPTY = 22,
    SSIE_FIFO_24_EMPTY = 23,
    SSIE_FIFO_25_EMPTY = 24,
    SSIE_FIFO_26_EMPTY = 25,
    SSIE_FIFO_27_EMPTY = 26,
    SSIE_FIFO_28_EMPTY = 27,
    SSIE_FIFO_29_EMPTY = 28,
    SSIE_FIFO_30_EMPTY = 29,
    SSIE_FIFO_31_EMPTY = 30,
    SSIE_FIFO_32_EMPTY = 31,
} ssie_fifo_t;

typedef enum e_ssi_clock_div
{
    SSI_CLOCK_DIV_1    = 0,
    SSI_CLOCK_DIV_2    = 1,
    SSI_CLOCK_DIV_4    = 2,
    SSI_CLOCK_DIV_8    = 3,
    SSI_CLOCK_DIV_16   = 4,
    SSI_CLOCK_DIV_32   = 5,
    SSI_CLOCK_DIV_64   = 6,
    SSI_CLOCK_DIV_128  = 7,
    SSI_CLOCK_DIV_6    = 8,
    SSI_CLOCK_DIV_12   = 9,
    SSI_CLOCK_DIV_24   = 10,
    SSI_CLOCK_DIV_48   = 11,
    SSI_CLOCK_DIV_96   = 12,
    SSI_CLOCK_DIV_MAX,
} ssi_clock_div_t;

/** MCU information version 2 indicates SSIE peripheral.  This maps to the major version from the FMI. */
typedef enum e_ssi_version
{
    SSI_VERSION_SSI  = 1,   ///< Peripheral is SSI version
    SSI_VERSION_SSIE = 2,   ///< Peripheral is SSIE version
} ssi_version_t;

typedef union u_ssi_isr_event_flags
{
    __IO uint32_t  SSISR;                             /*!< Status Register                                                       */
    struct st_ssi_isr_event_flag
    {
        __I  uint32_t  IDST       :  1;               /*!< Idle Mode Status Flag                                                 */
        __I  uint32_t  RSWNO      :  1;               /*!< Receive Serial Word Number                                            */
        __I  uint32_t  RCHNO      :  2;               /*!< Receive Channel Number.These bits are read as 00b.                    */
        __I  uint32_t  TSWNO      :  1;               /*!< Transmit Serial Word Number                                           */
        __I  uint32_t  TCHNO      :  2;               /*!< Transmit Channel Number                                               */
             uint32_t             : 18;
        __IO uint32_t  IIRQ       :  1;               /*!< Idle Mode Interrupt Status Flag                                       */
        __IO uint32_t  ROIRQ      :  1;               /*!< Receive Overflow Error Interrupt Status FlagNOTE: This bit can
                                                         be set to 0 by writing 0 after reading it as 1.                       */
        __IO uint32_t  RUIRQ      :  1;               /*!< Receive Underflow Error Interrupt Status FlagNOTE: This bit
                                                         can be set to 0 by writing 0 after reading it as 1.                   */
        __IO uint32_t  TOIRQ      :  1;               /*!< Transmit Overflow Error Interrupt Status FlagNOTE: This bit
                                                         can be set to 0 by writing 0 after reading it as 1.                   */
        __IO uint32_t  TUIRQ      :  1;               /*!< Transmit Underflow Error Interrupt Status FlagNOTE: This bit
                                                         can be set to 0 by writing 0 after reading it as 1.                   */
    } SSISR_b;                                        /*!< BitSize                                                               */
}ssi_irq_events;

/***********************************************************************************************************************
Private function prototypes
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables
***********************************************************************************************************************/

/***********************************************************************************************************************
Private Functions
***********************************************************************************************************************/
__STATIC_INLINE void HW_SSI_Reset(R_SSI0_Type * p_ssi_reg)
{
    /* reset SSI peripheral */
    volatile int32_t timeout;
    timeout = SSI_TIMEOUT_416USEC; /* initializes timeout counter. */
    p_ssi_reg->SSIFCR_b.SSIRST = 1U; /* sets SSIRST bit to "1" */
    while( (p_ssi_reg->SSIFCR_b.SSIRST != 1U) && (timeout > 0) )
    {
        /* waits till SSIRST to be "1" */
        timeout--;
    }

    timeout = SSI_TIMEOUT_416USEC; /* initializes timeout counter. */
    p_ssi_reg->SSIFCR_b.SSIRST = 0U; /* clears SSIRST bit to "0" */
    while( (p_ssi_reg->SSIFCR_b.SSIRST != 0U) && (timeout > 0) )
    {
        /* waits till SSIRST to be "0" */
        timeout--;
    }
}

__STATIC_INLINE void HW_SSI_ResetRestore(R_SSI0_Type * p_ssi_reg)
{
    uint32_t ssicr = p_ssi_reg->SSICR;
    uint32_t ssifcr = p_ssi_reg->SSIFCR;
    uint32_t ssitdmr = p_ssi_reg->SSITDMR;
    uint32_t ssiscr = p_ssi_reg->SSISCR;
    HW_SSI_Reset(p_ssi_reg);
    p_ssi_reg->SSICR = ssicr;
    p_ssi_reg->SSIFCR = ssifcr;
    p_ssi_reg->SSITDMR = ssitdmr;
    p_ssi_reg->SSISCR = ssiscr;
}

__STATIC_INLINE void HW_SSI_RegisterReset(R_SSI0_Type * p_ssi_reg)
{
    /* Reset entire registers to default values. */
    p_ssi_reg->SSIFCR = 0U;
    p_ssi_reg->SSICR = 0U;
    p_ssi_reg->SSITDMR = 0U;

    /* SSIFSR, SSISR Initialize */
    uint32_t dummy = p_ssi_reg->SSIFSR;       /* dummy reading is needed to write SSIFSR. */
    p_ssi_reg->SSIFSR = 0U;
    dummy = p_ssi_reg->SSISR;        /* dummy reading is needed to write SSISR. */
    p_ssi_reg->SSISR = 0U;
    SSP_PARAMETER_NOT_USED(dummy);
}

__STATIC_INLINE void HW_SSI_DataDelaySet(R_SSI0_Type * p_ssi_reg, ssi_data_delay_t data)
{
    p_ssi_reg->SSICR_b.DEL = data;
}

__STATIC_INLINE void HW_SSI_MasterSlaveSet(R_SSI0_Type * p_ssi_reg, i2s_mode_t mode, ssi_version_t ssi_version)
{
    p_ssi_reg->SSICR_b.SWSD = mode;
    if (SSI_VERSION_SSI == ssi_version)
    {
        /* This bit does not exist on SSIE. */
        p_ssi_reg->SSICR_b.SCKD = mode;
    }
    p_ssi_reg->SSIFCR_b.AUCKE = mode;
}

__STATIC_INLINE void HW_SSI_FifoInit(R_SSI0_Type * p_ssi_reg, ssi_fifo_t setting)
{
    p_ssi_reg->SSIFCR_b.TTRG = setting;
    p_ssi_reg->SSIFCR_b.RTRG = (ssi_fifo_t) (4U - (uint32_t) setting);
}

__STATIC_INLINE void HW_SSIE_FifoInit(R_SSI0_Type * p_ssi_reg, ssie_fifo_t setting)
{
    uint32_t setting_val = (uint32_t) setting;
    uint32_t write_val = (setting_val << 8) | setting_val;
    p_ssi_reg->SSISCR = write_val;
}

__STATIC_INLINE void HW_SSI_WordLengthSet(R_SSI0_Type * p_ssi_reg, i2s_word_length_t word_length)
{
    p_ssi_reg->SSICR_b.SWL = word_length;
}

__STATIC_INLINE void HW_SSI_ClockDivBitsSet(R_SSI0_Type * p_ssi_reg, ssi_clock_div_t audio_clock_div_bits)
{
    p_ssi_reg->SSICR_b.CKDV = audio_clock_div_bits;
}

__STATIC_INLINE void HW_SSI_PcmWidthSet(R_SSI0_Type * p_ssi_reg, i2s_pcm_width_t width)
{
    p_ssi_reg->SSICR_b.DWL = width;
    if (width <= I2S_PCM_WIDTH_16_BITS)
    {
        p_ssi_reg->SSICR_b.PDTA = 0U;
    }
    else
    {
        p_ssi_reg->SSICR_b.PDTA = 1U;
    }
}

__STATIC_INLINE void HW_SSI_TxReset(R_SSI0_Type * p_ssi_reg)
{
    /* Tx FIFO Reset */
    p_ssi_reg->SSIFCR_b.TFRST = 1U;
    p_ssi_reg->SSIFCR_b.TFRST = 0U;
}

__STATIC_INLINE void HW_SSI_RxReset(R_SSI0_Type * p_ssi_reg)
{
    /* Tx FIFO Reset */
    p_ssi_reg->SSIFCR_b.RFRST = 1U;
    p_ssi_reg->SSIFCR_b.RFRST = 0U;
}

__STATIC_INLINE void HW_SSI_TxFifoRegisterReset(R_SSI0_Type * p_ssi_reg)
{
    /* SSIFSR, SSISR Initialize */
    p_ssi_reg->SSISR_b.TOIRQ = 0U;
    p_ssi_reg->SSISR_b.TUIRQ = 0U;
}

__STATIC_INLINE void HW_SSI_RxFifoRegisterReset(R_SSI0_Type * p_ssi_reg)
{
    /* SSIFSR, SSISR Initialize */
    p_ssi_reg->SSISR_b.ROIRQ = 0U;
    p_ssi_reg->SSISR_b.RUIRQ = 0U;
}

__STATIC_INLINE void HW_SSI_TxInterruptEnable(R_SSI0_Type * p_ssi_reg, IRQn_Type irq)
{
    /* Setting to start SSI Tx */
    p_ssi_reg->SSICR_b.TUIEN = 1U;
    p_ssi_reg->SSIFCR_b.TIE = 1U;

    if (SSP_INVALID_VECTOR != irq)
    {
        R_BSP_IrqStatusClear(irq);
        NVIC_ClearPendingIRQ(irq);
        NVIC_EnableIRQ(irq);
    }
}

__STATIC_INLINE void HW_SSI_TxInterruptDisable(R_SSI0_Type * p_ssi_reg, IRQn_Type irq)
{
    /* Setting to start SSI Tx */
    if (SSP_INVALID_VECTOR != irq)
    {
        NVIC_DisableIRQ(irq);
    }
    p_ssi_reg->SSIFCR_b.TIE = 0U;
    if (SSP_INVALID_VECTOR != irq)
    {
        R_BSP_IrqStatusClear(irq);
        NVIC_ClearPendingIRQ(irq);
    }
}

__STATIC_INLINE void HW_SSI_TxUnderflowDisable(R_SSI0_Type * p_ssi_reg)
{
    p_ssi_reg->SSICR_b.TUIEN = 0U;
}

__STATIC_INLINE void HW_SSI_TxEnable(R_SSI0_Type * p_ssi_reg)
{
    p_ssi_reg->SSICR_b.TEN = 1U;
}

__STATIC_INLINE void HW_SSI_TxDisable(R_SSI0_Type * p_ssi_reg)
{
    p_ssi_reg->SSICR_b.TEN = 0U;
}

__STATIC_INLINE void HW_SSI_IdleInterruptEnable(R_SSI0_Type * p_ssi_reg)
{
    p_ssi_reg->SSICR_b.IIEN = 1U;
}

__STATIC_INLINE void HW_SSI_ErrorInterruptEnable(IRQn_Type irq)
{
    R_BSP_IrqStatusClear(irq);
    NVIC_ClearPendingIRQ(irq);
    NVIC_EnableIRQ(irq);
}

__STATIC_INLINE void HW_SSI_IdleInterruptDisable(R_SSI0_Type * p_ssi_reg, IRQn_Type irq)
{
    if (SSP_INVALID_VECTOR != irq)
    {
        NVIC_DisableIRQ(irq);
    }
    p_ssi_reg->SSICR_b.IIEN = 0U;
    if (SSP_INVALID_VECTOR != irq)
    {
        R_BSP_IrqStatusClear(irq);
        NVIC_ClearPendingIRQ(irq);
    }
}

__STATIC_INLINE void HW_SSI_RxInterruptEnable(R_SSI0_Type * p_ssi_reg, IRQn_Type irq)
{
    /* Setting to start SSI Rx */
    p_ssi_reg->SSIFCR_b.RIE = 1U;
    p_ssi_reg->SSICR_b.ROIEN = 1U;

    if (SSP_INVALID_VECTOR != irq)
    {
        R_BSP_IrqStatusClear(irq);
        NVIC_ClearPendingIRQ(irq);
        NVIC_EnableIRQ(irq);
    }
}

__STATIC_INLINE void HW_SSI_RxInterruptDisable(R_SSI0_Type * p_ssi_reg, IRQn_Type irq)
{
    if (SSP_INVALID_VECTOR != irq)
    {
        NVIC_DisableIRQ(irq);
    }
    p_ssi_reg->SSIFCR_b.RIE = 0U;
    p_ssi_reg->SSICR_b.ROIEN = 0U;
    if (SSP_INVALID_VECTOR != irq)
    {
        R_BSP_IrqStatusClear(irq);
        NVIC_ClearPendingIRQ(irq);
    }
}

__STATIC_INLINE void HW_SSI_RxEnable(R_SSI0_Type * p_ssi_reg)
{
    p_ssi_reg->SSICR_b.REN = 1U;
}

__STATIC_INLINE void HW_SSI_RxDisable(R_SSI0_Type * p_ssi_reg)
{
    p_ssi_reg->SSICR_b.REN = 0U;
}

__STATIC_INLINE uint32_t HW_SSI_RxTxEnabledGet(R_SSI0_Type * p_ssi_reg)
{
    return (p_ssi_reg->SSICR & SSI_REN_TEN_MASK);
}

__STATIC_INLINE void HW_SSI_Stop(R_SSI0_Type * p_ssi_reg)
{
    p_ssi_reg->SSICR &= (~SSI_REN_TEN_MASK);
}

/*LDRA_INSPECTED 219 s __STATIC_INLINE is defined by CMSIS and cannot be changed to not start with underscore. */
__STATIC_INLINE bool HW_SSI_IsIdle(R_SSI0_Type * p_ssi_reg)
{
    return (1U == p_ssi_reg->SSISR_b.IIRQ);
}

__STATIC_INLINE bool HW_SSI_TxWordNumberGet(R_SSI0_Type * p_ssi_reg, ssi_version_t ssi_version)
{
    if (SSI_VERSION_SSI == ssi_version)
    {
        return (1U == p_ssi_reg->SSISR_b.TSWNO);
    }
    else
    {
        return 0U;
    }
}

__STATIC_INLINE bool HW_SSI_IsTxEnabled(R_SSI0_Type * p_ssi_reg)
{
    return (1U == p_ssi_reg->SSICR_b.TEN);
}

__STATIC_INLINE bool HW_SSI_IsRxEnabled(R_SSI0_Type * p_ssi_reg)
{
    return (1U == p_ssi_reg->SSICR_b.REN);
}

__STATIC_INLINE void HW_SSI_MuteOn(R_SSI0_Type * p_ssi_reg)
{
    p_ssi_reg->SSICR_b.MUEN = 1U;
}

__STATIC_INLINE void HW_SSI_MuteOff(R_SSI0_Type * p_ssi_reg)
{
    p_ssi_reg->SSICR_b.MUEN = 0U;
}

__STATIC_INLINE uint32_t HW_SSI_TxFifoFullGet(R_SSI0_Type * p_ssi_reg)
{
    return (uint32_t) p_ssi_reg->SSIFSR_b.TDC;
}

__STATIC_INLINE uint32_t HW_SSI_RxFifoFullGet(R_SSI0_Type * p_ssi_reg)
{
    return (uint32_t) p_ssi_reg->SSIFSR_b.RDC;
}

__STATIC_INLINE void HW_SSI_IdleFlagClear(R_SSI0_Type * p_ssi_reg)
{
    uint32_t dummy = p_ssi_reg->SSIFSR_b.TDE;              /* dummy read */
    p_ssi_reg->SSIFSR_b.TDE = 0U;
    SSP_PARAMETER_NOT_USED(dummy);
}

__STATIC_INLINE void HW_SSI_TxFifoEmptyFlagClear(R_SSI0_Type * p_ssi_reg)
{
    uint32_t dummy = p_ssi_reg->SSIFSR_b.TDE;              /* dummy read */
    p_ssi_reg->SSIFSR_b.TDE = 0U;
    SSP_PARAMETER_NOT_USED(dummy);
}

__STATIC_INLINE bool HW_SSI_TxFifoEmptyFlagGet(R_SSI0_Type * p_ssi_reg)
{
    return (1U == p_ssi_reg->SSIFSR_b.TDE);
}

__STATIC_INLINE void HW_SSI_RxFifoFullFlagClear(R_SSI0_Type * p_ssi_reg)
{
    uint32_t dummy = p_ssi_reg->SSIFSR_b.RDF;              /* dummy read */
    p_ssi_reg->SSIFSR_b.RDF = 0U;
    SSP_PARAMETER_NOT_USED(dummy);
}

__STATIC_INLINE void HW_SSI_TxFifoWrite(R_SSI0_Type * p_ssi_reg, uint32_t const * p_value, uint8_t fifo_access_bytes)
{
    if (SSI_FIFO_ACCESS_WORD == fifo_access_bytes)
    {
        p_ssi_reg->SSIFTDR = *p_value;
    }
    if (SSI_FIFO_ACCESS_HALFWORD == fifo_access_bytes)
    {
        uint16_t * p_value16 = (uint16_t *) p_value;
        p_ssi_reg->SSIFTDR16 = (uint16_t) *p_value16;
        p_value16++;
        p_ssi_reg->SSIFTDR16 = (uint16_t) *p_value16;
    }
    if (SSI_FIFO_ACCESS_BYTE == fifo_access_bytes)
    {
        uint8_t * p_value8 = (uint8_t *) p_value;
        p_ssi_reg->SSIFTDR8 = (uint8_t) *p_value8;
        p_value8++;
        p_ssi_reg->SSIFTDR8 = (uint8_t) *p_value8;
        p_value8++;
        p_ssi_reg->SSIFTDR8 = (uint8_t) *p_value8;
        p_value8++;
        p_ssi_reg->SSIFTDR8 = (uint8_t) *p_value8;
    }
}

__STATIC_INLINE uint32_t HW_SSI_RxFifoReadLimit(R_SSI0_Type * p_ssi_reg, uint8_t fifo_access_bytes, uint32_t limit)
{
    if (SSI_FIFO_ACCESS_WORD == fifo_access_bytes)
    {
        return p_ssi_reg->SSIFRDR;
    }
    if (SSI_FIFO_ACCESS_HALFWORD == fifo_access_bytes)
    {
        volatile uint32_t retval = 0U;
        uint16_t * p_value16 = (uint16_t *) &retval;
        *p_value16 = p_ssi_reg->SSIFRDR16;
        if (1U == limit)
        {
            return retval;
        }
        p_value16++;
        *p_value16 = p_ssi_reg->SSIFRDR16;
        return retval;
    }
    if (SSI_FIFO_ACCESS_BYTE == fifo_access_bytes)
    {
        volatile uint32_t retval = 0U;
        uint8_t * p_value8 = (uint8_t *) &retval;
        for (uint32_t i = 0U; i < sizeof(uint32_t); i++)
        {
            if (i == limit)
            {
                return retval;
            }
            *p_value8 = p_ssi_reg->SSIFRDR8;
            p_value8++;
        }
        return retval;
    }
    return 0U;
}

__STATIC_INLINE uint32_t HW_SSI_RxFifoRead(R_SSI0_Type * p_ssi_reg, uint8_t fifo_access_bytes)
{
    if (SSI_FIFO_ACCESS_WORD == fifo_access_bytes)
    {
        return p_ssi_reg->SSIFRDR;
    }
    if (SSI_FIFO_ACCESS_HALFWORD == fifo_access_bytes)
    {
        volatile uint32_t retval = 0U;
        uint16_t * p_value16 = (uint16_t *) &retval;
        *p_value16 = p_ssi_reg->SSIFRDR16;
        p_value16++;
        *p_value16 = p_ssi_reg->SSIFRDR16;
        return retval;
    }
    if (SSI_FIFO_ACCESS_BYTE == fifo_access_bytes)
    {
        volatile uint32_t retval = 0U;
        uint8_t * p_value8 = (uint8_t *) &retval;
        *p_value8 = p_ssi_reg->SSIFRDR8;
        p_value8++;
        *p_value8 = p_ssi_reg->SSIFRDR8;
        p_value8++;
        *p_value8 = p_ssi_reg->SSIFRDR8;
        p_value8++;
        *p_value8 = p_ssi_reg->SSIFRDR8;
        return retval;
    }
    return 0U;
}

__STATIC_INLINE uint32_t volatile * HW_SSI_TxAddrGet(R_SSI0_Type * p_ssi_reg)
{
    return &(p_ssi_reg->SSIFTDR);
}

__STATIC_INLINE uint32_t const volatile * HW_SSI_RxAddrGet(R_SSI0_Type * p_ssi_reg)
{
    return &(p_ssi_reg->SSIFRDR);
}

__STATIC_INLINE uint32_t HW_SSI_EventsGet(R_SSI0_Type * p_ssi_reg)
{
    /* Get events of non-IRQ status flags */
    uint32_t events = (uint32_t) (p_ssi_reg->SSISR & 0x3E000000);
    return events;
}

__STATIC_INLINE void HW_SSI_EventsClear(R_SSI0_Type * p_ssi_reg, uint32_t reg_val)
{
    /* Mask off non-IRQ status flags */
    p_ssi_reg->SSISR = (uint32_t) (~(reg_val)) & 0x3E00007FU;
}

__STATIC_INLINE bool HW_SSI_StatusGet(R_SSI0_Type * p_ssi_reg)
{
    return (1U == p_ssi_reg->SSISR_b.IIRQ);
}

__STATIC_INLINE void HW_SSI_WsContinueEnable(R_SSI0_Type * p_ssi_reg)
{
    p_ssi_reg->SSITDMR_b.CONT = 1U;
}

__STATIC_INLINE bool HW_SSI_WsContinueIsEnabled(R_SSI0_Type * p_ssi_reg)
{
    return (1U == p_ssi_reg->SSITDMR_b.CONT);
}

__STATIC_INLINE void HW_SSI_AudioClockSet(R_SSI0_Type * p_ssi_reg, ssi_audio_clock_t source)
{
    p_ssi_reg->SSICR_b.CKS = source;
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_SSI_COMMON_H */
