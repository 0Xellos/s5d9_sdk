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
 * File Name    : hw_doc_common.h
 * Description  : LLDs for the Data Operation Circuit (DOC) module driver.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @ingroup R_DOC
 * @{
***********************************************************************************************************************/
#ifndef HW_DOC_COMMON_H
#define HW_DOC_COMMON_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_doc.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

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
 * Writes to the DODIR and DODSR registers.
 * When changing both the DODSR and DODIR the DODSR should be updated first.
 * @param   p_data		Pointer to data to be written to DOC DODIR and DODSR registers.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_DOC_Write (R_DOC_Type * p_doc_reg, doc_data_t * const p_data)
{
    p_doc_reg->DODSR = p_data->dodsr;
    p_doc_reg->DODIR = p_data->dodir;
}

/*******************************************************************************************************************//**
 * Writes to the DOCR register.
 * @param   data	Value to be written to DOCR.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_DOC_DOCRWrite (R_DOC_Type * p_doc_reg, uint8_t data)
{
    p_doc_reg->DOCR = data;
}

/*******************************************************************************************************************//**
 * Writes to the DOCR register.
 * @param   data	Value to be written to DODIR.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_DOC_DODIRWrite (R_DOC_Type * p_doc_reg, doc_size_t data)
{
    p_doc_reg->DODIR = data;
}

/*******************************************************************************************************************//**
 * Reads state of DOPCF in DOCR register.
 * @retval  Value of DOPCF bit.
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_DOC_INTRead (R_DOC_Type * p_doc_reg)
{
    return p_doc_reg->DOCR_b.DOPCF;
}

/*******************************************************************************************************************//**
 * Clears DOPCF bit in DOCR by writing 1 to DOPCFCL bit.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_DOC_INTClear (R_DOC_Type * p_doc_reg)
{
    p_doc_reg->DOCR_b.DOPCFCL = 1;
}

/*******************************************************************************************************************//**
 * Supplies the address of the DODIR register.
 **********************************************************************************************************************/
__STATIC_INLINE void HW_DOC_DODIRAddressGet (R_DOC_Type * p_doc_reg, volatile doc_size_t ** p_addr)
{
    *p_addr = &p_doc_reg->DODIR;
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_DOC_COMMON_H */
/*******************************************************************************************************************//**
 * @} (end ingroup R_DOC)
 **********************************************************************************************************************/
