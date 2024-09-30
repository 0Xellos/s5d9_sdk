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
/***********************************************************************************************************************
* File Name    : r_analog_connect.c
* Description  : Primary source code for Low Power Analog Comparator
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes   <System Includes> , "Project Includes"
 **********************************************************************************************************************/

#include "bsp_api.h"
#include "r_analog_connect_cfg.h"
#include "r_analog_connect.h"

#include "hw/hw_analog_connect_private.h"
#include "r_analog_connect_private_api.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define ANALOG_CONNECT_PRIV_ACMPHS_200_NS_LOOPS        (12U)

#define ANALOG_CONNECT_PRIV_ACMPLP_SET_OR_CLEAR_BIT_7  (0x88U)

#define ANALOG_CONNECT_PRIV_OPAMP_CHARGE_PUMP_DELAY_US (100U)

/** Macro for error logger. */
#ifndef ANALOG_CONNECT_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define ANALOG_CONNECT_ERROR_RETURN(a, err)  SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_analog_connect_version)
#endif

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
typedef union u_analog_connect_priv_data
{
    uint32_t word;

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    struct
    {
        uint32_t                      option       :8;  // Module specific option
        uint32_t                      connection   :8;  // Module specific connection
        analog_connect_priv_channel_t channel      :4;  // Channel number

        /* Module specific flag:
         * OPAMP:  If set, break other connections not being set in this connection.
         * ACMPHS: If set, enable the internal reference voltage
         * ACMPLP: If set, modify upper 4 bits of 8 bit register.  If cleared, modify lower 4 bits.
         */
        uint32_t                      flag         :1;
        uint32_t                                   :3;  // Reserved for future use
        analog_connect_priv_module_t  module       :4;  // Module
        uint32_t                                   :4;  // Reserved for future use
    } bit;
} analog_connect_priv_data_t;

typedef union u_analog_connect_priv_acmplp_option
{
    uint8_t byte;

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    struct
    {
        uint8_t                       clear_mask   :4;  // Mask of bits to clear
        uint8_t                       set_mask     :4;  // Mask of bits to set
    } bit;
} analog_connect_priv_acmplp_option_t;

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
void r_analog_connect_module_specific(ssp_feature_t * const p_feature, void * p_base, analog_connect_t connection);
void r_analog_connect_acmphs(ssp_feature_t * const p_feature, void * p_reg, analog_connect_t connection);
void r_analog_connect_acmplp(void * p_reg, analog_connect_t connection);
void r_analog_connect_opamp(void * p_reg, analog_connect_t connection);

/** Version data structure used by error logger macro. */
static const ssp_version_t g_analog_connect_version =
{
    .api_version_minor  = ANALOG_CONNECT_API_VERSION_MINOR,
    .api_version_major  = ANALOG_CONNECT_API_VERSION_MAJOR,
    .code_version_major = ANALOG_CONNECT_CODE_VERSION_MAJOR,
    .code_version_minor = ANALOG_CONNECT_CODE_VERSION_MINOR
};

static const ssp_ip_t g_analog_connect_priv_ip_lookup[] =
{
    [ANALOG_CONNECT_PRIV_MODULE_ACMPHS] = SSP_IP_COMP_HS,
    [ANALOG_CONNECT_PRIV_MODULE_ACMPLP] = SSP_IP_COMP_LP,
    [ANALOG_CONNECT_PRIV_MODULE_OPAMP]  = SSP_IP_OPAMP,
};

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "analog_connect";
#endif

/***********************************************************************************************************************
 Global Variables
 **********************************************************************************************************************/

/** ANALOG_CONNECT Implementation of analog_connect interface. */
/*LDRA_NOANALYSIS LDRA_INSPECTED below not working. */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const analog_connect_api_t g_analog_connect_on_analog_connect =
{
    .init                   = R_ANALOG_CONNECT_Init,
    .connect                = R_ANALOG_CONNECT_Connect,
    .connectMultiple        = R_ANALOG_CONNECT_ConnectMultiple,
    .versionGet             = R_ANALOG_CONNECT_VersionGet
};
/*LDRA_ANALYSIS */

/*******************************************************************************************************************//**
 * @addtogroup ANALOG_CONNECT
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Placeholder function for analog connection initialization code.  Currently unused.
 *
 * @retval SSP_SUCCESS                    Init successful.
 **********************************************************************************************************************/
ssp_err_t R_ANALOG_CONNECT_Init (analog_connect_cfg_t const * const p_cfg)
{
    SSP_PARAMETER_NOT_USED(p_cfg);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Makes an internal analog connection.
 *
 * For ACMPHS connections, output and interrupts are disabled while the connection is configured, then the output and
 * interrupts are restored to their original state.  Since this function enables output if it was enabled before,
 * R_ACMPHS_Close() should not be called during this function.
 *
 * ACMPLP connections must be made prior to enabling output.
 *
 * OPAMP connections can be reconfigured while the OPAMP is operating.  If an OPAMP connection is already set and
 * a new connection is made, the new connection overwrites the existing connection.  OPAMP connections can be OR'd
 * together if they are for the same signal:
 *    - Valid example:   ANALOG_CONNECT_OPAMP0_PLUS_TO_PIN_AMP0_PLUS | ANALOG_CONNECT_OPAMP0_PLUS_TO_PIN_AMP1_PLUS
 *      - Both start with ANALOG_CONNECT_OPAMP0_PLUS
 *    - Invalid example: ANALOG_CONNECT_OPAMP0_PLUS_TO_PIN_AMP0_PLUS | ANALOG_CONNECT_OPAMP0_MINUS_TO_PIN_AMP0_MINUS
 *      - Do not mix PLUS and MINUS
 *    - Invalid example: ANALOG_CONNECT_OPAMP0_PLUS_TO_PIN_AMP0_PLUS | ANALOG_CONNECT_OPAMP1_PLUS_TO_PIN_AMP1_PLUS
 *      - Do not mix channels 0 and 1
 *
 * If AVCC0 < 2.7 V, MOCO must be enabled to make OPAMP connections because the internal OPAMP switches require a
 * charge pump, and the MOCO is required for charge pump operation. When using the charge pump for the amplifier:
 *    * Turn on no more than a total of 5 connections for OPAMP0.
 *    * Turn on no more than a total of 5 connections for OPAMP1.
 *    * Turn on no more than a total of 2 connections for OPAMP2.
 *
 * @retval  SSP_SUCCESS                Connection made.
 * @return                             See @ref Common_Error_Codes or functions called by this function for other possible
 *                                     return codes. This function calls:
 *                                       * fmi_api_t::productFeatureGet
***********************************************************************************************************************/
ssp_err_t R_ANALOG_CONNECT_Connect(analog_connect_t const connection)
{
    analog_connect_priv_data_t data = {0U};
    data.word = (uint32_t) connection;

    ssp_feature_t feature = {{(ssp_ip_t) 0U}};
    feature.id = g_analog_connect_priv_ip_lookup[data.bit.module];
    feature.channel = (uint32_t) data.bit.channel;
    feature.unit = 0U;

    fmi_feature_info_t info = {0};
    ssp_err_t err = g_fmi_on_fmi.productFeatureGet(&feature, &info);
    ANALOG_CONNECT_ERROR_RETURN(SSP_SUCCESS == err, err);

    R_BSP_ModuleStart(&feature);

    r_analog_connect_module_specific(&feature, info.ptr, connection);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Makes all connections in the table.
 *
 * See R_ANALOG_CONNECT_Connect() for modules specific usage notes regarding connections.
 *
 * @retval  SSP_SUCCESS                All connections made.
 * @retval  SSP_ERR_ASSERTION          Data table pointer is NULL or size is 0.
 * @return                             See @ref Common_Error_Codes or functions called by this function for other possible
 *                                     return codes. This function calls:
 *                                       * fmi_api_t::productFeatureGet
***********************************************************************************************************************/
ssp_err_t R_ANALOG_CONNECT_ConnectMultiple(analog_connect_table_t const * const p_table)
{
#if (1 == ANALOG_CONNECT_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointers are not NULL and ensure the ANALOG_CONNECT is already open. */
    SSP_ASSERT(NULL != p_table);
    SSP_ASSERT(NULL != p_table->p_connection_table);
    SSP_ASSERT(0U != p_table->number_of_connections);
#endif

    ssp_feature_t previous_feature = {{(ssp_ip_t) 0U}};
    void * p_reg = NULL;

    for (uint32_t i = 0U; i < p_table->number_of_connections; i++)
    {
        analog_connect_priv_data_t data = {0U};

        analog_connect_t connection = p_table->p_connection_table[i];
        data.word = (uint32_t) connection;

        ssp_feature_t feature = {{(ssp_ip_t) 0U}};
        feature.id = g_analog_connect_priv_ip_lookup[data.bit.module];
        feature.channel = (uint32_t) data.bit.channel;
        feature.unit = 0U;
        if (previous_feature.word != feature.word)
        {
            fmi_feature_info_t info = {0};
            ssp_err_t err = g_fmi_on_fmi.productFeatureGet(&feature, &info);
            ANALOG_CONNECT_ERROR_RETURN(SSP_SUCCESS == err, err);

            R_BSP_ModuleStart(&feature);

            p_reg = info.ptr;

            previous_feature.word = feature.word;
        }

        r_analog_connect_module_specific(&feature, p_reg, connection);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Gets the API and code version. Implements analog_connect_api_t::versionGet().
 *
 * @retval  SSP_SUCCESS        Version information available in p_version.
 * @retval  SSP_ERR_ASSERTION  The parameter p_version is NULL.
***********************************************************************************************************************/
ssp_err_t R_ANALOG_CONNECT_VersionGet(ssp_version_t * const p_version)
{
#if (1 == ANALOG_CONNECT_CFG_PARAM_CHECKING_ENABLE)
    /* Verify the pointer is not NULL. */
    SSP_ASSERT(NULL != p_version);
#endif

    /** Return the version number */
    p_version->version_id =  g_analog_connect_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup ANALOG_CONNECT)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 Private Functions
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * Routes to module specific connect function.
 *
 * @param[in]   p_feature    Defines which module and channel the connection is for.
 * @param[in]   p_reg        Pointer to base register.
 * @param[in]   connection   Which connection to make.
 **********************************************************************************************************************/
void r_analog_connect_module_specific(ssp_feature_t * const p_feature, void * p_reg, analog_connect_t connection)
{
    analog_connect_priv_data_t data = {0U};
    data.word = (uint32_t) connection;

    switch (data.bit.module)
    {
        case ANALOG_CONNECT_PRIV_MODULE_ACMPHS:
        {
            r_analog_connect_acmphs(p_feature, p_reg, connection);
            break;
        }
        case ANALOG_CONNECT_PRIV_MODULE_ACMPLP:
        {
            r_analog_connect_acmplp(p_reg, connection);
            break;
        }
        case ANALOG_CONNECT_PRIV_MODULE_OPAMP:
        {
            r_analog_connect_opamp(p_reg, connection);
            break;
        }
        default:
        {
            /* Do nothing.  Execution will never get here if a valid enum is used. */
            break;
        }
    }
}

/*******************************************************************************************************************//**
 * Makes a connection for ACMPHS.
 *
 * @param[in]   p_feature    Defines which module and channel the connection is for.
 * @param[in]   p_reg        Pointer to base register.
 * @param[in]   connection   Which connection to make.
 **********************************************************************************************************************/
void r_analog_connect_acmphs(ssp_feature_t * const p_feature, void * p_reg, analog_connect_t connection)
{
    analog_connect_priv_data_t data = {0U};
    data.word = connection;

    uint8_t * p_acmphs_reg = (uint8_t *) p_reg;

    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_COMP_HS_INT, &event_info);

    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    /** Disable comparator output and interrupt and store the previous setting. */
    uint32_t irq_previously_enabled = 0U;
    if (SSP_INVALID_VECTOR != event_info.irq)
    {
        irq_previously_enabled = NVIC_GetEnableIRQ(event_info.irq);
        NVIC_DisableIRQ(event_info.irq);
    }
    uint8_t previously_enabled = HW_ANALOG_CONNECT_AcmphsOutputDisable(p_acmphs_reg);

    /** Make the connection. */
    HW_ANALOG_CONNECT_AcmphsConnectionSet(p_acmphs_reg, data.bit.connection, (uint8_t) data.bit.option);

    /** If the flag is set, enable the internal reference voltage. */
    if (data.bit.flag)
    {
        HW_ANALOG_CONNECT_AcmphsIvrefEnable();
    }

    SSP_CRITICAL_SECTION_EXIT;

    /** If output was previously enabled, wait at least 200 ns then re-enable the output. */
    if (previously_enabled)
    {
        /* The hardware manual requires an input switching stabilization wait time of 200 ns.
         *
         * The fastest ICLK on Synergy parts is 240 MHz.  48 cycles at 240 MHz = 200 ns. Busy loops take 4 cycles per
         * iteration, so a busy loop of 12 is used to ensure the 200 ns delay requirement is met. This busy loop is
         * used instead of R_BSP_SoftwareDelay because R_BSP_SoftwareDelay does not offer nanosecond resolution and
         * the overhead would be significant. */
        volatile uint32_t busy_loop = ANALOG_CONNECT_PRIV_ACMPHS_200_NS_LOOPS;
        while (busy_loop > 0U)
        {
            /* Just decrement the loop counter. */
            busy_loop--;
        }

        /* Enable output. */
        HW_ANALOG_CONNECT_AcmphsOutputEnable(p_acmphs_reg);

        /* Clear any pending interrupt that occurred before stabilization was completed then enable the interrupt. */
        if ((SSP_INVALID_VECTOR != event_info.irq) && (irq_previously_enabled))
        {
            R_BSP_IrqStatusClear(event_info.irq);
            NVIC_ClearPendingIRQ(event_info.irq);
            NVIC_EnableIRQ(event_info.irq);
        }
    }
}

/*******************************************************************************************************************//**
 * Makes a connection for ACMPLP.
 *
 * @param[in]   p_reg        Pointer to base register.
 * @param[in]   connection   Which connection to make.
 **********************************************************************************************************************/
void r_analog_connect_acmplp(void * p_reg, analog_connect_t connection)
{
    analog_connect_priv_data_t data = {0U};
    data.word = (uint32_t) connection;

    uint8_t * p_acmplp_reg = (uint8_t *) p_reg;

    analog_connect_priv_acmplp_option_t option = {0U};
    option.byte = (uint8_t) data.bit.option;

    uint8_t set_mask = option.bit.set_mask;
    uint8_t clear_mask = option.bit.clear_mask;

    /** The flag means the masks apply to the upper 4 bits. If the flag is set, shift the masks to the upper 4 bits. */
    if (data.bit.flag)
    {
        set_mask = (uint8_t) (set_mask << 4U);
        clear_mask = (uint8_t) (clear_mask << 4U);
    }

    /** Make the connection. */
    HW_ANALOG_CONNECT_AcmplpConnectionSet(p_acmplp_reg, data.bit.connection, set_mask, clear_mask);

    /** If the connection affects COMPSEL1.C1VRF2, COMPMDR.C1VRF must be cleared as well. */
    if (((ANALOG_CONNECT_PRIV_REG_ACMPLP_COMPSEL1 == data.bit.connection) && (data.bit.flag)) && \
            (0U != (ANALOG_CONNECT_PRIV_ACMPLP_SET_OR_CLEAR_BIT_7 & data.bit.option)))
    {
        r_analog_connect_acmplp(p_reg, (analog_connect_t) (ANALOG_CONNECT_DEFINE(ACMPLP, 0, COMPMDR, CLEAR_C1VRF, FLAG_CLEAR)));
    }
}

/*******************************************************************************************************************//**
 * Makes a connection for OPAMP.
 *
 * @param[in]   p_reg        Pointer to base register.
 * @param[in]   connection   Which connection to make.
 **********************************************************************************************************************/
void r_analog_connect_opamp(void * p_reg, analog_connect_t connection)
{
    analog_connect_priv_data_t data = {0U};
    data.word = (uint32_t) connection;

    uint8_t * p_opamp_reg = (uint8_t *) p_reg;

#if BSP_CFG_MCU_AVCC0_MV < 2700
    /** If AVCC0 < 2.7 V and the charge pump is not enabled, enable the charge pump and wait 100 us for the charge pump
     * to stabilize as required by the hardware manual. */
    if (!HW_ANALOG_CONNECT_OpampChargePumpIsEnabled(data.bit.connection, p_reg))
    {
        HW_ANALOG_CONNECT_OpampChargePumpEnable(data.bit.connection, p_reg);
        R_BSP_SoftwareDelay(ANALOG_CONNECT_PRIV_OPAMP_CHARGE_PUMP_DELAY_US, BSP_DELAY_UNITS_MICROSECONDS);
    }
#endif

    /** Make the connection. */
    HW_ANALOG_CONNECT_OpampConnectionSet(p_opamp_reg, data.bit.connection, (uint8_t) data.bit.option);
}
