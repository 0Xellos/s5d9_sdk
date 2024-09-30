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
* File Name    : hw_kint_common.h
* Description  : Key Matrix Interrupt (KINT) Module hardware common header file.
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @addtogroup KINT
 * @{
***********************************************************************************************************************/

#ifndef HW_KINT_COMMON_H
#define HW_KINT_COMMON_H

/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define HW_KINT_KRF_RESET (0x00) /*!< Reset for Key Return Flag Register */
#define HW_KINT_KRM_RESET (0x00) /*!< Reset for Key Return Mode Register */

/** Selection of Detection Edge (KR0 to KR7)
    KINT.KRCTL.KRMD */
typedef enum e_hw_kint_krctl_krmd
{
    HW_KINT_KRCTL_KRMD_DOES_NOT_USE_KEY_INTERRUPT_FLAGS_0 = (0x0),  ///< tbd
    HW_KINT_KRCTL_KRMD_USES_KEY_INTERRUPT_FLAGS_1= (0x1),           ///< tbd
    HW_KINT_KRCTL_KRMD_MAX                                          ///< tbd
} hw_kint_krctl_krmd_t;

/** Usage of Key Interrupt Flags (KRF0 to KRF7)
    KINT.KRCTL.KREG */
typedef enum e_hw_kint_krctl_kreg
{
    HW_KINT_KRCTL_KREG_FALLING_EDGE_0 = (0x0),
    HW_KINT_KRCTL_KREG_RISING_EDGE_1  = (0x1),
    HW_KINT_KRCTL_KREG_MAX
} hw_kint_krctl_kreg_t;

/** Key interrupt flag 7
    KINT.KRF.KRF7 */
typedef enum e_hw_kint_krf_krf7
{
    HW_KINT_KRF_KRF7_NO_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_0 = (0x0),
    HW_KINT_KRF_KRF7_A_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_1 = (0x1),
    HW_KINT_KRF_KRF7_MAX
} hw_kint_krf_krf7_t;

/** Key interrupt flag 6
    KINT.KRF.KRF6 */
typedef enum e_hw_kint_krf_krf6
{
    HW_KINT_KRF_KRF6_NO_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_0 = (0x0),
    HW_KINT_KRF_KRF6_A_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_1 = (0x1),
    HW_KINT_KRF_KRF6_MAX
} hw_kint_krf_krf6_t;

/** Key interrupt flag 5
    KINT.KRF.KRF5 */
typedef enum e_hw_kint_krf_krf5
{
    HW_KINT_KRF_KRF5_NO_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_0 = (0x0),
    HW_KINT_KRF_KRF5_A_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_1 = (0x1),
    HW_KINT_KRF_KRF5_MAX
} hw_kint_krf_krf5_t;

/** Key interrupt flag 4
    KINT.KRF.KRF4 */
typedef enum e_hw_kint_krf_krf4
{
    HW_KINT_KRF_KRF4_NO_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_0 = (0x0),
    HW_KINT_KRF_KRF4_A_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_1 = (0x1),
    HW_KINT_KRF_KRF4_MAX
} hw_kint_krf_krf4_t;

/** Key interrupt flag 3
    KINT.KRF.KRF3 */
typedef enum e_hw_kint_krf_krf3
{
    HW_KINT_KRF_KRF3_NO_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_0 = (0x0),
    HW_KINT_KRF_KRF3_A_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_1 = (0x1),
    HW_KINT_KRF_KRF3_MAX
} hw_kint_krf_krf3_t;

/** Key interrupt flag 2
    KINT.KRF.KRF2 */
typedef enum e_hw_kint_krf_krf2
{
    HW_KINT_KRF_KRF2_NO_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_0 = (0x0),
    HW_KINT_KRF_KRF2_A_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_1 = (0x1),
    HW_KINT_KRF_KRF2_MAX
} hw_kint_krf_krf2_t;

/** Key interrupt flag 1
    KINT.KRF.KRF1 */
typedef enum e_hw_kint_krf_krf1
{
    HW_KINT_KRF_KRF1_NO_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_0 = (0x0),
    HW_KINT_KRF_KRF1_A_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_1 = (0x1),
    HW_KINT_KRF_KRF1_MAX
} hw_kint_krf_krf1_t;

/** Key interrupt flag 0
    KINT.KRF.KRF0 */
typedef enum e_hw_kint_krf_krf0
{
    HW_KINT_KRF_KRF0_NO_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_0 = (0x0),
    HW_KINT_KRF_KRF0_A_KEY_INTERRUPT_SIGNAL_HAS_BEEN_DETECTED_1 = (0x1),
    HW_KINT_KRF_KRF0_MAX
} hw_kint_krf_krf0_t;

/** Key interrupt mode control 7
    KINT.KRM.KRM7 */
typedef enum e_hw_kint_krm_krm7
{
    HW_KINT_KRM_KRM7_DOES_NOT_DETECT_KEY_INTERRUPT_SIGNAL_0 = (0x0),
    HW_KINT_KRM_KRM7_DETECTS_KEY_INTERRUPT_SIGNAL_1 = (0x1),
    HW_KINT_KRM_KRM7_MAX
} hw_kint_krm_krm7_t;

/** Key interrupt mode control 6
    KINT.KRM.KRM6 */
typedef enum e_hw_kint_krm_krm6
{
    HW_KINT_KRM_KRM6_DOES_NOT_DETECT_KEY_INTERRUPT_SIGNAL_0 = (0x0),
    HW_KINT_KRM_KRM6_DETECTS_KEY_INTERRUPT_SIGNAL_1 = (0x1),
    HW_KINT_KRM_KRM6_MAX
} hw_kint_krm_krm6_t;

/** Key interrupt mode control 5
    KINT.KRM.KRM5 */
typedef enum e_hw_kint_krm_krm5
{
    HW_KINT_KRM_KRM5_DOES_NOT_DETECT_KEY_INTERRUPT_SIGNAL_0 = (0x0),
    HW_KINT_KRM_KRM5_DETECTS_KEY_INTERRUPT_SIGNAL_1 = (0x1),
    HW_KINT_KRM_KRM5_MAX
} hw_kint_krm_krm5_t;

/** Key interrupt mode control 4
    KINT.KRM.KRM4 */
typedef enum e_hw_kint_krm_krm4
{
    HW_KINT_KRM_KRM4_DOES_NOT_DETECT_KEY_INTERRUPT_SIGNAL_0 = (0x0),
    HW_KINT_KRM_KRM4_DETECTS_KEY_INTERRUPT_SIGNAL_1 = (0x1),
    HW_KINT_KRM_KRM4_MAX
} hw_kint_krm_krm4_t;

/** Key interrupt mode control 3
    KINT.KRM.KRM3 */
typedef enum e_hw_kint_krm_krm3
{
    HW_KINT_KRM_KRM3_DOES_NOT_DETECT_KEY_INTERRUPT_SIGNAL_0 = (0x0),
    HW_KINT_KRM_KRM3_DETECTS_KEY_INTERRUPT_SIGNAL_1 = (0x1),
    HW_KINT_KRM_KRM3_MAX
} hw_kint_krm_krm3_t;

/** Key interrupt mode control 2
    KINT.KRM.KRM2 */
typedef enum e_hw_kint_krm_krm2
{
    HW_KINT_KRM_KRM2_DOES_NOT_DETECT_KEY_INTERRUPT_SIGNAL_0 = (0x0),
    HW_KINT_KRM_KRM2_DETECTS_KEY_INTERRUPT_SIGNAL_1 = (0x1),
    HW_KINT_KRM_KRM2_MAX
} hw_kint_krm_krm2_t;

/** Key interrupt mode control 1
    KINT.KRM.KRM1 */
typedef enum e_hw_kint_krm_krm1
{
    HW_KINT_KRM_KRM1_DOES_NOT_DETECT_KEY_INTERRUPT_SIGNAL_0 = (0x0),
    HW_KINT_KRM_KRM1_DETECTS_KEY_INTERRUPT_SIGNAL_1 = (0x1),
    HW_KINT_KRM_KRM1_MAX
} hw_kint_krm_krm1_t;

/** Key interrupt mode control 0
    KINT.KRM.KRM0 */
typedef enum e_hw_kint_krm_krm0
{
    HW_KINT_KRM_KRM0_DOES_NOT_DETECT_KEY_INTERRUPT_SIGNAL_0 = (0x0),
    HW_KINT_KRM_KRM0_DETECTS_KEY_INTERRUPT_SIGNAL_1 = (0x1),
    HW_KINT_KRM_KRM0_MAX
} hw_kint_krm_krm0_t;

/** Key interrupt mode control
    KINT.KRM */
typedef enum e_hw_kint_krm
{
    HW_KINT_KRM_DOES_NOT_DETECT_ANY_KEY_INTERRUPT_SIGNALS = (0x00),
    HW_KINT_KRM_DETECTS_ALL_KEY_INTERRUPT_SIGNALS = (0xFF),
    HW_KINT_KRM_MAX
} hw_kint_krm_t;

/** Set function for KINT.KRCTL.KRMD */
__STATIC_INLINE void HW_KINT_KRCTL_KRMD_Set(R_KINT_Type * p_kint_reg, hw_kint_krctl_krmd_t value)
{
    p_kint_reg->KRCTL_b.KRMD = value;
}
/** Get function for KINT.KRCTL.KRMD */
__STATIC_INLINE hw_kint_krctl_krmd_t HW_KINT_KRCTL_KRMD_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krctl_krmd_t)p_kint_reg->KRCTL_b.KRMD);
}

/** Set function for KINT.KRCTL.KREG */
__STATIC_INLINE void HW_KINT_KRCTL_KREG_Set(R_KINT_Type * p_kint_reg, hw_kint_krctl_kreg_t value)
{
    p_kint_reg->KRCTL_b.KREG = value;
}
/** Get function for KINT.KRCTL.KREG */
__STATIC_INLINE hw_kint_krctl_kreg_t HW_KINT_KRCTL_KREG_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krctl_kreg_t)p_kint_reg->KRCTL_b.KREG);
}

/** Set function for KINT.KRF.KRF7 */
__STATIC_INLINE void HW_KINT_KRF_KRF7_Set(R_KINT_Type * p_kint_reg, hw_kint_krf_krf7_t value)
{
    p_kint_reg->KRF_b.KRF7 = value;
}
/** Get function for KINT.KRF.KRF7 */
__STATIC_INLINE hw_kint_krf_krf7_t HW_KINT_KRF_KRF7_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krf_krf7_t)p_kint_reg->KRF_b.KRF7);
}

/** Set function for KINT.KRF.KRF6 */
__STATIC_INLINE void HW_KINT_KRF_KRF6_Set(R_KINT_Type * p_kint_reg, hw_kint_krf_krf6_t value)
{
    p_kint_reg->KRF_b.KRF6 = value;
}
/** Get function for KINT.KRF.KRF6 */
__STATIC_INLINE hw_kint_krf_krf6_t HW_KINT_KRF_KRF6_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krf_krf6_t)p_kint_reg->KRF_b.KRF6);
}

/** Set function for KINT.KRF.KRF5 */
__STATIC_INLINE void HW_KINT_KRF_KRF5_Set(R_KINT_Type * p_kint_reg, hw_kint_krf_krf5_t value)
{
    p_kint_reg->KRF_b.KRF5 = value;
}
/** Get function for KINT.KRF.KRF5 */
__STATIC_INLINE hw_kint_krf_krf5_t HW_KINT_KRF_KRF5_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krf_krf5_t)p_kint_reg->KRF_b.KRF5);
}

/** Set function for KINT.KRF.KRF4 */
__STATIC_INLINE void HW_KINT_KRF_KRF4_Set(R_KINT_Type * p_kint_reg, hw_kint_krf_krf4_t value)
{
    p_kint_reg->KRF_b.KRF4 = value;
}
/** Get function for KINT.KRF.KRF4 */
__STATIC_INLINE hw_kint_krf_krf4_t HW_KINT_KRF_KRF4_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krf_krf4_t)p_kint_reg->KRF_b.KRF4);
}

/** Set function for KINT.KRF.KRF3 */
__STATIC_INLINE void HW_KINT_KRF_KRF3_Set(R_KINT_Type * p_kint_reg, hw_kint_krf_krf3_t value)
{
    p_kint_reg->KRF_b.KRF3 = value;
}
/** Get function for KINT.KRF.KRF3 */
__STATIC_INLINE hw_kint_krf_krf3_t HW_KINT_KRF_KRF3_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krf_krf3_t)p_kint_reg->KRF_b.KRF3);
}

/** Set function for KINT.KRF.KRF2 */
__STATIC_INLINE void HW_KINT_KRF_KRF2_Set(R_KINT_Type * p_kint_reg, hw_kint_krf_krf2_t value)
{
    p_kint_reg->KRF_b.KRF2 = value;
}
/** Get function for KINT.KRF.KRF2 */
__STATIC_INLINE hw_kint_krf_krf2_t HW_KINT_KRF_KRF2_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krf_krf2_t)p_kint_reg->KRF_b.KRF2);
}

/** Set function for KINT.KRF.KRF1 */
__STATIC_INLINE void HW_KINT_KRF_KRF1_Set(R_KINT_Type * p_kint_reg, hw_kint_krf_krf1_t value)
{
    p_kint_reg->KRF_b.KRF1 = value;
}
/** Get function for KINT.KRF.KRF1 */
__STATIC_INLINE hw_kint_krf_krf1_t HW_KINT_KRF_KRF1_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krf_krf1_t)p_kint_reg->KRF_b.KRF1);
}

/** Set function for KINT.KRF.KRF0 */
__STATIC_INLINE void HW_KINT_KRF_KRF0_Set(R_KINT_Type * p_kint_reg, hw_kint_krf_krf0_t value)
{
    p_kint_reg->KRF_b.KRF0 = value;
}
/** Get function for KINT.KRF.KRF0 */
__STATIC_INLINE hw_kint_krf_krf0_t HW_KINT_KRF_KRF0_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krf_krf0_t)p_kint_reg->KRF_b.KRF0);
}

/** Set function for KINT.KRF */
__STATIC_INLINE void HW_KINT_KRF_Set(R_KINT_Type * p_kint_reg, uint8_t value)
{
    p_kint_reg->KRF = value;
}
/** Get function for KINT.KRF */
__STATIC_INLINE uint8_t HW_KINT_KRF_Get(R_KINT_Type * p_kint_reg)
{
    return((uint8_t)p_kint_reg->KRF);
}
/** Set function for KINT.KRM.KRM7 */
__STATIC_INLINE void HW_KINT_KRM_KRM7_Set(R_KINT_Type * p_kint_reg, hw_kint_krm_krm7_t value)
{
    p_kint_reg->KRM_b.KRM7 = value;
}
/** Get function for KINT.KRM.KRM7 */
__STATIC_INLINE hw_kint_krm_krm7_t HW_KINT_KRM_KRM7_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krm_krm7_t)p_kint_reg->KRM_b.KRM7);
}

/** Set function for KINT.KRM.KRM6 */
__STATIC_INLINE void HW_KINT_KRM_KRM6_Set(R_KINT_Type * p_kint_reg, hw_kint_krm_krm6_t value)
{
    p_kint_reg->KRM_b.KRM6 = value;
}
/** Get function for KINT.KRM.KRM6 */
__STATIC_INLINE hw_kint_krm_krm6_t HW_KINT_KRM_KRM6_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krm_krm6_t)p_kint_reg->KRM_b.KRM6);
}

/** Set function for KINT.KRM.KRM5 */
__STATIC_INLINE void HW_KINT_KRM_KRM5_Set(R_KINT_Type * p_kint_reg, hw_kint_krm_krm5_t value)
{
    p_kint_reg->KRM_b.KRM5 = value;
}
/** Get function for KINT.KRM.KRM5 */
__STATIC_INLINE hw_kint_krm_krm5_t HW_KINT_KRM_KRM5_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krm_krm5_t)p_kint_reg->KRM_b.KRM5);
}

/** Set function for KINT.KRM.KRM4 */
__STATIC_INLINE void HW_KINT_KRM_KRM4_Set(R_KINT_Type * p_kint_reg, hw_kint_krm_krm4_t value)
{
    p_kint_reg->KRM_b.KRM4 = value;
}
/** Get function for KINT.KRM.KRM4 */
__STATIC_INLINE hw_kint_krm_krm4_t HW_KINT_KRM_KRM4_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krm_krm4_t)p_kint_reg->KRM_b.KRM4);
}

/** Set function for KINT.KRM.KRM3 */
__STATIC_INLINE void HW_KINT_KRM_KRM3_Set(R_KINT_Type * p_kint_reg, hw_kint_krm_krm3_t value)
{
    p_kint_reg->KRM_b.KRM3 = value;
}
/** Get function for KINT.KRM.KRM3 */
__STATIC_INLINE hw_kint_krm_krm3_t HW_KINT_KRM_KRM3_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krm_krm3_t)p_kint_reg->KRM_b.KRM3);
}

/** Set function for KINT.KRM.KRM2 */
__STATIC_INLINE void HW_KINT_KRM_KRM2_Set(R_KINT_Type * p_kint_reg, hw_kint_krm_krm2_t value)
{
    p_kint_reg->KRM_b.KRM2 = value;
}
/** Get function for KINT.KRM.KRM2 */
__STATIC_INLINE hw_kint_krm_krm2_t HW_KINT_KRM_KRM2_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krm_krm2_t)p_kint_reg->KRM_b.KRM2);
}

/** Set function for KINT.KRM.KRM1 */
__STATIC_INLINE void HW_KINT_KRM_KRM1_Set(R_KINT_Type * p_kint_reg, hw_kint_krm_krm1_t value)
{
    p_kint_reg->KRM_b.KRM1 = value;
}
/** Get function for KINT.KRM.KRM1 */
__STATIC_INLINE hw_kint_krm_krm1_t HW_KINT_KRM_KRM1_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krm_krm1_t)p_kint_reg->KRM_b.KRM1);
}

/** Set function for KINT.KRM.KRM0 */
__STATIC_INLINE void HW_KINT_KRM_KRM0_Set(R_KINT_Type * p_kint_reg, hw_kint_krm_krm0_t value)
{
    p_kint_reg->KRM_b.KRM0 = value;
}
/** Get function for KINT.KRM.KRM0 */
__STATIC_INLINE hw_kint_krm_krm0_t HW_KINT_KRM_KRM0_Get(R_KINT_Type * p_kint_reg)
{
    return((hw_kint_krm_krm0_t)p_kint_reg->KRM_b.KRM0);
}

/** Set function for KINT.KRM */
__STATIC_INLINE void HW_KINT_KRM_Set(R_KINT_Type * p_kint_reg, uint8_t value)
{
    p_kint_reg->KRM = value;
}
/** Get function for KINT.KRM */
__STATIC_INLINE uint8_t HW_KINT_KRM_Get(R_KINT_Type * p_kint_reg)
{
    return(p_kint_reg->KRM);
}
/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
 
/***********************************************************************************************************************
Private function prototypes
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables
***********************************************************************************************************************/


/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_KINT_COMMON_H */
