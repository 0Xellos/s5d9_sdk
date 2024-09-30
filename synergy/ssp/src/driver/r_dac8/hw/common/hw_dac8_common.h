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
 * File Name    : hw_dac8_common.h
 * Description  : Low Level Driver functions
 **********************************************************************************************************************/

#ifndef HW_DAC8_COMMON_H
#define HW_DAC8_COMMON_H

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
__STATIC_INLINE void HW_DAC8_Control (R_DAC8_Type * p_dac_reg, uint8_t channel, hw_dac8_control_t value)
{
    /* Visual inspection of this code during group code review shows that there is no error here. */
    if (0U == channel)
    {
        p_dac_reg->DACR_b.DACE0 = value;
    }
    else if (1U == channel)
    {
        p_dac_reg->DACR_b.DACE1 = value;
    }
    else
    {
        p_dac_reg->DACR_b.DACE2 = value;
    }
}

__STATIC_INLINE void HW_DAC8_ControlMode (R_DAC8_Type * p_dac_reg, uint8_t channel, dac8_mode_t value)
{
    /* Visual inspection of this code during group code review shows that there is no error here. */
    if (0U == channel)
    {
        p_dac_reg->DACR_b.DAMD0 = value;
    }
    else if (1U == channel)
    {
        p_dac_reg->DACR_b.DAMD1 = value;
    }
    else
    {
        p_dac_reg->DACR_b.DAMD2 = value;
    }
}

__STATIC_INLINE void HW_DAC8_DataWrite (R_DAC8_Type * p_dac_reg, uint8_t channel, uint8_t value)
{
    p_dac_reg->DADRn[channel] = value;
}

__STATIC_INLINE void HW_DAC8_DataAddressGet (R_DAC8_Type * p_dac_reg, uint8_t channel, volatile uint8_t ** p_addr)
{
    *p_addr = &p_dac_reg->DADRn[channel];
}

__STATIC_INLINE void HW_DAC8_DaAdSyncCfg (R_DAC8_Type * p_dac_reg, uint8_t value)
{
    p_dac_reg->DACADSCR_b.DACADST = (0x01U & value);
}

__STATIC_INLINE void HW_DAC8_ChargePumpCfg (R_DAC8_Type * p_dac_reg, bool enable_charge_pump)
{
    if (enable_charge_pump)
    {
        p_dac_reg->DACPC_b.PUMPEN = 1;
    }
    else
    {
        p_dac_reg->DACPC_b.PUMPEN = 0;
    }
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_DAC8_COMMON_H */

