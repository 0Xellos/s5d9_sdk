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
* File Name    : hw_sdadc_private.h
* Description  : SDADC LLD implementation
***********************************************************************************************************************/

#ifndef HW_ADC_PRIVATE_H
#define HW_ADC_PRIVATE_H

/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"
#include "r_sdadc.h"


/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/**********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/** SDADC power mode. */
typedef enum e_sdadc_power_mode
{
    SDADC_POWER_MODE_NORMAL    = 0,           ///< Normal mode (reference clock 4 MHz, oversampling clock 1 MHz)
    SDADC_POWER_MODE_LOW_POWER = 1,           ///< Low power mode (reference clock 500 kHz, oversampling clock 125 kHz)
} sdadc_power_mode_t;

/***********************************************************************************************************************
 * Private Global Variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SDADC_RegisterReset(R_SDADC0_Type * p_reg)
{
    /* All bits in STC1 and PGACn are set during initialization. ADCR, ADAR, and CLBSSR are read-only. */
    p_reg->STC2 = 0U;
    p_reg->ADC1 = 0U;
    p_reg->ADC2 = 0U;
    p_reg->CLBC = 0U;
    p_reg->CLBSTR = 0U;
}

__STATIC_INLINE void HW_SDADC_ClkdivSet(R_SDADC0_Type * p_reg, uint16_t div)
{
    /* Note: There is a warning here for GCC since the function try to allocate parameter "div"
     * to a 4 bit bitfield (CLKDIV). Changing the type of function parameter might result in a change of value.
     * For this reason, the warning is explicitly ignored in this case.
     */
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wconversion"
#endif
    p_reg->STC1_b.CLKDIV = div;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

__STATIC_INLINE void HW_SDADC_VrefSourceSet(R_SDADC0_Type * p_reg, sdadc_vref_src_t source)
{
    p_reg->STC1_b.VREFSEL = source;
}

__STATIC_INLINE void HW_SDADC_VrefVoltageSet(R_SDADC0_Type * p_reg, sdadc_vref_voltage_t voltage)
{
    p_reg->STC1_b.VSBIAS = voltage;
}

__STATIC_INLINE sdadc_vref_voltage_t HW_SDADC_VrefVoltageGet(R_SDADC0_Type * p_reg)
{
    return (sdadc_vref_voltage_t) p_reg->STC1_b.VSBIAS;
}

__STATIC_INLINE void HW_SDADC_OperationModeSet(R_SDADC0_Type * p_reg, sdadc_power_mode_t mode)
{
    p_reg->STC1_b.SDADLPM = mode;
}

__STATIC_INLINE sdadc_power_mode_t HW_SDADC_OperationModeGet(R_SDADC0_Type * p_reg)
{
    return (sdadc_power_mode_t) p_reg->STC1_b.SDADLPM;
}

__STATIC_INLINE void HW_SDADC_BgrPowerOn(R_SDADC0_Type * p_reg)
{
    p_reg->STC2_b.BGRPON = 1U;
}

__STATIC_INLINE void HW_SDADC_BgrPowerOff(R_SDADC0_Type * p_reg)
{
    p_reg->STC2_b.BGRPON = 0U;
}

__STATIC_INLINE void HW_SDADC_AdcPowerOn(R_SDADC0_Type * p_reg)
{
    p_reg->STC2_b.ADCPON = 1U;
}

__STATIC_INLINE void HW_SDADC_AdcPowerOff(R_SDADC0_Type * p_reg)
{
    p_reg->STC2_b.ADCPON = 0U;
}

__STATIC_INLINE void HW_SDADC_ChannelCfg(R_SDADC0_Type * p_reg, adc_register_t pga, uint32_t cfg)
{
    p_reg->PGAC[pga] = cfg;
}

__STATIC_INLINE uint8_t HW_SDADC_CalibrateStatusGet(R_SDADC0_Type * p_reg)
{
    return p_reg->CLBSSR;
}

__STATIC_INLINE uint8_t HW_SDADC_ConversionStatusGet(R_SDADC0_Type * p_reg)
{
    return p_reg->ADC2;
}

__STATIC_INLINE void HW_SDADC_SoftwareStart(R_SDADC0_Type * p_reg)
{
    p_reg->ADC2 = 1U;
}

__STATIC_INLINE void HW_SDADC_SoftwareStop(R_SDADC0_Type * p_reg)
{
    p_reg->ADC2 = 0U;
}

__STATIC_INLINE void HW_SDADC_ElcTriggerEnable(R_SDADC0_Type * p_reg)
{
    p_reg->ADC1_b.SDADTMD = 1U;
}

__STATIC_INLINE void HW_SDADC_ElcTriggerDisable(R_SDADC0_Type * p_reg)
{
    p_reg->ADC1_b.SDADTMD = 0U;
}

__STATIC_INLINE uint32_t HW_SDADC_ElcTriggerGet(R_SDADC0_Type * p_reg)
{
    return p_reg->ADC1_b.SDADTMD;
}

__STATIC_INLINE void HW_SDADC_ChannelOffsetSet(R_SDADC0_Type * p_reg, adc_register_t pga, int32_t offset)
{
    /* Note: There is a warning here for GCC since the function try to allocate parameter "offset"
     * to a 5 bit bitfield (PGAOFS). Changing the type of function parameter might result in a change of value.
     * For this reason, the warning is explicitly ignored in this case.
     */
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wconversion"
#endif
    p_reg->PGAC_b[pga].PGAOFS = (uint32_t) offset;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

__STATIC_INLINE void HW_SDADC_ChannelCalibrateEnable(R_SDADC0_Type * p_reg, adc_register_t pga)
{
    p_reg->PGAC_b[pga].PGACVE = 1U;
}

__STATIC_INLINE void HW_SDADC_ChannelCalibrateDisable(R_SDADC0_Type * p_reg, adc_register_t pga)
{
    p_reg->PGAC_b[pga].PGACVE = 1U;
}

__STATIC_INLINE void HW_SDADC_CalibrateModeSet(R_SDADC0_Type * p_reg, sdadc_calibration_t mode)
{
    p_reg->CLBC = mode;
}

__STATIC_INLINE void HW_SDADC_CalibrateStart(R_SDADC0_Type * p_reg)
{
    p_reg->CLBSTR = 1U;
}

__STATIC_INLINE uint32_t HW_SDADC_ResultGet(R_SDADC0_Type * p_reg)
{
    return p_reg->ADCR;
}

__STATIC_INLINE uint32_t HW_SDADC_AveragedResultGet(R_SDADC0_Type * p_reg)
{
    return p_reg->ADAR;
}

__STATIC_INLINE sdadc_channel_input_t HW_SDADC_ChannelInputGet(R_SDADC0_Type * p_reg, adc_register_t pga)
{
    return (sdadc_channel_input_t) p_reg->PGAC_b[pga].PGASEL;
}

__STATIC_INLINE bool HW_SDADC_IsChannelAverageEnabled(R_SDADC0_Type * p_reg, adc_register_t pga)
{
    return (3U == p_reg->PGAC_b[pga].PGAAVE);
}

__STATIC_INLINE void HW_SDADC_ScanCfg(R_SDADC0_Type * p_reg, uint32_t setting)
{
    p_reg->ADC1 = setting;
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_ADC_PRIVATE_H */
