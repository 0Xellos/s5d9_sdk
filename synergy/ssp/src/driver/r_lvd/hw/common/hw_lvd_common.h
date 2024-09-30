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
 * File Name    : hw_lvd_common.h
 * Description  : Hardware common file header for LVD driver
 **********************************************************************************************************************/

#ifndef HW_LVD_COMMON_H
#define HW_LVD_COMMON_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"

#include "../hw_lvd_private.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* LVDnCR0 */
#define LVDnCR0_RI_BIT_MASK         (1U)
#define LVDnCR0_RN_BIT_MASK         (1U)

/* LVDnCR1 */
#define LVDnCR1_IDTSEL_BIT_MASK     (0x3U)
#define LVDnCR1_IRQSEL_BIT_MASK     (0x1U)

/* LVDLVLR */
#define LVDLVLR_LVD1LVL_BIT_MASK    (0x1FU)
#define LVDLVLR_LVD2LVL_BIT_MASK    (0x7U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** irq generation condition LVD2CR0.RI */
typedef enum e_hw_lvd_detection_response
{
    LVD_DETECTION_RESPONSE_INTERRUPT    = 0,    ///< When VCC >= Vdet2 (rise) is detected
    LVD_DETECTION_RESPONSE_RESET        = 1,    ///< When VCC < Vdet2 (drop) is detected
} hw_lvd_detection_response_t;

/** irq is either maskable or non-maskable LVDnCR1.IRQSEL */
typedef enum e_lvd_irq_masking
{
    LVD_IRQ_MASKING_NON_MASKABLE    = 0,    ///< Generated interrupt is non-maskable
    LVD_IRQ_MASKING_MASKABLE        = 1,    ///< Generated interrupt is maskable
} lvd_irq_masking_t;

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

/*
 * Configuration checking functions
 */
bool HW_LVD_VoltageThresholdCheck(lvd_cfg_t const * const p_cfg);

/*
 * Hardware configuration helper functions
 */
__STATIC_INLINE void HW_LVD_VoltageDetectionLevelSet (R_SYSTEM_Type * p_system_reg, lvd_cfg_t const * const p_cfg);
__STATIC_INLINE void HW_LVD_DigitalFilterConfigure(R_SYSTEM_Type * p_system_reg, lvd_cfg_t const * const p_cfg);
__STATIC_INLINE void HW_LVD_NmiEnable (uint32_t monitor_number, void (* p_callback)(bsp_grp_irq_t irq));
__STATIC_INLINE void HW_LVD_NmiDisable (uint32_t monitor_number);
__STATIC_INLINE void HW_LVD_IrqEnable (IRQn_Type irq);
__STATIC_INLINE void HW_LVD_IrqDisable (IRQn_Type irq);

/*
 * Hardware get and set functions
 */
__STATIC_INLINE void HW_LVD_LVDnCR0_DFDIS_Set (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief      This function locks LVD registers
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_RegisterLock (void)
{
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_LVD);
}

/*******************************************************************************************************************//**
 * @brief      This function unlocks LVD registers
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_RegisterUnLock (void)
{
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_LVD);
}

/*******************************************************************************************************************//**
 * @brief      Set LVCMPCR[LVD1E]
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVCMPCR_LVDnE_Set (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number)
{
    if(1U == monitor_number)
    {
        p_system_reg->LVCMPCR_b.LVD1E = 0x1U;
    }
    else
    {
        p_system_reg->LVCMPCR_b.LVD2E = 0x1U;
    }
}

/*******************************************************************************************************************//**
 * @brief      Clear LVCMPCR[LVD1E]
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVCMPCR_LVDnE_Clear (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number)
{
    if(1U == monitor_number)
    {
        p_system_reg->LVCMPCR_b.LVD1E = 0x0U;
    }
    else
    {
        p_system_reg->LVCMPCR_b.LVD2E = 0x0U;
    }
}

/*******************************************************************************************************************//**
 * @brief      Set LVDnCR0[CMPE]
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDnCR0_CMPE_Set (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number)
{
    p_system_reg->LVDnCR0_b[monitor_number - 1].CMPE = 0x1U;
}

/*******************************************************************************************************************//**
 * @brief      Clear LVDnCR0[CMPE]
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDnCR0_CMPE_Clear (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number)
{
    p_system_reg->LVDnCR0_b[monitor_number - 1].CMPE = 0x0U;
}

/*******************************************************************************************************************//**
 * @brief      Set LVDLVLR[LVD1LVL]
 *
 * @param[in]  value    Value to be written to LVD1LVL.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDLVLR_LVD1LVL_Set (R_SYSTEM_Type * p_system_reg, uint8_t value)
{
    p_system_reg->LVDLVLR_b.LVD1LVL = (value & LVDLVLR_LVD1LVL_BIT_MASK);
}

/*******************************************************************************************************************//**
 * @brief      Set LVDLVLR[LVD2LVL]
 *
 * @param[in]  value    Value to be written to LVD2LVL.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDLVLR_LVD2LVL_Set (R_SYSTEM_Type * p_system_reg, uint8_t value)
{
    p_system_reg->LVDLVLR_b.LVD2LVL = (value & LVDLVLR_LVD2LVL_BIT_MASK);
}

/*******************************************************************************************************************//**
 * @brief      Set LVDnCR0[RI]
 *
 * @param[in]  monitor_number      Monitor number.
 * @param[in]  value        Value to be written to RI.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDnCR0_RI_Set (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number, uint8_t value)
{
    p_system_reg->LVDnCR0_b[monitor_number - 1].RI = (value & LVDnCR0_RI_BIT_MASK);
}

/*******************************************************************************************************************//**
 * @brief      Set LVDnCR0[RIE]
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDnCR0_RIE_Set (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number)
{
    p_system_reg->LVDnCR0_b[monitor_number - 1].RIE = 0x1U;
}

/*******************************************************************************************************************//**
 * @brief      Clear LVDnCR0[RIE]
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDnCR0_RIE_Clear (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number)
{
    p_system_reg->LVDnCR0_b[monitor_number - 1].RIE = 0x0U;
}

/*******************************************************************************************************************//**
 * @brief      Set LVDnCR0[RN]
 *
 * @param[in]  monitor_number      Monitor number.
 * @param[in]  value        Value to be written to LVDnCR0_RN.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDnCR0_RN_Set (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number, uint8_t value)
{
    p_system_reg->LVDnCR0_b[monitor_number - 1].RN = (value & LVDnCR0_RN_BIT_MASK);
}

/*******************************************************************************************************************//**
 * @brief      Set LVDnCR1[IDTSEL]
 *
 * @param[in]  monitor_number  Monitor number.
 * @param[in]  value    Value to be written to LVDnCR1_IDTSEL.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDnCR1_IDTSEL_Set (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number, uint8_t value)
{
    p_system_reg->LVDnRC0[monitor_number - 1].LVDnCR1_b.IDTSEL = (value & LVDnCR1_IDTSEL_BIT_MASK);
}

/*******************************************************************************************************************//**
 * @brief      Set LVDnCR1[IRQSEL]
 *
 * @param[in]  monitor_number  Monitor number.
 * @param[in]  value    Value to be written to LVDnCR1_IRQSEL.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDnCR1_IRQSEL_Set (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number, uint8_t value)
{
    p_system_reg->LVDnRC0[monitor_number - 1].LVDnCR1_b.IRQSEL = (value & LVDnCR1_IRQSEL_BIT_MASK);
}

/*******************************************************************************************************************//**
 * @brief      Clear LVDnSR[DET]
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_LVDnSR_DET_Clear (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number)
{
    p_system_reg->LVDnRC0[monitor_number - 1].LVDnSR_b.DET = 0x0U;
}

/*******************************************************************************************************************//**
 * @brief      Get value of LVDnSR DET bit field
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     uint8_t      LVDnSR[DET]
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_LVD_LVDnSR_DET_Get (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number)
{
    return p_system_reg->LVDnRC0[monitor_number - 1].LVDnSR_b.DET;
}

/*******************************************************************************************************************//**
 * @brief      Get value of LVDnSR MON bit field
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     uint8_t      LVDnSR[MON]
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_LVD_LVDnSR_MON_Get (R_SYSTEM_Type * p_system_reg, uint32_t monitor_number)
{
    return p_system_reg->LVDnRC0[monitor_number - 1].LVDnSR_b.MON;
}

/*******************************************************************************************************************//**
 * @brief  Check monitor_number number.
 *
 * @param[in]  monitor_number   Monitor number
 * @retval     bool             true if valid, false if invalid
 *
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_LVD_MonitorNumberCheck(uint32_t monitor_number)
{
    bool valid = true;

    if((1U != monitor_number) &&
       (2U != monitor_number))
    {
        valid = false;
    }

    return valid;
}

/*******************************************************************************************************************//**
 * @brief      Enable LVD interrupt
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_IrqEnable (IRQn_Type irq)
{
    R_BSP_IrqStatusClear(irq);    /* Clear isr flag in ICU */
    NVIC_ClearPendingIRQ(irq);    /* Clear isr in NVIC */
    NVIC_EnableIRQ(irq);          /* Enable isr in NVIC */
}

/*******************************************************************************************************************//**
 * @brief      Disable LVD interrupt
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_IrqDisable (IRQn_Type irq)
{

    NVIC_DisableIRQ(irq);         /* Enable isr in NVIC */
    R_BSP_IrqStatusClear(irq);    /* Clear isr flag in ICU */
    NVIC_ClearPendingIRQ(irq);    /* Clear isr in NVIC */
}

/*******************************************************************************************************************//**
 * @brief  Set voltage detection level.
 *
 * @param[in]  p_cfg    Pointer to LVD configuration struct.
 * @retval     none
 *
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_VoltageDetectionLevelSet (R_SYSTEM_Type * p_system_reg, lvd_cfg_t const * const p_cfg)
{
   if(1U == p_cfg->monitor_number)
   {
       HW_LVD_LVDLVLR_LVD1LVL_Set(p_system_reg, p_cfg->voltage_threshold);
   }
   else
   {
       HW_LVD_LVDLVLR_LVD2LVL_Set(p_system_reg, p_cfg->voltage_threshold);
   }
}

/*******************************************************************************************************************//**
 * @brief      Enable LVD NMI interrupt
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_NmiEnable (uint32_t monitor_number, void (* p_callback)(bsp_grp_irq_t irq))
{
    if(1U == monitor_number)
    {
        R_BSP_GroupIrqWrite(BSP_GRP_IRQ_LVD1, p_callback);
        R_ICU->NMIER_b.LVD1EN = 1U;
    }
    else
    {
        R_BSP_GroupIrqWrite(BSP_GRP_IRQ_LVD2, p_callback);
        R_ICU->NMIER_b.LVD2EN = 1U;
    }
}

/*******************************************************************************************************************//**
 * @brief      Disable LVD NMI interrupt
 *
 * @param[in]  monitor_number      Monitor number.
 * @retval     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_LVD_NmiDisable (uint32_t monitor_number)
{
    if(1U == monitor_number)
    {
        R_BSP_GroupIrqWrite(BSP_GRP_IRQ_LVD1, NULL);
    }
    else
    {
        R_BSP_GroupIrqWrite(BSP_GRP_IRQ_LVD2, NULL);
    }
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_LVD_COMMON_H */
