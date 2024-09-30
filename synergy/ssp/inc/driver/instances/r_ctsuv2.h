/***********************************************************************************************************************
 * Copyright [2021-2024] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : r_ctsuv2.h
 * Description  : CTSU HAL module header
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup CTSU_V2 CTSU v2
 * @brief Driver for the Capacitive Touch Sensing Unit (CTSU).
 *
 * @{
 **********************************************************************************************************************/

#ifndef R_CTSU_H
#define R_CTSU_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#ifdef QE_TOUCH_CONFIGURATION
 #include "qe_touch_define.h"
#endif
#include "r_ctsuv2_cfg.h"
#include "r_ctsuv2_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define CTSU_CODE_VERSION_MAJOR       (2U)
#define CTSU_CODE_VERSION_MINOR       (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** CTSU run state */
typedef enum e_ctsu_state
{
    CTSU_STATE_INIT,                   ///< Not open.
    CTSU_STATE_IDLE,                   ///< Opened.
    CTSU_STATE_SCANNING,               ///< Scanning now.
    CTSU_STATE_SCANNED                 ///< Scan end.
} ctsu_state_t;

/** CTSU Initial offset tuning status */
typedef enum e_ctsu_tuning
{
    CTSU_TUNING_INCOMPLETE,            ///< Initial offset tuning incomplete
    CTSU_TUNING_COMPLETE               ///< Initial offset tuning complete
} ctsu_tuning_t;

/** CTSU Correction status */
typedef enum e_ctsu_correction_status
{
    CTSU_CORRECTION_INIT,              ///< Correction initial status.
    CTSU_CORRECTION_RUN,               ///< Correction scan running.
    CTSU_CORRECTION_COMPLETE,          ///< Correction complete.
    CTSU_CORRECTION_ERROR              ///< Correction error.
} ctsu_correction_status_t;

 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
/** CTSU Diagnosis status */
typedef enum e_ctsu_diagnosis_status
{
    CTSU_DIAG_INIT,                    ///< Diagnosis initial status.
    CTSU_DIAG_OVER_VOLTAGE,            ///< Diagnosis of over_voltage running.
    CTSU_DIAG_CCO_HIGH,                ///< Diagnosis of oscillator 19.2uA running.
    CTSU_DIAG_CCO_LOW,                 ///< Diagnosis of oscillator 2.4uA running.
    CTSU_DIAG_SSCG,                    ///< Diagnosis of sscg_oscillator running.
    CTSU_DIAG_DAC,                     ///< Diagnosis of dac running.
    CTSU_DIAG_COMPLETE                 ///< Diagnosis complete.
} ctsu_diagnosis_status_t;
 #endif

/** CTSUWR write register value */
typedef struct st_ctsu_wr
{
    uint16_t ctsussc;                  ///< Copy from (ssdiv << 8) by Open API.
    uint16_t ctsuso0;                  ///< Copy from ((snum << 10) | so) by Open API.
    uint16_t ctsuso1;                  ///< Copy from (sdpa << 8) by Open API. ICOG and RICOA is set recommend value.
} ctsu_ctsuwr_t;

/** Scan buffer data formats (Self) */
typedef struct st_ctsu_self_buf
{
    uint16_t sen;                      ///< Sensor counter data
    uint16_t ref;                      ///< Reference counter data (Not used)
} ctsu_self_buf_t;

/** Scan buffer data formats (Mutual) */
typedef struct st_ctsu_mutual_buf
{
    uint16_t pri_sen;                  ///< Primary sensor data
    uint16_t pri_ref;                  ///< Primary reference data (Not used)
    uint16_t snd_sen;                  ///< Secondary sensor data
    uint16_t snd_ref;                  ///< Secondary reference data (Not used)
} ctsu_mutual_buf_t;

/** Correction information */
typedef struct st_ctsu_correction_info
{
    ctsu_correction_status_t status;                                 ///< Correction status
    ctsu_ctsuwr_t            ctsuwr;                                 ///< Correction scan parameter
    volatile ctsu_self_buf_t scanbuf;                                ///< Correction scan buffer
    uint16_t first_val;                                              ///< 1st correction value
    uint16_t second_val;                                             ///< 2nd correction value
    uint32_t first_coefficient;                                      ///< 1st correction coefficient
    uint32_t second_coefficient;                                     ///< 2nd correction coefficient
    uint32_t ctsu_clock;                                             ///< CTSU clock [MHz]
} ctsu_correction_info_t;

 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
/** Correction information */
typedef struct st_ctsu_diag_info
{
    volatile ctsu_diagnosis_status_t state; ///< Diagnosis state
    ctsu_ctsuwr_t            ctsuwr;        ///< Correction scan parameter
    uint8_t                  loop_count;    ///< Diagnosis loop counter
    volatile ctsu_self_buf_t scanbuf;       ///< Diagnosis scan buffer
    uint16_t                 correct_data;  ///< Diagnosis correct value
    volatile uint8_t         icomp;         ///< Diagnosis icomp flag
    uint16_t                 cco_high;      ///< Diagnosis cco high count
    uint16_t                 cco_low;       ///< Diagnosis cco low count
    uint16_t                 sscg;          ///< Diagnosis sscg count
    uint16_t                 dac_cnt[6];    ///< Diagnosis dac count
    uint16_t                 so0_4uc_val;   ///< Diagnosis SO value at 4UC in dac test
    uint16_t                 dac_init;      ///< Diagnosis initialize flag in dac test
    ctsu_tuning_t            tuning;        ///< Diagnosis tuning flag in dac test
    int32_t                  tuning_diff;   ///< Diagnosis dac initial tuning differance value
} ctsu_diag_info_t;
 #endif

/** CTSU private control block. DO NOT MODIFY. Initialization occurs when R_CTSU_Open() is called. */
typedef struct st_ctsu_instance_ctrl
{
    uint32_t                 open;               ///< Whether or not driver is open.
    ctsu_state_t             state;              ///< CTSU run state.
    ctsu_md_t                md;                 ///< CTSU Measurement Mode Select(copy to cfg)
    ctsu_tuning_t            tuning;             ///< CTSU Initial offset tuning status.
    uint16_t                 num_elements;       ///< Number of elements to scan
    uint16_t                 wr_index;           ///< Word index into ctsuwr register array.
    uint16_t                 rd_index;           ///< Word index into scan data buffer.
    uint8_t                * p_tuning_count;     ///< Pointer to tuning count of each element. g_ctsu_tuning_count[] is set by Open API.
    int32_t                * p_tuning_diff;      ///< Pointer to difference from base value of each element. g_ctsu_tuning_diff[] is set by Open API.
    uint16_t                 average;            ///< CTSU Moving average counter.
    uint16_t                 num_moving_average; ///< Copy from config by Open API.
    uint8_t                  ctsucr1;            ///< Copy from (atune1 << 3, md << 6) by Open API. CLK, ATUNE0, CSW, and PON is set by HAL driver.
    ctsu_ctsuwr_t          * p_ctsuwr;           ///< CTSUWR write register value. g_ctsu_ctsuwr[] is set by Open API.
    ctsu_self_buf_t        * p_self_raw;         ///< Pointer to Self raw data. g_ctsu_self_raw[] is set by Open API.
    uint16_t               * p_self_data;        ///< Pointer to Self moving average data. g_ctsu_self_data[] is set by Open API.
    ctsu_mutual_buf_t      * p_mutual_raw;       ///< Pointer to Mutual raw data. g_ctsu_mutual_raw[] is set by Open API.
    uint16_t               * p_mutual_pri_data;  ///< Pointer to Mutual primary moving average data. g_ctsu_mutual_pri_data[] is set by Open API.
    uint16_t               * p_mutual_snd_data;  ///< Pointer to Mutual secondary moving average data. g_ctsu_mutual_snd_data[] is set by Open API.
    ctsu_correction_info_t * p_correction_info;  ///< Pointer to correction info
    uint8_t                  ctsuchac0;          ///< TS00-TS07 enable mask
    uint8_t                  ctsuchac1;          ///< TS08-TS15 enable mask
    uint8_t                  ctsuchac2;          ///< TS16-TS23 enable mask
    uint8_t                  ctsuchac3;          ///< TS24-TS31 enable mask
    uint8_t                  ctsuchac4;          ///< TS32-TS39 enable mask
    uint8_t                  ctsuchtrc0;         ///< TS00-TS07 mutual-tx mask
    uint8_t                  ctsuchtrc1;         ///< TS08-TS15 mutual-tx mask
    uint8_t                  ctsuchtrc2;         ///< TS16-TS23 mutual-tx mask
    uint8_t                  ctsuchtrc3;         ///< TS24-TS31 mutual-tx mask
    uint8_t                  ctsuchtrc4;         ///< TS32-TS39 mutual-tx mask
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    ctsu_diag_info_t * p_diag_info;              ///< pointer to diagnosis info
 #endif
    ctsu_cfg_t const * p_ctsu_cfg;               ///< Pointer to initial configurations.
    IRQn_Type          write_irq;                ///< Copy from config by Open API. CTSU_CTSUWR interrupt vector
    IRQn_Type          read_irq;                 ///< Copy from config by Open API. CTSU_CTSURD interrupt vector
    IRQn_Type          end_irq;                  ///< Copy from config by Open API. CTSU_CTSUFN interrupt vector
    void (* p_callback)(ctsu_callback_args_t *); ///< Callback provided when a CTSUFN occurs.
    ctsu_callback_args_t * p_callback_memory;    ///< Pointer to non-secure memory that can be used to pass arguments to a callback in non-secure memory.
    void const           * p_context;            ///< Placeholder for user data.
    R_CTSU_Type          * p_reg;                ///< Pointer to base register address
    uint16_t tuning_self_target_value;           ///< Target self value for initial offset tuning
    uint16_t tuning_mutual_target_value;         ///< Target mutual value for initial offset tuning
} ctsu_instance_ctrl_t;

typedef struct st_ctsu_diag_save_reg
{
    uint8_t  ctsucr0;
    uint8_t  ctsucr1;
    uint8_t  ctsusdprs;
    uint8_t  ctsusst;
    uint8_t  ctsuchac0;
    uint8_t  ctsuchac1;
    uint8_t  ctsuchac2;
    uint8_t  ctsuchtrc0;
    uint8_t  ctsuchtrc1;
    uint8_t  ctsuchtrc2;
    uint8_t  ctsudclkc;
    uint16_t ctsuerrs;
} ctsu_diag_save_reg_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const ctsu_api_t g_ctsu_on_ctsu;

/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_CTSU_H */

/*******************************************************************************************************************//**
 * @} (end defgroup CTSU_V2)
 **********************************************************************************************************************/
