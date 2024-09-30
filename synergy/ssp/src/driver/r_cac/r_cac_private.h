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
 * File Name    : r_cac_private.h
 * Description  : Private include file for CAC
 **********************************************************************************************************************/
#ifndef R_CAC_PRIVATE_H
#define R_CAC_PRIVATE_H

#include "r_cgc_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER
/*******************************************************************************************************************//**
 * @addtogroup R_CAC
 * @{
 **********************************************************************************************************************/


/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define CGC_CLK_NOT_SUPPORTED 0xFFU
#define CAC_IWDT_FREQ         (15000)       // IWDT clock is 15 kHz

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
const uint8_t cac_clock_to_cgc_clock[] =
{
    (uint8_t) CGC_CLOCK_MAIN_OSC,            //CAC_CLOCK_SOURCE_MAIN_OSC  = 0x00,
    (uint8_t) CGC_CLOCK_SUBCLOCK,            //CAC_CLOCK_SOURCE_SUBCLOCK  = 0x01,
    (uint8_t) CGC_CLOCK_HOCO,                //CAC_CLOCK_SOURCE_HOCO      = 0x02,
    (uint8_t) CGC_CLOCK_MOCO,                //CAC_CLOCK_SOURCE_MOCO      = 0x03,
    (uint8_t) CGC_CLOCK_LOCO,                //CAC_CLOCK_SOURCE_LOCO      = 0x04,
    CGC_CLK_NOT_SUPPORTED,                   //CAC_IWDT  ==> Not supported by CGC (15 kHz)
    CGC_CLK_NOT_SUPPORTED,                   //CAC_PCLKB ==> Not supported by CGC
    CGC_CLK_NOT_SUPPORTED                    //CAC_EXTERNAL ==> Not supported by CGC
};

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_CAC_PRIVATE_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup R_CAC)
 **********************************************************************************************************************/
