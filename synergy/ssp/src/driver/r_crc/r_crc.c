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
 * File Name    : r_crc.c
 * Description  : HAL API code for the CRC module
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_crc.h"
#include "r_crc_private.h"
#include "r_crc_private_api.h"
#include <string.h>

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/** Maximum number of channels supported by CRC */
#define CRC_SNOOP_MAX_CHANNEL (10UL)

/** "CRC" in ASCII, used to determine if channel is open. */
#define CRC_OPEN              (0x00435243ULL)

/** Macro for error logger. */
#ifndef CRC_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define CRC_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &s_crc_version)
#endif

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
ssp_err_t calculate_polynomial (crc_ctrl_t * const p_api_ctrl, void * inputBuffer, uint32_t length,
                                uint32_t crc_seed, uint32_t * calculatedValue);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
#if defined(__GNUC__)
/* This structure is affected by warnings from the GCC compiler bug https:/gcc.gnu.org/bugzilla/show_bug.cgi?id=60784
 * This pragma suppresses the warnings in this structure only, and will be removed when the SSP compiler is updated to
 * v5.3.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t s_crc_version =
{
	.api_version_minor  = CRC_API_VERSION_MINOR,
	.api_version_major  = CRC_API_VERSION_MAJOR,
    .code_version_major = CRC_CODE_VERSION_MAJOR,
    .code_version_minor = CRC_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char          g_module_name[] = "crc";
#endif

/** Filled in Interface API structure for this Instance. */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const crc_api_t g_crc_on_crc = 
{
    .open           = R_CRC_Open,
    .close          = R_CRC_Close,
    .crcResultGet   = R_CRC_CalculatedValueGet,
    .snoopEnable    = R_CRC_SnoopEnable,
    .snoopDisable   = R_CRC_SnoopDisable,
    .snoopCfg       = R_CRC_SnoopCfg,
    .calculate      = R_CRC_Calculate,
    .versionGet     = R_CRC_VersionGet
};

/** @addtogroup CRC
 * @{
 */

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Open the CRC driver module
 *
 *  Implements crc_api_t::open
 *
 * Open the CRC driver module and initialize the driver control block according to the passed-in
 * configuration structure.
 *
 * @retval SSP_SUCCESS             Configuration was successful.
 * @retval SSP_ERR_ASSERTION       p_ctrl or p_cfg is NULL.
 * @return                         See @ref Common_Error_Codes or functions called by this function for other possible
 *                                 return codes. This function calls:
 *                                   * fmi_api_t::productFeatureGet
 **********************************************************************************************************************/
ssp_err_t R_CRC_Open (crc_ctrl_t * const p_api_ctrl, crc_cfg_t const * const p_cfg)
{
    crc_instance_ctrl_t * p_ctrl = (crc_instance_ctrl_t *) p_api_ctrl;

#if CRC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_cfg);
#endif

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_CRC;
    fmi_feature_info_t info = {0U};
    ssp_err_t err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    CRC_ERROR_RETURN(SSP_SUCCESS == err, err);

    p_ctrl->p_reg = (R_CRC_Type *) info.ptr;

    /* Store these configurations for later calculation */
    p_ctrl->bit_order  = p_cfg->bit_order;
    p_ctrl->polynomial = p_cfg->polynomial;
    p_ctrl->fifo_mode  = p_cfg->fifo_mode;

    /** Mark driver as initialized by setting the open value to the ASCII equivalent of "CRC" */
    p_ctrl->open = CRC_OPEN;

    /** Enable clocks to the CRC peripheral. */
    R_BSP_ModuleStart(&ssp_feature);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Close the CRC module driver.
 *
 *  Implements crc_api_t::close
 *
 * @retval SSP_SUCCESS             Configuration was successful.
 * @retval SSP_ERR_ASSERTION       p_ctrl is NULL.
 * @retval SSP_ERR_NOT_OPEN        The driver is not opened.
 *
 **********************************************************************************************************************/
ssp_err_t R_CRC_Close (crc_ctrl_t * const p_api_ctrl)
{
    crc_instance_ctrl_t * p_ctrl = (crc_instance_ctrl_t *) p_api_ctrl;

#if CRC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    CRC_ERROR_RETURN(CRC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** Release the CRC Hardware Resource */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_CRC;

    SSP_PARAMETER_NOT_USED(ssp_feature);

    /** Mark driver as closed */
    p_ctrl->open  =  0U;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Return the current calculated value.
 *
 *  Implements crc_api_t::crcResultGet
 *
 * CRC calculation operates on a running value. This function returns the current calculated value.
 *
 * @retval SSP_SUCCESS             Return of calculated value successful.
 * @retval SSP_ERR_ASSERTION       Either p_ctrl or calculatedValue is NULL.
 * @retval SSP_ERR_NOT_OPEN        The driver is not opened.
 *
 **********************************************************************************************************************/
ssp_err_t R_CRC_CalculatedValueGet (crc_ctrl_t * const p_api_ctrl, uint32_t * calculatedValue)
{
    crc_instance_ctrl_t * p_ctrl = (crc_instance_ctrl_t *) p_api_ctrl;

#if CRC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(calculatedValue);
    CRC_ERROR_RETURN(CRC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** Based on the selected polynomial, return the calculated CRC value */
    switch (p_ctrl->polynomial)
    {
        case CRC_POLYNOMIAL_CRC_8:
            *calculatedValue = (uint32_t) HW_CRC_8bitCalculatedValueGet(p_ctrl->p_reg);
            break;

        case CRC_POLYNOMIAL_CRC_16:
        case CRC_POLYNOMIAL_CRC_CCITT:
            *calculatedValue = (uint32_t) HW_CRC_16bitCalculatedValueGet(p_ctrl->p_reg);
            break;

        default:
            *calculatedValue = HW_CRC_32bitCalculatedValueGet(p_ctrl->p_reg);
            break;

    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Enable snooping.
 *
 *  Implements crc_api_t::snoopEnable
 *
 * @retval SSP_SUCCESS             Snoop enabled.
 * @retval SSP_ERR_ASSERTION       Either p_ctrl or crc_seed is NULL.
 * @retval SSP_ERR_NOT_OPEN        The driver is not opened.
 *
 **********************************************************************************************************************/
ssp_err_t R_CRC_SnoopEnable (crc_ctrl_t * const p_api_ctrl, uint32_t crc_seed)
{
    crc_instance_ctrl_t * p_ctrl = (crc_instance_ctrl_t *) p_api_ctrl;

#if CRC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    CRC_ERROR_RETURN(CRC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** Based on the selected polynomial, set the initial CRC seed value */
    switch (p_ctrl->polynomial)
    {
        case CRC_POLYNOMIAL_CRC_8:
            HW_CRC_8bitCalculatedValueSet(p_ctrl->p_reg, (uint8_t) (0xFFUL & crc_seed));
            break;

        case CRC_POLYNOMIAL_CRC_16:
        case CRC_POLYNOMIAL_CRC_CCITT:
            HW_CRC_16bitCalculatedValueSet(p_ctrl->p_reg, 0xFFFF & crc_seed);
            break;

        default:
            HW_CRC_32bitCalculatedValueSet(p_ctrl->p_reg, crc_seed);
            break;

    }

    /** Enable the snoop operation */
    HW_CRC_SnoopEnable(p_ctrl->p_reg);
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Disable snooping.
 *
 *  Implements crc_api_t::snoopDisable
 *
 * @retval SSP_SUCCESS             Snoop disabled.
 * @retval SSP_ERR_ASSERTION       p_ctrl is NULL.
 * @retval SSP_ERR_NOT_OPEN        The driver is not opened.
 *
 **********************************************************************************************************************/
ssp_err_t R_CRC_SnoopDisable (crc_ctrl_t * const p_api_ctrl)
{
    crc_instance_ctrl_t * p_ctrl = (crc_instance_ctrl_t *) p_api_ctrl;
    
#if CRC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    CRC_ERROR_RETURN(CRC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** Disable the snoop operation */
    HW_CRC_SnoopDisable(p_ctrl->p_reg);
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Configure the snoop channel and direction.
 *
 *  Implements crc_api_t::snoopCfg
 *
 * The CRC calculator can operate on reads and writes over any of the first ten SCI channels.
 * For example, if set to channel 0, transmit, every byte written out SCI channel 0 is also
 * sent to the CRC calculator as if the value was explicitly written directly to the CRC calculator.
 *
 * @retval SSP_SUCCESS             Snoop configured successfully.
 * @retval SSP_ERR_ASSERTION       - This is due to below conditions
 *                                 - Either p_ctrl or p_snoop_cfg is NULL
 *                                 - snoop_channel is greater than or equal to CRC_SNOOP_MAX_CHANNEL.
 * @retval SSP_ERR_NOT_OPEN        The driver is not opened.
 *
 **********************************************************************************************************************/
ssp_err_t R_CRC_SnoopCfg (crc_ctrl_t * const p_api_ctrl, crc_snoop_cfg_t * const p_snoop_cfg)
{
    crc_instance_ctrl_t * p_ctrl = (crc_instance_ctrl_t *) p_api_ctrl;

#if CRC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_snoop_cfg);
    CRC_ERROR_RETURN(CRC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    SSP_ASSERT((CRC_SNOOP_MAX_CHANNEL) > p_snoop_cfg->snoop_channel);
#endif

    /** Set the bit order */
    HW_CRC_BitorderSet(p_ctrl->p_reg, p_ctrl->bit_order);

    /** Set CRC polynomial */
    HW_CRC_PolynomialSet(p_ctrl->p_reg, p_ctrl->polynomial);

    /** Set CRC snoop channel and direction */
    HW_CRC_SnoopChannelSet(p_ctrl->p_reg, p_snoop_cfg->snoop_channel, p_snoop_cfg->snoop_direction, p_ctrl->fifo_mode);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Perform a CRC calculation on a block of 8-bit/32-bit(for 32-bit polynomial) data.
 *
 *  Implements crc_api_t::calculate
 *
 * This function performs a CRC calculation on an array of 8-bit/32-bit(for 32-bit polynomial) values and
 * returns an 8-bit/32-bit(for 32-bit polynomial) calculated value
 *
 * @retval SSP_SUCCESS              Calculation successful.
 * @retval SSP_ERR_ASSERTION        Either p_ctrl, inputBuffer, or calculatedValue is NULL.
 * @retval SSP_ERR_INVALID_ARGUMENT length value is NULL.
 * @retval SSP_ERR_NOT_OPEN        The driver is not opened.
 * @retval SSP_ERR_IN_USE          CRC peripheral is currently in use by another instance of the driver.
 * @return                         See @ref Common_Error_Codes or functions called by this function for other possible
 *                                 return codes. This function calls:
 *                                   * fmi_api_t::productFeatureGet
 **********************************************************************************************************************/
ssp_err_t R_CRC_Calculate (crc_ctrl_t * const p_api_ctrl,
                           void               * inputBuffer,
                           uint32_t           length,
                           uint32_t           crc_seed,
                           uint32_t           * calculatedValue)
{
    crc_instance_ctrl_t * p_ctrl = (crc_instance_ctrl_t *) p_api_ctrl;

#if CRC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    CRC_ERROR_RETURN(CRC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    SSP_ASSERT(inputBuffer);
    SSP_ASSERT(calculatedValue);
#endif

    CRC_ERROR_RETURN((0UL != length), SSP_ERR_INVALID_ARGUMENT);

    /** Lock the peripheral during calculation */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_CRC;
    fmi_feature_info_t info = {0U};
    ssp_err_t err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    CRC_ERROR_RETURN(SSP_SUCCESS == err, err);
    err = R_BSP_HardwareLock(&ssp_feature);
    CRC_ERROR_RETURN((SSP_SUCCESS == err), SSP_ERR_IN_USE);

    /** Set the bit order */
    HW_CRC_BitorderSet(p_ctrl->p_reg, p_ctrl->bit_order);

    /** Set CRC polynomial */
    HW_CRC_PolynomialSet(p_ctrl->p_reg, p_ctrl->polynomial);

    /** Calculate CRC value for the input buffer */
    err = calculate_polynomial(p_ctrl, inputBuffer, length, crc_seed, calculatedValue);

    /** Release the hardware lock */
    R_BSP_HardwareUnlock(&ssp_feature);

    return err;
}


/*******************************************************************************************************************//**
 * @brief      Get the driver version based on compile time macros.
 *
 *   Implements crc_api_t::versionGet
 *
 * @retval     SSP_SUCCESS          Successful close.
 * @retval     SSP_ERR_ASSERTION    p_version is NULL.
 *
 **********************************************************************************************************************/
ssp_err_t R_CRC_VersionGet (ssp_version_t * const p_version)
{
#if CRC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_version);
#endif

    p_version->version_id = s_crc_version.version_id;

    return SSP_SUCCESS;
}
/******************************************************************************************************************//**
 * @} (end defgroup CRC)
**********************************************************************************************************************/


/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Perform a CRC calculation on a block of 8-bit data.
 *
 *  Implements crc_api_t::calculate
 *
 * This function performs a CRC calculation on an array of 8-bit/32-bit(for 32-bit polynomial) values and
 * returns an 8-bit32-bit(for 32-bit polynomial) calculated value.
 *
 * @retval SSP_SUCCESS             Calculation successful.
 *
 **********************************************************************************************************************/
ssp_err_t calculate_polynomial (crc_ctrl_t * const p_api_ctrl,
                                void               * inputBuffer,
                                uint32_t           length,
                                uint32_t           crc_seed,
                                uint32_t           * calculatedValue)
{

    uint32_t i;

    crc_instance_ctrl_t * p_ctrl = (crc_instance_ctrl_t *) p_api_ctrl;

    /* Write each element of the inputBuffer to the CRC Data Input Register. Each write to the
     * Data Input Register generates a new calculated value in the Data Output Register.
     */
    switch (p_ctrl->polynomial)
    {
        case CRC_POLYNOMIAL_CRC_8:
        {
            uint8_t * p_data = (uint8_t *) inputBuffer;

            /* CRC seed is masked to use only the lower 8 bits*/
            HW_CRC_8bitCalculatedValueSet(p_ctrl->p_reg, (uint8_t) (0xFFUL & crc_seed));

            for (i = (uint32_t) 0; i < length; i++)
            {
                HW_CRC_8bitInputWrite(p_ctrl->p_reg, (uint8_t) *p_data);
                p_data = (p_data + 1UL);
            }

            /* Return the calculated value */
            *calculatedValue = (uint32_t) HW_CRC_8bitCalculatedValueGet(p_ctrl->p_reg);
            break;
        }

        case CRC_POLYNOMIAL_CRC_16:
        case CRC_POLYNOMIAL_CRC_CCITT:
        {
            uint8_t * p_data = (uint8_t *) inputBuffer;

            /* CRC seed is masked to use only the lower 16 bits*/
            HW_CRC_16bitCalculatedValueSet(p_ctrl->p_reg, 0xFFFF & crc_seed);

            for (i = (uint32_t) 0; i < length; i++)
            {
                HW_CRC_8bitInputWrite(p_ctrl->p_reg, (uint8_t) *p_data);
                p_data = (p_data + 1UL);
            }

            /* Return the calculated value */
            *calculatedValue = (uint32_t) HW_CRC_16bitCalculatedValueGet(p_ctrl->p_reg);
            break;
        }

        default:
        {
            uint32_t * p_data = (uint32_t *) inputBuffer;

            /* CRC seed uses the 32 bits*/
            HW_CRC_32bitCalculatedValueSet(p_ctrl->p_reg, crc_seed);

            for (i = (uint32_t) 0; i < (length / 4); i++)
            {
                HW_CRC_32bitInputWrite(p_ctrl->p_reg, (uint32_t) *p_data);
                p_data = p_data + 1;
            }

            /* Return the calculated value */
            *calculatedValue = (uint32_t) HW_CRC_32bitCalculatedValueGet(p_ctrl->p_reg);
            break;
        }

    }
    return SSP_SUCCESS;
}
