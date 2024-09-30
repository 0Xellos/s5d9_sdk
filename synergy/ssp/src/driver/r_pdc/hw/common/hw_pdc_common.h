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
 * File Name    : hw_pdc_common.h
 * Description  : LLD implementation of the PDC hardware.
 **********************************************************************************************************************/

#ifndef HW_PDC_COMMON_H
#define HW_PDC_COMMON_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define PCSR_MASK           0x0000007F     // Mask of PCSR flag bits
#define HCR_HST_MASK        0xFFFFF000
#define HCR_HSZ_MASK        0xF000FFFF
#define VCR_VST_MASK        0xFFFFF000
#define VCR_VSZ_MASK        0xF000FFFF
#define PDC_INTERRUPT_NONE  0
#define PDC_INTERRUPT_DFIE  0x10UL
#define PDC_INTERRUPT_UDRIE 0x80UL
#define PDC_INTERRUPT_OVIE  0x40UL
#define PDC_INTERRUPT_FEIE  0x20UL
#define PDC_INTERRUPT_VERIE 0x100UL
#define PDC_INTERRUPT_HERIE 0x200UL

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

/*******************************************************************************************************************//**
 * Enables reception operation of the PDC.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_Enable (R_PDC_Type * p_pdc_reg)
{
    p_pdc_reg->PCCR1_b.PCE = 1;
}

/*******************************************************************************************************************//**
 * Disables reception operation of the PDC.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_Disable (R_PDC_Type * p_pdc_reg)
{
    p_pdc_reg->PCCR1_b.PCE = (uint32_t)0UL;
}

/*******************************************************************************************************************//**
 * Resets the PDC.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_Reset (R_PDC_Type * p_pdc_reg)
{
    /* Reset the PCD peripheral */
    p_pdc_reg->PCCR0_b.PRST = 1;
}

/*******************************************************************************************************************//**
 * Returns the reset state of the PDC
 * @retval Value of the PRST bit in the PCCR0 register.
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_PDC_GetResetState (R_PDC_Type * p_pdc_reg)
{
    return (p_pdc_reg->PCCR0_b.PRST);
}

/*******************************************************************************************************************//**
 * Enables the PIXCLK input.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_PixclkEnable (R_PDC_Type * p_pdc_reg)
{
    p_pdc_reg->PCCR0_b.PCKE = 1;
}

/*******************************************************************************************************************//**
 * Disables the PIXCLK input.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_PixclkDisable (R_PDC_Type * p_pdc_reg)
{
    p_pdc_reg->PCCR0_b.PCKE = (uint32_t)0UL;
}

/*******************************************************************************************************************//**
 * Sets the clock divider.
 * @param   divider     Clock divider.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_DividerSet (R_PDC_Type * p_pdc_reg, pdc_clock_division_t divider)
{
    p_pdc_reg->PCCR0_b.PCKDIV = divider;
}

/*******************************************************************************************************************//**
 * Enables the PCKO clock output.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_PCKOEnable (R_PDC_Type * p_pdc_reg)
{
    p_pdc_reg->PCCR0_b.PCKOE = 1;
}

/*******************************************************************************************************************//**
 * Disables the PCKO clock output.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_PCKODisable (R_PDC_Type * p_pdc_reg)
{
    p_pdc_reg->PCCR0_b.PCKOE = (uint32_t)0UL;
}

/*******************************************************************************************************************//**
 * Sets the horizontal capture byte position.
 * @param   hst     Horizontal position in bytes.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_HSTSet (R_PDC_Type * p_pdc_reg, uint32_t hst)
{
    uint32_t hcr;
    uint32_t new_hst;
    hcr        = p_pdc_reg->HCR;
    /* Clear HST bits. */
    hcr        = hcr & HCR_HST_MASK;
    /* Clear all bits apart from valid HST bits. */
    new_hst    = hst & ~(HCR_HST_MASK);
    hcr        = hcr | new_hst;
    p_pdc_reg->HCR = hcr;
}

/*******************************************************************************************************************//**
 * Sets the horizontal capture size.
 * @param   hsz     Number of bytes to capture horizontal.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_HSZSet (R_PDC_Type * p_pdc_reg, uint32_t hsz)
{
    uint32_t hcr;
    uint32_t new_hsz;
    hcr        = p_pdc_reg->HCR;
    /* Clear HSZ bits. */
    hcr        = hcr & HCR_HSZ_MASK;
    /* Clear all bits apart from valid HSZ bits. */
    new_hsz    = hsz << 16;
    new_hsz    = new_hsz & ~(HCR_HSZ_MASK);
    hcr        = hcr | new_hsz;
    p_pdc_reg->HCR = hcr;
}

/*******************************************************************************************************************//**
 * Sets the vertical capture line position.
 * @param   vst     Vertical position in lines.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_VSTSet (R_PDC_Type * p_pdc_reg, uint32_t vst)
{
    uint32_t vcr;
    uint32_t new_vst;
    vcr        = p_pdc_reg->VCR;
    /* Clear VST bits. */
    vcr        = vcr & VCR_VST_MASK;
    /* Clear all bits apart from valid VST bits. */
    new_vst    = vst & ~(VCR_VST_MASK);
    vcr        = vcr | new_vst;
    p_pdc_reg->VCR = vcr;
}

/*******************************************************************************************************************//**
 * Sets the vertical capture size.
 * @param   vsz     Number of lines to capture.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_VSZSet (R_PDC_Type * p_pdc_reg, uint32_t vsz)
{
    uint32_t vcr;
    uint32_t new_vsz;
    vcr        = p_pdc_reg->VCR;
    /* Clear HSZ bits. */
    vcr        = vcr & VCR_VSZ_MASK;
    /* Clear all bits apart from valid HSZ bits. */
    new_vsz    = vsz << 16;
    new_vsz    = new_vsz & ~(VCR_VSZ_MASK);
    vcr        = vcr | new_vsz;
    p_pdc_reg->VCR = vcr;
}

/*******************************************************************************************************************//**
 * Sets the VSYNC polarity.
 * @param   vps     VSYNC polarity.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_VPSSet (R_PDC_Type * p_pdc_reg, pdc_vsync_polarity_t vps)
{
    p_pdc_reg->PCCR0_b.VPS = vps;
}

/*******************************************************************************************************************//**
 * Sets the HSYNC polarity.
 * @param   hps     HSYNC polarity.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_HPSSet (R_PDC_Type * p_pdc_reg, pdc_hsync_polarity_t hps)
{
    p_pdc_reg->PCCR0_b.HPS = hps;
}

/*******************************************************************************************************************//**
 * Sets the captured data endian.
 * @param   endian     Endian of captured data.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_EndianSet (R_PDC_Type * p_pdc_reg, pdc_endian_t endian)
{
    p_pdc_reg->PCCR0_b.EDS = endian;
}

/*******************************************************************************************************************//**
 * Enable/disable PDC interrupts.
 * @param   ints     Bit mask of interrupts.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_InterruptSet (R_PDC_Type * p_pdc_reg, uint32_t ints)
{
    /* Clear interrupt bits to zero */
    p_pdc_reg->PCCR0 &= 0xFFFFFC0Fu;

    /* Set enabled interrupt bits */
    p_pdc_reg->PCCR0 |= ints;
}

/*******************************************************************************************************************//**
 * Gets the PDC Status Register.
 * @retval   Status register contents.
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_PDC_StatusGet (R_PDC_Type * p_pdc_reg)
{
    return p_pdc_reg->PCSR;
}

/*******************************************************************************************************************//**
 * Clears PDC status bits.
 * @param   status     Bit mask of status bits to clear.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_PDC_StatusClear (R_PDC_Type * p_pdc_reg, uint32_t status)
{
    /* Clear bits set in status */
    status      = ~status;
    status     &= PCSR_MASK;
    p_pdc_reg->PCSR = status;
}

/*******************************************************************************************************************//**
 * Gets the PDC Pin Monitor Register.
 * @retval   PCMONR register contents.
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_PDC_PCMONRGet (R_PDC_Type * p_pdc_reg)
{
    return p_pdc_reg->PCMONR;
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_PDC_COMMON_H */
