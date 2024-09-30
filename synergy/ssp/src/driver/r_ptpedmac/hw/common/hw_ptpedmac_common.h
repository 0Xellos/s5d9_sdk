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
 * File Name    : hw_ptpedmac_common.h
 * Description  : PTPEDMAC Module hardware common header file.
 **********************************************************************************************************************/

#ifndef HW_PTPEDMAC_COMMON_H
#define HW_PTPEDMAC_COMMON_H
/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define PTPEDMAC_TRANS_DISABLE (0x00000000L)
/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables (to be accessed by other files)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables and functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Function for resetting PTPEDMAC.
 * @param[in]   p_reg   R_PTPEDMAC_Type Base register
 * @param[in]   value   Sets or resets the PTPEDMAC
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTPEDMAC_Reset(R_PTPEDMAC_Type * p_reg, bool value)
{
    /* Set or reset PTPEDMAC */
    p_reg->EDMR_b.SWR = value;
}/*End of function HW_PTPEDMAC_Reset()*/

/*******************************************************************************************************************//**
 * Function for clearing all PTPEDMAC status flags.
 * @param[in]   p_reg   R_PTPEDMAC_Type Base register
 * @param[in]   value   Sets the PTPEDMAC status flag
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTPEDMAC_SetStatusFlag(R_PTPEDMAC_Type * p_reg, uint32_t value)
{
    /* Set PTPEDMAC status flags */
    p_reg->EESR  = value;
}/*End of function HW_PTPEDMAC_SetStatusFlag()*/

/*******************************************************************************************************************//**
 * Function for setting PTPEDMAC receive operation.
 * @param[in]   p_reg   R_PTPEDMAC_Type Base register
 * @param[in]   value   Sets the value to enable PTPEDMAC reception
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTPEDMAC_EnableReceiveRequest(R_PTPEDMAC_Type * p_reg, uint32_t value)
{
    /* Set PTPEDMAC receive operation to enable */
    p_reg->EDRRR = value;
}/*End of function HW_PTPEDMAC_EnableReceiveRequest()*/

/*******************************************************************************************************************//**
 * Function for restarting PTPEDMAC receive operation.
 * @param[in]   p_reg   R_PTPEDMAC_Type Base register
 * @retval      int8_t
 **********************************************************************************************************************/
__STATIC_INLINE int8_t HW_PTPEDMAC_RestartReceiveRequest(R_PTPEDMAC_Type * p_reg)
{
    /* Restart, if receive operation was disabled */
    if (PTPEDMAC_TRANS_DISABLE == p_reg->EDRRR)
    {
       return 1;
    }
    return 0;
}/*End of function HW_PTPEDMAC_RestartReceiveRequest()*/

/*******************************************************************************************************************//**
 * Function for getting EESR.
 * @param[in]   p_reg   R_PTPEDMAC_Type Base register
 * @param[out]   value   Gets the status of EESR
 * @retval      uint32_t.
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_PTPEDMAC_GetEESR(R_PTPEDMAC_Type * p_reg)
{
    /* Gets the status of EESR */
    uint32_t value = p_reg->EESR;
    return value;
}/*End of function HW_PTPEDMAC_GetEESR()*/

/*******************************************************************************************************************//**
 * Function for setting EESR.
 * @param[in]   p_reg   R_PTPEDMAC_Type Base register
 * @param[in]   value   Sets the value of EESR register
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTPEDMAC_SetEESR(R_PTPEDMAC_Type * p_reg, uint32_t value)
{
    /* Sets the status of EESR */
    p_reg->EESR= value;
}/*End of function HW_PTPEDMAC_SetEESR()*/

/*******************************************************************************************************************//**
 * Function for enabling error interrupt.
 * @param[in]   p_reg   R_PTPEDMAC_Type Base register
 * @param[in]   value   Sets the value to enable error interrupt
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTPEDMAC_EnableInterruptRequest(R_PTPEDMAC_Type * p_reg, uint32_t value)
{
    /* Enable/disable error interrupt */
    p_reg->EESIPR = value;
}/*End of function HW_PTPEDMAC_EnableInterruptRequest()*/

/*******************************************************************************************************************//**
 * Function for enabling frame receive interrupt (FR).
 * @param[in]   p_reg   R_PTPEDMAC_Type Base register
 * @param[in]   value   Sets the value to enable frame receive interrupt
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTPEDMAC_EnableFrameReceiveInterrupt(R_PTPEDMAC_Type * p_reg, bool value)
{
    /* Enable/disable frame receive interrupt (FR) */
    p_reg->EESIPR_b.FRIP = value;
}/*End of function HW_PTPEDMAC_EnableFrameReceiveInterrupt()*/

/*******************************************************************************************************************//**
 * Function for enabling frame transmit complete interrupt (TC).
 * @param[in]   p_reg   R_PTPEDMAC_Type Base register
 * @param[in]   value   Sets the value to enable frame transmit complete interrupt
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTPEDMAC_EnableFrameTransmitInterrupt(R_PTPEDMAC_Type * p_reg, bool value)
{
    /* Enable/disable frame transmit complete interrupt (TC) */
    p_reg->EESIPR_b.TCIP = value;
}/*End of function HW_PTPEDMAC_EnableFrameTransmitInterrupt()*/

/*******************************************************************************************************************//**
 * Function for setting endian mode.
 * @param[in]   p_reg   R_PTPEDMAC_Type Base register
 * @param[in]   value   Sets the endian mode
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTPEDMAC_SetEndianMode(R_PTPEDMAC_Type * p_reg, bool value)
{
    /* Set endian mode */
    p_reg->EDMR_b.DE = value;
}/*End of function HW_PTPEDMAC_SetEndianMode()*/

/*******************************************************************************************************************//**
 * Function for initializing receive descriptor list address.
 * @param[in]   p_reg     R_PTPEDMAC_Type Base register
 * @param[in]   rx_desc   Sets the receive descriptor start address
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTPEDMAC_InitializeReceiveDescriptors(R_PTPEDMAC_Type * p_reg, uint32_t rx_desc)
{
    /* Initialize receive descriptor list address */
    p_reg->RDLAR = rx_desc;
}/*End of function HW_PTPEDMAC_InitializeReceiveDescriptors()*/

/*******************************************************************************************************************//**
 * Function for setting threshold of transmit FIFO.
 * @param[in]   p_reg   R_PTPEDMAC_Type Base register
 * @param[in]   value   Sets the threshold for transmit FIFO
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTPEDMAC_SetTransmitFIFOthreshold(R_PTPEDMAC_Type * p_reg, uint32_t value)
{
    /* Set threshold of transmit FIFO */
    p_reg->TFTR = value;
}/*End of function HW_PTPEDMAC_SetTransmitFIFOthreshold()*/

/*******************************************************************************************************************//**
 * Function for setting transmit FIFO depth.
 * @param[in]   p_reg   R_PTPEDMAC_Type Base register
 * @param[in]   value   Sets the transmit FIFO depth
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTPEDMAC_SetTransmitFIFOdepth(R_PTPEDMAC_Type * p_reg, uint32_t value)
{
    /* Set transmit FIFO depth */
    p_reg->FDR  = value;
}/*End of function HW_PTPEDMAC_SetTransmitFIFOdepth()*/

/*******************************************************************************************************************//**
 * Function for configuring receive method.
 * @param[in]   p_reg   R_PTPEDMAC_Type Base register
 * @param[in]   value   Sets value to configure receive method
 * @retval      void
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PTPEDMAC_ReceiveRequest(R_PTPEDMAC_Type * p_reg, bool value)
{
    /* Configure receiving method */
    p_reg->RMCR  = value;
}/*End of function HW_PTPEDMAC_ReceiveRequest()*/

/*******************************************************************************************************************//**
 * Function to check if relaying between ports is enabled.
 * @param[in]   void
 * @retval      uint8_t
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_PTPEDMAC_RelayModeCheck(void)
{
    uint32_t syrfl1r0 = R_EPTPC0->SYRFL1R;
    uint32_t syrfl1r1 = R_EPTPC1->SYRFL1R;
    if((((syrfl1r0 >> 5) & 0x01) == 1) || (((syrfl1r0 >> 17) & 0x01) == 1) || (((syrfl1r0 >> 21) & 0x01) == 1) || (((syrfl1r0 >> 25) & 0x01) == 1))
    {
        return 1;
    }
    else if((((syrfl1r1 >> 5) & 0x01) == 1) || (((syrfl1r1 >> 17) & 0x01) == 1) || (((syrfl1r1 >> 21) & 0x01) == 1) || (((syrfl1r1 >> 25) & 0x01) == 1))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}/*End of function HW_PTPEDMAC_RelayModeCheck()*/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file.*/
SSP_FOOTER

#endif /* HW_PTPEDMAC_COMMON_H */
