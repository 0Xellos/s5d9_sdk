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
 * File Name    : hw_lvd_private.h
 * Description  : LVD S7G2 specific header.
 **********************************************************************************************************************/

#ifndef HW_LVD_S7G2_H_
#define HW_LVD_S7G2_H_

#include "bsp_api.h"

#include "r_lvd.h"
#include "r_lvd_api.h"
#include "r_cgc.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* LVDnCR0 */
#define LVDnCR0_DFDIS_SET_MASK      (0x1U)
#define LVDnCR0_FSAMP_BIT_MASK      (0x3U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

__STATIC_INLINE uint32_t calc_loco_delay_in_microseconds(lvd_sample_clock_t loco_div);
__STATIC_INLINE bool HW_LVD_SampleClockSelectCheck(lvd_cfg_t const * const p_cfg);
__STATIC_INLINE bool HW_LVD_ResetDelayClockCheck(lvd_cfg_t const * const p_cfg);
__STATIC_INLINE void HW_LVD_DelayAfterComparisonOutputDisable(R_SYSTEM_Type * p_system_reg, lvd_instance_ctrl_t * const p_ctrl);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
extern bsp_feature_lvd_t g_lvd_feature;

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief      Set LVDnCR0[DFDIS]
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     none
 *
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDnCR0_DFDIS_Set (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number)
{
    if (1U == g_lvd_feature.has_digital_filter)
    {
        p_system_reg->LVDnCR0_b[monitor_number - 1].DFDIS = LVDnCR0_DFDIS_SET_MASK;
    }
}

/*******************************************************************************************************************//**
 * @brief      Clear LVDnCR0[DFDIS]
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     none
 *
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDnCR0_DFDIS_Clear (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number)
{
    if (1U == g_lvd_feature.has_digital_filter)
    {
        p_system_reg->LVDnCR0_b[monitor_number - 1].DFDIS = 0U;
    }
}

/*******************************************************************************************************************//**
 * @brief      Set LVDnCR0[FSAMP]
 *
 * @param[in]  monitor_number  Monitor number.
 * @param[in]  value    Value to be written to LVDnCR0_FSAMP.
 * @retval     none
 *
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDnCR0_FSAMP_Set (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number, uint8_t value)
{
    if (1U == g_lvd_feature.has_digital_filter)
    {
        p_system_reg->LVDnCR0_b[monitor_number - 1].FSAMP = (value & LVDnCR0_FSAMP_BIT_MASK);
    }
}

/*******************************************************************************************************************//**
 * @brief      Set LVDnCR0[FSAMP]
 *
 * @param[in]  monitor_number  Monitor number.
 * @retval              Value of LVDnCR0_FSAMP.
 *
 **********************************************************************************************************************/
/*LDRA_INSPECTED 219 S */
__STATIC_INLINE uint8_t HW_LVD_LVDnCR0_FSAMP_Get (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number)
{
    if (1U == g_lvd_feature.has_digital_filter)
    {
        return p_system_reg->LVDnCR0_b[monitor_number - 1].FSAMP;
    }
    return 0U;
}

/*******************************************************************************************************************//**
 * @brief  Check sample clock setting.
 *
 * @param[in]  p_cfg    Pointer to monitor configuration struct.
 * @retval              true if valid, otherwise false
 *
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_LVD_SampleClockSelectCheck(lvd_cfg_t const * const p_cfg)
{
    bool valid = false;

    if (1U == g_lvd_feature.has_digital_filter)
    {
        if(((LVD_SAMPLE_CLOCK_LOCO_DIV_2  <= ((lvd_extend_t*)(p_cfg->p_extend))->sample_clock_divisor) &&
            (LVD_SAMPLE_CLOCK_LOCO_DIV_16 >= ((lvd_extend_t*)(p_cfg->p_extend))->sample_clock_divisor))  ||
            (LVD_SAMPLE_CLOCK_DISABLED == ((lvd_extend_t*)(p_cfg->p_extend))->sample_clock_divisor))
        {
            valid = true;
        }
    }
    else
    {
        if(LVD_SAMPLE_CLOCK_DISABLED == ((lvd_extend_t*)(p_cfg->p_extend))->sample_clock_divisor)
        {
            valid = true;
        }
    }

    return valid;
}

/*******************************************************************************************************************//**
 * @brief  Check MOCO clock state wrt negation delay.
 *
 * @param[in]  p_cfg    Pointer to monitor configuration struct.
 * @retval              true if valid, otherwise false
 *
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_LVD_ResetDelayClockCheck(lvd_cfg_t const * const p_cfg)
{
    bool valid = true;

    if((LVD_RESPONSE_RESET == p_cfg->detection_response) &&
       (LVD_NEGATION_DELAY_FROM_RESET == ((lvd_extend_t*)(p_cfg->p_extend))->negation_delay)
       )
    {
        ssp_err_t clock_status = SSP_SUCCESS;

        clock_status = g_cgc_on_cgc.clockCheck((cgc_clock_t)g_lvd_feature.negation_delay_clock);

        if ((SSP_ERR_STABILIZED     != clock_status)    &&
            (SSP_ERR_CLOCK_ACTIVE   != clock_status)    &&
            (SSP_SUCCESS            != clock_status)
            )
        {
            valid = false;
        }
    }
    else
    {
        /* Do nothing */
    }

    return valid;
}

/*******************************************************************************************************************//**
 * @brief  Check voltage detection level setting.
 *
 * @param[in]  p_cfg    Pointer to monitor configuration struct.
 * @retval              true if valid, otherwise false
 *
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_LVD_VoltageThresholdCheck(lvd_cfg_t const * const p_cfg)
{
    bool valid = false;

    switch(p_cfg->monitor_number)
    {
        case 1U:
            /* This looks reversed, but high voltage thresholds correspond to low register settings. */
            if((p_cfg->voltage_threshold >= g_lvd_feature.monitor_1_hi_threshold) &&
               (p_cfg->voltage_threshold <= g_lvd_feature.monitor_1_low_threshold))
            {
                valid = true;
            }
            break;
        case 2U:
            /* This looks reversed, but high voltage thresholds correspond to low register settings. */
            if((p_cfg->voltage_threshold >= g_lvd_feature.monitor_2_hi_threshold) &&
               (p_cfg->voltage_threshold <= g_lvd_feature.monitor_2_low_threshold))
            {
                valid = true;
            }
            break;
        default:
            break;
    }

    return valid;
}

/*******************************************************************************************************************//**
 * @brief  Configure the digital filter.
 *
 * @param[in]  p_cfg    Pointer to monitor configuration struct.
 * @retval     none
 *
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_DigitalFilterConfigure(R_SYSTEM_Type * p_system_reg, lvd_cfg_t const * const p_cfg)
{
    if (1U == g_lvd_feature.has_digital_filter)
    {
        if((LVD_SAMPLE_CLOCK_LOCO_DIV_2  <= ((lvd_extend_t*)(p_cfg->p_extend))->sample_clock_divisor)   &&
           (LVD_SAMPLE_CLOCK_LOCO_DIV_16 >= ((lvd_extend_t*)(p_cfg->p_extend))->sample_clock_divisor)
           )
        {
            /* Wait for at least td(E-A) (LVD operation stabilization time after LVD is enabled). */
            R_BSP_SoftwareDelay(10U, BSP_DELAY_UNITS_MICROSECONDS);

            /* Select the sampling clock for the digital filter by setting the LVDnCR0.FSAMP[1:0] bits. */
            HW_LVD_LVDnCR0_FSAMP_Set(p_system_reg, p_cfg->monitor_number,
                                     ((lvd_extend_t*)(p_cfg->p_extend))->sample_clock_divisor);

            /* Enable the digital filter LVDnCR0.DFDIS = 0 */
            HW_LVD_LVDnCR0_DFDIS_Clear(p_system_reg, p_cfg->monitor_number);

            /* Wait for at least 2n + 3 cycles of the LOCO (where n = 2, 4, 8, 16, and the
               sampling clock for the digital filter is the LOCO frequency-divided by n). */
            R_BSP_SoftwareDelay(calc_loco_delay_in_microseconds(((lvd_extend_t*)(p_cfg->p_extend))->sample_clock_divisor),
                                BSP_DELAY_UNITS_MICROSECONDS);
        }
    }
    else
    {
        /* Wait for at least td(E-A) (LVD operation stabilization time after LVD is enabled). */
        R_BSP_SoftwareDelay(300U, BSP_DELAY_UNITS_MICROSECONDS);
    }
}

/*******************************************************************************************************************//**
 * @brief After disabling the comparison output, wait at least 2n + 3 cycles of LOCO clock before disabling
 *        digital filtering.
 *        (where n = 2, 4, 8, 16, and the sampling clock for the digital filter is
 *         the LOCO frequency-divided by n)
 *
 * @param[in]  p_ctrl    Pointer to monitor control struct.
 * @retval     none
 *
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_DelayAfterComparisonOutputDisable(R_SYSTEM_Type * p_system_reg, lvd_instance_ctrl_t * const p_ctrl)
{
    if (1U == g_lvd_feature.has_digital_filter)
    {
        /* Wait for at least 2n + 3 cycles of the LOCO (where n = 2, 4, 8, 16,
           and the sampling clock for the digital filter is the LOCO frequency-divided by n). */
        const lvd_sample_clock_t loco_division =
                (lvd_sample_clock_t)HW_LVD_LVDnCR0_FSAMP_Get(p_system_reg, p_ctrl->monitor_number);
        R_BSP_SoftwareDelay(calc_loco_delay_in_microseconds(loco_division),
                            BSP_DELAY_UNITS_MICROSECONDS);
    }
}

/*******************************************************************************************************************//**
 * @brief  Calculate delay needed after configuration of sample clock.
 *
 * @param[in]   loco_div    Sample clock divisor
 * @retval                  delay in microseconds
 *
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t calc_loco_delay_in_microseconds(lvd_sample_clock_t loco_div)
{
    static const uint32_t loco_freq = 32768U;
    const uint32_t loco_cycles = (2U * ((uint32_t)loco_div)) + 3U;
    return ((loco_cycles * 1000000U)/loco_freq);
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_LVD_S7G2_H_ */
