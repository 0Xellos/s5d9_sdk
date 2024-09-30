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
 * File Name    : hw_crc_common.h
 * Description  : CRC LLD layer
 **********************************************************************************************************************/


#ifndef HW_CRC_COMMON_H
#define HW_CRC_COMMON_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"


/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER


/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Sets the polynomial value.
 * @param  polynomial   The polynomial to be used for calculation.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CRC_PolynomialSet (R_CRC_Type * p_crc_reg, crc_polynomial_t polynomial)
{
    /* Set polynomial value */
    p_crc_reg->CRCCR0_b.GPS = polynomial;
}

/*******************************************************************************************************************//**
 * Gets the currently set polynomial value.
 **********************************************************************************************************************/
__STATIC_INLINE crc_polynomial_t HW_CRC_PolynomialGet (R_CRC_Type * p_crc_reg)
{
    /* Set polynomial value */
    return (crc_polynomial_t)p_crc_reg->CRCCR0_b.GPS;
}

/*******************************************************************************************************************//**
 * Sets the bit order value.
 * @param  bitOrder   The bitOrder to be used for calculation.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CRC_BitorderSet (R_CRC_Type * p_crc_reg, crc_bit_order_t bitOrder)
{
    /* Set bit order value */
    p_crc_reg->CRCCR0_b.LMS = bitOrder;
}

/*******************************************************************************************************************//**
 * Sets the starting CRC Calculated value.
 * @param  presetValue   The value written to the CRC Data Output Register.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CRC_CalculatedValueSet (R_CRC_Type * p_crc_reg, uint32_t presetValue)
{
    /* Set bit order value */
    p_crc_reg->CRCDOR = presetValue;
}

/*******************************************************************************************************************//**
 * Gets the current 8-bit CRC Calculated value.
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_CRC_8bitCalculatedValueGet (R_CRC_Type * p_crc_reg)
{
    /* Set bit order value */
    return p_crc_reg->CRCDOR_BY;
}

/*******************************************************************************************************************//**
 * Gets the current 16-bit CRC Calculated value.
 **********************************************************************************************************************/
__STATIC_INLINE uint16_t HW_CRC_16bitCalculatedValueGet (R_CRC_Type * p_crc_reg)
{
    /* Set bit order value */
    return p_crc_reg->CRCDOR_HA;
}

/*******************************************************************************************************************//**
 * Gets the current 32-bit CRC Calculated value.
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_CRC_32bitCalculatedValueGet (R_CRC_Type * p_crc_reg)
{
    /* Set bit order value */
    return p_crc_reg->CRCDOR;
}

/*******************************************************************************************************************//**
 * Sets the starting 8-bit CRC Calculated value.
 * @param  presetValue   The value written to the CRC Data Output Register.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CRC_8bitCalculatedValueSet (R_CRC_Type * p_crc_reg, uint8_t presetValue)
{
    /* Set bit order value */
    p_crc_reg->CRCDOR = 0;
    p_crc_reg->CRCDOR_BY = presetValue;
}

/*******************************************************************************************************************//**
 * Sets the starting 16-bit CRC Calculated value.
 * @param  presetValue   The value written to the CRC Data Output Register.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CRC_16bitCalculatedValueSet (R_CRC_Type * p_crc_reg, uint16_t presetValue)
{
    /* Set bit order value */
    p_crc_reg->CRCDOR = 0;
    p_crc_reg->CRCDOR_HA = presetValue;
}

/*******************************************************************************************************************//**
 * Sets the starting 32-bit CRC Calculated value.
 * @param  presetValue   The value written to the CRC Data Output Register.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CRC_32bitCalculatedValueSet (R_CRC_Type * p_crc_reg, uint32_t presetValue)
{
    /* Set bit order value */
    p_crc_reg->CRCDOR = presetValue;
}

/*******************************************************************************************************************//**
 * Write an 8-bit value to the input register of the CRC Calculator.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CRC_8bitInputWrite (R_CRC_Type * p_crc_reg, uint8_t value)
{
    /* Set bit order value */
    p_crc_reg->CRCDIR_BY = value;
}

/*******************************************************************************************************************//**
 * Write a 32-bit value to the input register of the CRC Calculator.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CRC_32bitInputWrite (R_CRC_Type * p_crc_reg, uint32_t value)
{
    /* Set bit order value */
    p_crc_reg->CRCDIR = value;
}

/*******************************************************************************************************************//**
 * Enable snooping
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CRC_SnoopEnable (R_CRC_Type * p_crc_reg)
{
    /* Set bit order value */
    p_crc_reg->CRCCR1_b.CRCSEN = 1;
}

/*******************************************************************************************************************//**
 * Disable snooping
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CRC_SnoopDisable (R_CRC_Type * p_crc_reg)
{
    /* Set bit order value */
    p_crc_reg->CRCCR1_b.CRCSEN = 0;
}

/*******************************************************************************************************************//**
 * Set the SCI channel to snoop
 **********************************************************************************************************************/
#define CRC_SNOOP_STARTING_CHANNEL_ADDRESS         (0x03)
#define CRC_SNOOP_FIFO_STARTING_CHANNEL_ADDRESS    (0x0F)
#define CRC_SNOOP_CHANNEL_SPACING                  (0x20)
__STATIC_INLINE void HW_CRC_SnoopChannelSet (R_CRC_Type * p_crc_reg, uint32_t channel, crc_snoop_direction_t direction, bool fifo_mode)
{
    /* Selects the direction of the access in the address monitoring function */
    p_crc_reg->CRCCR1_b.CRCSWR = direction;

    if(true == fifo_mode)
    {
        /*
         * Only the following address can be used to set CRCSA[13:0] bit
         * in fifo mode
         * 4007 000Fh: SCI0.FTDRL,4007 0011h: SCI0.FRDRL
         * 4007 002Fh: SCI1.FTDRL,4007 0031h: SCI1.FRDRL
         * 4007 004Fh: SCI2.FTDRL,4007 0051h: SCI2.FRDRL
         * 4007 006Fh: SCI3.FTDRL,4007 0071h: SCI3.FRDRL
         * 4007 008Fh: SCI4.FTDRL,4007 0091h: SCI4.FRDRL
         * 4007 00AFh: SCI5.FTDRL,4007 00B1h: SCI5.FRDRL
         * 4007 00CFh: SCI6.FTDRL,4007 00D1h: SCI6.FRDRL
         * 4007 00EFh: SCI7.FTDRL,4007 00F1h: SCI7.FRDRL
         * 4007 010Fh: SCI8.FTDRL,4007 0111h: SCI8.FRDRL
         * 4007 012Fh: SCI9.FTDRL,4007 0131h: SCI9.FRDRL
         *
         * direction = 0 for read and 1 for write
         */

        p_crc_reg->CRCSAR_b.CRCSA =
                0x3FFF & (CRC_SNOOP_FIFO_STARTING_CHANNEL_ADDRESS +   /* Start at the first fifo channel address */
                        (channel * CRC_SNOOP_CHANNEL_SPACING) +       /* Add the channel offset */
                        (direction ? 0 : 2));                         /* Add the direction offset (0=read, 1=write) */
    }
    else
    {
        /*
         * Only the following address can be used to set CRCSA[13:0] bit
         * in non fifo mode
         *  4007 0003h: SCI0.TDR, 4007 0005h: SCI0.RDR
         *  4007 0023h: SCI1.TDR, 4007 0025h: SCI1.RDR
         *  4007 0043h: SCI2.TDR, 4007 0045h: SCI2.RDR
         *  4007 0063h: SCI3.TDR, 4007 0065h: SCI3.RDR
         *  4007 0083h: SCI4.TDR, 4007 0085h: SCI4.RDR
         *  4007 00A3h: SCI5.TDR, 4007 00A5h: SCI5.RDR
         *  4007 00C3h: SCI6.TDR, 4007 00C5h: SCI6.RDR
         *  4007 00E3h: SCI7.TDR, 4007 00E5h: SCI7.RDR
         *  4007 0103h: SCI8.TDR, 4007 0105h: SCI8.RDR
         *  4007 0123h: SCI9.TDR, 4007 0125h: SCI9.RDR
         *
         *  direction = 0 for read and 1 for write
         */

        p_crc_reg->CRCSAR_b.CRCSA =
                0x3FFF & (CRC_SNOOP_STARTING_CHANNEL_ADDRESS +      /* Start at the first channel address */
                        (channel * CRC_SNOOP_CHANNEL_SPACING) +     /* Add the channel offset */
                        (direction ? 0 : 2));                       /* Add the direction offset (0=read, 1=write) */
    }
}



/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_CRC_COMMON_H */
