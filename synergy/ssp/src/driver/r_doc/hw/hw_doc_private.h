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
 * File Name    : hw_doc_private.h
 * Description  : Data Operation Circuit (DOC) HW layer APIs.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @addtogroup R_DOC
 * @{
 **********************************************************************************************************************/
#ifndef HW_DOC_PRIVATE_H
#define HW_DOC_PRIVATE_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_doc.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Prototypes
 **********************************************************************************************************************/
__STATIC_INLINE void HW_DOC_Write (R_DOC_Type * p_doc_reg, doc_data_t * const p_data);

__STATIC_INLINE void HW_DOC_DOCRWrite (R_DOC_Type * p_doc_reg, uint8_t data);

__STATIC_INLINE void HW_DOC_DODIRWrite (R_DOC_Type * p_doc_reg, doc_size_t data);

__STATIC_INLINE bool HW_DOC_INTRead (R_DOC_Type * p_doc_reg);

__STATIC_INLINE void HW_DOC_INTClear (R_DOC_Type * p_doc_reg);

__STATIC_INLINE void HW_DOC_DODIRAddressGet (R_DOC_Type * p_doc_reg, volatile doc_size_t ** p_addr);

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "common/hw_doc_common.h"

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_DOC_PRIVATE_H */
/*******************************************************************************************************************//**
 * @} (end addtogroup R_DOC)
 **********************************************************************************************************************/
