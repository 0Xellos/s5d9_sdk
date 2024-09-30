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
 * File Name    : r_lvd.h
 * Description  : LVD HAL API header file
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup LVD LVD
 * @brief Driver for Low Voltage Detection.
 *
 * Implementation of the LVD API.For a detailed description see the @ref LVD_API.
 *
 * @section LVD_SUMMARY Summary
 * This module implements the following interface: @ref LVD_API.
 *
 * @{
 **********************************************************************************************************************/

#ifndef R_LVD_H
#define R_LVD_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_lvd_cfg.h"
#include "r_lvd_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define LVD_CODE_VERSION_MAJOR (2U)
#define LVD_CODE_VERSION_MINOR (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** LVD instance control structure */
typedef struct st_lvd_instance_ctrl
{
    /** Monitor number, 1, 2, ... */
    uint32_t                monitor_number;                             ///< Monitor number
    R_SYSTEM_Type         * p_reg;                                      ///< Pointer to LVD register base address
    void                 (* p_callback)(lvd_callback_args_t * p_args);  ///< Pointer to user callback
    lvd_callback_args_t     lvd_callback_args;                          ///< LVD callback parameters arguments
    uint32_t                opened;                                     ///< Whether or not channel is open
} lvd_instance_ctrl_t;

/** Sample clock divider, use LVD_SAMPLE_CLOCK_DISABLED to disable digital filtering */
typedef enum e_lvd_sample_clock
{
    LVD_SAMPLE_CLOCK_LOCO_DIV_2     = 0,    ///< Digital filter sample clock is LOCO divided by 2
    LVD_SAMPLE_CLOCK_LOCO_DIV_4     = 1,    ///< Digital filter sample clock is LOCO divided by 4
    LVD_SAMPLE_CLOCK_LOCO_DIV_8     = 2,    ///< Digital filter sample clock is LOCO divided by 8
    LVD_SAMPLE_CLOCK_LOCO_DIV_16    = 3,    ///< Digital filter sample clock is LOCO divided by 16
    LVD_SAMPLE_CLOCK_DISABLED       = -1,   ///< Digital filter is disabled
} lvd_sample_clock_t;

/** Negation of LVD signal follows reset or voltage in range */
typedef enum e_lvd_negation_delay
{
    /**
     *  Negation follows a stabilization time (tLVDn)
     *  after VCC > Vdet1 is detected.
     *  If a transition to software standby or deep software
     *  standby is to be made, the only possible value for
     *  the RN bit is LVD_NEGATION_DELAY_FROM_VOLTAGE
     */
    LVD_NEGATION_DELAY_FROM_VOLTAGE     = 0,
    /**
     *  Negation follows a stabilization time (tLVDn) after
     *  assertion of the LVDn reset.
     *  If a transition to software standby or deep software
     *  standby is to be made, the only possible value for
     *  the RN bit is LVD_NEGATION_DELAY_FROM_VOLTAGE
     */
    LVD_NEGATION_DELAY_FROM_RESET       = 1,
} lvd_negation_delay_t;

/** Hardware extend structure */
typedef struct st_lvd_extend
{
    /** Negation of LVD signal follows reset or voltage in range */
    lvd_negation_delay_t    negation_delay;
    /** Sample clock divider, use LVD_SAMPLE_CLOCK_DISABLED to disable digital filtering */
    lvd_sample_clock_t      sample_clock_divisor;
} lvd_extend_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const lvd_api_t g_lvd_on_lvd;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_LVD_H */

/*******************************************************************************************************************//**
 * @} (end defgroup LVD)
 **********************************************************************************************************************/
