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
 * File Name    : r_cac.c
 * Description  : High level drivers for CAC peripheral
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_cac.h"
#include "r_cac_private.h"
#include "r_cac_private_api.h" 

/* Configuration for this package. */
#include "r_cac_cfg.h"
#include "r_cgc_api.h"
#include "r_cgc.h"
#include "hw/hw_cac_private.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** Macro for error logger. */
#ifndef CAC_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define CAC_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_cac_version)
#endif

/** "CAC" in ASCII, used to determine if channel is open. */
#define CAC_OPEN                (0x00434143ULL)

#define    FREQ_GET_ERROR_OR_EXT_CLOCK (0U)      ///< Returned by failed systemClockFreqGet() or cac_get_effective_frequency()
                                                 ///< for external clock

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static ssp_err_t cac_fmi_setup (cac_instance_ctrl_t * const p_ctrl, ssp_feature_t *p_ssp);
void cac_frequency_error_isr (void);
void cac_measurement_end_isr (void);
void cac_overflow_isr (void);
static void cac_enable_interrupts (cac_instance_ctrl_t * const p_ctrl, cac_cfg_t const * const p_cfg);
static void cac_irq_cfg (cac_instance_ctrl_t * const p_ctrl, bool state, cac_cfg_t const * const p_cfg);
static void cac_setup (cac_instance_ctrl_t * const p_ctrl, cac_cfg_t const * const p_cfg);
static void cac_disable_nvic_interrupts (cac_instance_ctrl_t * const p_ctrl);
static ssp_err_t cac_setup_vectors (cac_instance_ctrl_t * const p_ctrl, cac_cfg_t const * const p_cfg);


#if (CAC_CFG_PARAM_CHECKING_ENABLE)
static uint32_t  cac_get_effective_frequency (cac_cfg_t const * const p_cfg, cac_clock_type_t clockType);
static ssp_err_t cac_parameter_checking (cac_instance_ctrl_t * const p_ctrl, cac_cfg_t const * const p_cfg);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
static const uint8_t cac_clock_meas_div_shifters[] =
{
    0U,    // divisor = 1
    2U,    // divisor = 4
    3U,    // divisor = 8
    5U     // divisor = 32
};

static const uint8_t  cac_clock_ref_div_shifters[] =
{
    5U,    // divisor = 32
    7U,    // divisor = 128
    10U,   // divisor = 1024
    13U    // divisor = 8192
};
#endif


/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char          g_module_name[] = "cac";
#endif


/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

#if defined(__GNUC__)
/* This pragma suppresses the warnings in this structure only, and will be removed when the SSP compiler is updated to v5.3.*/

/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_cac_version =
{
    .api_version_minor  = CAC_API_VERSION_MINOR,
    .api_version_major  = CAC_API_VERSION_MAJOR,
    .code_version_major = CAC_CODE_VERSION_MAJOR,
    .code_version_minor = CAC_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const cac_api_t g_cac_on_cac =
{
    .open             = R_CAC_Open,
    .close            = R_CAC_Close,
    .read             = R_CAC_Read,
    .stopMeasurement  = R_CAC_StopMeasurement,
    .startMeasurement = R_CAC_StartMeasurement,
    .reset            = R_CAC_Reset,
    .versionGet       = R_CAC_VersionGet
};


/*******************************************************************************************************************//**
 * @addtogroup CAC
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Initialize the CAC peripheral.
 *
 * The Open function applies power to the CAC peripheral, checks/sets the interrupt priority, and configures the
 * CAC based on the provided user configuration settings. If a user defined callback function has been provided in the
 * configuration, then the CAC interrupt(s) will be enabled and the user callback function called accordingly.
 * Implements   r_cac_t::open.
 * @retval SSP_SUCCESS              CAC is available and available for measurement(s).
 * @retval SSP_ERR_ASSERTION        Null Pointer.
 * @retval SSP_ERR_INVALID_ARGUMENT One or more configuration options are invalid.
 * @retval SSP_ERR_HW_LOCKED        Hardware lock for CAC peripheral is already taken.
 * @retval SSP_ERR_INVALID_CAC_REF_CLOCK  Measured clock rate smaller than reference clock rate.
 * @return                          See @ref Common_Error_Codes or functions called by this function for other possible
 *                                  return codes. This function calls:
 *                                     * fmi_api_t::productFeatureGet
 *                                     * fmi_api_t::eventInfoGet
 *
 *
 * @note There is only a single CAC peripheral. It is not reentrant.
 **********************************************************************************************************************/
ssp_err_t R_CAC_Open (cac_ctrl_t * const p_api_ctrl, cac_cfg_t const * const p_cfg)
{
    cac_instance_ctrl_t * p_ctrl = (cac_instance_ctrl_t *) p_api_ctrl;
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0}};

    /** g_cac_version is accessed by the ASSERT macro only and so compiler toolchain can issue a
     *  warning that they are not accessed. The code below eliminates this warning and also ensures these data
     *  structures are not optimised away. */
    SSP_PARAMETER_NOT_USED(g_cac_version);

    /** Eliminate warning if parameter checking is disabled. */
    SSP_PARAMETER_NOT_USED(p_ctrl);

    ssp_err_t err = SSP_SUCCESS;

#if (CAC_CFG_PARAM_CHECKING_ENABLE)
    err = cac_parameter_checking(p_ctrl, p_cfg);
    CAC_ERROR_RETURN((err == SSP_SUCCESS), err);
#endif

    err = cac_fmi_setup (p_ctrl, &ssp_feature);
    CAC_ERROR_RETURN((err == SSP_SUCCESS), err);

    /** Take the hardware lock for the CAC. */
    CAC_ERROR_RETURN(!(SSP_SUCCESS != R_BSP_HardwareLock(&ssp_feature)), SSP_ERR_HW_LOCKED);

    /** Setup the interrupt vectors and priorities */
    err = cac_setup_vectors (p_ctrl, p_cfg);
    if (SSP_SUCCESS != err)
    {
        /** Return the hardware lock for the CAC. */
        R_BSP_HardwareUnlock(&ssp_feature);
    }
    else
    {
        /** Apply power to the peripheral */
        R_BSP_ModuleStart(&ssp_feature);

        /** Configure the CAC per the configuration. */
        cac_setup(p_ctrl, p_cfg);

        p_ctrl->measurement_clock = p_cfg->cac_meas_clock.clock;
        p_ctrl->reference_clock   = p_cfg->cac_ref_clock.clock;

        /** Store the callback and context information */
        p_ctrl->p_callback  = p_cfg->p_callback;
        p_ctrl->p_context   = p_cfg->p_context;

        /** Mark driver as open by initializing it to "CAC" - its ASCII equivalent. */
        p_ctrl->cac_api_open      = CAC_OPEN;   // API is now open
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief      Release any resources that were allocated by the Open() or any subsequent CAC operations.
 *             Implements r_cac_t::close.
 * @retval SSP_SUCCESS           Successful close.
 * @retval SSP_ERR_ASSERTION     NULL provided for p_ctrl or p_cfg.
 * @retval SSP_ERR_NOT_OPEN      R_CAC_Open() has not been successfully called.
 **********************************************************************************************************************/
ssp_err_t R_CAC_Close (cac_ctrl_t * const p_api_ctrl)
{
    cac_instance_ctrl_t * p_ctrl = (cac_instance_ctrl_t *) p_api_ctrl;
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};;

    /** Eliminate warning if parameter checking is disabled. */
    SSP_PARAMETER_NOT_USED(p_ctrl);

#if (CAC_CFG_PARAM_CHECKING_ENABLE == 1)
    SSP_ASSERT(NULL != p_ctrl);

    /** Insure API has been opened */
    CAC_ERROR_RETURN((CAC_OPEN == p_ctrl->cac_api_open), SSP_ERR_NOT_OPEN);
#endif

    cac_irq_cfg(p_ctrl, false, NULL);     /// Disable interrupts in the peripheral and NVIC

    /** Disable interrupts in NVIC. */
    cac_disable_nvic_interrupts(p_ctrl);

    R_CAC_Type * p_cac_reg = (R_CAC_Type *) p_ctrl->p_reg;
    HW_CAC_DisableCACInterrupts(p_cac_reg);          /// Disable the CAC ints.

    HW_CAC_StopMeasuring(p_cac_reg);              /// Stop measuring.

    /** Power down peripheral. */
    R_BSP_ModuleStop(&ssp_feature);

    p_ctrl->cac_api_open = 0U;        // API is now closed

    /** Return the hardware lock for the CAC. */
    ssp_feature.id = SSP_IP_CAC;
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    R_BSP_HardwareUnlock(&ssp_feature);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Stop the CAC measurement process.
 *             Implements r_cac_t::stopMeasurement.
 * @retval SSP_SUCCESS           CAC measuring has been stopped.
 * @retval SSP_ERR_ASSERTION     NULL provided for p_ctrl or p_cfg.
 * @retval SSP_ERR_NOT_OPEN      R_CAC_Open() has not been successfully called.
 **********************************************************************************************************************/
ssp_err_t R_CAC_StopMeasurement (cac_ctrl_t * const p_api_ctrl)
{
    cac_instance_ctrl_t * p_ctrl = (cac_instance_ctrl_t *) p_api_ctrl;

    /** Eliminate warning if parameter checking is disabled. */
    SSP_PARAMETER_NOT_USED(p_ctrl);

#if (CAC_CFG_PARAM_CHECKING_ENABLE == 1)
    SSP_ASSERT(NULL != p_ctrl);

    /** Insure API has been opened. */
    CAC_ERROR_RETURN((CAC_OPEN == p_ctrl->cac_api_open), SSP_ERR_NOT_OPEN);
#endif
    R_CAC_Type * p_cac_reg = (R_CAC_Type *) p_ctrl->p_reg;
    HW_CAC_StopMeasuring(p_cac_reg);              /// Stop measuring.
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Start the CAC measurement process.
 *             Implements r_cac_t::startMeasurement.
 *
 * @retval SSP_SUCCESS            CAC measurement started.
 * @retval SSP_ERR_ASSERTION      NULL provided for p_ctrl or p_cfg.
 * @retval SSP_ERR_NOT_OPEN       R_CAC_Open() has not been successfully called.
 * @retval SSP_ERR_CLOCK_INACTIVE Either the provided Measurement or Reference clock is not running
 * @return                        See @ref Common_Error_Codes or functions called by this function for other possible
 *                                return codes. This function calls:
 *                                     * cgc_api_t::clockCheck
 *
 **********************************************************************************************************************/
ssp_err_t R_CAC_StartMeasurement (cac_ctrl_t * const p_api_ctrl)
{
    cac_instance_ctrl_t * p_ctrl = (cac_instance_ctrl_t *) p_api_ctrl;

    /** Eliminate warnings if parameter checking is disabled. */
    SSP_PARAMETER_NOT_USED(p_ctrl);

    cgc_clock_t clk;
    ssp_err_t err;

#if (CAC_CFG_PARAM_CHECKING_ENABLE == 1)
    SSP_ASSERT(NULL != p_ctrl);

    /** Insure API has been opened. */
    CAC_ERROR_RETURN((CAC_OPEN == p_ctrl->cac_api_open), SSP_ERR_NOT_OPEN);
#endif

    /* Are the requested clocks running? */
    clk =  (cgc_clock_t) (cac_clock_to_cgc_clock[p_ctrl->measurement_clock]);
    if (CGC_CLK_NOT_SUPPORTED != clk)
    {
        err = g_cgc_on_cgc.clockCheck((cgc_clock_t) clk);
        CAC_ERROR_RETURN(((err == SSP_ERR_STABILIZED) || (err == SSP_ERR_CLOCK_ACTIVE)), SSP_ERR_CLOCK_INACTIVE);
    }

    if (CAC_CLOCK_SOURCE_EXTERNAL != p_ctrl->reference_clock)
    {
        clk = (cgc_clock_t) (cac_clock_to_cgc_clock[p_ctrl->reference_clock]);
        if (clk != CGC_CLK_NOT_SUPPORTED)
        {
            err = g_cgc_on_cgc.clockCheck((cgc_clock_t) clk);
            CAC_ERROR_RETURN(((err == SSP_ERR_STABILIZED) || (err == SSP_ERR_CLOCK_ACTIVE)), SSP_ERR_CLOCK_INACTIVE);
        }
    }

    R_CAC_Type * p_cac_reg = (R_CAC_Type *) p_ctrl->p_reg;
    HW_CAC_StartMeasuring(p_cac_reg);              /// Start measuring.
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Resets the Overflow, Measurement End and Frequency Error interrupt flags. This will clear any of the
 *             CASTR status bits that have been set, but only if the CFME bit is off (Not measuring).
 *             Implements r_cac_t::reset.
 *
 * @retval SSP_SUCCESS           CAC reset completed.
 * @retval SSP_ERR_ASSERTION     NULL provided for p_ctrl or p_cfg.
 * @retval SSP_ERR_NOT_OPEN      R_CAC_Open() has not been successfully called.
 **********************************************************************************************************************/
ssp_err_t R_CAC_Reset (cac_ctrl_t * const p_api_ctrl)
{
    cac_instance_ctrl_t * p_ctrl = (cac_instance_ctrl_t *) p_api_ctrl;

    /** Eliminate warning if parameter checking is disabled. */
    SSP_PARAMETER_NOT_USED(p_ctrl);

#if (CAC_CFG_PARAM_CHECKING_ENABLE == 1)
    SSP_ASSERT(NULL != p_ctrl);

    /** Insure API has been opened. */
    CAC_ERROR_RETURN((CAC_OPEN == p_ctrl->cac_api_open), SSP_ERR_NOT_OPEN);
#endif
    R_CAC_Type * p_cac_reg = (R_CAC_Type *) p_ctrl->p_reg;
    HW_CAC_Reset(p_cac_reg);              /// Reset the CAC.
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Read and return the CAC status and counter registers.
 *             Implements r_cac_t::read.
 * @retval SSP_SUCCESS        CAC read successful.
 * @retval SSP_ERR_ASSERTION  NULL provided for p_ctrl or p_cfg.
 * @retval SSP_ERR_NOT_OPEN   R_CAC_Open() has not been successfully called.
 **********************************************************************************************************************/
ssp_err_t R_CAC_Read (cac_ctrl_t * const p_api_ctrl, uint8_t  * const p_status,  uint16_t * const p_counter)
{
    cac_instance_ctrl_t * p_ctrl = (cac_instance_ctrl_t *) p_api_ctrl;

    /** Eliminate warning if parameter checking is disabled. */
    SSP_PARAMETER_NOT_USED(p_ctrl);

#if (CAC_CFG_PARAM_CHECKING_ENABLE == 1)
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_status);
    SSP_ASSERT(NULL != p_counter);

    /** Insure API has been opened. */
    CAC_ERROR_RETURN((CAC_OPEN == p_ctrl->cac_api_open), SSP_ERR_NOT_OPEN);
#endif
    R_CAC_Type * p_cac_reg = (R_CAC_Type *) p_ctrl->p_reg;
    HW_CAC_Read(p_cac_reg, p_status, p_counter);
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   Get the API and code version information.
 * @retval  SSP_SUCCESS  Version info returned.
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_CAC_VersionGet (ssp_version_t * const p_version)
{
#if CAC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_version);
#endif

    p_version->version_id = g_cac_version.version_id;
    return SSP_SUCCESS;
}  /* End of function R_CAC_VersionGet() */
/*******************************************************************************************************************//**
 * @} (end addtogroup CAC)
 **********************************************************************************************************************/

#if (CAC_CFG_PARAM_CHECKING_ENABLE)
/*******************************************************************************************************************//**
 * Calculate the actual divided down frequency for either the reference or measured clock.
 * This function is not called when the measurement clock is External as it is not possible to determine the
 * frequency of an externally supplied clock.
 * @param[in]  cac_cfg_t * const  p_cfg - pointer to the CAC configuration settings.
 * @param[in]  cac_clock_type_t clockType  - CAC_MEASURED_CLOCK or CAC_REFERENCE_CLOCK
 * @return  The calculated, divided down frequency for the specified clock, or FREQ_GET_ERROR_OR_EXT_CLOCK if error.
 *          See @ref Common_Error_Codes or functions called by this function for other possible return codes.
 *          This function calls:
 *                                     * cgc_api_t::systemClockFreqGet
 **********************************************************************************************************************/
static uint32_t cac_get_effective_frequency (cac_cfg_t const * const p_cfg, cac_clock_type_t clockType)
{
    uint32_t           freq    = 0;
    ssp_err_t          err;
    cac_clock_source_t clock   = p_cfg->cac_meas_clock.clock;
    uint8_t            divisor = cac_clock_meas_div_shifters[p_cfg->cac_meas_clock.divider];

    if (clockType == CAC_CLOCK_REFERENCE)
    {
        clock   = p_cfg->cac_ref_clock.clock;
        divisor = cac_clock_ref_div_shifters[p_cfg->cac_ref_clock.divider];
    }

    switch (clock)
    {
        case CAC_CLOCK_SOURCE_MAIN_OSC:
            freq = BSP_CFG_XTAL_HZ;
            break;

        case CAC_CLOCK_SOURCE_SUBCLOCK:
            freq = BSP_SUB_CLOCK_HZ;
            break;

        case CAC_CLOCK_SOURCE_HOCO:
            freq = BSP_HOCO_HZ;
            break;

        case CAC_CLOCK_SOURCE_MOCO:
            freq = BSP_MOCO_HZ;
            break;

        case CAC_CLOCK_SOURCE_LOCO:
            freq = BSP_LOCO_HZ;
            break;

        case CAC_CLOCK_SOURCE_PCLKB:
            err = g_cgc_on_cgc.systemClockFreqGet(CGC_SYSTEM_CLOCKS_PCLKB, &freq);
            if (err != SSP_SUCCESS)
            {
                return (FREQ_GET_ERROR_OR_EXT_CLOCK);
            }
            break;

        case CAC_CLOCK_SOURCE_IWDT:
            freq = CAC_IWDT_FREQ;
            break;

        default:                 // External is always unknown
            return (FREQ_GET_ERROR_OR_EXT_CLOCK);
    }

    return (freq >> (uint32_t)divisor);
}
#endif

/*******************************************************************************************************************//**
 * Configure the CAC per the user's configuration
 * @param[in]  p_ctrl - pointer to control structure
 * @param[in]  p_cfg - pointer to the CAC configuration settings.
 **********************************************************************************************************************/
static void cac_setup (cac_instance_ctrl_t * const p_ctrl, cac_cfg_t const * const p_cfg)
{
    R_CAC_Type * p_cac_reg = (R_CAC_Type *) p_ctrl->p_reg;

    /** Disable interrupt in ICU*/
    cac_disable_nvic_interrupts(p_ctrl);

    p_cac_reg->CACR0_b.CFME    = 0U;             // Stop measuring
    p_cac_reg->CAICR_b.MENDFCL = 1U;             // Clear any existing interrupt condition.
    p_cac_reg->CAICR_b.OVFFCL  = 1U;
    p_cac_reg->CAICR_b.FERRFCL = 1U;

    p_cac_reg->CACR1_b.FMCS    = p_cfg->cac_meas_clock.clock;    // count clock source for measurement circuit (the signal
                                                             // being measured)
    p_cac_reg->CACR1_b.EDGES   = p_cfg->cac_ref_clock.edge;      // edge selection
    p_cac_reg->CACR1_b.TCSS    = p_cfg->cac_meas_clock.divider;  // Timer count clock select

    p_cac_reg->CACR2_b.DFS     = p_cfg->cac_ref_clock.digfilter; // Digital filtering
    p_cac_reg->CACR2_b.RCDS    = p_cfg->cac_ref_clock.divider;   // Reference signal generation clock frequency division
                                                             // ratio = 1/32

    if (p_cfg->cac_ref_clock.clock == CAC_CLOCK_SOURCE_EXTERNAL)
    {
        /* There is no RSCS setting for External reference clock. That is controlled by the RPS bit. We want to
         * insure that there is a valid setting for these bits so we'll write the value after Reset (0) which is the
         * Main clock oscillator.
         */
        p_cac_reg->CACR2_b.RSCS    = CAC_CLOCK_SOURCE_MAIN_OSC;
        p_cac_reg->CACR1_b.CACREFE = 1U;        // CACREF pin is enabled
        p_cac_reg->CACR2_b.RPS     = 0U;        // Externally supplied reference signal
    }
    else
    {
        p_cac_reg->CACR2_b.RSCS    = p_cfg->cac_ref_clock.clock;     // Specify the reference clock source
        p_cac_reg->CACR1_b.CACREFE = 0U;                 // CACREF pin is disabled
        p_cac_reg->CACR2_b.RPS     = 1U;                 // Internally generated signal
    }

    // Configure the CAC interrupts
    cac_irq_cfg(p_ctrl, true, p_cfg);

    p_cac_reg->CAULVR = p_cfg->cac_upper_limit;
    p_cac_reg->CALLVR = p_cfg->cac_lower_limit;
}

/*******************************************************************************************************************//**
 * Disable the CAC interrupts in the NVIC
 * @param[in]  p_ctrl Pointer to control structure
 * @return     None
 **********************************************************************************************************************/
static void cac_disable_nvic_interrupts (cac_instance_ctrl_t * const p_ctrl)
{
    if (SSP_INVALID_VECTOR != p_ctrl->frequency_error_irq)
    {
        NVIC_DisableIRQ(p_ctrl->frequency_error_irq);
    }
    if (SSP_INVALID_VECTOR != p_ctrl->measurement_end_irq)
    {
        NVIC_DisableIRQ(p_ctrl->measurement_end_irq);
    }
    if (SSP_INVALID_VECTOR != p_ctrl->overflow_irq)
    {
        NVIC_DisableIRQ(p_ctrl->overflow_irq);
    }
}

/*******************************************************************************************************************//**
 * @brief   This function gets the interrupt vector indexes from the FMI for the CAC interrupts.
 * @param[in]  p_ctrl       - CAC instance control block
 * @param[in]  p_ssp - pointer to ssp feature information
 *
 * @retval SSP_SUCCESS               FMI based setup success.
 * @retval SSP_ERR_ASSERTION         Problem getting FMI information.
 *
 *
 **********************************************************************************************************************/
static ssp_err_t cac_fmi_setup (cac_instance_ctrl_t * const p_ctrl, ssp_feature_t *p_ssp)
{
    ssp_err_t err = SSP_SUCCESS;
    fmi_event_info_t event_info = {(IRQn_Type) 0};

    p_ssp->channel = 0U;
    p_ssp->unit = 0U;
    p_ssp->id = SSP_IP_CAC;
    fmi_feature_info_t info = {0};
    err = g_fmi_on_fmi.productFeatureGet(p_ssp, &info);
    CAC_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = info.ptr;


    g_fmi_on_fmi.eventInfoGet(p_ssp, SSP_SIGNAL_CAC_FREQUENCY_ERROR, &event_info);
    p_ctrl->frequency_error_irq = event_info.irq;
    g_fmi_on_fmi.eventInfoGet(p_ssp, SSP_SIGNAL_CAC_MEASUREMENT_END, &event_info);
    p_ctrl->measurement_end_irq = event_info.irq;
    g_fmi_on_fmi.eventInfoGet(p_ssp, SSP_SIGNAL_CAC_OVERFLOW, &event_info);
    p_ctrl->overflow_irq = event_info.irq;

    return(err);
}

/*******************************************************************************************************************//**
 * @brief   This function verfies that there is a callback function specified if one or more interrupts are enabled
 *           and sets the vector table entries and priority for enabled CAC interrupts.
 * @param[in]  p_ctrl       - CAC instance control block
 * @param[in]  p_cfg - pointer to the CAC configuration settings.
 *
 * @retval SSP_SUCCESS               Enabled vectors and priorities configured.
 * @retval SSP_ERR_INVALID_POINTER   Interrupt specified with NULL callback.
 *
 *
 **********************************************************************************************************************/
static ssp_err_t cac_setup_vectors (cac_instance_ctrl_t * const p_ctrl, cac_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;
    ssp_vector_info_t * p_vector_info;


    if ((SSP_INVALID_VECTOR != p_ctrl->frequency_error_irq) && (p_cfg->ferr_interrupt_enabled == true))
    {
        if (NULL == p_cfg->p_callback)
        {
            return SSP_ERR_INVALID_POINTER;
        }
        R_SSP_VectorInfoGet(p_ctrl->frequency_error_irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->frequency_error_irq, p_cfg->frequency_error_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }

    if ((SSP_INVALID_VECTOR != p_ctrl->measurement_end_irq) && (p_cfg->mei_interrupt_enabled == true))
    {
        if (NULL == p_cfg->p_callback)
        {
            return SSP_ERR_INVALID_POINTER;
        }
        R_SSP_VectorInfoGet(p_ctrl->measurement_end_irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->measurement_end_irq, p_cfg->measurement_end_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }
    if ((SSP_INVALID_VECTOR != p_ctrl->overflow_irq) && (p_cfg->ovf_interrupt_enabled == true))
    {
        if (NULL == p_cfg->p_callback)
        {
            return SSP_ERR_INVALID_POINTER;
        }
        R_SSP_VectorInfoGet(p_ctrl->overflow_irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->overflow_irq, p_cfg->overflow_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }
    return(err);
}

/*******************************************************************************************************************//**
 * @brief   This function enables or disables the CAC interrupts. There are 3 possible CAC interrupts, each of
 *          which the user may choose to enable or disable.
 *          These are the Frequency Error interrupt, the Measurement Complete interrupt and the Overflow
 *          interrupt. If the caller has provided a callback
 *          function as part of the provided p_cfg pointer, then that function will be called as a result of the
 *          interrupt.
 * @param[in]  p_ctrl       CAC instance control block
 * @param[in]  state        true ==> enable interrupts, false ==> disable interrupts.
 * @param[in]  p_cfg        Pointer to the CAC configuration structure.
 **********************************************************************************************************************/
static void cac_irq_cfg (cac_instance_ctrl_t * const p_ctrl, bool state, cac_cfg_t const * const p_cfg)
{
    R_CAC_Type * p_cac_reg = (R_CAC_Type *) p_ctrl->p_reg;

    /** Regardless of whether we will enable or disable, Clear the Interrupt Request bits */
    if (SSP_INVALID_VECTOR != p_ctrl->frequency_error_irq)
    {
        R_BSP_IrqStatusClear(p_ctrl->frequency_error_irq);
    }
    if (SSP_INVALID_VECTOR != p_ctrl->measurement_end_irq)
    {
        R_BSP_IrqStatusClear(p_ctrl->measurement_end_irq);
    }
    if (SSP_INVALID_VECTOR != p_ctrl->overflow_irq)
    {
        R_BSP_IrqStatusClear(p_ctrl->overflow_irq);
    }

    /** Enable the Interrupts if requested */
    if (true == state)
    {
        /** Assign the callback*/
    	cac_enable_interrupts(p_ctrl, p_cfg);
    }
    else
    {
        /** Disable interrupt in NVIC */
        cac_disable_nvic_interrupts(p_ctrl);
        HW_CAC_DisableCACInterrupts(p_cac_reg);  ///< Disable interrupts in the peripheral
    }

}

/*******************************************************************************************************************//**
 * @brief   This function enables the CAC interrupts enabled in the supplied configuration.
 * @param[in]  p_ctrl       CAC instance control block
 * @param[in]  p_cfg        Pointer to the CAC configuration structure.
 *
 **********************************************************************************************************************/
static void cac_enable_interrupts (cac_instance_ctrl_t * const p_ctrl, cac_cfg_t const * const p_cfg)
{
    R_CAC_Type * p_cac_reg = (R_CAC_Type *) p_ctrl->p_reg;
    p_ctrl->cac_continous_mode = p_cfg->continuous_mode;


    HW_CAC_DisableCACInterrupts(p_cac_reg);  ///< Disable interrupts in the peripheral

    if ((SSP_INVALID_VECTOR != p_ctrl->frequency_error_irq) && (p_cfg->ferr_interrupt_enabled == true))
    {
        /** Enable/Disable CAC interrupts as requested by the user. */
        NVIC_ClearPendingIRQ(p_ctrl->frequency_error_irq);
        NVIC_EnableIRQ(p_ctrl->frequency_error_irq);
        p_cac_reg->CAICR_b.FERRIE = 1U;            // freq error interrupt enabled
    }

    if ((SSP_INVALID_VECTOR != p_ctrl->measurement_end_irq) && (p_cfg->mei_interrupt_enabled == true))
    {
        NVIC_ClearPendingIRQ(p_ctrl->measurement_end_irq);
        NVIC_EnableIRQ(p_ctrl->measurement_end_irq);
        p_cac_reg->CAICR_b.MENDIE = 1U;            // measurement complete interrupt enabled
    }

    if ((SSP_INVALID_VECTOR != p_ctrl->overflow_irq) && (p_cfg->ovf_interrupt_enabled == true))
    {
        NVIC_ClearPendingIRQ(p_ctrl->overflow_irq);
        NVIC_EnableIRQ(p_ctrl->overflow_irq);
        p_cac_reg->CAICR_b.OVFIE = 1U;            // overflow error interrupt enabled
    }
}

/*******************************************************************************************************************//**
 * @brief  CAC Frequency error interrupt routine.
 *
 * @details This function implements the CAC Frequency error isr. The function clears the interrupt request source on
 *entry
 *          populates the callback structure with the relevant event, and providing
 *          a callback routine has been provided, calls the callback function with the event. <br>
 *
 * @retval        none
 **********************************************************************************************************************/
void cac_frequency_error_isr (void)
{
    SF_CONTEXT_SAVE
    cac_callback_args_t cb_data;

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    cac_instance_ctrl_t * p_ctrl = (cac_instance_ctrl_t *) *(p_vector_info->pp_ctrl);
    R_CAC_Type * p_cac_reg = (R_CAC_Type *) p_ctrl->p_reg;

    /** The CAC interrupts are level based which means the bits in the peripheral must be cleared before clearing
     *  the IR bit in the IELSRn register. */
    p_cac_reg->CAICR_b.FERRFCL = 1U;                /** Clear the interrupt condition in the CAC peripheral */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());    /** Clear the IR bit in the IELSRn register */

    cb_data.event = CAC_EVENT_FREQUENCY_ERROR;            // Pass result back to callback

    if (NULL != p_ctrl->p_callback)
    {
        /** Set data to identify callback to user, then call user callback. */
        cb_data.p_context = p_ctrl->p_context;
        p_ctrl->p_callback(&cb_data);
    }

    if (p_ctrl->cac_continous_mode == false)
    {
        HW_CAC_StopMeasuring(p_cac_reg);                // Stop measuring
    }

    SF_CONTEXT_RESTORE
}  /* End of function cac_frequency_error_isr() */

/*******************************************************************************************************************//**
 * @brief  CAC Overflow interrupt routine.
 *
 * @details This function implements the CAC overflow isr. The function clears the interrupt request source on entry
 *          populates the callback structure with the relevant event, and providing
 *          a callback routine has been provided, calls the callback function with the event. <br>
 *
 * @retval        none
 **********************************************************************************************************************/
void cac_overflow_isr (void)
{
    SF_CONTEXT_SAVE
    cac_callback_args_t cb_data;

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    cac_instance_ctrl_t * p_ctrl = (cac_instance_ctrl_t *) *(p_vector_info->pp_ctrl);
    R_CAC_Type * p_cac_reg = (R_CAC_Type *) p_ctrl->p_reg;

    /** The CAC interrupts are level based which means the bits in the peripheral must be cleared before clearing
     *  the IR bit in the IELSRn register. */
    p_cac_reg->CAICR_b.OVFFCL = 1U;                /** Clear the interrupt condition in the CAC peripheral */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());   /** Clear the IR bit in the IELSRn register */

    cb_data.event = CAC_EVENT_COUNTER_OVERFLOW;            // Pass result back to callback

    if (NULL != p_ctrl->p_callback)
    {
        /** Set data to identify callback to user, then call user callback. */
        cb_data.p_context = p_ctrl->p_context;
        p_ctrl->p_callback(&cb_data);
    }

    if (p_ctrl->cac_continous_mode == false)
    {
        HW_CAC_StopMeasuring(p_cac_reg);                // Stop measuring
    }

    SF_CONTEXT_RESTORE
}  /* End of function cac_overflow_isr() */

/*******************************************************************************************************************//**
 * @brief  CAC Measurement Complete interrupt routine.
 *
 * @details This function implements the CAC measurement complete isr. The function clears the interrupt request source
 *          populates the callback structure with the relevant event, and providing
 *          a callback routine has been provided, calls the callback function with the event. <br>
 *
 * @retval        none
 **********************************************************************************************************************/
void cac_measurement_end_isr (void)
{
    SF_CONTEXT_SAVE
    cac_callback_args_t cb_data;

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    cac_instance_ctrl_t * p_ctrl = (cac_instance_ctrl_t *) *(p_vector_info->pp_ctrl);
    R_CAC_Type * p_cac_reg = (R_CAC_Type *) p_ctrl->p_reg;

    /** The CAC interrupts are level based which means the bits in the peripheral must be cleared before clearing
     *  the IR bit in the IELSRn register. */
    p_cac_reg->CAICR_b.MENDFCL = 1U;                /** Clear the interrupt condition in the CAC peripheral */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());    /** Clear the IR bit in the IELSRn register */

    cb_data.event = CAC_EVENT_MEASUREMENT_COMPLETE;            // Pass result back to callback

    if (NULL != p_ctrl->p_callback)
    {
        /** Set data to identify callback to user, then call user callback. */
        cb_data.p_context = p_ctrl->p_context;
        p_ctrl->p_callback(&cb_data);
   }

    if (p_ctrl->cac_continous_mode == false)
    {
        HW_CAC_StopMeasuring(p_cac_reg);                // Stop measuring
    }

    SF_CONTEXT_RESTORE
}  /* End of function cac_overflow_isr() */

#if (CAC_CFG_PARAM_CHECKING_ENABLE)
/*******************************************************************************************************************//**
 * @brief   This function performs the parameter checking required by the R_CAC_Open() function.
 * @param[in]  p_ctrl       - CAC instance control block
 * @param[in]  cac_cfg_t * const  p_cfg - pointer to the CAC configuration settings.
 *
 * @retval SSP_SUCCESS               Parameter checking completed without error.
 **********************************************************************************************************************/
static ssp_err_t cac_parameter_checking (cac_instance_ctrl_t * const p_ctrl, cac_cfg_t const * const p_cfg)
{
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_ctrl);
    CAC_ERROR_RETURN(!(p_cfg->cac_meas_clock.clock == p_cfg->cac_ref_clock.clock), SSP_ERR_INVALID_ARGUMENT);

    /** Ensure that upper limit is not less than the lower limit. If they are both the same (ie. 0) we will allow
     * measurement to proceed. This can
     * be useful for setting up the clocks and monitoring what the counter value actually is. */
    CAC_ERROR_RETURN(!(p_cfg->cac_upper_limit < p_cfg->cac_lower_limit), SSP_ERR_INVALID_ARGUMENT);

    /** Is Measure clock rate >= Reference clock rate? */
    if ((p_cfg->cac_ref_clock.clock != CAC_CLOCK_SOURCE_EXTERNAL))
    {
        CAC_ERROR_RETURN(!((cac_get_effective_frequency(p_cfg, CAC_CLOCK_MEASURED)) < (cac_get_effective_frequency(p_cfg, CAC_CLOCK_REFERENCE))), SSP_ERR_INVALID_CAC_REF_CLOCK);
    }

	return(SSP_SUCCESS);
}
#endif
