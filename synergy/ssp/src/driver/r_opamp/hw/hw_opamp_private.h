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
* File Name    : hw_opamp_private.h
* Description  : OPAMP LLD implementation
***********************************************************************************************************************/

#ifndef HW_OPAMP_PRIVATE_H
#define HW_OPAMP_PRIVATE_H

/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"
#include "r_opamp.h"


/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/**********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Global Variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
__STATIC_INLINE void HW_OPAMP_Start(R_OPAMP_Type * p_reg, uint8_t mask)
{
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    p_reg->AMPC |= mask;

    SSP_CRITICAL_SECTION_EXIT;
}

__STATIC_INLINE void HW_OPAMP_Stop(R_OPAMP_Type * p_reg, uint8_t mask)
{
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    p_reg->AMPC &= (uint8_t) (~mask);

    SSP_CRITICAL_SECTION_EXIT;
}

__STATIC_INLINE void HW_OPAMP_StopAll(R_OPAMP_Type * p_reg)
{
    p_reg->AMPC = 0U;
}

__STATIC_INLINE void HW_OPAMP_PowerSpeedSet(R_OPAMP_Type * p_reg, uint8_t mode)
{
    p_reg->AMPMC = mode;
}

__STATIC_INLINE uint8_t HW_OPAMP_PowerSpeedGet(R_OPAMP_Type * p_reg)
{
    return p_reg->AMPMC;
}

__STATIC_INLINE void HW_OPAMP_AgtLinkSet(R_OPAMP_Type * p_reg, opamp_agt_link_t mode)
{
    p_reg->AMPTRS = mode;
}

__STATIC_INLINE void HW_OPAMP_TriggerSet(R_OPAMP_Type * p_reg, uint8_t mode)
{
    p_reg->AMPTRM = mode;
}

__STATIC_INLINE uint8_t HW_OPAMP_StatusGet(R_OPAMP_Type * p_reg)
{
    return p_reg->AMPMON;
}

__STATIC_INLINE void HW_OPAMP_TrimEnable(R_OPAMP_Type * p_reg, uint32_t channel)
{
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    p_reg->AMPUOTE |= (uint8_t) (1U << channel);

    SSP_CRITICAL_SECTION_EXIT;
}

__STATIC_INLINE void HW_OPAMP_TrimDisable(R_OPAMP_Type * p_reg)
{
    p_reg->AMPUOTE = 0U;
}

__STATIC_INLINE void HW_OPAMP_TrimRegisterGet(R_OPAMP_Type * p_reg, uint32_t channel, opamp_trim_input_t input, uint8_t volatile ** pp_trim_reg)
{
    uint8_t volatile * p_trim_reg = &p_reg->AMP0OTP + (2 * channel);
    p_trim_reg += input;
    *pp_trim_reg = p_trim_reg;
}


/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_OPAMP_PRIVATE_H */
