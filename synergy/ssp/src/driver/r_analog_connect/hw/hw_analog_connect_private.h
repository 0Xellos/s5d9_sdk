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
* File Name    : hw_analog_connect_private.h
* Description  : ANALOG_CONNECT LLD implementation
***********************************************************************************************************************/

#ifndef HW_ANALOG_CONNECT_PRIVATE_H
#define HW_ANALOG_CONNECT_PRIVATE_H

/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"
#include "r_analog_connect.h"


/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define ANALOG_CONNECT_ACMPHS_CMPCTL_OFFSET  (0U)
#define ANALOG_CONNECT_ACMPHS_CMPCTL_COE_BIT (1U)

#define ANALOG_CONNECT_OPAMP_AMPCPC_OFFSET   (0x1AU)
#define ANALOG_CONNECT_OPAMP_REG_PER_CHANNEL (3U)

/**********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Global Variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
static inline uint8_t HW_ANALOG_CONNECT_AcmphsOutputDisable(uint8_t * p_acmphs_reg)
{
    /* Read the COE value. */
    uint8_t * p_cmpctl = &p_acmphs_reg[ANALOG_CONNECT_ACMPHS_CMPCTL_OFFSET];
    uint8_t cmpctl = *p_cmpctl;
    uint8_t previously_enabled = 1U & (cmpctl >> ANALOG_CONNECT_ACMPHS_CMPCTL_COE_BIT);

    /* Disable the CMPCTL COE bit. */
    uint8_t coe_disable_mask = (uint8_t) ~(1U << ANALOG_CONNECT_ACMPHS_CMPCTL_COE_BIT);
    cmpctl &= coe_disable_mask;
    *p_cmpctl = cmpctl;

    /* Return the previous value of COE. */
    return previously_enabled;
}

static inline void HW_ANALOG_CONNECT_AcmphsIvrefEnable(void)
{
    /* This bit only exists in channel 0. */
#ifdef R_ACMPHS0
    ssp_feature_t acmphs0_feature = {{(ssp_ip_t) 0U}};
    acmphs0_feature.channel = 0U;
    acmphs0_feature.id = SSP_IP_COMP_HS;
    acmphs0_feature.unit = 0U;
    R_BSP_ModuleStart(&acmphs0_feature);
    R_ACMPHS0->CPIOC_b.VREFEN = 1U;
#endif
}

static inline void HW_ANALOG_CONNECT_AcmphsOutputEnable(uint8_t * p_acmphs_reg)
{
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    /* Enable the CMPCTL COE bit. */
    uint8_t * p_cmpctl = &p_acmphs_reg[ANALOG_CONNECT_ACMPHS_CMPCTL_OFFSET];
    uint8_t cmpctl = *p_cmpctl;
    uint8_t coe_enable_mask = (1U << ANALOG_CONNECT_ACMPHS_CMPCTL_COE_BIT);
    cmpctl |= coe_enable_mask;
    *p_cmpctl = cmpctl;

    SSP_CRITICAL_SECTION_EXIT;
}

static inline void HW_ANALOG_CONNECT_AcmphsConnectionSet(uint8_t * p_acmphs_reg, uint32_t offset, uint8_t value)
{
    volatile uint8_t * p_connection_register = &p_acmphs_reg[offset];
    *p_connection_register = 0U;
    *p_connection_register = value;
}

static inline void HW_ANALOG_CONNECT_AcmplpConnectionSet(uint8_t * p_acmplp_reg, uint32_t offset, uint8_t set_mask, uint8_t clear_mask)
{
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    /* Clear bits first. */
    volatile uint8_t * p_connection_register = &p_acmplp_reg[offset];
    uint8_t register_value = *p_connection_register;
    register_value &= (uint8_t) (~clear_mask);
    *p_connection_register = register_value;

    /* Then set new bits. */
    register_value |= set_mask;
    *p_connection_register = register_value;

    SSP_CRITICAL_SECTION_EXIT;
}

static inline void HW_ANALOG_CONNECT_OpampConnectionSet(uint8_t * p_opamp_reg, uint32_t offset, uint8_t value)
{
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    /* Set new bits. */
    uint8_t * p_connection_register = &p_opamp_reg[offset];
    *p_connection_register = value;

    SSP_CRITICAL_SECTION_EXIT;
}

static inline bool HW_ANALOG_CONNECT_OpampChargePumpIsEnabled(uint32_t offset, uint8_t * p_reg)
{
    uint8_t * p_ampcpc = &p_reg[ANALOG_CONNECT_OPAMP_AMPCPC_OFFSET];
    uint32_t opamp_channel = (offset - (uint32_t) ANALOG_CONNECT_PRIV_REG_OPAMP_AMP0OS) / ANALOG_CONNECT_OPAMP_REG_PER_CHANNEL;
    return (0U != (*p_ampcpc & (1U << opamp_channel)));
}

static inline void HW_ANALOG_CONNECT_OpampChargePumpEnable(uint32_t offset, uint8_t * p_reg)
{
    uint8_t * p_ampcpc = &p_reg[ANALOG_CONNECT_OPAMP_AMPCPC_OFFSET];
    uint32_t opamp_channel = (offset - (uint32_t) ANALOG_CONNECT_PRIV_REG_OPAMP_AMP0OS) / ANALOG_CONNECT_OPAMP_REG_PER_CHANNEL;

    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    *p_ampcpc |= (uint8_t) (1U << opamp_channel);

    SSP_CRITICAL_SECTION_EXIT;
}


/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_ANALOG_CONNECT_PRIVATE_H */
