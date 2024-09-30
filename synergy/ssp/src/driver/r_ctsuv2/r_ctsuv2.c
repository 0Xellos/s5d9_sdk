/***********************************************************************************************************************
 * Copyright [2024] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : r_ctsuv2.c
 * Description  : CTSU HAL driver
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#ifdef QE_TOUCH_CONFIGURATION
 #include "qe_touch_define.h"
#endif
#include "hw/hw_ctsuv2_private.h"
#include "r_ctsuv2_private_api.h"
#include "r_ctsuv2.h"
#include "r_ioport.h"
#include "r_cgc.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** Macro for error logger. */
#ifndef CTSU_ERROR_RETURN
 /*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
 #define CTSU_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_ctsu_version)
#endif

/** "CTSU" in ASCII, used to determine if device is open. */
#define CTSU_OPEN                            (0x43545355U)

/* Macro definitions for instance */
#define CTSU_INSTANCE_MAX                    ((uint8_t) 8U)

/* Macro definitions for register setting */
#define CTSU_CORRECTION_AVERAGE              ((uint16_t) 32U)
#define CTSU_SHIFT_AMOUNT                    (15)
#define CTSU_COUNT_MAX                       ((uint16_t) 0xFFFFU)
#define CTSU_PCLKB_FREQ_MHZ                  ((uint32_t) 1000000U)
#define CTSU_PCLKB_FREQ_RANGE1               ((uint32_t) 32U)
#define CTSU_PCLKB_FREQ_RANGE2               ((uint32_t) 64U)
#define CTSU_WAIT_TIME                       ((uint32_t) 500U)

/* Macro definitions for initial offset tuning */
#define CTSU_TUNING_MAX                      ((uint16_t) 0x03FFU)
#define CTSU_TUNING_MIN                      ((uint16_t) 0x0000U)
#define CTSU_TUNING_VALUE_SELF               ((uint16_t) 15360U)
#define CTSU_TUNING_VALUE_MUTUAL             ((uint16_t) 10240U)
#define CTSU_TUNING_OT_COUNT                 ((uint8_t)  25U)

#define CTSU_SNUM_MAX                        ((uint16_t) 0x3FU) // The maximum value of SNUM
#define CTSU_SDPA_MAX                        ((uint16_t) 0x1FU) // The maximum value of SDPA
#define CTSU_RICOA_RECOMMEND                 ((uint16_t) 0x0FU) // Recommended setting value
#define CTSU_ICOG_100                        ((uint16_t) 0U)    // ICOG = 100%
#define CTSU_ICOG_66                         ((uint16_t) 1U)    // ICOG = 66%
#define CTSU_ICOG_50                         ((uint16_t) 2U)    // ICOG = 50%
#define CTSU_ICOG_40                         ((uint16_t) 3U)    // ICOG = 40%

/* Macro definitions for correction */
#if (BSP_CFG_MCU_PART_SERIES == 1) || (BSP_CFG_MCU_PART_SERIES == 3) || (BSP_CFG_MCU_PART_SERIES == 7)
 #define CTSU_CORRECTION_1ST_STD_VAL         (40960UL)           // ICOG = 66%
 #define CTSU_CORRECTION_2ND_STD_VAL         (23961UL)           // ICOG = 40%, (x = 40960 * 40 / 66) * 0.96523525
 #define CTSU_ICOG_RECOMMEND                 (CTSU_ICOG_66)      // Recommended setting value
#endif
#if (BSP_CFG_MCU_PART_SERIES == 5)
 #define CTSU_CORRECTION_1ST_STD_VAL         (27306UL)           // ICOG = 66%, (x = 40960 * 66 / 100)
 #define CTSU_CORRECTION_2ND_STD_VAL         (16384UL)           // ICOG = 40%, (x = 40960 * 40 / 100) * 1
 #define CTSU_ICOG_RECOMMEND                 (CTSU_ICOG_100)     // Recommended setting value
#endif

#if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
 #define CTSU_DIAG_DAC_1UC                   ((uint16_t) 0x0010U) // 0x10 for so dac value
 #define CTSU_DIAG_DAC_2UC                   ((uint16_t) 0x0020U) // 0x20 for so dac value
 #define CTSU_DIAG_DAC_4UC                   ((uint16_t) 0x0040U) // 0x40 for so dac value
 #define CTSU_DIAG_DAC_8UC                   ((uint16_t) 0x0080U) // 0x80 for so dac value
 #define CTSU_DIAG_DAC_16UC                  ((uint16_t) 0x0100U) // 0x100 for so dac value

 #define CTSU_DIAG_DAC_SO_MAX                ((uint16_t) 0x03FFU) // so dac max

 #define CTSU_DIAG_DAC_INIT_VALUE            ((uint16_t) 241U)    // SO value of dac test
 #define CTSU_DIAG_DAC_TARGET_VALUE          ((uint16_t) 15360U)  // 6UC value dac test target
 #define CTSU_DIAG_DAC_START_VALUE           ((uint16_t) 0x0100U) // so value dac test tuning
#endif

/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/
typedef struct st_ctsu_correction_calc
{
    uint16_t     snum;
    uint16_t     sdpa;
} ctsu_correction_calc_t;

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static ssp_err_t ctsu_hw_initialize (ctsu_instance_ctrl_t * const p_instance_ctrl);
static ssp_err_t ctsu_sw_initialize (ctsu_instance_ctrl_t * const p_instance_ctrl);
#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
static ssp_err_t ctsu_transfer_open(ctsu_instance_ctrl_t * const p_instance_ctrl);
static ssp_err_t ctsu_transfer_close(ctsu_instance_ctrl_t * const p_instance_ctrl);
static ssp_err_t ctsu_transfer_configure(ctsu_instance_ctrl_t * const p_instance_ctrl);
#endif
static void ctsu_state_scanned_process (ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_initial_offset_tuning(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_initial_offset_calc (uint16_t * const p_ctsuso0, int32_t * const p_tuning_diff,
                                      int32_t diff_value, uint32_t * const p_complete_flag);
static void ctsu_moving_average(uint16_t * p_average, uint16_t new_data, uint16_t average_num);
static ssp_err_t ctsu_correction_process(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_correction_measurement(ctsu_instance_ctrl_t * const p_instance_ctrl, uint16_t * data);
static void ctsu_correction_calc(uint16_t * correction_data, uint16_t raw_data, ctsu_correction_calc_t * p_calc);
static void ctsu_correction_exec(ctsu_instance_ctrl_t * const p_instance_ctrl);
void        ctsu_write_isr(void);
void        ctsu_read_isr(void);
void        ctsu_end_isr(void);

#if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
static void ctsu_diag_dac_initial_tuning(void);

static void ctsu_diag_ldo_over_voltage_scan_start(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_diag_oscillator_high_scan_start(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_diag_oscillator_low_scan_start(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_diag_sscg_scan_start(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_diag_dac_scan_start(ctsu_instance_ctrl_t * const p_instance_ctrl);

static void ctsu_diag_oscillator_high_data_get(void);
static void ctsu_diag_oscillator_low_data_get(void);
static void ctsu_diag_sscg_data_get(void);
static void ctsu_diag_dac_data_get(void);

static ssp_err_t ctsu_diag_ldo_over_voltage_result(void);
static ssp_err_t ctsu_diag_oscillator_high_result(void);
static ssp_err_t ctsu_diag_oscillator_low_result(void);
static ssp_err_t ctsu_diag_sscg_result(void);
static ssp_err_t ctsu_diag_dac_result(void);

static void      ctsu_diag_scan_start1(ctsu_instance_ctrl_t * const p_instance_ctrl);
static ssp_err_t ctsu_diag_data_get1(ctsu_instance_ctrl_t * const p_instance_ctrl);
#endif

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
/** Name of module used by error logger macro */
#if (BSP_CFG_ERROR_LOG != 0)
static const char g_module_name[] = "ctsu";
#endif

/** Version data structure used by error logger macro. */
static const ssp_version_t g_ctsu_version =
{
    .api_version_minor  = CTSU_API_VERSION_MINOR,
    .api_version_major  = CTSU_API_VERSION_MAJOR,
    .code_version_major = CTSU_CODE_VERSION_MAJOR,
    .code_version_minor = CTSU_CODE_VERSION_MINOR
};

static uint8_t       g_ctsu_instance_cnt  = 0U;
static uint16_t      g_ctsu_element_index = 0U;
static uint8_t       g_ctsu_tuning_count[CTSU_CFG_NUM_SELF_ELEMENTS + CTSU_CFG_NUM_MUTUAL_ELEMENTS] = {0U};
static int32_t       g_ctsu_tuning_diff[CTSU_CFG_NUM_SELF_ELEMENTS + CTSU_CFG_NUM_MUTUAL_ELEMENTS]  = {0};
static ctsu_ctsuwr_t g_ctsu_ctsuwr[CTSU_CFG_NUM_SELF_ELEMENTS + CTSU_CFG_NUM_MUTUAL_ELEMENTS]       = {0U};
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
static uint16_t        g_ctsu_self_element_index = 0U;
static ctsu_self_buf_t g_ctsu_self_raw[CTSU_CFG_NUM_SELF_ELEMENTS]  = {0U};
static uint16_t        g_ctsu_self_data[CTSU_CFG_NUM_SELF_ELEMENTS] = {0U};
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
static uint16_t          g_ctsu_mutual_element_index = 0U;
static ctsu_mutual_buf_t g_ctsu_mutual_raw[CTSU_CFG_NUM_MUTUAL_ELEMENTS]      = {0U};
static uint16_t          g_ctsu_mutual_pri_data[CTSU_CFG_NUM_MUTUAL_ELEMENTS] = {0U};
static uint16_t          g_ctsu_mutual_snd_data[CTSU_CFG_NUM_MUTUAL_ELEMENTS] = {0U};
#endif
static ctsu_correction_info_t g_ctsu_correction_info =
{
    .status             = CTSU_CORRECTION_INIT,
    .ctsuwr             = {0U},
    .scanbuf            = {0U},
    .first_val          = 0U,
    .second_val         = 0U,
    .first_coefficient  = 0U,
    .second_coefficient = 0U,
    .ctsu_clock         = 0U,
};

#if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
static ctsu_diag_info_t     g_ctsu_diag_info =
{
    .state              = CTSU_DIAG_INIT,
    .ctsuwr             = {0U},
    .loop_count         = 0U,
    .scanbuf            = {0U},
    .correct_data       = 0U,
    .icomp              = 0U,
    .cco_high           = 0U,
    .cco_low            = 0U,
    .sscg               = 0U,
    .dac_cnt            = {0U},
    .so0_4uc_val        = 0U,
    .dac_init           = 0U,
    .tuning             = CTSU_TUNING_INCOMPLETE,
    .tuning_diff        = (int32_t) 0,
};
static ctsu_diag_save_reg_t g_ctsu_diag_reg =
{
    .ctsucr0            = 0U,
    .ctsucr1            = 0U,
    .ctsusdprs          = 0U,
    .ctsusst            = 0U,
    .ctsuchac0          = 0U,
    .ctsuchac1          = 0U,
    .ctsuchac2          = 0U,
    .ctsuchtrc0         = 0U,
    .ctsuchtrc1         = 0U,
    .ctsuchtrc2         = 0U,
    .ctsudclkc          = 0U,
    .ctsuerrs           = 0U,
};
#endif

static ioport_pin_cfg_t       g_ctsu_tscap_pin_cfg_data =
{
    .pin = (ioport_port_pin_t) CTSU_CFG_TSCAP_PORT,
};
static const ioport_cfg_t g_ctsu_tscap_pin_cfg =
{
    .number_of_pins = (uint16_t) 1U,
    .p_pin_cfg_data = &g_ctsu_tscap_pin_cfg_data,
};

static bsp_feature_ctsu_t g_ctsu_bsp_feature = {0U};

#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 0)
static uint8_t g_ctsu_interrupt_reverse_flag = 0U;
#endif

#if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
static const uint16_t dac_oscil_table[6][2] =
{
    {CTSU_CFG_DIAG_DAC1_MAX, CTSU_CFG_DIAG_DAC1_MIN},
    {CTSU_CFG_DIAG_DAC2_MAX, CTSU_CFG_DIAG_DAC2_MIN},
    {CTSU_CFG_DIAG_DAC3_MAX, CTSU_CFG_DIAG_DAC3_MIN},
    {CTSU_CFG_DIAG_DAC4_MAX, CTSU_CFG_DIAG_DAC4_MIN},
    {CTSU_CFG_DIAG_DAC5_MAX, CTSU_CFG_DIAG_DAC5_MIN},
    {CTSU_CFG_DIAG_DAC6_MAX, CTSU_CFG_DIAG_DAC6_MIN},
};
#endif

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const ctsu_api_t g_ctsu_on_ctsu =
{
    .open        = R_CTSU_Open,
    .scanStart   = R_CTSU_ScanStart,
    .dataGet     = R_CTSU_DataGet,
    .diagnosis   = R_CTSU_Diagnosis,
    .close       = R_CTSU_Close,
    .callbackSet = R_CTSU_CallbackSet,
    .versionGet  = R_CTSU_VersionGet,
};

/*******************************************************************************************************************//**
 * @addtogroup CTSU_V2
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Opens and configures the CTSU driver module. Implements @ref ctsu_api_t::open.
 *
 * @retval SSP_SUCCESS              CTSU successfully configured.
 * @retval SSP_ERR_ASSERTION        Null pointer, or one or more configuration options is invalid.
 * @retval SSP_ERR_ALREADY_OPEN     Module is already open.  This module can only be opened once.
 * @retval SSP_ERR_INVALID_ARGUMENT Configuration parameter error.
 * @retval SSP_ERR_IN_USE           Control block has already been opened or channel is being used by another
 *                                  instance. Call close() then open() to reconfigure.
 * @retval SSP_ERR_IRQ_BSP_DISABLED A required interrupt does not exist in the vector table
 *
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * fmi_api_t::productFeatureGet
 *                                   * fmi_api_t::eventInfoGet
 *                                   * cgc_api_t::systemClockFreqGet
 *                                   * transfer_api_t::open
 *
 * @note In the first Open, measurement for correction works, and it takes several tens of milliseconds.
 **********************************************************************************************************************/
ssp_err_t R_CTSU_Open (ctsu_ctrl_t * const p_ctrl, ctsu_cfg_t const * const p_cfg)
{
    ctsu_instance_ctrl_t     * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;
    ssp_err_t                  err             = SSP_SUCCESS;

#if (CTSU_CFG_PARAM_CHECKING_ENABLE == 1)
    SSP_ASSERT(p_instance_ctrl);
    SSP_ASSERT(p_cfg);
#endif
    CTSU_ERROR_RETURN(CTSU_OPEN != p_instance_ctrl->open, SSP_ERR_ALREADY_OPEN);

    /* Check element number */
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == p_cfg->md)
    {
        CTSU_ERROR_RETURN(CTSU_CFG_NUM_SELF_ELEMENTS >=
                         (uint8_t) (g_ctsu_self_element_index + p_cfg->num_rx),
                         SSP_ERR_INVALID_ARGUMENT);
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_cfg->md)
    {
        CTSU_ERROR_RETURN(CTSU_CFG_NUM_MUTUAL_ELEMENTS >=
                         (uint8_t) (g_ctsu_mutual_element_index + (p_cfg->num_rx * p_cfg->num_tx)),
                         SSP_ERR_INVALID_ARGUMENT);
    }
#endif

    /* Initialize hardware instance */
    err = ctsu_hw_initialize(p_instance_ctrl);
    CTSU_ERROR_RETURN(SSP_SUCCESS == err, err);

    p_instance_ctrl->state = CTSU_STATE_INIT;

    /* Save configurations. */
    p_instance_ctrl->p_ctsu_cfg = p_cfg;

    /* Initialize software instance */
    err = ctsu_sw_initialize(p_instance_ctrl);

    /* Mark driver as open */
    p_instance_ctrl->open = CTSU_OPEN;

    return err;
}

/*******************************************************************************************************************//**
 * @brief This function should be called each time a periodic timer expires.
 * If initial offset tuning is enabled, The first several calls are used to tuning for the sensors.
 * Before starting the next scan, first get the data with R_CTSU_DataGet().
 * If a different control block scan should be run, check the scan is complete before executing.
 * Implements @ref ctsu_api_t::scanStart.
 *
 * @retval SSP_SUCCESS              CTSU successfully configured.
 * @retval SSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval SSP_ERR_NOT_OPEN         Module is not open.
 * @retval SSP_ERR_CTSU_SCANNING    Scanning this instance or other.
 * @retval SSP_ERR_CTSU_NOT_GET_DATA    The previous data has not been retrieved by DataGet.
 **********************************************************************************************************************/
ssp_err_t R_CTSU_ScanStart (ctsu_ctrl_t * const p_ctrl)
{
    ssp_err_t              err             = SSP_SUCCESS;
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;

#if (CTSU_CFG_PARAM_CHECKING_ENABLE == 1)
    SSP_ASSERT(p_instance_ctrl);
    CTSU_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, SSP_ERR_NOT_OPEN);
#endif
    if (CTSU_CAP_SOFTWARE == p_instance_ctrl->p_ctsu_cfg->cap)
    {
        /* Can be checked if the previous measurement was a software trigger */
        if (HW_CTSU_IsSoftwareTrigger(p_instance_ctrl->p_reg))
        {
            CTSU_ERROR_RETURN(HW_CTSU_IsMeasurementStopped(p_instance_ctrl->p_reg), SSP_ERR_CTSU_SCANNING);
        }
    }

    CTSU_ERROR_RETURN(CTSU_STATE_SCANNED != p_instance_ctrl->state, SSP_ERR_CTSU_NOT_GET_DATA);

    ctsu_isr_context_set(p_instance_ctrl->write_irq, p_instance_ctrl);
    ctsu_isr_context_set(p_instance_ctrl->read_irq, p_instance_ctrl);
    ctsu_isr_context_set(p_instance_ctrl->end_irq, p_instance_ctrl);

#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
    err = ctsu_transfer_configure(p_instance_ctrl);
    CTSU_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    /* Set the measurement start trigger and the measurement mode */
    HW_CTSU_MeasurementSet(p_instance_ctrl->p_reg, p_instance_ctrl);

    /* Write Channel setting */
    HW_CTSU_ChannelSet(p_instance_ctrl->p_reg, &g_ctsu_bsp_feature, p_instance_ctrl->p_ctsu_cfg);

#if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        ctsu_diag_scan_start1(p_instance_ctrl);
    }
#endif

    p_instance_ctrl->state = CTSU_STATE_SCANNING;

    /* Set CTSU_STRT bit to start scan */
    HW_CTSU_MeasurementStart(p_instance_ctrl->p_reg);   ///< CTSU_STRT

    return err;
}

/*******************************************************************************************************************//**
 * @brief This function gets the sensor values as scanned by the CTSU.
 * If initial offset tuning is enabled, The first several calls are used to tuning for the sensors.
 * Implements @ref ctsu_api_t::dataGet.
 *
 * @retval SSP_SUCCESS              CTSU successfully configured.
 * @retval SSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval SSP_ERR_NOT_OPEN         Module is not open.
 * @retval SSP_ERR_CTSU_SCANNING    Scanning this instance.
 * @retval SSP_ERR_CTSU_INCOMPLETE_TUNING      Incomplete initial offset tuning.
 * @retval SSP_ERR_CTSU_DIAG_NOT_YET      Diagnosis of data collected no yet.
 **********************************************************************************************************************/
ssp_err_t R_CTSU_DataGet (ctsu_ctrl_t * const p_ctrl, uint16_t * p_data)
{
    ssp_err_t              err             = SSP_SUCCESS;
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;
    uint16_t               element_id;

#if (CTSU_CFG_PARAM_CHECKING_ENABLE == 1)
    SSP_ASSERT(p_instance_ctrl);
    SSP_ASSERT(p_data);
    CTSU_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, SSP_ERR_NOT_OPEN);
#endif
    CTSU_ERROR_RETURN(CTSU_STATE_SCANNING != p_instance_ctrl->state, SSP_ERR_CTSU_SCANNING);

#if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        err = ctsu_diag_data_get1(p_instance_ctrl);
        p_instance_ctrl->state = CTSU_STATE_IDLE;
        if (SSP_ERR_CTSU_DIAG_NOT_YET == err)
        {
            err = SSP_ERR_CTSU_DIAG_NOT_YET;
        }
        else
        {
            err = SSP_SUCCESS;
        }

        return err;
    }
#endif

    ctsu_state_scanned_process(p_instance_ctrl);

    CTSU_ERROR_RETURN((uint16_t) 0U < p_instance_ctrl->average, SSP_ERR_CTSU_INCOMPLETE_TUNING);

#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
    {
        for (element_id = (uint16_t) 0U; element_id < p_instance_ctrl->num_elements; element_id++)
        {
            *p_data = *(p_instance_ctrl->p_self_data + element_id);
            p_data++;
        }
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
    {
        for (element_id = (uint16_t) 0U; element_id < p_instance_ctrl->num_elements; element_id++)
        {
            *p_data = *(p_instance_ctrl->p_mutual_pri_data + element_id);
            p_data++;
            *p_data = *(p_instance_ctrl->p_mutual_snd_data + element_id);
            p_data++;
        }
    }
#endif

    return err;
}

/*******************************************************************************************************************//**
 * Updates the user callback and has option of providing memory for callback structure.
 * Implements ctsu_api_t::callbackSet
 *
 * @retval  SSP_SUCCESS                  Callback updated successfully.
 * @retval  SSP_ERR_ASSERTION            A required pointer is NULL.
 * @retval  SSP_ERR_NOT_OPEN             The control block has not been opened.
 **********************************************************************************************************************/
ssp_err_t R_CTSU_CallbackSet (ctsu_ctrl_t * const          p_api_ctrl,
                              void (                     * p_callback)(ctsu_callback_args_t *),
                              void const * const           p_context,
                              ctsu_callback_args_t * const p_callback_memory)
{
    ctsu_instance_ctrl_t * p_ctrl = (ctsu_instance_ctrl_t *) p_api_ctrl;

#if (CTSU_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_callback);
    CTSU_ERROR_RETURN(CTSU_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /* Store callback and context */
    p_ctrl->p_callback        = p_callback;
    p_ctrl->p_context         = p_context;
    p_ctrl->p_callback_memory = p_callback_memory;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Disables specified CTSU control block. Implements @ref ctsu_api_t::close.
 *
 * @retval SSP_SUCCESS              CTSU successfully configured.
 * @retval SSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval SSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
ssp_err_t R_CTSU_Close (ctsu_ctrl_t * const p_ctrl)
{
    ssp_err_t              err             = SSP_SUCCESS;
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;

    SSP_ASSERT(p_instance_ctrl);
    CTSU_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, SSP_ERR_NOT_OPEN);

    g_ctsu_element_index = (uint16_t) ((uint8_t) (g_ctsu_element_index - p_instance_ctrl->num_elements));
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
    {
        g_ctsu_self_element_index = (uint16_t) ((uint8_t) (g_ctsu_self_element_index - p_instance_ctrl->num_elements));
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
    {
        g_ctsu_mutual_element_index = (uint16_t) ((uint8_t) (g_ctsu_mutual_element_index - p_instance_ctrl->num_elements));
    }
#endif
    if ((uint16_t) 0U == g_ctsu_element_index)
    {
        NVIC_DisableIRQ(p_instance_ctrl->write_irq);
        NVIC_DisableIRQ(p_instance_ctrl->read_irq);
        NVIC_DisableIRQ(p_instance_ctrl->end_irq);

#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
        err = ctsu_transfer_close(p_instance_ctrl);
#endif
    }

    /** Stops peripheral and clears any internal state to allow driver to be reconfigured. */
    if ((uint8_t) 1U >= g_ctsu_instance_cnt)
    {
        HW_CTSU_PowerOff(p_instance_ctrl->p_reg);

        ctsu_irq_cfg_clear(p_instance_ctrl->write_irq);
        ctsu_irq_cfg_clear(p_instance_ctrl->read_irq);
        ctsu_irq_cfg_clear(p_instance_ctrl->end_irq);

        /* Power down and unlock the CTSU module */
        ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
        ssp_feature.channel = 0U;
        ssp_feature.unit = 0U;
        ssp_feature.id = SSP_IP_CTSU;
        R_BSP_ModuleStop(&ssp_feature);
        R_BSP_HardwareUnlock(&ssp_feature);

        g_ctsu_instance_cnt = (uint8_t) 0U;
    }
    else
    {
        g_ctsu_instance_cnt--;
    }

    p_instance_ctrl->state = CTSU_STATE_INIT;
    p_instance_ctrl->open  = false;

    return err;
}

/*******************************************************************************************************************//**
 * @brief Return CTSU HAL driver version. Implements @ref ctsu_api_t::versionGet.
 *
 * @retval SSP_SUCCESS             Version information successfully read.
 * @retval SSP_ERR_ASSERTION       Null pointer passed as a parameter
 **********************************************************************************************************************/
ssp_err_t R_CTSU_VersionGet (ssp_version_t * const p_version)
{
#if CTSU_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_version);
#endif

    p_version->version_id = g_ctsu_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Diagnosis the CTSU peripheral.
 * Implements @ref ctsu_api_t::diagnosis.
 *
 * @retval SSP_SUCCESS                             CTSU successfully configured.
 * @retval SSP_ERR_ASSERTION                       Null pointer passed as a parameter.
 * @retval SSP_ERR_NOT_OPEN                        Module is not open.
 * @retval SSP_ERR_NOT_ENABLED                     Diagnosis is not enabled in S7 Series
 * @retval SSP_ERR_CTSU_NOT_GET_DATA               The previous data has not been retrieved by DataGet.
 * @retval SSP_ERR_CTSU_DIAG_LDO_OVER_VOLTAGE      Diagnosis of LDO over voltage failed.
 * @retval SSP_ERR_CTSU_DIAG_CCO_HIGH              Diagnosis of CCO into 19.2uA failed.
 * @retval SSP_ERR_CTSU_DIAG_CCO_LOW               Diagnosis of CCO into 2.4uA failed.
 * @retval SSP_ERR_CTSU_DIAG_SSCG                  Diagnosis of SSCG frequency failed.
 * @retval SSP_ERR_CTSU_DIAG_DAC                   Diagnosis of non-touch count value failed.
 **********************************************************************************************************************/
ssp_err_t R_CTSU_Diagnosis (ctsu_ctrl_t * const p_ctrl)
{
#if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    ssp_err_t diag_err;
#endif
    (void) p_ctrl;

#if (BSP_CFG_MCU_PART_SERIES == 7)
    return SSP_ERR_NOT_ENABLED;
#endif

#if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_DIAG_COMPLETE == g_ctsu_diag_info.state)
    {
        diag_err = ctsu_diag_ldo_over_voltage_result();
        if (SSP_SUCCESS != diag_err)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_INIT;

            return SSP_ERR_CTSU_DIAG_LDO_OVER_VOLTAGE;
        }

        diag_err = ctsu_diag_oscillator_high_result();
        if (SSP_SUCCESS != diag_err)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_INIT;

            return SSP_ERR_CTSU_DIAG_CCO_HIGH;
        }

        diag_err = ctsu_diag_oscillator_low_result();
        if (SSP_SUCCESS != diag_err)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_INIT;

            return SSP_ERR_CTSU_DIAG_CCO_LOW;
        }

        diag_err = ctsu_diag_sscg_result();
        if (SSP_SUCCESS != diag_err)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_INIT;

            return SSP_ERR_CTSU_DIAG_SSCG;
        }

        diag_err = ctsu_diag_dac_result();
        if (SSP_SUCCESS != diag_err)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_INIT;

            return SSP_ERR_CTSU_DIAG_DAC;
        }

        g_ctsu_diag_info.state = CTSU_DIAG_INIT;
    }
#endif

    /* if all checks passed to this point, return success */
    return SSP_SUCCESS;
}


/*******************************************************************************************************************//**
 * @} (end addtogroup CTSU_V2)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Initialize the hardware instance using the SSP feature.
 *
 * @param[in,out] p_instance_ctrl   Pointer to CTSU control structure
 *
 * @retval SSP_SUCCESS              CTSU successfully configured.
 * @retval SSP_ERR_IN_USE           Control block has already been opened or channel is being used by another
 *                                  instance. Call close() then open() to reconfigure.
 * @retval SSP_ERR_IRQ_BSP_DISABLED A required interrupt does not exist in the vector table
 *
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * fmi_api_t::productFeatureGet
 *                                   * fmi_api_t::eventInfoGet
 **********************************************************************************************************************/
static ssp_err_t ctsu_hw_initialize (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    ssp_err_t        err         = SSP_SUCCESS;
    ssp_feature_t    ssp_feature = {{(ssp_ip_t) 0U}};
    fmi_event_info_t event_info  = {(IRQn_Type) 0U};

    CTSU_ERROR_RETURN(CTSU_INSTANCE_MAX > g_ctsu_instance_cnt, SSP_ERR_IN_USE);

    R_BSP_FeatureCtsuGet(&g_ctsu_bsp_feature);

    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_CTSU;
    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    CTSU_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_instance_ctrl->p_reg = (R_CTSU_Type *) info.ptr;

    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_CTSU_WRITE, &event_info);
    CTSU_ERROR_RETURN(SSP_INVALID_VECTOR != event_info.irq, SSP_ERR_IRQ_BSP_DISABLED);
    p_instance_ctrl->write_irq = event_info.irq;
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_CTSU_READ, &event_info);
    CTSU_ERROR_RETURN(SSP_INVALID_VECTOR != event_info.irq, SSP_ERR_IRQ_BSP_DISABLED);
    p_instance_ctrl->read_irq  = event_info.irq;
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_CTSU_END, &event_info);
    CTSU_ERROR_RETURN(SSP_INVALID_VECTOR != event_info.irq, SSP_ERR_IRQ_BSP_DISABLED);
    p_instance_ctrl->end_irq   = event_info.irq;

    if ((uint8_t) 0U == g_ctsu_instance_cnt)
    {
        err = R_BSP_HardwareLock(&ssp_feature);
        CTSU_ERROR_RETURN(SSP_SUCCESS == err, err);
    }
    g_ctsu_instance_cnt++;

    return err;
}

/*******************************************************************************************************************//**
 * Initialize the CTSU software instance.
 *
 * @param[in,out] p_instance_ctrl   Pointer to CTSU control structure
 *
 * @retval SSP_SUCCESS           CTSU successfully configured.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * cgc_api_t::systemClockFreqGet
 *                                   * transfer_api_t::open
 **********************************************************************************************************************/
static ssp_err_t ctsu_sw_initialize (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    ssp_err_t                  err         = SSP_SUCCESS;
    ssp_feature_t              ssp_feature = {{(ssp_ip_t) 0U}};
    uint16_t                   element_id;
    const ctsu_element_cfg_t * element_cfgs;

    /* Initialize driver control structure (address setting) */
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
    {
        p_instance_ctrl->p_self_raw   = &g_ctsu_self_raw[g_ctsu_self_element_index];
        p_instance_ctrl->p_self_data  = &g_ctsu_self_data[g_ctsu_self_element_index];
        p_instance_ctrl->num_elements = (uint16_t) p_instance_ctrl->p_ctsu_cfg->num_rx;
        g_ctsu_self_element_index     =
            (uint16_t) ((uint8_t) (g_ctsu_self_element_index + p_instance_ctrl->num_elements));
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
    {
        p_instance_ctrl->p_mutual_raw      = &g_ctsu_mutual_raw[g_ctsu_mutual_element_index];
        p_instance_ctrl->p_mutual_pri_data =
            &g_ctsu_mutual_pri_data[g_ctsu_mutual_element_index];
        p_instance_ctrl->p_mutual_snd_data =
            &g_ctsu_mutual_snd_data[g_ctsu_mutual_element_index];
        p_instance_ctrl->num_elements =
            (uint16_t) ((uint8_t) (p_instance_ctrl->p_ctsu_cfg->num_rx * p_instance_ctrl->p_ctsu_cfg->num_tx));
        g_ctsu_mutual_element_index   =
            (uint16_t) ((uint8_t) (g_ctsu_mutual_element_index + p_instance_ctrl->num_elements));
    }
#endif
    p_instance_ctrl->p_tuning_count = &g_ctsu_tuning_count[g_ctsu_element_index];
    p_instance_ctrl->p_tuning_diff  = &g_ctsu_tuning_diff[g_ctsu_element_index];
    p_instance_ctrl->p_ctsuwr       = &g_ctsu_ctsuwr[g_ctsu_element_index];
    g_ctsu_element_index            = (uint16_t) ((uint8_t) (g_ctsu_element_index + p_instance_ctrl->num_elements));

    /* Set Value */
    p_instance_ctrl->num_moving_average = p_instance_ctrl->p_ctsu_cfg->num_moving_average;
    p_instance_ctrl->average            = (uint16_t) 0U;
    if (true == p_instance_ctrl->p_ctsu_cfg->tunning_enable)
    {
        p_instance_ctrl->tuning = CTSU_TUNING_INCOMPLETE;
    }
    else
    {
        p_instance_ctrl->tuning = CTSU_TUNING_COMPLETE;
    }

    p_instance_ctrl->ctsucr1 = (uint8_t) (p_instance_ctrl->p_ctsu_cfg->atune1 << 3);
    p_instance_ctrl->ctsucr1 |= (uint8_t) (p_instance_ctrl->p_ctsu_cfg->md << 6);

    p_instance_ctrl->ctsuchac0 = p_instance_ctrl->p_ctsu_cfg->ctsuchac0;
    p_instance_ctrl->ctsuchac1 = p_instance_ctrl->p_ctsu_cfg->ctsuchac1;
    p_instance_ctrl->ctsuchac2 = p_instance_ctrl->p_ctsu_cfg->ctsuchac2;
    p_instance_ctrl->ctsuchac3 = p_instance_ctrl->p_ctsu_cfg->ctsuchac3;
    p_instance_ctrl->ctsuchac4 = p_instance_ctrl->p_ctsu_cfg->ctsuchac4;

    p_instance_ctrl->ctsuchtrc0 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc0;
    p_instance_ctrl->ctsuchtrc1 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc1;
    p_instance_ctrl->ctsuchtrc2 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc2;
    p_instance_ctrl->ctsuchtrc3 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc3;
    p_instance_ctrl->ctsuchtrc4 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc4;

    p_instance_ctrl->md = p_instance_ctrl->p_ctsu_cfg->md;

    for (element_id = (uint16_t) 0U; element_id < p_instance_ctrl->num_elements; element_id++)
    {
        p_instance_ctrl->p_tuning_count[element_id] = (uint8_t) 0U;
        p_instance_ctrl->p_tuning_diff[element_id]  = 0;
        element_cfgs = (p_instance_ctrl->p_ctsu_cfg->p_elements + element_id);
        p_instance_ctrl->p_ctsuwr[element_id].ctsussc = (uint16_t) (element_cfgs->ssdiv << 8);
        p_instance_ctrl->p_ctsuwr[element_id].ctsuso0 = (uint16_t) ((element_cfgs->snum << 10) | element_cfgs->so);
        p_instance_ctrl->p_ctsuwr[element_id].ctsuso1 =
            (uint16_t) ((CTSU_ICOG_RECOMMEND << 13) | ((uint16_t) element_cfgs->sdpa << 8) | CTSU_RICOA_RECOMMEND);
    }

    /* Enable interrupts for CTSUWR, CTSURD, CTSUFN */
    ctsu_irq_cfg_enable(p_instance_ctrl->write_irq, (uint32_t) CTSU_CFG_INT_PRIORITY_LEVEL, p_instance_ctrl);
    ctsu_irq_cfg_enable(p_instance_ctrl->read_irq, (uint32_t) CTSU_CFG_INT_PRIORITY_LEVEL, p_instance_ctrl);
    ctsu_irq_cfg_enable(p_instance_ctrl->end_irq, (uint32_t) CTSU_CFG_INT_PRIORITY_LEVEL, p_instance_ctrl);

    if (p_instance_ctrl->num_elements == g_ctsu_element_index)
    {
        /* TSCAP discharge process */
        g_ctsu_tscap_pin_cfg_data.pin_cfg =
            (uint32_t) (IOPORT_CFG_PORT_DIRECTION_OUTPUT + IOPORT_CFG_PORT_OUTPUT_LOW);
        g_ioport_on_ioport.pinsCfg(&g_ctsu_tscap_pin_cfg);
        R_BSP_SoftwareDelay((uint32_t) 10U, BSP_DELAY_UNITS_MICROSECONDS);
        g_ctsu_tscap_pin_cfg_data.pin_cfg =
            (uint32_t) ((uint32_t) IOPORT_CFG_PERIPHERAL_PIN + (uint32_t) IOPORT_PERIPHERAL_CTSU);
        g_ioport_on_ioport.pinsCfg(&g_ctsu_tscap_pin_cfg);

        /* Get CTSU out of stop state (supply power/clock) */
        ssp_feature.channel = 0U;
        ssp_feature.unit = 0U;
        ssp_feature.id = SSP_IP_CTSU;
        R_BSP_ModuleStart(&ssp_feature);

#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
        err = ctsu_transfer_open(p_instance_ctrl);
#endif

        /* Set power on */
        HW_CTSU_PowerOn(p_instance_ctrl->p_reg);

        if (CTSU_CORRECTION_INIT == g_ctsu_correction_info.status)
        {
            if (SSP_SUCCESS == err)
            {
                err = ctsu_correction_process(p_instance_ctrl);
            }
        }

        /* Since CLK is rewritten by correction, set here. */
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, (hw_ctsu_clock_t) CTSU_CFG_PCLK_DIVISION);
    }

#if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        p_instance_ctrl->p_diag_info = &g_ctsu_diag_info;
        g_ctsu_diag_info.state       = CTSU_DIAG_INIT;
        g_ctsu_diag_info.dac_init    = 0U;
    }
#endif

    p_instance_ctrl->p_correction_info = &g_ctsu_correction_info;
    p_instance_ctrl->rd_index          = (uint16_t) 0U;
    p_instance_ctrl->wr_index          = (uint16_t) 0U;
    p_instance_ctrl->state             = CTSU_STATE_IDLE;

    p_instance_ctrl->p_callback        = p_instance_ctrl->p_ctsu_cfg->p_callback;
    p_instance_ctrl->p_context         = p_instance_ctrl->p_ctsu_cfg->p_context;
    p_instance_ctrl->p_callback_memory = NULL;

#if defined(CTSU_CFG_TARGET_VALUE_QE_SUPPORT)
    p_instance_ctrl->tuning_self_target_value   = p_instance_ctrl->p_ctsu_cfg->tuning_self_target_value;
    p_instance_ctrl->tuning_mutual_target_value = p_instance_ctrl->p_ctsu_cfg->tuning_mutual_target_value;
#else
    p_instance_ctrl->tuning_self_target_value   = CTSU_TUNING_VALUE_SELF;
    p_instance_ctrl->tuning_mutual_target_value = CTSU_TUNING_VALUE_MUTUAL;
#endif

    return err;
}

#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)

/*******************************************************************************************************************//**
 * Opens and enables the transfer driver module.
 *
 * @param[in]  p_instance_ctrl   Pointer to CTSU control structure
 *
 * @retval     SSP_SUCCESS       Successfully configured.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * transfer_api_t::open
 *                                   * transfer_api_t::enable
 **********************************************************************************************************************/
static ssp_err_t ctsu_transfer_open (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    ssp_err_t err;
    transfer_instance_t const * p_transfer;
    transfer_info_t           * p_info;
    transfer_cfg_t              cfg;

    /* CTSUWR setting */
    p_transfer = p_instance_ctrl->p_ctsu_cfg->p_transfer_tx;
    p_info     = p_transfer->p_cfg->p_info;
    cfg        = *(p_transfer->p_cfg);

    p_info->chain_mode    = TRANSFER_CHAIN_MODE_DISABLED;
    p_info->src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
    p_info->irq           = TRANSFER_IRQ_END;
    p_info->dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
    p_info->size           = TRANSFER_SIZE_2_BYTE;
    p_info->p_dest         = (void *) &R_CTSU->CTSUSSC;
    p_info->mode           = TRANSFER_MODE_BLOCK;
    p_info->repeat_area    = TRANSFER_REPEAT_AREA_DESTINATION;
    p_info->length         = (uint16_t) 3U;
    p_info->p_src = p_instance_ctrl->p_ctsuwr;

    err = p_transfer->p_api->open(p_transfer->p_ctrl, &cfg);
    CTSU_ERROR_RETURN(SSP_SUCCESS == err, err);
    err = p_transfer->p_api->enable(p_transfer->p_ctrl);
    CTSU_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* CTSURD setting */
    p_transfer = p_instance_ctrl->p_ctsu_cfg->p_transfer_rx;
    p_info     = p_transfer->p_cfg->p_info;
    cfg        = *(p_transfer->p_cfg);

    p_info->chain_mode     = TRANSFER_CHAIN_MODE_DISABLED;
    p_info->dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
    p_info->irq            = TRANSFER_IRQ_END;
    p_info->src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
    p_info->size          = TRANSFER_SIZE_2_BYTE;
    p_info->mode          = TRANSFER_MODE_BLOCK;
    p_info->repeat_area   = TRANSFER_REPEAT_AREA_SOURCE;
    p_info->length        = (uint16_t) 2U;
    p_info->p_src  = (void *) &R_CTSU->CTSUSC;
    p_info->p_dest = p_instance_ctrl->p_self_raw;
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
    {
        p_info->p_dest = p_instance_ctrl->p_mutual_raw;
    }
 #endif

    err = p_transfer->p_api->open(p_transfer->p_ctrl, &cfg);
    CTSU_ERROR_RETURN(SSP_SUCCESS == err, err);
    err = p_transfer->p_api->enable(p_transfer->p_ctrl);
    CTSU_ERROR_RETURN(SSP_SUCCESS == err, err);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Closes the transfer driver module.
 *
 * @param[in] p_instance_ctrl    Pointer to CTSU control structure
 *
 * @retval    SSP_SUCCESS        Successfully configured.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * transfer_api_t::close
 **********************************************************************************************************************/
static ssp_err_t ctsu_transfer_close (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    ssp_err_t err;
    transfer_instance_t const * p_transfer;

    /* CTSUWR setting */
    p_transfer = p_instance_ctrl->p_ctsu_cfg->p_transfer_tx;
    err        = p_transfer->p_api->close(p_transfer->p_ctrl);
    CTSU_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* CTSURD setting */
    p_transfer = p_instance_ctrl->p_ctsu_cfg->p_transfer_rx;
    err        = p_transfer->p_api->close(p_transfer->p_ctrl);
    CTSU_ERROR_RETURN(SSP_SUCCESS == err, err);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Reconfigures the transfer driver module.
 *
 * @param[in] p_instance_ctrl    Pointer to CTSU control structure
 *
 * @retval    SSP_SUCCESS        Successfully configured.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * transfer_api_t::blockReset
 **********************************************************************************************************************/
static ssp_err_t ctsu_transfer_configure (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    ssp_err_t err;
    transfer_instance_t const * p_transfer;
    void                const * p_src;
    void                      * p_dest;
    uint16_t                    num_blocks;
    uint16_t                    length;
    transfer_size_t             size;

    /* CTSUWR setting */
    p_transfer = p_instance_ctrl->p_ctsu_cfg->p_transfer_tx;
    p_dest     = p_transfer->p_cfg->p_info->p_dest;
    size       = p_transfer->p_cfg->p_info->size;
    length     = (uint16_t) 3U;
    if (CTSU_CORRECTION_RUN == g_ctsu_correction_info.status)
    {
        num_blocks = (uint16_t) 1U;
        p_src      = (void *) &(g_ctsu_correction_info.ctsuwr);
    }
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    else if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        num_blocks = (uint16_t) 1U;
        p_src      = (void *) &(g_ctsu_diag_info.ctsuwr);
    }
 #endif
    else
    {
        num_blocks = p_instance_ctrl->num_elements;
        p_src      = p_instance_ctrl->p_ctsuwr;
    }
    err = p_transfer->p_api->blockReset(p_transfer->p_ctrl, p_src, p_dest, length, size, num_blocks);
    CTSU_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* CTSURD setting */
    p_transfer = p_instance_ctrl->p_ctsu_cfg->p_transfer_rx;
    p_src      = p_transfer->p_cfg->p_info->p_src;
    size       = p_transfer->p_cfg->p_info->size;
    length     = (uint16_t) 2U;
    if (CTSU_CORRECTION_RUN == g_ctsu_correction_info.status)
    {
        num_blocks = (uint16_t) 1U;
        p_dest     = (void *) &g_ctsu_correction_info.scanbuf;
    }

 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    else if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        num_blocks = (uint16_t) 1U;
        p_dest     = (void *) &g_ctsu_diag_info.scanbuf;
    }
 #endif
    else
    {
        num_blocks = p_instance_ctrl->num_elements;
        p_dest     = p_instance_ctrl->p_self_raw;
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
        {
            p_dest     = p_instance_ctrl->p_mutual_raw;
            num_blocks = (uint16_t) (num_blocks * (uint16_t) 2U); ///< Primary and Secondary
        }
 #endif
    }
    err = p_transfer->p_api->blockReset(p_transfer->p_ctrl, p_src, p_dest, length, size, num_blocks);
    CTSU_ERROR_RETURN(SSP_SUCCESS == err, err);

    return SSP_SUCCESS;
}

#endif

/*******************************************************************************************************************//**
 * Corrects and tunes the sensor data when the CTSU run state is scan end.
 *
 * @param[in,out] p_instance_ctrl   Pointer to CTSU control structure
 **********************************************************************************************************************/
static void ctsu_state_scanned_process (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    if (CTSU_STATE_SCANNED != p_instance_ctrl->state)
    {
        return;
    }

    if (CTSU_TUNING_COMPLETE == p_instance_ctrl->tuning)
    {
        if (p_instance_ctrl->average == p_instance_ctrl->num_moving_average)
        {
            /* Do nothing */
        }
        else if (p_instance_ctrl->average < p_instance_ctrl->num_moving_average)
        {
            (p_instance_ctrl->average)++;
        }
        else
        {
            p_instance_ctrl->average = p_instance_ctrl->num_moving_average;
        }
    }

    ctsu_correction_exec(p_instance_ctrl);

    if (CTSU_TUNING_INCOMPLETE == p_instance_ctrl->tuning)
    {
        if ((CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md) ||
            (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md))
        {
            ctsu_initial_offset_tuning(p_instance_ctrl);
        }
    }

    p_instance_ctrl->state = CTSU_STATE_IDLE;
}

/*******************************************************************************************************************//**
 * Tunes the amount of the sensor ICO input current offset.
 *
 * @param[in]     p_instance_ctrl   Pointer to CTSU control structure
 **********************************************************************************************************************/
static void ctsu_initial_offset_tuning (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t element_id;
    int32_t  diff          = 0;
    uint32_t complete_flag = 0U;
    uint32_t num_complete  = 0U;
    uint16_t target_val;
    ctsu_correction_calc_t calc;

    /* element_id through each element for control block */
    for (element_id = (uint16_t) 0U; element_id < p_instance_ctrl->num_elements; element_id++)
    {
        if (CTSU_TUNING_OT_COUNT != *(p_instance_ctrl->p_tuning_count + element_id))
        {
            if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
            {
                target_val = (p_instance_ctrl->tuning_self_target_value);
            }
            else
            {
                target_val = (p_instance_ctrl->tuning_mutual_target_value);
            }

            calc.snum  = (p_instance_ctrl->p_ctsuwr[element_id].ctsuso0 >> 10) & CTSU_SNUM_MAX;
            calc.sdpa  = (p_instance_ctrl->p_ctsuwr[element_id].ctsuso1 >> 8) & CTSU_SDPA_MAX;
            target_val = (uint16_t) ((target_val *
                                     (uint32_t) ((calc.snum + 1) * (calc.sdpa + 1))) /
                                     g_ctsu_correction_info.ctsu_clock);
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
            if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
            {
                diff = (int32_t) ((uint16_t) *(p_instance_ctrl->p_self_data + element_id) - target_val);
            }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
            if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
            {
                diff = (int32_t) ((uint16_t) *(p_instance_ctrl->p_mutual_pri_data + element_id) - target_val);
            }
#endif
            ctsu_initial_offset_calc((uint16_t *) &p_instance_ctrl->p_ctsuwr[element_id].ctsuso0,
                                     (int32_t *) (p_instance_ctrl->p_tuning_diff + element_id),
                                     diff,
                                     &complete_flag);
        }
        else
        {
            complete_flag = 1U;
        }

        if (1U == complete_flag)
        {
            complete_flag = 0U;
            num_complete++;
            *(p_instance_ctrl->p_tuning_count + element_id) = CTSU_TUNING_OT_COUNT;
        }
    }

    if (num_complete == p_instance_ctrl->num_elements)
    {
        p_instance_ctrl->tuning = CTSU_TUNING_COMPLETE;
    }
}

/*******************************************************************************************************************//**
 * Calculates the amount of the sensor ICO input current offset.
 *
 * @param[in,out] p_ctsuso0         Pointer to CTSUSO0 register value
 * @param[in,out] p_tuning_diff     Pointer to difference from base value of each element
 * @param[in]     diff_value        Difference from tuning value
 * @param[out]    p_complete_flag   Pointer to tuning complete flag
 **********************************************************************************************************************/
static void ctsu_initial_offset_calc (uint16_t * const p_ctsuso0,
                                      int32_t  * const p_tuning_diff,
                                      int32_t          diff_value,
                                      uint32_t * const p_complete_flag)
{
    uint16_t ctsuso;

    ctsuso = (uint16_t) ((*p_ctsuso0) & CTSU_TUNING_MAX);

    if (0 < diff_value)
    {
        if ((*p_tuning_diff) < 0)
        {
            if ((-diff_value) > (*p_tuning_diff))
            {
                ctsuso++;      ///< Decrease count
            }
            *p_complete_flag = 1U;
        }
        else
        {
            if (CTSU_TUNING_MAX == ctsuso) /* CTSUSO limit check    */
            {
                *p_complete_flag = 1U;
            }
            else
            {
                ctsuso++;                    ///< Decrease count
                *p_tuning_diff = diff_value; ///< Plus
            }
        }
    }
    else if (0 == diff_value)
    {
        *p_complete_flag = 1U;
    }
    else
    {
        if ((*p_tuning_diff) > 0)
        {
            if ((-diff_value) > (*p_tuning_diff))
            {
                ctsuso--;      ///< Increase count
            }
            *p_complete_flag = 1U;
        }
        else
        {
            if (CTSU_TUNING_MIN == ctsuso) /* CTSUSO limit check    */
            {
                *p_complete_flag = 1U;
            }
            else
            {
                ctsuso--;                    ///< Increase count
                *p_tuning_diff = diff_value; ///< Minus
            }
        }
    }

    (*p_ctsuso0) &= (uint16_t) (~CTSU_TUNING_MAX);
    (*p_ctsuso0) |= ctsuso;
}

/*******************************************************************************************************************//**
 * Calculates the moving average.

 * @param[in,out] p_average      Pointer to moving average data
 * @param[in]     new_data       New data to moving average
 * @param[in]     average_num    Number of moving averages
 **********************************************************************************************************************/
static void ctsu_moving_average (uint16_t * p_average, uint16_t new_data, uint16_t average_num)
{
    uint32_t work;

    work       = (uint32_t) (*p_average * ((average_num) - 1)); /* Average * (num - 1) */
    work      += new_data;                                      /* Add Now data        */
    *p_average = (uint16_t) (work / average_num);               /* Average calculation */
}

/*******************************************************************************************************************//**
 * Measures the sensor correction value, calculates the correction coefficient.
 *
 * @param[in]     p_instance_ctrl   Pointer to CTSU control structure
 *
 * @retval SSP_SUCCESS           Successfully configured.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * cgc_api_t::systemClockFreqGet
 **********************************************************************************************************************/
static ssp_err_t ctsu_correction_process (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t  second_std_val;
    uint32_t  ctsu_sdpa;
    uint32_t  pclkb_mhz = CTSU_PCLKB_FREQ_RANGE1;
    ssp_err_t err       = SSP_SUCCESS;

    g_ctsu_correction_info.status = CTSU_CORRECTION_RUN;

    err = g_cgc_on_cgc.systemClockFreqGet(CGC_SYSTEM_CLOCKS_PCLKB, &pclkb_mhz);
    pclkb_mhz /= CTSU_PCLKB_FREQ_MHZ;
    if (CTSU_PCLKB_FREQ_RANGE1 >= pclkb_mhz)
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_1);
        ctsu_sdpa        = pclkb_mhz - 1U;
    }
    else if ((CTSU_PCLKB_FREQ_RANGE1 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE2 >= pclkb_mhz))
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_2);
        ctsu_sdpa        = (pclkb_mhz / 2U) - 1U;
    }
    else
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_4);
        ctsu_sdpa        = (pclkb_mhz / 4U) - 1U;
    }

    g_ctsu_correction_info.ctsu_clock = pclkb_mhz >> CTSU_CFG_PCLK_DIVISION;

    g_ctsu_correction_info.ctsuwr.ctsussc = (CTSU_SSDIV_0500 << 8);
    g_ctsu_correction_info.ctsuwr.ctsuso0 = (uint16_t) 0x0000U;

    /* Set CTSUSO1 */
    g_ctsu_correction_info.ctsuwr.ctsuso1 =
        (uint16_t) ((CTSU_ICOG_66 << 13) | ((uint16_t) ctsu_sdpa << 8) | CTSU_RICOA_RECOMMEND);

    /* Correction measurement setting */
    HW_CTSU_CalibrationModeStart(p_instance_ctrl->p_reg);
    R_BSP_SoftwareDelay(CTSU_WAIT_TIME, BSP_DELAY_UNITS_MICROSECONDS);

    /* First value measurement */
    ctsu_correction_measurement(p_instance_ctrl, &g_ctsu_correction_info.first_val);

    /* Second standard value create */
    second_std_val = (uint16_t) CTSU_CORRECTION_2ND_STD_VAL;
    g_ctsu_correction_info.ctsuwr.ctsuso1 |= (uint16_t) (CTSU_ICOG_40 << 13); /* ICO gain  66% -> 40% */

    /* Second value measurement */
    ctsu_correction_measurement(p_instance_ctrl, &g_ctsu_correction_info.second_val);

    /* Normal measurement setting */
    HW_CTSU_CalibrationModeEnd(p_instance_ctrl->p_reg);

    R_BSP_SoftwareDelay(CTSU_WAIT_TIME, BSP_DELAY_UNITS_MICROSECONDS);

    if (((uint16_t) 0U != g_ctsu_correction_info.first_val) && ((uint16_t) 0U != g_ctsu_correction_info.second_val))
    {
        if (g_ctsu_correction_info.second_val < g_ctsu_correction_info.first_val)
        {
            /* 1st coefficient create */
            g_ctsu_correction_info.first_coefficient = (CTSU_CORRECTION_1ST_STD_VAL << CTSU_SHIFT_AMOUNT) /
                                                       g_ctsu_correction_info.first_val;

            /* 2nd coefficient create */
            g_ctsu_correction_info.second_coefficient = ((uint32_t) second_std_val << CTSU_SHIFT_AMOUNT) /
                                                        g_ctsu_correction_info.second_val;

            g_ctsu_correction_info.status = CTSU_CORRECTION_COMPLETE;
        }
        else
        {
            g_ctsu_correction_info.status = CTSU_CORRECTION_ERROR;
        }
    }
    else
    {
        g_ctsu_correction_info.status = CTSU_CORRECTION_ERROR;
    }

    return err;
}

/*******************************************************************************************************************//**
 * Measures the sensor correction value.
 *
 * @param[in]     p_instance_ctrl   Pointer to CTSU control structure
 * @param[out]    data              Pointer to correction value
 **********************************************************************************************************************/
static void ctsu_correction_measurement (ctsu_instance_ctrl_t * const p_instance_ctrl, uint16_t * data)
{
    uint16_t i;
    uint32_t sum = 0U;

    for (i = (uint16_t) 0U; i < CTSU_CORRECTION_AVERAGE; i++)
    {
#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
        ctsu_transfer_configure(p_instance_ctrl);
#else
        SSP_PARAMETER_NOT_USED(p_instance_ctrl);
#endif
        HW_CTSU_MeasurementStart(p_instance_ctrl->p_reg);
        while (HW_CTSU_IsDuringMeasurement(p_instance_ctrl->p_reg))
        {
        }

        sum += g_ctsu_correction_info.scanbuf.sen;
    }

    *data = (uint16_t) (sum / CTSU_CORRECTION_AVERAGE);
}

/*******************************************************************************************************************//**
 * Corrects the sensor data.
 *
 * @param[out] correction_data      Pointer to corrected data
 * @param[in]  raw_data             Uncorrected data
 * @param[in]  p_calc               Pointer to calculation data
 **********************************************************************************************************************/
static void ctsu_correction_calc (uint16_t * correction_data, uint16_t raw_data, ctsu_correction_calc_t * p_calc)
{
    uint32_t answer;
    uint16_t coefficient;
    int32_t  cmp_data;
    uint8_t  calc_flag = (uint8_t) 0U;
    uint16_t diff_val;
    int32_t  diff_coefficient;
    int32_t  mul_diffcoff_diff1valsval;
    uint32_t mul_coff1val_diffcorr;

    if (CTSU_CORRECTION_COMPLETE == g_ctsu_correction_info.status)
    {
        calc_flag = (uint8_t) 1U;
    }

    if (calc_flag)
    {
        /* Since the correction coefficient table is created with the recommended measurement time, */
        /* If the measurement time is different, adjust the value level. */
        cmp_data = (int32_t) (((uint32_t) raw_data * g_ctsu_correction_info.ctsu_clock) /
                              (((uint32_t) p_calc->snum + 1U) * ((uint32_t) p_calc->sdpa + 1U)));

        /*               g_mul_coff1val_diffcorr - g_diff_cofficient * (g_ctsu_correction_info.first_val - raw_data) */
        /*  coefficient= ------------------------------------------------------------------------------------------  */
        /*                                      g_diff_correct_val                                                   */
        /*                                                                                                           */

        diff_val = (uint16_t) (g_ctsu_correction_info.first_val - g_ctsu_correction_info.second_val);

        /* Get multiplication of g_ctsu_correction_info.first_coefficient and difference of Correction value */
        mul_coff1val_diffcorr = g_ctsu_correction_info.first_coefficient * diff_val;

        /* Get difference of Correction coefficient */
        diff_coefficient =
            (int32_t) (g_ctsu_correction_info.first_coefficient - g_ctsu_correction_info.second_coefficient);

        /* Get multiplication of  g_diff_cofficient  and (g_ctsu_correction_info.first_val - raw_data_coff) */
        mul_diffcoff_diff1valsval = (diff_coefficient * ((int32_t) g_ctsu_correction_info.first_val - cmp_data));

        /* Get correction coefficient of scan data */
        coefficient = (uint16_t) (((int32_t) mul_coff1val_diffcorr - mul_diffcoff_diff1valsval) / (int32_t) diff_val);

        /* Get output count data */
        answer = (uint32_t) ((raw_data * coefficient) >> CTSU_SHIFT_AMOUNT);

        /* Value Overflow Check */
        if ((uint32_t) CTSU_COUNT_MAX < answer)
        {
            *correction_data = CTSU_COUNT_MAX;
        }
        else
        {
            *correction_data = (uint16_t) answer;
        }
    }
    else
    {
        *correction_data = raw_data;
    }
}

/*******************************************************************************************************************//**
 * Corrects and averages the sensor data.
 *
 * @param[in] p_instance_ctrl    Pointer to CTSU control structure
 **********************************************************************************************************************/
static void ctsu_correction_exec (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t element_id;

    ctsu_correction_calc_t calc;
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    uint16_t * p_self_data;
    uint16_t   average_self;
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    uint16_t * p_pri_data;
    uint16_t * p_snd_data;
    uint16_t   average_pri;
    uint16_t   average_snd;
#endif

    for (element_id = (uint16_t) 0U; element_id < p_instance_ctrl->num_elements; element_id++)
    {
        calc.snum = (p_instance_ctrl->p_ctsuwr[element_id].ctsuso0 >> 10) & CTSU_SNUM_MAX;
        calc.sdpa = (p_instance_ctrl->p_ctsuwr[element_id].ctsuso1 >> 8) & CTSU_SDPA_MAX;
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
        if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
        {
            p_self_data  = (p_instance_ctrl->p_self_data + element_id);
            average_self = *p_self_data;
            ctsu_correction_calc(p_self_data, (p_instance_ctrl->p_self_raw + element_id)->sen, &calc);
            if ((uint16_t) 1U < p_instance_ctrl->average)
            {
                ctsu_moving_average(&average_self, *p_self_data, p_instance_ctrl->average);
                *p_self_data = average_self;
            }
        }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        p_pri_data = (p_instance_ctrl->p_mutual_pri_data + element_id);
        p_snd_data = (p_instance_ctrl->p_mutual_snd_data + element_id);
        if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
        {
            average_pri = *p_pri_data;
            average_snd = *p_snd_data;
            ctsu_correction_calc(p_pri_data, (p_instance_ctrl->p_mutual_raw + element_id)->pri_sen, &calc);
            ctsu_correction_calc(p_snd_data, (p_instance_ctrl->p_mutual_raw + element_id)->snd_sen, &calc);
            if ((uint16_t) 1U < p_instance_ctrl->average)
            {
                ctsu_moving_average(&average_pri, *p_pri_data, p_instance_ctrl->average);
                ctsu_moving_average(&average_snd, *p_snd_data, p_instance_ctrl->average);
                *p_pri_data = average_pri;
                *p_snd_data = average_snd;
            }
        }
#endif
    }
}

#if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
/*******************************************************************************************************************//**
 * Start to the diagnosis after saving the normal scan register.
 *
 * @param[in] p_instance_ctrl    Pointer to CTSU control structure
 **********************************************************************************************************************/
static void ctsu_diag_scan_start1 (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    if (CTSU_DIAG_INIT == g_ctsu_diag_info.state)
    {
        g_ctsu_diag_info.state = CTSU_DIAG_OVER_VOLTAGE;
    }

    /* ctsu normal scan register save */
    HW_CTSU_DiagRegisterSave(p_instance_ctrl->p_reg, &g_ctsu_diag_reg);

    /* scan register setting */
    if (CTSU_DIAG_OVER_VOLTAGE == g_ctsu_diag_info.state)
    {
        ctsu_diag_ldo_over_voltage_scan_start(p_instance_ctrl);
    }

    if (CTSU_DIAG_CCO_HIGH == g_ctsu_diag_info.state)
    {
        ctsu_diag_oscillator_high_scan_start(p_instance_ctrl);
    }

    if (CTSU_DIAG_CCO_LOW == g_ctsu_diag_info.state)
    {
        ctsu_diag_oscillator_low_scan_start(p_instance_ctrl);
    }

    if (CTSU_DIAG_SSCG == g_ctsu_diag_info.state)
    {
        ctsu_diag_sscg_scan_start(p_instance_ctrl);
    }

    if (CTSU_DIAG_DAC == g_ctsu_diag_info.state)
    {
        ctsu_diag_dac_scan_start(p_instance_ctrl);
    }
}

/*******************************************************************************************************************//**
 * Gets the diagnosis data then restores the normal scan register.
 *
 * @param[in] p_instance_ctrl    Pointer to CTSU control structure
 *
 * @retval    SSP_SUCCESS               Successfully configured.
 * @retval    SSP_ERR_CTSU_DIAG_NOT_YET Data for diagnosis has not been collected yet.
 **********************************************************************************************************************/
static ssp_err_t ctsu_diag_data_get1 (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    ssp_err_t err;

    /* data get */
    if (CTSU_DIAG_OVER_VOLTAGE == g_ctsu_diag_info.state)
    {
        g_ctsu_diag_info.state = CTSU_DIAG_CCO_HIGH;
    }
    else if (CTSU_DIAG_CCO_HIGH == g_ctsu_diag_info.state)
    {
        ctsu_diag_oscillator_high_data_get();

        g_ctsu_diag_info.state = CTSU_DIAG_CCO_LOW;
    }
    else if (CTSU_DIAG_CCO_LOW == g_ctsu_diag_info.state)
    {
        ctsu_diag_oscillator_low_data_get();

        g_ctsu_diag_info.state = CTSU_DIAG_SSCG;
    }
    else if (CTSU_DIAG_SSCG == g_ctsu_diag_info.state)
    {
        ctsu_diag_sscg_data_get();

        g_ctsu_diag_info.state = CTSU_DIAG_DAC;
    }
    else if (CTSU_DIAG_DAC == g_ctsu_diag_info.state)
    {
        ctsu_diag_dac_data_get();
        if (CTSU_TUNING_INCOMPLETE == g_ctsu_diag_info.tuning)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_DAC;
        }
        else
        {
            g_ctsu_diag_info.loop_count++;
            if (6U <= g_ctsu_diag_info.loop_count)
            {
                g_ctsu_diag_info.state      = CTSU_DIAG_COMPLETE;
                g_ctsu_diag_info.loop_count = 0U;
            }
        }
    }
    else
    {
        /* Do Nothing */
    }

    /* register restore */
    HW_CTSU_DiagRegisterRestore(&g_ctsu_diag_reg, p_instance_ctrl->p_reg);

    if (CTSU_DIAG_COMPLETE == g_ctsu_diag_info.state)
    {
        err = SSP_SUCCESS;
    }
    else
    {
        err = SSP_ERR_CTSU_DIAG_NOT_YET;
    }

    return err;
}

/*******************************************************************************************************************//**
 * Start to the diagnosis of over voltage.
 *
 * @param[in] p_instance_ctrl    Pointer to CTSU control structure
 **********************************************************************************************************************/
static void ctsu_diag_ldo_over_voltage_scan_start (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint32_t pclkb_mhz = CTSU_PCLKB_FREQ_RANGE1;
    uint32_t ctsu_sdpa;

    /* Set power on */
    HW_CTSU_PowerOn(p_instance_ctrl->p_reg);

    /* Since CLK is rewritten by correction, set here. */
    HW_CTSU_ClockSet(p_instance_ctrl->p_reg, (hw_ctsu_clock_t) CTSU_CFG_PCLK_DIVISION);

    g_cgc_on_cgc.systemClockFreqGet(CGC_SYSTEM_CLOCKS_PCLKB, &pclkb_mhz);
    pclkb_mhz /= CTSU_PCLKB_FREQ_MHZ;
    if (CTSU_PCLKB_FREQ_RANGE1 >= pclkb_mhz)
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_1);
        ctsu_sdpa        = pclkb_mhz - 1U;
    }
    else if ((CTSU_PCLKB_FREQ_RANGE1 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE2 >= pclkb_mhz))
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_2);
        ctsu_sdpa        = (pclkb_mhz / 2U) - 1U;
    }
    else
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_4);
        ctsu_sdpa        = (pclkb_mhz / 4U) - 1U;
    }

    HW_CTSU_DiagSelfscanModeSet(p_instance_ctrl->p_reg);

    /* Correction measurement setting */
    HW_CTSU_DiagOverVoltageCorrectionSet(p_instance_ctrl->p_reg);
    g_ctsu_diag_info.icomp         = 0U;

    g_ctsu_diag_info.ctsuwr.ctsussc = (CTSU_SSDIV_0500 << 8);
    g_ctsu_diag_info.ctsuwr.ctsuso0 = CTSU_DIAG_DAC_SO_MAX;
    g_ctsu_diag_info.ctsuwr.ctsuso1 = (uint16_t) ((CTSU_ICOG_66 << 13) | ((uint16_t) ctsu_sdpa << 8) | CTSU_RICOA_RECOMMEND);
}

/*******************************************************************************************************************//**
 * Returns the diagnosis result of over voltage.
 *
 * @retval SSP_SUCCESS                             CTSU successfully configured.
 * @retval SSP_ERR_CTSU_DIAG_LDO_OVER_VOLTAGE      Diagnosis of LDO over voltage failed.
 **********************************************************************************************************************/
static ssp_err_t ctsu_diag_ldo_over_voltage_result (void)
{
    if (1U != g_ctsu_diag_info.icomp)
    {
        return SSP_ERR_CTSU_DIAG_LDO_OVER_VOLTAGE;
    }

    /* if all checks passed to this point, return success */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Start to the diagnosis of oscillator 19.2uA.
 *
 * @param[in] p_instance_ctrl    Pointer to CTSU control structure
 **********************************************************************************************************************/
static void ctsu_diag_oscillator_high_scan_start (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint32_t ctsu_sdpa;
    uint32_t pclkb_mhz = CTSU_PCLKB_FREQ_RANGE1;

    /* Set power on */
    HW_CTSU_PowerOn(p_instance_ctrl->p_reg);

    /* Since CLK is rewritten by correction, set here. */
    HW_CTSU_ClockSet(p_instance_ctrl->p_reg, (hw_ctsu_clock_t) CTSU_CFG_PCLK_DIVISION);

    g_cgc_on_cgc.systemClockFreqGet(CGC_SYSTEM_CLOCKS_PCLKB, &pclkb_mhz);
    pclkb_mhz /= CTSU_PCLKB_FREQ_MHZ;
    if (CTSU_PCLKB_FREQ_RANGE1 >= pclkb_mhz)
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_1);
        ctsu_sdpa        = pclkb_mhz - 1U;
    }
    else if ((CTSU_PCLKB_FREQ_RANGE1 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE2 >= pclkb_mhz))
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_2);
        ctsu_sdpa        = (pclkb_mhz / 2U) - 1U;
    }
    else
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_4);
        ctsu_sdpa        = (pclkb_mhz / 4U) - 1U;
    }

    HW_CTSU_DiagSelfscanModeSet(p_instance_ctrl->p_reg);
    HW_CTSU_DiagSensorOffsetSet(p_instance_ctrl->p_reg);

    g_ctsu_diag_info.ctsuwr.ctsussc = (CTSU_SSDIV_0500 << 8);
    g_ctsu_diag_info.ctsuwr.ctsuso0 = (uint16_t) 0x0000U;
    g_ctsu_diag_info.ctsuwr.ctsuso1 = (uint16_t) ((CTSU_ICOG_66 << 13) | ((uint16_t) ctsu_sdpa << 8) | CTSU_RICOA_RECOMMEND);

    /* Correction measurement setting */
    HW_CTSU_DiagOscillatorHighCorrectionSet(p_instance_ctrl->p_reg);
    R_BSP_SoftwareDelay((uint32_t) 10U, BSP_DELAY_UNITS_MILLISECONDS);
}

/*******************************************************************************************************************//**
 * Get the diagnosis data of oscillator 19.2uA.
 **********************************************************************************************************************/
static void ctsu_diag_oscillator_high_data_get (void)
{
    g_ctsu_diag_info.cco_high = g_ctsu_diag_info.scanbuf.sen;
}

/*******************************************************************************************************************//**
 * Returns the diagnosis result of oscillator 19.2uA.
 *
 * @retval SSP_SUCCESS                             CTSU successfully configured.
 * @retval SSP_ERR_CTSU_DIAG_CCO_HIGH              Diagnosis of CCO into 19.2uA failed.
 **********************************************************************************************************************/
static ssp_err_t ctsu_diag_oscillator_high_result (void)
{
    if ((g_ctsu_diag_info.cco_high < CTSU_CFG_DIAG_CCO_HIGH_MAX) &&
        (g_ctsu_diag_info.cco_high > CTSU_CFG_DIAG_CCO_HIGH_MIN))
    {
    }
    else
    {
        return SSP_ERR_CTSU_DIAG_CCO_HIGH;
    }

    /* if all checks passed to this point, return success */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Start to the diagnosis of oscillator 2.4uA.
 *
 * @param[in] p_instance_ctrl    Pointer to CTSU control structure
 **********************************************************************************************************************/
static void ctsu_diag_oscillator_low_scan_start (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint32_t ctsu_sdpa;
    uint32_t pclkb_mhz = CTSU_PCLKB_FREQ_RANGE1;

    /* Set power on */
    HW_CTSU_PowerOn(p_instance_ctrl->p_reg);

    /* Since CLK is rewritten by correction, set here. */
    HW_CTSU_ClockSet(p_instance_ctrl->p_reg, (hw_ctsu_clock_t) CTSU_CFG_PCLK_DIVISION);

    g_cgc_on_cgc.systemClockFreqGet(CGC_SYSTEM_CLOCKS_PCLKB, &pclkb_mhz);
    pclkb_mhz /= CTSU_PCLKB_FREQ_MHZ;
    if (CTSU_PCLKB_FREQ_RANGE1 >= pclkb_mhz)
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_1);
        ctsu_sdpa        = pclkb_mhz - 1U;
    }
    else if ((CTSU_PCLKB_FREQ_RANGE1 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE2 >= pclkb_mhz))
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_2);
        ctsu_sdpa        = (pclkb_mhz / 2U) - 1U;
    }
    else
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_4);
        ctsu_sdpa        = (pclkb_mhz / 4U) - 1U;
    }

    HW_CTSU_DiagSelfscanModeSet(p_instance_ctrl->p_reg);
    HW_CTSU_DiagSensorOffsetSet(p_instance_ctrl->p_reg);

    g_ctsu_diag_info.ctsuwr.ctsussc = (CTSU_SSDIV_0500 << 8);
    g_ctsu_diag_info.ctsuwr.ctsuso0 = (uint16_t) 0x0000U;
    g_ctsu_diag_info.ctsuwr.ctsuso1 = (uint16_t) ((CTSU_ICOG_66 << 13) | ((uint16_t) ctsu_sdpa << 8) | CTSU_RICOA_RECOMMEND);

    /* Correction measurement setting */
    HW_CTSU_DiagOscillatorLowCorrectionSet(p_instance_ctrl->p_reg);
    R_BSP_SoftwareDelay((uint32_t) 10U, BSP_DELAY_UNITS_MILLISECONDS);
}

/*******************************************************************************************************************//**
 * Get the diagnosis data of oscillator 2.4uA.
 **********************************************************************************************************************/
static void ctsu_diag_oscillator_low_data_get (void)
{
    g_ctsu_diag_info.cco_low = g_ctsu_diag_info.scanbuf.sen;
}

/*******************************************************************************************************************//**
 * Returns the diagnosis result of oscillator 2.4uA.
 *
 * @retval SSP_SUCCESS                             CTSU successfully configured.
 * @retval SSP_ERR_CTSU_DIAG_CCO_LOW               Diagnosis of CCO into 2.4uA failed.
 **********************************************************************************************************************/
static ssp_err_t ctsu_diag_oscillator_low_result (void)
{
    if ((g_ctsu_diag_info.cco_low < CTSU_CFG_DIAG_CCO_LOW_MAX) &&
        (g_ctsu_diag_info.cco_low > CTSU_CFG_DIAG_CCO_LOW_MIN))
    {
    }
    else
    {
        return SSP_ERR_CTSU_DIAG_CCO_LOW;
    }

    /* if all checks passed to this point, return success */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Start to the diagnosis of sscg oscillator.
 *
 * @param[in] p_instance_ctrl    Pointer to CTSU control structure
 **********************************************************************************************************************/
static void ctsu_diag_sscg_scan_start (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint32_t ctsu_sdpa;
    uint32_t pclkb_mhz = CTSU_PCLKB_FREQ_RANGE1;

    /* Set power on */
    HW_CTSU_PowerOn(p_instance_ctrl->p_reg);

    /* Since CLK is rewritten by correction, set here. */
    HW_CTSU_ClockSet(p_instance_ctrl->p_reg, (hw_ctsu_clock_t) CTSU_CFG_PCLK_DIVISION);

    g_cgc_on_cgc.systemClockFreqGet(CGC_SYSTEM_CLOCKS_PCLKB, &pclkb_mhz);
    pclkb_mhz /= CTSU_PCLKB_FREQ_MHZ;
    if (CTSU_PCLKB_FREQ_RANGE1 >= pclkb_mhz)
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_1);
        ctsu_sdpa        = pclkb_mhz - 1U;
    }
    else if ((CTSU_PCLKB_FREQ_RANGE1 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE2 >= pclkb_mhz))
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_2);
        ctsu_sdpa        = (pclkb_mhz / 2U) - 1U;
    }
    else
    {
        HW_CTSU_ClockSet(p_instance_ctrl->p_reg, HW_CTSU_CLOCK_DIV_4);
        ctsu_sdpa        = (pclkb_mhz / 4U) - 1U;
    }

    HW_CTSU_DiagSelfscanModeSet(p_instance_ctrl->p_reg);
    HW_CTSU_DiagSensorOffsetSet(p_instance_ctrl->p_reg);

    g_ctsu_diag_info.ctsuwr.ctsussc = (CTSU_SSDIV_0500 << 8);
    g_ctsu_diag_info.ctsuwr.ctsuso0 = (uint16_t) 0x0000U;
    g_ctsu_diag_info.ctsuwr.ctsuso1 = (uint16_t) ((CTSU_ICOG_66 << 13) | ((uint16_t) ctsu_sdpa << 8) | CTSU_RICOA_RECOMMEND);

    /* Correction measurement setting */
    HW_CTSU_DiagSscgCorrectionSet(p_instance_ctrl->p_reg);
}

/*******************************************************************************************************************//**
 * Get the diagnosis data of sscg oscillator.
 **********************************************************************************************************************/
static void ctsu_diag_sscg_data_get (void)
{
    g_ctsu_diag_info.sscg = g_ctsu_diag_info.scanbuf.ref;
}

/*******************************************************************************************************************//**
 * Returns the diagnosis result of sscg oscillator.
 *
 * @retval SSP_SUCCESS                             CTSU successfully configured.
 * @retval SSP_ERR_CTSU_DIAG_SSCG                  Diagnosis of SSCG frequency failed.
 **********************************************************************************************************************/
static ssp_err_t ctsu_diag_sscg_result (void)
{
    if ((g_ctsu_diag_info.sscg > CTSU_CFG_DIAG_SSCG_MAX) || (g_ctsu_diag_info.sscg < CTSU_CFG_DIAG_SSCG_MIN))
    {
        return SSP_ERR_CTSU_DIAG_SSCG;
    }

    /* if all checks passed to this point, return success */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Run the initial tuning for diagnosis of dac.
 **********************************************************************************************************************/
static void ctsu_diag_dac_initial_tuning (void)
{
    int32_t  diff          = 0;
    uint32_t complete_flag = 0;
    uint16_t ctsuso;

    diff = (int32_t) (g_ctsu_diag_info.correct_data - CTSU_DIAG_DAC_TARGET_VALUE);

    ctsuso = g_ctsu_diag_info.ctsuwr.ctsuso0 & CTSU_TUNING_MAX;
    if (0 < diff)
    {
        if (g_ctsu_diag_info.tuning_diff < 0)
        {
            if ((-diff) > g_ctsu_diag_info.tuning_diff)
            {
                ctsuso++;
            }

            complete_flag = 1U;
        }
        else
        {
            if (CTSU_TUNING_MAX == ctsuso)
            {
                complete_flag = 1U;
            }
            else
            {
                ctsuso++;
                g_ctsu_diag_info.tuning_diff = diff;
            }
        }
    }
    else if (0 == diff)
    {
        complete_flag = 1U;
    }
    else
    {
        if (g_ctsu_diag_info.tuning_diff > 0)
        {
            if ((-diff) > g_ctsu_diag_info.tuning_diff)
            {
                ctsuso--;
            }

            complete_flag = 1U;
        }
        else
        {
            complete_flag = 1U;
        }
    }

    g_ctsu_diag_info.ctsuwr.ctsuso0 &= (uint16_t) (~CTSU_TUNING_MAX);
    g_ctsu_diag_info.ctsuwr.ctsuso0 |= ctsuso;

    if (1U == complete_flag)
    {
        g_ctsu_diag_info.tuning_diff = CTSU_TUNING_OT_COUNT;
        g_ctsu_diag_info.so0_4uc_val = ctsuso;
        g_ctsu_diag_info.dac_init    = 3U;
        g_ctsu_diag_info.tuning      = CTSU_TUNING_COMPLETE;
        g_ctsu_diag_info.loop_count  = 0U;
        g_ctsu_diag_info.dac_cnt[0]  = g_ctsu_diag_info.correct_data;
    }
}

/*******************************************************************************************************************//**
 * Start to the diagnosis of dac.
 *
 * @param[in] p_instance_ctrl    Pointer to CTSU control structure
 **********************************************************************************************************************/
static void ctsu_diag_dac_scan_start (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    /* Set power on */
    HW_CTSU_PowerOn(p_instance_ctrl->p_reg);

    /* Since CLK is rewritten by correction, set here. */
    HW_CTSU_ClockSet(p_instance_ctrl->p_reg, (hw_ctsu_clock_t) CTSU_CFG_PCLK_DIVISION);

    HW_CTSU_DiagDacSelfscanModeSet(p_instance_ctrl->p_reg, p_instance_ctrl->ctsucr1, (uint8_t) CTSU_CFG_DIAG_DAC_TS);

    g_ctsu_diag_info.ctsuwr.ctsussc = (uint16_t) (CTSU_SSDIV_1330 << 8);
    g_ctsu_diag_info.ctsuwr.ctsuso1 = (uint16_t) ((CTSU_ICOG_RECOMMEND << 13) | ((uint16_t) 7U << 8) | CTSU_RICOA_RECOMMEND);

    if (g_ctsu_diag_info.dac_init > 2U)
    {
        /* Apply DAC current */
        if (0U == g_ctsu_diag_info.loop_count)
        {
            g_ctsu_diag_info.ctsuwr.ctsuso0 = (uint16_t) (((uint16_t) 3U << 10) | g_ctsu_diag_info.so0_4uc_val);
        }
        else if (1U == g_ctsu_diag_info.loop_count)
        {
            g_ctsu_diag_info.ctsuwr.ctsuso0 =
                (uint16_t) (((uint16_t) 3U << 10) | (g_ctsu_diag_info.so0_4uc_val - CTSU_DIAG_DAC_1UC));
        }
        else if (2U == g_ctsu_diag_info.loop_count)
        {
            g_ctsu_diag_info.ctsuwr.ctsuso0 =
                (uint16_t) (((uint16_t) 3U << 10) | (g_ctsu_diag_info.so0_4uc_val - CTSU_DIAG_DAC_2UC));
        }
        else if (3U == g_ctsu_diag_info.loop_count)
        {
            g_ctsu_diag_info.ctsuwr.ctsuso0 =
                (uint16_t) (((uint16_t) 3U << 10) | (g_ctsu_diag_info.so0_4uc_val - CTSU_DIAG_DAC_4UC));
        }
        else if (4U == g_ctsu_diag_info.loop_count)
        {
            g_ctsu_diag_info.ctsuwr.ctsuso0 =
                (uint16_t) (((uint16_t) 3U << 10) | (g_ctsu_diag_info.so0_4uc_val - CTSU_DIAG_DAC_8UC));
        }
        else if (5U == g_ctsu_diag_info.loop_count)
        {
            g_ctsu_diag_info.ctsuwr.ctsuso0 =
                (uint16_t) (((uint16_t) 3U << 10) | (g_ctsu_diag_info.so0_4uc_val - CTSU_DIAG_DAC_16UC));
        }
        else
        {
            /* Do nothing */
        }
    }

    if (0U == g_ctsu_diag_info.dac_init)
    {
        g_ctsu_diag_info.dac_init       = 1U;
        g_ctsu_diag_info.so0_4uc_val    = 0U;
        g_ctsu_diag_info.tuning_diff    = 0;
        g_ctsu_diag_info.ctsuwr.ctsuso0 = (uint16_t) (((uint16_t) 3U << 10) + CTSU_DIAG_DAC_START_VALUE);
    }
}

/*******************************************************************************************************************//**
 * Get the diagnosis data of dac.
 **********************************************************************************************************************/
static void ctsu_diag_dac_data_get (void)
{
    ctsu_correction_calc_t calc;

    calc.snum = (g_ctsu_diag_info.ctsuwr.ctsuso0 >> 10) & CTSU_SNUM_MAX;

    if (CTSU_DIAG_DAC == g_ctsu_diag_info.state)
    {
        calc.snum = 3U;
    }

    calc.sdpa = (g_ctsu_diag_info.ctsuwr.ctsuso1 >> 8) & CTSU_SDPA_MAX;

    /* Correction process */
    ctsu_correction_calc(&g_ctsu_diag_info.correct_data, g_ctsu_diag_info.scanbuf.sen, &calc);

    if (CTSU_TUNING_COMPLETE == g_ctsu_diag_info.tuning)
    {
        g_ctsu_diag_info.dac_cnt[g_ctsu_diag_info.loop_count] = g_ctsu_diag_info.correct_data;
    }
    else
    {
        ctsu_diag_dac_initial_tuning();
    }
}

/*******************************************************************************************************************//**
 * Returns the diagnosis result of dac.
 *
 * @retval SSP_SUCCESS                             CTSU successfully configured.
 * @retval SSP_ERR_CTSU_DIAG_DAC                   Diagnosis of non-touch count value failed.
 **********************************************************************************************************************/
static ssp_err_t ctsu_diag_dac_result (void)
{
    uint8_t k;
    if (CTSU_TUNING_COMPLETE == g_ctsu_diag_info.tuning)
    {
        for (k = 0U; k < 6U; k++)
        {
            if ((g_ctsu_diag_info.dac_cnt[k] > dac_oscil_table[k][0]) ||
                (g_ctsu_diag_info.dac_cnt[k] < dac_oscil_table[k][1]))
            {
                return SSP_ERR_CTSU_DIAG_DAC;
            }
        }
    }

    /* if all checks passed to this point, return success */
    return SSP_SUCCESS;
}
#endif

/*******************************************************************************************************************//**
 * CTSUWR interrupt handler. This service routine sets the tuning for the next element to be scanned by hardware.
 **********************************************************************************************************************/
void ctsu_write_isr (void)
{
#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)

    /** Clear the BSP IRQ Flag     */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());
#else
    IRQn_Type              irq             = R_SSP_CurrentIrqGet();
    ssp_vector_info_t    * p_vector_info   = NULL;
    R_SSP_VectorInfoGet(irq, &p_vector_info);
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /** Clear the BSP IRQ Flag     */
    R_BSP_IrqStatusClear(irq);

    /* Write settings for current element */
    if (CTSU_CORRECTION_RUN == g_ctsu_correction_info.status)
    {
        HW_CTSU_SensorSet(p_instance_ctrl->p_reg, &g_ctsu_correction_info.ctsuwr);
    }
    else if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
  #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
        HW_CTSU_SensorSet(p_instance_ctrl->p_reg, &g_ctsu_diag_info.ctsuwr);
  #endif
    }
    else
    {
        HW_CTSU_SensorSet(p_instance_ctrl->p_reg, &p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index]);
        p_instance_ctrl->wr_index++;
    }
#endif
}

/*******************************************************************************************************************//**
 * CTSURD interrupt handler. This service routine reads the sensor count and reference counter for
 * the current element and places the value in the scan data buffer. Note that the reference counter
 * does not work properly but is saved anyway for backward compatibility and potential future use.
 * Additionally, the SC register cannot be read again until RC is read.
 **********************************************************************************************************************/
void ctsu_read_isr (void)
{
#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)

    /** Clear the BSP IRQ Flag     */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());
#else
    IRQn_Type              irq             = R_SSP_CurrentIrqGet();
    ssp_vector_info_t    * p_vector_info   = NULL;
    R_SSP_VectorInfoGet(irq, &p_vector_info);
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /** Clear the BSP IRQ Flag     */
    R_BSP_IrqStatusClear(irq);

    /* read current channel/element value */
    /* Store the reference counter for possible future use. Register must be read or scan will hang. */

    if (CTSU_CORRECTION_RUN == g_ctsu_correction_info.status)
    {
        ctsu_self_buf_t * p_buf = (ctsu_self_buf_t *) &g_ctsu_correction_info.scanbuf;
        HW_CTSU_MeasurementResultGet(p_instance_ctrl->p_reg, &p_buf->sen, &p_buf->ref);
    }
  #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    else if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        ctsu_self_buf_t * p_buf = (ctsu_self_buf_t *) &g_ctsu_diag_info.scanbuf;
        HW_CTSU_MeasurementResultGet(p_instance_ctrl->p_reg, &p_buf->sen, &p_buf->ref);
    }
  #endif
    else
    {
  #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
        if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
        {
            ctsu_self_buf_t * p_buf = p_instance_ctrl->p_self_raw + p_instance_ctrl->rd_index;
            HW_CTSU_MeasurementResultGet(p_instance_ctrl->p_reg, &p_buf->sen, &p_buf->ref);
            p_instance_ctrl->rd_index++;
        }
  #endif
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
        {
            ctsu_mutual_buf_t * p_buf = p_instance_ctrl->p_mutual_raw + p_instance_ctrl->rd_index;
            if (HW_CTSU_IsSecondMeasurement(p_instance_ctrl->p_reg))
            {
                HW_CTSU_MeasurementResultGet(p_instance_ctrl->p_reg, &p_buf->pri_sen, &p_buf->pri_ref);
            }
            else
            {
                HW_CTSU_MeasurementResultGet(p_instance_ctrl->p_reg, &p_buf->snd_sen, &p_buf->snd_ref);
                p_instance_ctrl->rd_index++;
            }
        }
  #endif
    }

    /* Countermeasure for the problem that RD interrupt and FN interrupt are reversed. */ 
    if (1 == g_ctsu_interrupt_reverse_flag)
    {
        ctsu_callback_args_t   args = {(ctsu_event_t) 0U};

        g_ctsu_interrupt_reverse_flag = (uint8_t) 0U;
        
        /* Store callback arguments in memory provided by user if available.  This allows callback arguments to be
        * stored in non-secure memory so they can be accessed by a non-secure callback function. */
        ctsu_callback_args_t * p_args = p_instance_ctrl->p_callback_memory;
        if (NULL == p_args)
        {
            /* Store on stack */
            p_args = &args;
        }
        else
        {
            /* Save current arguments on the stack in case this is a nested interrupt. */
            args = *p_args;
        }

        p_args->event = CTSU_EVENT_SCAN_COMPLETE;

        if (HW_CTSU_IsSensorCounterOverflow(p_instance_ctrl->p_reg))
        {
            p_args->event += CTSU_EVENT_OVERFLOW;
            HW_CTSU_SensorCounterOverflowFlagClear(p_instance_ctrl->p_reg);
        }

        if (HW_CTSU_IsTscapVoltageError(p_instance_ctrl->p_reg))
        {
            HW_CTSU_TscapVoltageErrorClear(p_instance_ctrl->p_reg);
            p_args->event += CTSU_EVENT_ICOMP;
        }

        p_instance_ctrl->state = CTSU_STATE_SCANNED;
        p_args->p_context      = p_instance_ctrl->p_context;

        /* If a callback was provided, call it with the argument */
        if (NULL != p_instance_ctrl->p_callback)
        {
            p_instance_ctrl->p_callback(p_args);
        }

        if (NULL != p_instance_ctrl->p_callback_memory)
        {
            /* Restore callback memory in case this is a nested interrupt. */
            *p_instance_ctrl->p_callback_memory = args;
        }

        /* reset indexes */
        p_instance_ctrl->wr_index = (uint16_t) 0U;
        p_instance_ctrl->rd_index = (uint16_t) 0U;
    }

#endif
}

/*******************************************************************************************************************//**
 * CTSUFN interrupt handler. This service routine occurs when all elements have been scanned (finished).
 * The user's callback function is called if available.
 **********************************************************************************************************************/
void ctsu_end_isr (void)
{
    IRQn_Type              irq             = R_SSP_CurrentIrqGet();
    ssp_vector_info_t    * p_vector_info   = NULL;
    R_SSP_VectorInfoGet(irq, &p_vector_info);
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) *(p_vector_info->pp_ctrl);
    ctsu_callback_args_t   args = {(ctsu_event_t) 0U};

    /** Clear the BSP IRQ Flag     */
    R_BSP_IrqStatusClear(irq);

#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 0)
    /* Countermeasure for the problem that RD interrupt and FN interrupt are reversed. */ 
    if (p_instance_ctrl->rd_index != p_instance_ctrl->wr_index)
    {
        g_ctsu_interrupt_reverse_flag = (uint8_t) 1U;
        return;
    }
#endif

    /* Store callback arguments in memory provided by user if available.  This allows callback arguments to be
     * stored in non-secure memory so they can be accessed by a non-secure callback function. */
    ctsu_callback_args_t * p_args = p_instance_ctrl->p_callback_memory;
    if (NULL == p_args)
    {
        /* Store on stack */
        p_args = &args;
    }
    else
    {
        /* Save current arguments on the stack in case this is a nested interrupt. */
        args = *p_args;
    }

#if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_DIAG_OVER_VOLTAGE == g_ctsu_diag_info.state)
    {
        if (HW_CTSU_IsTscapVoltageError(p_instance_ctrl->p_reg))
        {
            g_ctsu_diag_info.icomp = 1U;
        }
    }
#endif

    p_args->event = CTSU_EVENT_SCAN_COMPLETE;

    if (HW_CTSU_IsSensorCounterOverflow(p_instance_ctrl->p_reg))
    {
        p_args->event += CTSU_EVENT_OVERFLOW;
        HW_CTSU_SensorCounterOverflowFlagClear(p_instance_ctrl->p_reg);
    }

    if (HW_CTSU_IsTscapVoltageError(p_instance_ctrl->p_reg))
    {
        HW_CTSU_TscapVoltageErrorClear(p_instance_ctrl->p_reg);
        p_args->event += CTSU_EVENT_ICOMP;
    }

    p_instance_ctrl->state = CTSU_STATE_SCANNED;
    p_args->p_context      = p_instance_ctrl->p_context;

    /* If a callback was provided, call it with the argument */
    if (NULL != p_instance_ctrl->p_callback)
    {
        p_instance_ctrl->p_callback(p_args);
    }

    if (NULL != p_instance_ctrl->p_callback_memory)
    {
        /* Restore callback memory in case this is a nested interrupt. */
        *p_instance_ctrl->p_callback_memory = args;
    }

    /* reset indexes */
    p_instance_ctrl->wr_index = (uint16_t) 0U;
    p_instance_ctrl->rd_index = (uint16_t) 0U;
}
