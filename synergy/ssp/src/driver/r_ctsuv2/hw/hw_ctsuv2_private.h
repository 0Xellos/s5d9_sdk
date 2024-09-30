/***********************************************************************************************************************
 * Copyright [2024] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : hw_ctsuv2_private.h
 * Description  : CTSU LLD private header
 **********************************************************************************************************************/

#ifndef HW_CTSU_PRIVATE_H
#define HW_CTSU_PRIVATE_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_ctsuv2.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** Value of CTSU Synchronous Noise Reduction Setting Register (CTSUSDPRS) */
#define HW_CTSU_PRRATIO_RECOMMEND   ((uint8_t) 3U)  /* Measurement time and pulse count  : 3 (Recommended) */
#define HW_CTSU_PRMODE_62_PULSES    ((uint8_t) 2U)  /* Base period and pulse count       : 62 pulses (Recommended) */
#define HW_CTSU_SOFF_ON             ((uint8_t) 0U)  /* High-pass noise reduction function: Turn spectrum diffusion on */

/** Value of CTSU High-Pass Noise Reduction Control Register (CTSUDCLKC) */
#define HW_CTSU_SSMOD               ((uint8_t) 0U)  /* The value of SSMOD should be fixed to 00b */
#define HW_CTSU_SSCNT               ((uint8_t) 3U)  /* The value of SSCNT should be fixed to 11b */

/** Value of CTSU Sensor Stabilization Wait Control Register (CTSUSST) */
#define HW_CTSU_SST_RECOMMEND       ((uint8_t) 0x10U)   /* The value of SST should be fixed to 00010000b */

/** Position of bit */
#define HW_CTSU_PON                 ((uint8_t) 0x01U)   /* CTSUPON bit */
#define HW_CTSU_CSW                 ((uint8_t) 0x02U)   /* CTSUCSW bit */
#define HW_CTSU_STRT                ((uint8_t) 0x01U)   /* CTSUSTRT bit */
#define HW_CTSU_SNZ_CAP             ((uint8_t) 0x06U)   /* CTSUSNZ, CTSUCAP bits */
#define HW_CTSU_SOVF                ((uint8_t) 0x20U)   /* CTSUSOVF bit */
#define HW_CTSU_MD_ATUNE1           ((uint8_t) 0xC8U)   /* CTSUMD[1:0], CTSUATUNE1 bits */
#define HW_CTSU_DAC_TEST_ATUNE1     ((uint8_t) 0x08U)   /* CTSUATUNE1 bit */

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** CTSU operating clock */
typedef enum e_hw_ctsu_clock
{
    HW_CTSU_CLOCK_DIV_1 = 0U,       ///< PCLKB
    HW_CTSU_CLOCK_DIV_2 = 1U,       ///< PCLKB/2 (PCLKB divided by 2)
    HW_CTSU_CLOCK_DIV_4 = 2U,       ///< PCLKB/4 (PCLKB divided by 4)
} hw_ctsu_clock_t;

/***********************************************************************************************************************
 * Private Global Variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Sets the interrupt context.
 *
 * @param[in] irq            Interrupt number.
 * @param[in] p_context      The interrupt context is a pointer to data required in the ISR.
 *
 * @warning Do not call this function for system exceptions where the IRQn_Type value is < 0.
 **********************************************************************************************************************/
__STATIC_INLINE void ctsu_isr_context_set (IRQn_Type const irq, void * p_context)
{
    ssp_vector_info_t * p_vector_info;

    R_SSP_VectorInfoGet(irq, &p_vector_info);
    *(p_vector_info->pp_ctrl) = p_context;
}

/*******************************************************************************************************************//**
 * Clear pending interrupt, IR flag and context.
 *
 * @param[in] irq            Interrupt number
 *
 * @warning Do not call this function for system exceptions where the IRQn_Type value is < 0.
 **********************************************************************************************************************/
__STATIC_INLINE void ctsu_irq_cfg_clear (IRQn_Type const irq)
{
    NVIC_ClearPendingIRQ(irq);
    R_BSP_IrqStatusClear(irq);

    ctsu_isr_context_set(irq, NULL);
}

/*******************************************************************************************************************//**
 * Disables the interrupt, clear interrupts and context.
 *
 * @param[in] irq            Interrupt number
 *
 * @warning Do not call this function for system exceptions where the IRQn_Type value is < 0.
 **********************************************************************************************************************/
__STATIC_INLINE void ctsu_irq_cfg_disable (IRQn_Type const irq)
{
    NVIC_DisableIRQ(irq);
    ctsu_irq_cfg_clear(irq);
}

/*******************************************************************************************************************//**
 * Sets the interrupt priority and context, clears pending interrupts, then enables the interrupt.
 *
 * @param[in] irq            Interrupt number
 * @param[in] priority       NVIC priority of the interrupt
 * @param[in] p_context      The interrupt context is a pointer to data required in the ISR
 *
 * @warning Do not call this function for system exceptions where the IRQn_Type value is < 0.
 **********************************************************************************************************************/
__STATIC_INLINE void ctsu_irq_cfg_enable (IRQn_Type const irq, uint32_t priority, void * p_context)
{
    ctsu_isr_context_set(irq, p_context);
    NVIC_SetPriority(irq, priority);

    R_BSP_IrqStatusClear(irq);
    NVIC_ClearPendingIRQ(irq);
    NVIC_EnableIRQ(irq);
}

/*******************************************************************************************************************//**
 * Stops the measurement operation and initialize the internal control registers.
 * Sets the CTSU power off and turn off capacitance switch, then disable power-saving function during wait state.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_PowerOff (R_CTSU_Type * p_reg)
{
    /* Stop the operation and initialize the internal control registers */
    p_reg->CTSUCR0 = (uint8_t) ((p_reg->CTSUCR0 & (uint8_t) 0xFEU) | (uint8_t) 0x10U);

    /* Disable the CTSU power supply and turn off the LPF capacitance switch connected to the TSCAP pin */
    p_reg->CTSUCR1 &= (uint8_t) ~(HW_CTSU_CSW | HW_CTSU_PON);

    /* Disable power-saving function during wait state */
    p_reg->CTSUCR0_b.CTSUSNZ = 0U;
}

/*******************************************************************************************************************//**
 * Enable the CTSU power supply, turn on the LPF capacitance switch connected to the TSCAP pin.
 * Sets the power supply operating mode, noise reduction, and stabilization wait time for the TSCAP pin voltage.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_PowerOn (R_CTSU_Type * p_reg)
{
    /* Enable the CTSU power supply and turn on the LPF capacitance switch connected to the TSCAP pin */
    p_reg->CTSUCR1 = (uint8_t) (((uint8_t) CTSU_CFG_LOW_VOLTAGE_MODE << 2) | HW_CTSU_CSW | HW_CTSU_PON);

    /* Set the synchronous noise reduction */
    p_reg->CTSUSDPRS = (uint8_t) ((HW_CTSU_SOFF_ON << 6) | (HW_CTSU_PRMODE_62_PULSES << 4) | HW_CTSU_PRRATIO_RECOMMEND);

    /* Set the high-pass noise reduction */
    p_reg->CTSUDCLKC = (uint8_t) ((HW_CTSU_SSCNT << 4) | HW_CTSU_SSMOD);

    /* Set the stabilization wait time for the TSCAP pin voltage */
    p_reg->CTSUSST = HW_CTSU_SST_RECOMMEND;
}

/*******************************************************************************************************************//**
 * Sets the CTSU operating clock.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 * @param[in] clock          CTSU operating clock
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_ClockSet (R_CTSU_Type * p_reg, hw_ctsu_clock_t clock)
{
    p_reg->CTSUCR1_b.CTSUCLK = clock;
}

/*******************************************************************************************************************//**
 * Sets the measurement start trigger and the measurement mode.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 * @param[in] p_ctrl         Pointer to control structure
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_MeasurementSet (R_CTSU_Type * p_reg, ctsu_instance_ctrl_t * p_ctrl)
{
    uint8_t temp;

    /* Sets the measurement start condition, power-saving function during wait state. */
    if (CTSU_CAP_SOFTWARE == p_ctrl->p_ctsu_cfg->cap)
    {
        p_reg->CTSUCR0 &= (uint8_t) ~(HW_CTSU_SNZ_CAP); /* software trigger, disable power-saving */
    }
    else
    {
        p_reg->CTSUCR0 |= HW_CTSU_SNZ_CAP;              /* external trigger, enable power-saving */
        p_reg->CTSUCR0 &= (uint8_t) ~(HW_CTSU_STRT);    /* Clear CTSUSTRT bit to write CTSUCR1 register */
    }

    /* Sets the measurement mode, the capacity of the CTSU power supply. In general, set CTSUATUNE1 bit to 0. */
    temp           = (uint8_t) (p_reg->CTSUCR1 & ~(HW_CTSU_MD_ATUNE1));
    p_reg->CTSUCR1 = (uint8_t) (temp | (p_ctrl->ctsucr1 & HW_CTSU_MD_ATUNE1));
}

/*******************************************************************************************************************//**
 * Select whether the TS pin is measured or not.
 * In full-scan mode, assign the TS pin to reception or transmission.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 * @param[in] p_bsp          Pointer to BSP feature structure
 * @param[in] p_cfg          Pointer to initial configurations
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_ChannelSet (R_CTSU_Type * p_reg, bsp_feature_ctsu_t * p_bsp, ctsu_cfg_t const * p_cfg)
{
    p_reg->CTSUCHAC0 = p_cfg->ctsuchac0;
    p_reg->CTSUCHAC1 = p_cfg->ctsuchac1;
    p_reg->CTSUCHAC2 = p_cfg->ctsuchac2;
    if ((uint8_t) 3U < p_bsp->ctsuchac_register_count)
    {
        p_reg->CTSUCHAC3 = p_cfg->ctsuchac3;
    }
    if ((uint8_t) 4U < p_bsp->ctsuchac_register_count)
    {
        p_reg->CTSUCHAC4 = p_cfg->ctsuchac4;
    }

 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    p_reg->CTSUCHTRC0 = p_cfg->ctsuchtrc0;
    p_reg->CTSUCHTRC1 = p_cfg->ctsuchtrc1;
    p_reg->CTSUCHTRC2 = p_cfg->ctsuchtrc2;
    if ((uint8_t) 3U < p_bsp->ctsuchtrc_register_count)
    {
        p_reg->CTSUCHTRC3 = p_cfg->ctsuchtrc3;
    }
    if ((uint8_t) 4U < p_bsp->ctsuchtrc_register_count)
    {
        p_reg->CTSUCHTRC4 = p_cfg->ctsuchtrc4;
    }
 #endif
}

/*******************************************************************************************************************//**
 * Start to the measurement operation.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_MeasurementStart (R_CTSU_Type * p_reg)
{
    p_reg->CTSUCR0 |= HW_CTSU_STRT;
}

/*******************************************************************************************************************//**
 * Checks whether the measurement operation start trigger is software or external.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 * @retval    true           Software trigger
 * @retval    false          External trigger
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_CTSU_IsSoftwareTrigger (R_CTSU_Type * p_reg)
{
    return (0U == p_reg->CTSUCR0_b.CTSUCAP);
}

/*******************************************************************************************************************//**
 * Checks whether the measurement is stopped or not.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 * @retval    true           Stopped
 * @retval    false          Measurement in progress
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_CTSU_IsMeasurementStopped (R_CTSU_Type * p_reg)
{
    return (0U == p_reg->CTSUCR0_b.CTSUSTRT);
}

/*******************************************************************************************************************//**
 * Checks whether the measurement in progress or not.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 * @retval    true           Measurement in progress
 * @retval    false          Stopped
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_CTSU_IsDuringMeasurement (R_CTSU_Type * p_reg)
{
    return (0U != p_reg->CTSUCR0_b.CTSUSTRT);
}

/*******************************************************************************************************************//**
 * Sets the spectrum diffusion frequency division, the sensor offset, the number of measurements, 
 * the reference ICO current offset, the base clock, and the ICO Gain.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 * @param[in] p_ctsuwr       Pointer to CTSUSSC,CTSUSO0,CTSUSO1 registers value structure
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_SensorSet (R_CTSU_Type * p_reg, ctsu_ctsuwr_t * p_ctsuwr)
{
    /* Write first to the CTSUSSC register, next to the CTSUSO0 register, and then to the CTSUSO1 register */
    /* Set all the bits in a single operation when writing to the CTSUSO1 register */
    p_reg->CTSUSSC = p_ctsuwr->ctsussc;
    p_reg->CTSUSO0 = p_ctsuwr->ctsuso0;
    p_reg->CTSUSO1 = p_ctsuwr->ctsuso1;
}

/*******************************************************************************************************************//**
 * Get the measurement result of the sensor ICO and the reference ICO.
 *
 * @param[in]  p_reg         Pointer to CTSU base register
 * @param[out] p_sen         Pointer to get the measurement result of sensor ICO
 * @param[out] p_ref         Pointer to get the measurement result of reference ICO
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_MeasurementResultGet (R_CTSU_Type * p_reg, uint16_t * p_sen, uint16_t * p_ref)
{
    /* Read first from the CTSUSC counter, then from the CTSURC counter */
    *p_sen = p_reg->CTSUSC;
    *p_ref = p_reg->CTSURC;
}

/*******************************************************************************************************************//**
 * Checks whether the measurement is the first or second in mutual capacitance full-scan mode.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 * @retval    true           Second measurement
 * @retval    false          First measurement
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_CTSU_IsSecondMeasurement (R_CTSU_Type * p_reg)
{
    return (0U != p_reg->CTSUST_b.CTSUPS);
}

/*******************************************************************************************************************//**
 * Checks whether the overflow occurred on the sensor counter or not.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 * @retval    true           Overflow occurred
 * @retval    false          No overflow occurred
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_CTSU_IsSensorCounterOverflow (R_CTSU_Type * p_reg)
{
    return (0U != p_reg->CTSUST_b.CTSUSOVF);
}

/*******************************************************************************************************************//**
 * Clear the CTSU sensor counter overflow flag.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_SensorCounterOverflowFlagClear (R_CTSU_Type * p_reg)
{
    p_reg->CTSUST &= (uint8_t) ~(HW_CTSU_SOVF);
}

/*******************************************************************************************************************//**
 * Checks whether the TSCAP voltage becomes abnormal or not.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 * @retval    true           Abnormal TSCAP voltage
 * @retval    false          Normal TSCAP voltage
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_CTSU_IsTscapVoltageError (R_CTSU_Type * p_reg)
{
    return (0U != p_reg->CTSUERRS_b.CTSUICOMP);
}

/*******************************************************************************************************************//**
 * Clear the TSCAP voltage error.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_TscapVoltageErrorClear (R_CTSU_Type * p_reg)
{
    p_reg->CTSUCR1 &= (uint8_t) ~(HW_CTSU_PON);
    __NOP();
    __NOP();
    p_reg->CTSUCR1 |= HW_CTSU_PON;
}

/*******************************************************************************************************************//**
 * Start to the calibration mode.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_CalibrationModeStart (R_CTSU_Type * p_reg)
{
    p_reg->CTSUCR1_b.CTSUMD = CTSU_MODE_SELF_MULTI_SCAN;
    p_reg->CTSUCHAC0        = (uint8_t) 0x01U;
    p_reg->CTSUERRS         = (uint16_t) 0x0082U;
}

/*******************************************************************************************************************//**
 * End to the calibration mode.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_CalibrationModeEnd (R_CTSU_Type * p_reg)
{
    p_reg->CTSUERRS = (uint16_t) 0x0000U;
}

/*******************************************************************************************************************//**
 * Save the ctsu normal scan register.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 * @param[out] p_diag_reg    Pointer to get the register buffer for diagnosis
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_DiagRegisterSave (R_CTSU_Type * p_reg, ctsu_diag_save_reg_t * p_diag_reg)
{
    p_diag_reg->ctsucr0    = p_reg->CTSUCR0;
    p_diag_reg->ctsucr1    = p_reg->CTSUCR1;
    p_diag_reg->ctsusdprs  = p_reg->CTSUSDPRS;
    p_diag_reg->ctsusst    = p_reg->CTSUSST;
    p_diag_reg->ctsuchac0  = p_reg->CTSUCHAC0;
    p_diag_reg->ctsuchac1  = p_reg->CTSUCHAC1;
    p_diag_reg->ctsuchac2  = p_reg->CTSUCHAC2;
    p_diag_reg->ctsuchtrc0 = p_reg->CTSUCHTRC0;
    p_diag_reg->ctsuchtrc1 = p_reg->CTSUCHTRC1;
    p_diag_reg->ctsuchtrc2 = p_reg->CTSUCHTRC2;
    p_diag_reg->ctsudclkc  = p_reg->CTSUDCLKC;
    p_diag_reg->ctsuerrs   = p_reg->CTSUERRS;
}

/*******************************************************************************************************************//**
 * Restores the ctsu normal scan register.
 *
 * @param[in] p_diag_reg     Pointer to get the register buffer for diagnosis
 * @param[out] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_DiagRegisterRestore (ctsu_diag_save_reg_t * p_diag_reg, R_CTSU_Type * p_reg)
{
    p_reg->CTSUCR0    = p_diag_reg->ctsucr0;
    p_reg->CTSUCR1    = p_diag_reg->ctsucr1;
    p_reg->CTSUSDPRS  = p_diag_reg->ctsusdprs;
    p_reg->CTSUSST    = p_diag_reg->ctsusst;
    p_reg->CTSUCHAC0  = p_diag_reg->ctsuchac0;
    p_reg->CTSUCHAC1  = p_diag_reg->ctsuchac1;
    p_reg->CTSUCHAC2  = p_diag_reg->ctsuchac2;
    p_reg->CTSUCHTRC0 = p_diag_reg->ctsuchtrc0;
    p_reg->CTSUCHTRC1 = p_diag_reg->ctsuchtrc1;
    p_reg->CTSUCHTRC2 = p_diag_reg->ctsuchtrc2;
    p_reg->CTSUDCLKC  = p_diag_reg->ctsudclkc;
    p_reg->CTSUERRS   = (uint16_t) 0x0000U;
}

/*******************************************************************************************************************//**
 * Select the measurement mode and set the measurement channel for diagnosis.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_DiagSelfscanModeSet (R_CTSU_Type * p_reg)
{
    p_reg->CTSUCR1_b.CTSUMD = CTSU_MODE_SELF_MULTI_SCAN;
    p_reg->CTSUCHAC0  = (uint8_t) 0x01U;
    p_reg->CTSUCHAC1  = (uint8_t) 0x00U;
    p_reg->CTSUCHAC2  = (uint8_t) 0x00U;
    p_reg->CTSUCHAC3  = (uint8_t) 0x00U;
    p_reg->CTSUCHAC4  = (uint8_t) 0x00U;
    p_reg->CTSUCHTRC0 = (uint8_t) 0x00U;
    p_reg->CTSUCHTRC1 = (uint8_t) 0x00U;
    p_reg->CTSUCHTRC2 = (uint8_t) 0x00U;
    p_reg->CTSUCHTRC3 = (uint8_t) 0x00U;
    p_reg->CTSUCHTRC4 = (uint8_t) 0x00U;
}

/*******************************************************************************************************************//**
 * Set the measurement correction for Over current detection test.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_DiagOverVoltageCorrectionSet (R_CTSU_Type * p_reg)
{
    p_reg->CTSUERRS = (uint16_t) 0x0080U;
}

/*******************************************************************************************************************//**
 * Set the measurement correction for Current Controlled Oscillator Test when 19.2uA correction mode.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_DiagOscillatorHighCorrectionSet (R_CTSU_Type * p_reg)
{
    p_reg->CTSUERRS         = (uint16_t) 0x0082U;
}

/*******************************************************************************************************************//**
 * Set the measurement correction for Current Controlled Oscillator Test when 2.5uA correction mode.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_DiagOscillatorLowCorrectionSet (R_CTSU_Type * p_reg)
{
    p_reg->CTSUERRS         = (uint16_t) 0x0080U;
}

/*******************************************************************************************************************//**
 * Set the measurement correction for SSCG Oscillator Test.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_DiagSscgCorrectionSet (R_CTSU_Type * p_reg)
{
    p_reg->CTSUERRS         = (uint16_t) 0x00C0U;
}

/*******************************************************************************************************************//**
 * Adjusts the electronic capacitance to offset 0 when the electrode is not being touched.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_DiagSensorOffsetSet (R_CTSU_Type * p_reg)
{
    p_reg->CTSUSO0_b.CTSUSO = 0U;
}

/*******************************************************************************************************************//**
 * Select the measurement mode and set the measurement channel for Current Offset DAC Test.
 *
 * @param[in] p_reg          Pointer to CTSU base register
 * @param[in] p_ctrl         Pointer to control structure
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CTSU_DiagDacSelfscanModeSet (R_CTSU_Type * p_reg, uint8_t ctsucr1, uint8_t diag_dac_ts)
{
    uint8_t temp;

    temp           = (uint8_t) (p_reg->CTSUCR1 & ~(HW_CTSU_MD_ATUNE1));
    p_reg->CTSUCR1 = (uint8_t) (temp | (ctsucr1 & HW_CTSU_MD_ATUNE1) | HW_CTSU_DAC_TEST_ATUNE1); // MD1, MD0, ATUNE1=1

    /* Write Channel setting */
    if (diag_dac_ts < 8)
    {
        p_reg->CTSUCHAC0 = (uint8_t) (1 << diag_dac_ts);
        p_reg->CTSUCHAC1 = (uint8_t) 0x00U;
        p_reg->CTSUCHAC2 = (uint8_t) 0x00U;
        p_reg->CTSUCHAC3 = (uint8_t) 0x00U;
    }
    else if (diag_dac_ts < 16)
    {
        p_reg->CTSUCHAC0 = (uint8_t) 0x00U;
        p_reg->CTSUCHAC1 = (uint8_t) (1 << (diag_dac_ts - 8));
        p_reg->CTSUCHAC2 = (uint8_t) 0x00U;
        p_reg->CTSUCHAC3 = (uint8_t) 0x00U;
    }
    else if (diag_dac_ts < 24)
    {
        p_reg->CTSUCHAC0 = (uint8_t) 0x00U;
        p_reg->CTSUCHAC1 = (uint8_t) 0x00U;
        p_reg->CTSUCHAC2 = (uint8_t) (1 << (diag_dac_ts - 16));
        p_reg->CTSUCHAC3 = (uint8_t) 0x00U;
    }
    else if (diag_dac_ts < 32)
    {
        p_reg->CTSUCHAC0 = (uint8_t) 0x00U;
        p_reg->CTSUCHAC1 = (uint8_t) 0x00U;
        p_reg->CTSUCHAC2 = (uint8_t) 0x00U;
        p_reg->CTSUCHAC3 = (uint8_t) (1 << (diag_dac_ts - 24));
    }
    else
    {
        p_reg->CTSUCHAC0 = (uint8_t) 0x00U;
        p_reg->CTSUCHAC1 = (uint8_t) 0x00U;
        p_reg->CTSUCHAC2 = (uint8_t) 0x00U;
        p_reg->CTSUCHAC3 = (uint8_t) 0x00U;
    }
    p_reg->CTSUCHAC4  = (uint8_t) 0x00U;
    p_reg->CTSUCHTRC0 = (uint8_t) 0x00U;
    p_reg->CTSUCHTRC1 = (uint8_t) 0x00U;
    p_reg->CTSUCHTRC2 = (uint8_t) 0x00U;
    p_reg->CTSUCHTRC3 = (uint8_t) 0x00U;
    p_reg->CTSUCHTRC4 = (uint8_t) 0x00U;
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_CTSU_PRIVATE_H */
