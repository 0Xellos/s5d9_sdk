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
* File Name    : hw_acmplp_private.h
* Description  : ACMPLP LLD implementation
***********************************************************************************************************************/

#ifndef HW_ACMPLP_PRIVATE_H
#define HW_ACMPLP_PRIVATE_H

/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"
#include "r_acmplp.h"


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
__STATIC_INLINE void HW_ACMPLP_RegisterReset(R_ACMPLP_Type * p_reg, uint8_t channel)
{
    /* NOTE: All read-modify-write instructions to registers with bits for more than one channel must be
     * in critical sections to ensure the driver is reentrant for different channels. */
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    if (0U == channel)
    {
        /* Clear CnENB and CnWDE */
        p_reg->COMPMDR &= 0xFCU;

        /* Clear CnFCK, CnEPO, CnEDG */
        p_reg->COMPFIR &= 0xF0U;

        /* Clear CnOE and CnOP. */
        p_reg->COMPOCR &= 0xF9U;
    }
    else
    {
        /* Clear CnENB and CnWDE */
        p_reg->COMPMDR &= 0xCFU;

        /* Clear CnFCK, CnEPO, CnEDG */
        p_reg->COMPFIR &= 0x0FU;

        /* Clear CnOE and CnOP. */
        p_reg->COMPOCR &= 0x9FU;
    }

    /* Set high speed mode.  Low speed mode is not supported in SSP. */
    p_reg->COMPOCR_b.SPDMD = 1U;

    SSP_CRITICAL_SECTION_EXIT;
}

__STATIC_INLINE void HW_ACMPLP_ModeSet(R_ACMPLP_Type * p_reg, uint8_t channel, comparator_mode_t mode)
{
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    if (0U == channel)
    {
        p_reg->COMPMDR_b.C0WDE = mode;
    }
    else
    {
        p_reg->COMPMDR_b.C1WDE = mode;
    }

    SSP_CRITICAL_SECTION_EXIT;
}

__STATIC_INLINE void HW_ACMPLP_Start(R_ACMPLP_Type * p_reg, uint8_t channel)
{
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    if (0U == channel)
    {
        p_reg->COMPMDR_b.C0ENB = 1U;
    }
    else
    {
        p_reg->COMPMDR_b.C1ENB = 1U;
    }

    SSP_CRITICAL_SECTION_EXIT;
}

__STATIC_INLINE void HW_ACMPLP_PinOutputEnable(R_ACMPLP_Type * p_reg, uint8_t channel, comparator_pin_output_t pin_output)
{
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    if (0U == channel)
    {
        p_reg->COMPOCR_b.C0OE = pin_output;
    }
    else
    {
        p_reg->COMPOCR_b.C1OE = pin_output;
    }

    SSP_CRITICAL_SECTION_EXIT;
}

__STATIC_INLINE void HW_ACMPLP_Stop(R_ACMPLP_Type * p_reg, uint8_t channel)
{
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    if (0U == channel)
    {
        p_reg->COMPMDR_b.C0ENB = 0U;
        p_reg->COMPOCR_b.C0OE = 0U;
    }
    else
    {
        p_reg->COMPMDR_b.C1ENB = 0U;
        p_reg->COMPOCR_b.C0OE = 0U;
    }

    SSP_CRITICAL_SECTION_EXIT;
}

__STATIC_INLINE void HW_ACMPLP_TriggerSet(R_ACMPLP_Type * p_reg, uint8_t channel, comparator_trigger_t trigger)
{
    /* Set CiEPO and CiEDG at the same time since they are controlled by the same enum. */
    uint8_t lowest_edge_bit = 0U;
    if (0U == channel)
    {
        lowest_edge_bit = 2U;
    }
    else
    {
        lowest_edge_bit = 6U;
    }

    /* Used to set bits 2 and 3 for channel 0 or bits 6 and 7 for channel 1. */
    uint8_t set_mask = (uint8_t) (3U << lowest_edge_bit);

    /* Used to clear bits 2 and 3 for channel 0 or bits 6 and 7 for channel 1. */
    uint8_t clear_mask = (uint8_t) ~set_mask;

    /* The register value to write is one less than the value in the interface. */
    uint8_t value = (uint8_t) ((uint8_t) trigger - 1U);

    value = (uint8_t) (value << lowest_edge_bit);
    value &= set_mask;

    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    uint8_t compfir = p_reg->COMPFIR;

    /* Clear the old setting then set the new one. */
    compfir &= clear_mask;
    compfir |= value;

    p_reg->COMPFIR = compfir;

    SSP_CRITICAL_SECTION_EXIT;
}

/* Maps valid enums to register values. */
static uint8_t hw_acmplp_filter_map[] =
{
    [COMPARATOR_FILTER_OFF] = 0U,
    [COMPARATOR_FILTER_1] = 1U,
    [COMPARATOR_FILTER_8] = 2U,
    [COMPARATOR_FILTER_32] = 3U,

};
static inline void HW_ACMPLP_FilterSet (R_ACMPLP_Type * p_reg, uint8_t channel, comparator_filter_t filter)
{
    uint8_t value = hw_acmplp_filter_map[filter];
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    if (0U == channel)
    {
        p_reg->COMPFIR_b.C0FCK = value & 0x3U;
    }
    else
    {
        p_reg->COMPFIR_b.C1FCK = value & 0x3U;
    }

    SSP_CRITICAL_SECTION_EXIT;
}

__STATIC_INLINE uint8_t HW_ACMPLP_StatusGet(R_ACMPLP_Type * p_reg, uint8_t channel)
{
    if (0U == channel)
    {
        return p_reg->COMPMDR_b.C0MON;
    }
    else
    {
        return p_reg->COMPMDR_b.C1MON;
    }
}

static inline void HW_ACMPLP_PolaritySet(R_ACMPLP_Type * p_reg, uint8_t channel, comparator_polarity_invert_t invert)
{
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    if (0U == channel)
    {
        p_reg->COMPOCR_b.C0OP = invert;
    }
    else
    {
        p_reg->COMPOCR_b.C1OP = invert;
    }

    SSP_CRITICAL_SECTION_EXIT;
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_ACMPLP_PRIVATE_H */
