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
* File Name    : hw_slcdc_common.h
* Description  : Segment LCD Controller (SLCDC) Module hardware common header file.
***********************************************************************************************************************/

#ifndef HW_SLCDC_COMMON_H
#define HW_SLCDC_COMMON_H

/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
 
#define LCDC0_MASK  (0x3F)
#define VLCD_MASK   (0x1F)
/***********************************************************************************************************************
Private function prototypes
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables
***********************************************************************************************************************/

/***********************************************************************************************************************
Private Functions
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Specifies the voltage boosting pin initial value switching control bit.
 * @param[in] None
 * @retval    void
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_VoltageBoostPinInitSet(R_LCD_Type * p_lcd_reg)
{
	p_lcd_reg->LCDM1_b.LCDVLM = 1;
}  /* End of function HW_SLCDC_VoltageBoostPinInitSet() */

/*******************************************************************************************************************//**
 * Set waveform
 * @param[in] slcdc_wave_form_t
 * @retval    void
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_SetWaveform(R_LCD_Type * p_lcd_reg, slcdc_wave_form_t wave)
{
	p_lcd_reg->LCDM0_b.LWAVE = wave;
}  /* End of function HW_SLCDC_SetWaveform() */

/*******************************************************************************************************************//**
 * Set time slice of LCD display
 * @param[in] slcdc_time_slice_t
 * @retval    void
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_SetTimeSlice(R_LCD_Type * p_lcd_reg, slcdc_time_slice_t timeslice)
{
	p_lcd_reg->LCDM0_b.LDTY = timeslice;
}  /* End of function HW_SLCDC_SetTimeSlice() */

/*******************************************************************************************************************//**
 * Set bias method of LCD display
 * @param[in] slcdc_bias_method_t
 * @retval    void
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_SetBiasemethod(R_LCD_Type * p_lcd_reg, slcdc_bias_method_t bias)
{
	p_lcd_reg->LCDM0_b.LBAS = bias;
}  /* End of function HW_SLCDC_SetBiasemethod() */

/*******************************************************************************************************************//**
 * Set drive voltage generator LCD display
 * @param[in] slcdc_drive_volt_gen_t
 * @retval    void
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_DriveVoltageGenerator(R_LCD_Type * p_lcd_reg, slcdc_drive_volt_gen_t volt)
{
	p_lcd_reg->LCDM0_b.MDSET = volt;
}  /* End of function HW_SLCDC_DriveVoltageGenerator() */

/*******************************************************************************************************************//**
 * Set LCD blink ON
 * @param[in] void
 * @retval    void
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_BlinkON(R_LCD_Type * p_lcd_reg)
{
	p_lcd_reg->LCDM1_b.BLON = 1;
}  /* End of function HW_SLCDC_BlinkON() */

/*******************************************************************************************************************//**
 * Set LCD blonkOFF
 * @param[in] void
 * @retval    void
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_BlinkOFF(R_LCD_Type * p_lcd_reg)
{
	p_lcd_reg->LCDM1_b.BLON = 0;
}  /* End of function HW_SLCDC_BlinkOFF() */

/*******************************************************************************************************************//**
 * Set LCD display data area
 * @param[in] void
 * @retval    void
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_SetDisplayArea(R_LCD_Type * p_lcd_reg, slcdc_display_area_t area)
{
	p_lcd_reg->LCDM1_b.LCDSEL = area;
}  /* End of function HW_SLCDC_SetDisplayArea() */


/*******************************************************************************************************************//**
 * Set LCD clock division
 * @param[in] void
 * @retval    void
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_SetClockDivision(R_LCD_Type * p_lcd_reg, uint32_t div)
{
	p_lcd_reg->LCDC0_b.LCDC = div & LCDC0_MASK;
}  /* End of function HW_SLCDC_SetClockDivision() */


/*******************************************************************************************************************//**
 * Set LCD reference voltage
 * @param[in] void
 * @retval    current clock
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_SetRefVolatge (R_LCD_Type * p_lcd_reg, uint32_t volt)
{
	p_lcd_reg->VLCD_b.VLCD = volt & VLCD_MASK;
} /* End of function HW_SLCDC_SetRefVolatge() */

/*******************************************************************************************************************//**
 * Get LCD reference voltage
 * @param[in] void
 * @retval    current clock
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_SLCDC_GetRefVolatge (R_LCD_Type * p_lcd_reg)
{
	return p_lcd_reg->VLCD_b.VLCD;
} /* End of function HW_SLCDC_GetRefVolatge() */

/*******************************************************************************************************************//**
 * Enable Voltage generator
 * @param[in] void
 * @retval    void
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_VoltageGeneratorCircuitEnable(R_LCD_Type * p_lcd_reg)
{
	p_lcd_reg->LCDM1_b.VLCON = 1;
}  /* End of function HW_SLCDC_VoltageGeneratorCircuit() */

/*******************************************************************************************************************//**
 * Disable Voltage generator
 * @param[in] void
 * @retval    void
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_VoltageGeneratorCircuitDisable(R_LCD_Type * p_lcd_reg)
{
	p_lcd_reg->LCDM1_b.VLCON = 0;
}  /* End of function HW_SLCDC_VoltageGeneratorCircuitDisable() */

/*******************************************************************************************************************//**
 * LCD Display Enable
 * @param[in] void
 * @retval    void
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_LcdON(R_LCD_Type * p_lcd_reg)
{
	p_lcd_reg->LCDM1_b.SCOC = 1;
	p_lcd_reg->LCDM1_b.LCDON = 1;
}  /* End of function HW_SLCDC_LcdON() */

/*******************************************************************************************************************//**
 * LCD Display Disable
 * @param[in] void
 * @retval    void
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SLCDC_LcdOFF(R_LCD_Type * p_lcd_reg)
{
	p_lcd_reg->LCDM1_b.SCOC = 0;
	p_lcd_reg->LCDM1_b.LCDON = 0;
}  /* End of function HW_SLCDC_LcdOFF() */

/*******************************************************************************************************************//**
 * Get SLCDC SEG0 address
 * @param[in] void
 * @retval    SEG0 address
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE volatile uint8_t * HW_SLCDC_GetSEG0Addr(R_LCD_Type * p_lcd_reg)
{
	 return &p_lcd_reg->SEG0;
}  /* End of function HW_SLCDC_GetSEG0Addr() */

/*******************************************************************************************************************//**
 * Get RTC PIE bit value
 * @param[in] void
 * @retval    PIE bit value
 * @note      Parameter check is not held in this function
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_SLCDC_GetRtcPIEbit(R_RTC_Type * p_rtc_reg)
{
	 return p_rtc_reg->RCR1_b.PIE;
}  /* End of function HW_SLCDC_GetRtcPIEbit() */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_SLCDC_COMMON_H */
