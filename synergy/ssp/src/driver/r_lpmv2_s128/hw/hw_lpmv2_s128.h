/***********************************************************************************************************************
 * Copyright [2016-2024] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : hw_lpmv2_s128.h
 * Description  : LPMV2 S128 specific macros.
 **********************************************************************************************************************/

#ifndef HW_LPMV2_S128_H_
#define HW_LPMV2_S128_H_

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#define HW_LPMV2_SNOOZE_REQUEST_MASK        0x738200FFU
#define HW_LPMV2_SNOOZE_END_MASK            0x0000009FU
#define HW_LPMV2_STANDBY_WAKE_SOURCE_MASK   0xFB8F00FFU

#define SNZCR_RXDREQEN_SHIFT    (0U)
#define SNZCR_RXDREQEN_MASK     (1U << SNZCR_RXDREQEN_SHIFT)
#define SNZCR_SNZDTCEN_MASK     (1U)
#define SNZCR_SNZE_SHIFT        (7U)
#define SNZCR_SNZE_MASK         (1U << SNZCR_SNZE_SHIFT)
#define SBYCR_SSBY_SHIFT        (15U)
#define SBYCR_SSBY_MASK         (1U << SBYCR_SSBY_SHIFT)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

#if LPMV2_CFG_PARAM_CHECKING_ENABLE
    ssp_err_t HW_LPMV2_MCUSpecificLowPowerCheck (lpmv2_cfg_t const * const p_cfg);
#endif /* LPMV2_CFG_PARAM_CHECKING_ENABLE */
void HW_LPMV2_MCUSpecificConfigure (lpmv2_cfg_t const * const p_cfg);
ssp_err_t HW_LPMV2_LowPowerModeEnter (void);
void HW_LPMV2_SnoozeConfigure (lpmv2_snooze_dtc_t  dtc_state_in_snooze,
                               lpmv2_snooze_request_t request_source,
                               lpmv2_snooze_end_bits_t end_sources,
                               lpmv2_snooze_cancel_t cancel_sources);
bool HW_LPMV2_WaitForOperatingModeFlags(void);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief      This function locks LPMV2 registers
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_RegisterLock (void)
{
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_OM_LPC_BATT);
}

/*******************************************************************************************************************//**
 * @brief      This function unlocks LPMV2 registers
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_RegisterUnLock (void)
{
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_OM_LPC_BATT);
}

/*******************************************************************************************************************//**
 * @brief Set the value of the SNZREQCR register, sources to trigger entry to snooze mode from standby mode
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_SNZREQCRSet (uint32_t value)
{
    R_SYSTEM->SNZREQCR = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of the SNZEDCR register, sources to trigger exit from snooze mode back to standby mode
 * @param  value    The value to set
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_SNZEDCRSet (uint8_t value)
{
    R_SYSTEM->SNZEDCR = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of SELSR0 register with the sources to trigger exit from snooze mode back to normal mode
 * @param  value    The event as the source of canceling snooze mode.
 * @note This function assumes the register has been unlocked by the calling application
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_SELSR0Set (lpmv2_snooze_cancel_t value)
{
    R_ICU->SELSR0_b.SELS = value;
}

/*******************************************************************************************************************//**
 * @brief Set the value of WUPEN register, sources for waking from standby mode
 * @param  value    The value to set
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_WUPENSet (uint32_t value)
{
    R_ICU->WUPEN = value;
}

/*******************************************************************************************************************//**
 * @brief Clear the IOKEEP bit in DPSBYCR register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LPMV2_ClearIOKeep()
{
	R_SYSTEM->DPSBYCR_b.IOKEEP = 0U;
}

#endif /* HW_LPMV2_S128_H_ */
