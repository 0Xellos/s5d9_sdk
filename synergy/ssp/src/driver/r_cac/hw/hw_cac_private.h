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
 * File Name    : hw_cac_private.h
 * Description  : Clock Accuracy Circuit (CAC) Module hardware private header file.
 **********************************************************************************************************************/

#ifndef HW_CAC_PRIVATE_H
#define HW_CAC_PRIVATE_H


/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER


/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_cac.h"
#include "r_cgc_api.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Prototypes
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * Command the CAC to start measuring
 * @return  None
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAC_StartMeasuring (R_CAC_Type * p_cac_reg)
{
    p_cac_reg->CACR0_b.CFME = 1U;
}

/*******************************************************************************************************************//**
 * Command the CAC to stop measuring
 * @return  None
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAC_StopMeasuring (R_CAC_Type * p_cac_reg)
{
    p_cac_reg->CACR0_b.CFME = 0U;
}

/*******************************************************************************************************************//**
 * Reset the CAC
 * @return  None
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAC_Reset (R_CAC_Type * p_cac_reg)
{
    p_cac_reg->CAICR_b.OVFFCL  = 1U;
    p_cac_reg->CAICR_b.FERRFCL = 1U;
    p_cac_reg->CAICR_b.MENDFCL = 1U;
}

/*******************************************************************************************************************//**
 * Read and return the CAC status register and the CAC counter buffer register.
 * @param[out]  p_status      Pointer to variable in which to store the current CASTR register contents.
 * @param[out]  p_counter     Pointer to variable in which to store the current CACNTBR register contents
 * @return  None
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAC_Read (R_CAC_Type * p_cac_reg, uint8_t * const p_status, uint16_t * const p_counter)
{
    *p_status  = p_cac_reg->CASTR;
    *p_counter = p_cac_reg->CACNTBR;
}


/*******************************************************************************************************************//**
 * Disable the CAC interrupts in the CAC peripheral
 * @param[in]  None.
 * @return     None
 **********************************************************************************************************************/
__STATIC_INLINE void HW_CAC_DisableCACInterrupts (R_CAC_Type * p_cac_reg)
{
    p_cac_reg->CAICR_b.FERRIE = 0U;           // freq error interrupt disabled
    p_cac_reg->CAICR_b.OVFIE  = 0U;           // overflow error interrupt disabled
    p_cac_reg->CAICR_b.MENDIE = 0U;           // measurement complete interrupt disabled
}

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_CAC_PRIVATE_H */
