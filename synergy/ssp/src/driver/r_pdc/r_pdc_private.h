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
 * File Name    : r_pdc_private.h
 * Description  : Parallel Data Capture Unit (PDC) Module private header file.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @addtogroup R_PDC
 * @{
 **********************************************************************************************************************/

#ifndef R_PDC_PRIVATE_H
#define R_PDC_PRIVATE_H

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define PDC_SIGNAL_STATUS_FLAGS_VSYNC   1U
#define PDC_SIGNAL_STATUS_FLAGS_HSYNC   2U
#define PDC_STATUS_FLAGS_FBSY           0x01U
#define PDC_STATUS_FLAGS_FEMPF          0x02U
#define PDC_STATUS_FLAGS_FEF            0x04U
#define PDC_STATUS_FLAGS_OVRF           0x08U
#define PDC_STATUS_FLAGS_UDRF           0x10U
#define PDC_STATUS_FLAGS_VERF           0x20U
#define PDC_STATUS_FLAGS_HERF           0x40U

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

#endif /* R_PDC_PRIVATE_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup R_PDC)
 **********************************************************************************************************************/
