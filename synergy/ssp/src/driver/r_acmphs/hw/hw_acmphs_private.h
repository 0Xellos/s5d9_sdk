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
* File Name    : hw_acmphs_private.h
* Description  : ACMPHS LLD implementation
***********************************************************************************************************************/

#ifndef HW_ACMPHS_PRIVATE_H
#define HW_ACMPHS_PRIVATE_H

/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"
#include "r_acmphs.h"


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
__STATIC_INLINE void HW_ACMPHS_Start(R_ACMPHS0_Type * p_reg)
{
    p_reg->CMPCTL_b.HCMPON = 1U;
}

__STATIC_INLINE void HW_ACMPHS_OutputEnable(R_ACMPHS0_Type * p_reg)
{
    p_reg->CMPCTL_b.COE = 1U;
}

__STATIC_INLINE void HW_ACMPHS_PinOutputEnable(R_ACMPHS0_Type * p_reg, comparator_pin_output_t pin_output)
{
    p_reg->CPIOC_b.CPOE = pin_output;
}

__STATIC_INLINE void HW_ACMPHS_Stop(R_ACMPHS0_Type * p_reg)
{
    p_reg->CMPCTL = 0U;
    p_reg->CPIOC_b.CPOE = 0U;
}

__STATIC_INLINE void HW_ACMPHS_TriggerSet(R_ACMPHS0_Type * p_reg, comparator_trigger_t trigger)
{
    p_reg->CMPCTL_b.CEG = trigger;
}

__STATIC_INLINE uint8_t HW_ACMPHS_OutputStatusGet(R_ACMPHS0_Type * p_reg)
{
    return p_reg->CMPCTL_b.COE;
}

__STATIC_INLINE uint8_t HW_ACMPHS_StatusGet(R_ACMPHS0_Type * p_reg)
{
    return p_reg->CMPMON;
}

static inline void HW_ACMPHS_FilterSet (R_ACMPHS0_Type * p_reg, comparator_filter_t filter)
{
    p_reg->CMPCTL_b.CDFS = (filter & 3U);
}

static inline comparator_filter_t HW_ACMPHS_FilterGet (R_ACMPHS0_Type * p_reg)
{
    return (comparator_filter_t) p_reg->CMPCTL_b.CDFS;
}

static inline void HW_ACMPHS_PolaritySet(R_ACMPHS0_Type * p_reg, comparator_polarity_invert_t invert)
{
    p_reg->CMPCTL_b.CINV = invert;
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_ACMPHS_PRIVATE_H */
