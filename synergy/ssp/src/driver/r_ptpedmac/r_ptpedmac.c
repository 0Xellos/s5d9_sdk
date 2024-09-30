/***********************************************************************************************************************
 * Copyright [2019-2024] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : r_ptpedmac.c
 * Description  : HAL API for DMA controller for EPTPC driver
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <string.h>
#include "r_ptpedmac.h"
#include "r_ptpedmac_private_api.h"
#include "hw/hw_ptpedmac_private.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* Macro for error logger */
#ifndef PTPEDMAC_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define PTPEDMAC_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_version)
#endif

/** "DMAC" in ASCII.  Used to determine if the control block is open. */
#define PTPEDMAC_OPEN           (0x444D4143U)
/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables and functions
 **********************************************************************************************************************/
/** Version data structure used by error logger macro. */
static ssp_version_t const g_version =
{
    .api_version_major  = PTPEDMAC_API_VERSION_MAJOR,
    .api_version_minor  = PTPEDMAC_API_VERSION_MINOR,
    .code_version_major = PTPEDMAC_CODE_VERSION_MAJOR,
    .code_version_minor = PTPEDMAC_CODE_VERSION_MINOR
};

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "r_ptpedmac";
#endif

#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/***********************************************************************************************************************
 * Private Global Function Prototypes
 **********************************************************************************************************************/
static void ptpedmac_init_descriptors(ptpedmac_instance_ctrl_t * const p_ctrl);
static void ptpedmac_config_ethernet(ptpedmac_instance_ctrl_t * const p_ctrl);

static ssp_err_t r_ptpedmac_read_zc2(ptpedmac_instance_ctrl_t * const p_ctrl, uint32_t * p_channel, ptpedmac_descriptor_t ** pp_buf, int32_t * p_num_received);
static ssp_err_t r_ptpedmac_read_zc2_buf_release(ptpedmac_instance_ctrl_t * const p_ctrl);

/***********************************************************************************************************************
 * ISR function prototypes
 **********************************************************************************************************************/
void ptpedmac_isr(R_PTPEDMAC_Type * p_ptpedmac_reg);
void eptpc_pint_isr(void);

/***********************************************************************************************************************
 * Private Global Variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const ptpedmac_api_t g_ptpedmac_on_ptpedmac =
{
        .open                      = R_PTPEDMAC_Open,
        .linkProcess               = R_PTPEDMAC_LinkProcess,
        .linkCheck                 = R_PTPEDMAC_CheckLink,
        .read                      = R_PTPEDMAC_Read,
        .close                     = R_PTPEDMAC_Close,
        .versionGet                = R_PTPEDMAC_VersionGet
};

/*******************************************************************************************************************//**
 * @addtogroup PTPEDMAC
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Open the PTP host interface (PTPEDMAC), handles required initialization described in hardware manual.
 * Implements ptpedmac_api_t::open.
 *
 * @retval SSP_SUCCESS                 PTP host interface has opened successfully and initialization was successful.
 * @retval SSP_ERR_ASSERTION           Pointer to the control block is NULL.
 * @retval  SSP_ERR_IRQ_BSP_DISABLED   A required interrupt does not exist in the vector table
 * @return                             See @ref Common_Error_Codes or functions called by this function for other possible
 *                                     return codes. This function calls:
 *                                         * fmi_api_t::productFeatureGet
 *                                         * fmi_api_t::eventInfoGet
 **********************************************************************************************************************/
ssp_err_t R_PTPEDMAC_Open(ptpedmac_ctrl_t * const p_api_ctrl, ptpedmac_cfg_t const * const p_cfg)
{
    ptpedmac_instance_ctrl_t * p_ctrl = (ptpedmac_instance_ctrl_t *) p_api_ctrl;

#if PTPEDMAC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_cfg);
#endif

    ssp_err_t err = SSP_SUCCESS;
    uint8_t value;

    /** Reset PTPEDMAC */
    R_PTPEDMAC_Type * p_ptpedmac_reg = (R_PTPEDMAC_Type *) p_ctrl->p_reg;
    HW_PTPEDMAC_Reset(p_ptpedmac_reg, 1);

    /** Wait at least 64 cycles of PCLKA to reset the PTPEDMAC and EPTPC.
     * PCLKA must be at least 12.5 MHz to use Ethernet, so wait at least 5.12 us. */
    R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MILLISECONDS);

    /* To check if relaying between ports is enabled */
    value = HW_PTPEDMAC_RelayModeCheck();

    /** Make sure the peripheral exists. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;

    /* Workaround for Renesas technical update, document number: TN-SY*-A00055A/E */
    if(1U == value)
    {
        ssp_feature.id = SSP_IP_EDMAC;
    }
    else
    {
        ssp_feature.id = SSP_IP_EPTPC;
    }

    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    PTPEDMAC_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = info.ptr;

    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    ssp_vector_info_t * p_vector_info;
    if(1U == value)
    {
        err = g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_EDMAC_EINT, &event_info);
    }
    else
    {
        err = g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_EPTPC_PINT, &event_info);
    }

    IRQn_Type pint_irqn = event_info.irq;

    /** Initialize the channel state information. */
    p_ctrl->p_callback = p_cfg->p_callback;
    p_ctrl->p_context = p_cfg->p_context;

    /** If interrupt is registered in the vector table, disable interrupts, set priority, and store control block in
     * the vector information so it can be accessed from the callback. */
    if (SSP_SUCCESS == err)
    {
        NVIC_DisableIRQ(pint_irqn);
        R_BSP_IrqStatusClear(pint_irqn);
        NVIC_SetPriority(pint_irqn, p_ctrl->pint_irq);
        R_SSP_VectorInfoGet(pint_irqn, &p_vector_info);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }
    p_ctrl->p_app_ptp_rx_desc = NULL;

    memset(&p_ctrl->p_rx_descriptors, 0x00, sizeof(p_ctrl->p_rx_descriptors));
    memset(&p_ctrl->p_ptpedmac_buffer,  0x00, sizeof(p_ctrl->p_ptpedmac_buffer));

    /* Clear all PTPEDMAC status flags */
    HW_PTPEDMAC_SetStatusFlag(p_ptpedmac_reg, PTPEDMAC_STATUS_FLAG);

    /** Enable PINT interrupt */
    NVIC_EnableIRQ(pint_irqn);

    /** Initialize frame transfer status flag */
    p_ctrl->transfer_flag = PTPEDMAC_TRANS_FLAG_OFF;

    /** Mark driver as opened by initializing it to "DMAC" in its ASCII equivalent for this unit. */
    p_ctrl->open = PTPEDMAC_OPEN;

    return err;
} /* End of function R_PTPEDMAC_Open() */

/*******************************************************************************************************************//**
 * @brief Sets PTP host interface to transfer PTP messages.
 * Implements ptpedmac_api_t::linkProcess.
 *
 * @retval SSP_SUCCESS             PTP host interface has linked successfully to transfer PTP messages
 * @retval SSP_ERR_ASSERTION       Pointer to the control block is NULL.
 * @retval SSP_ERR_NOT_OPEN        PTPEDMAC driver is not opened.
 **********************************************************************************************************************/
ssp_err_t R_PTPEDMAC_LinkProcess(ptpedmac_ctrl_t * const p_api_ctrl)
{
    ptpedmac_instance_ctrl_t * p_ctrl = (ptpedmac_instance_ctrl_t *) p_api_ctrl;

#if PTPEDMAC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    PTPEDMAC_ERROR_RETURN(PTPEDMAC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_PTPEDMAC_Type * p_ptpedmac_reg = (R_PTPEDMAC_Type *) p_ctrl->p_reg;

    /** Initialize the receive descriptor */
    ptpedmac_init_descriptors(p_ctrl);

    /** Initialize and configure PTPEDMAC */
    ptpedmac_config_ethernet(p_ctrl);

    HW_PTPEDMAC_EnableReceiveRequest(p_ptpedmac_reg, 1U);

    /* Check if reception is enabled */
    int8_t check_status = HW_PTPEDMAC_RestartReceiveRequest(p_ptpedmac_reg);
    if (check_status)
    {
        p_ctrl->transfer_flag = PTPEDMAC_TRANS_FLAG_OFF;
    }
    else
    {
        /** Set PTP Host interface transfer flag to enable */
        p_ctrl->transfer_flag = PTPEDMAC_TRANS_FLAG_ON;
    }

    return SSP_SUCCESS;
} /* End of function R_PTPEDMAC_LinkProcess() */

/*******************************************************************************************************************//**
 * @brief Checks PTP host interface communication link.
 * Implements ptpedmac_api_t::checkLink.
 *
 * @retval SSP_SUCCESS             PTP host interface link status is verified successfully
 * @retval SSP_ERR_ASSERTION       Pointer to the control block is NULL.
 * @retval SSP_ERR_NOT_OPEN        PTPEDMAC driver is not opened.
 * @retval SSP_ERR_NOT_ENABLED     PTP host interface is not enabled
 **********************************************************************************************************************/
ssp_err_t R_PTPEDMAC_CheckLink(ptpedmac_ctrl_t * const p_api_ctrl)
{
    ptpedmac_instance_ctrl_t * p_ctrl = (ptpedmac_instance_ctrl_t *) p_api_ctrl;

#if PTPEDMAC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    PTPEDMAC_ERROR_RETURN(PTPEDMAC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    ptpedmac_trans_t status_flag = p_ctrl->transfer_flag;

    /** Check the current status of PTP message transfer */
    if (PTPEDMAC_TRANS_FLAG_ON == status_flag)
    {
        /* PTP message transfer is enabled */
        return SSP_SUCCESS;
    }
    else
    {
        /* PTP message transfer is disabled */
        return SSP_ERR_NOT_ENABLED;
    }
} /* End of function R_PTPEDMAC_CheckLink() */

/*******************************************************************************************************************//**
 * @brief Receive PTP message.
 *  Implements ptpedmac_api_t::read.
 *
 * @retval SSP_SUCCESS                   PTP message received successfully
 * @retval SSP_ERR_TIMEOUT               No data received
 * @retval SSP_ERR_NOT_OPEN              PTPEDMAC driver is not opened.
 * @retval SSP_ERR_ASSERTION             Pointer to the control block is NULL.
 * @retval SSP_ERR_NOT_ENABLED           PTP host interface is not enabled
 **********************************************************************************************************************/
ssp_err_t R_PTPEDMAC_Read(ptpedmac_ctrl_t * const p_api_ctrl, uint32_t * p_channel, void * p_buffer, int32_t * p_num_received)
{
    ptpedmac_instance_ctrl_t * p_ctrl = (ptpedmac_instance_ctrl_t *) p_api_ctrl;

#if PTPEDMAC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_channel);
    SSP_ASSERT(NULL != p_buffer);
    SSP_ASSERT(NULL != p_num_received);
    PTPEDMAC_ERROR_RETURN(PTPEDMAC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
    PTPEDMAC_ERROR_RETURN(PTPEDMAC_TRANS_FLAG_ON == p_ctrl->transfer_flag, SSP_ERR_NOT_ENABLED);
#endif

    ssp_err_t ret = SSP_SUCCESS;
    ptpedmac_descriptor_t * r_buffer = NULL;

    /** Set the allocated buffer pointer for received data */
    ret = r_ptpedmac_read_zc2(p_ctrl, p_channel, &r_buffer, p_num_received);
    if (SSP_SUCCESS == ret)
    {
        memcpy(p_buffer, r_buffer, (size_t)*p_num_received);

        /* Release the receive buffer */
        ret = r_ptpedmac_read_zc2_buf_release(p_ctrl);
        if (SSP_SUCCESS != ret)
        {
            return ret;
        }
        return SSP_SUCCESS;
    }
    else
    {
        return SSP_ERR_TIMEOUT;
    }
} /* End of function R_PTPEDMAC_Read() */

/*******************************************************************************************************************//**
 * @brief Disable PTP host interface.
 *  Implements ptpedmac_api_t::close.
 *
 * @retval SSP_SUCCESS             PTP host interface is closed successfully
 * @retval SSP_ERR_NOT_OPEN        PTPEDMAC driver is not opened.
 * @retval SSP_ERR_ASSERTION       Pointer to the control block is NULL.
 **********************************************************************************************************************/
ssp_err_t R_PTPEDMAC_Close(ptpedmac_ctrl_t * const p_api_ctrl)
{
    ptpedmac_instance_ctrl_t * p_ctrl = (ptpedmac_instance_ctrl_t *) p_api_ctrl;

#if PTPEDMAC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    PTPEDMAC_ERROR_RETURN(PTPEDMAC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    R_PTPEDMAC_Type * p_ptpedmac_reg = (R_PTPEDMAC_Type *) p_ctrl->p_reg;

    /** Clear PTPEDMAC interrupt */
    NVIC_DisableIRQ(p_ctrl->pint_irq);

    /* Disable frame reception interrupt */
    HW_PTPEDMAC_EnableReceiveRequest(p_ptpedmac_reg, 0U);
    HW_PTPEDMAC_InitializeReceiveDescriptors(p_ptpedmac_reg, 0U);
    HW_PTPEDMAC_EnableFrameReceiveInterrupt(p_ptpedmac_reg, false);

    /** Set PTP Host interface transfer flag to disable */
    p_ctrl->transfer_flag = PTPEDMAC_TRANS_FLAG_OFF;

    /** The device is now considered closed */
    p_ctrl->open = 0U;

    return SSP_SUCCESS;
} /* End of function R_PTPEDMAC_Close() */

/*******************************************************************************************************************//**
 * @brief    This function returns the API version.
 *  Implements ptpedmac_api_t::versionGet
 *
 * @param[in] p_version                Pointer to the API version block
 *
 * @retval SSP_SUCCESS                   Successful close.
 * @retval SSP_ERR_ASSERTION             The parameter p_version is NULL.
 ***********************************************************************************************************************/
ssp_err_t R_PTPEDMAC_VersionGet(ssp_version_t * const p_version)
{
#if PTPEDMAC_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid */
    SSP_ASSERT(NULL != p_version);
#endif

    /** Return the version number */
    p_version->version_id = g_version.version_id;

    return SSP_SUCCESS;
} /* End of function R_PTPEDMAC_GetVersion() */

/*******************************************************************************************************************//**
 * @} (end addtogroup PTPEDMAC)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Receive one PTP message frame or fragment.
 *
 * @param[in]   p_ctrl                   Pointer to PTPEDMAC instance structure
 * @param[in]   p_channel                Received port channel (0 or 1)
 * @param[in]   pp_buf                   Received data store buffer
 * @param[in]   p_num_received           Received data length
 *
 * @retval SSP_SUCCESS                   PTP message received successfully
 * @retval SSP_ERR_TIMEOUT               No data received
 **********************************************************************************************************************/
static ssp_err_t r_ptpedmac_read_zc2(ptpedmac_instance_ctrl_t * const p_ctrl, uint32_t * p_channel, ptpedmac_descriptor_t ** pp_buf, int32_t * p_num_received)
{
    ssp_err_t ret = SSP_SUCCESS;

    if (RACT != (p_ctrl->p_app_ptp_rx_desc->status & RACT))
    {
        /* Received data exists */
        /** Get received port */
        *p_channel = ((p_ctrl->p_app_ptp_rx_desc->status & PORT) >> PTPEDMAC_RECEIVE_PORT);

        /** Set the allocated buffer pointer for received data */
        *pp_buf = (ptpedmac_descriptor_t *)p_ctrl->p_app_ptp_rx_desc->p_buffer;

        /** Get received data length */
        *p_num_received = (int32_t)p_ctrl->p_app_ptp_rx_desc->size;
    }
    else
    {
        /* No data received */
        ret = SSP_ERR_TIMEOUT;
    }

    return ret;
} /* End of function r_ptpedmac_read_zc2() */

/*******************************************************************************************************************//**
 * @brief Release the receive data buffer
 *
 * @param[in]       p_ctrl              Pointer to PTPEDMAC instance structure
 *
 * @retval SSP_SUCCESS                   Receive data buffer released successfully
 **********************************************************************************************************************/
static ssp_err_t r_ptpedmac_read_zc2_buf_release(ptpedmac_instance_ctrl_t * const p_ctrl)
{
    R_PTPEDMAC_Type * p_ptpedmac_reg = (R_PTPEDMAC_Type *) p_ctrl->p_reg;

    if (RACT != (p_ctrl->p_app_ptp_rx_desc->status & RACT))
    {
        /* Received data exists */
        /* Move current descriptor to next one */
        p_ctrl->p_app_ptp_rx_desc->status |= RACT;
        p_ctrl->p_app_ptp_rx_desc->status &= (uint32_t)(~(RFP1 | RFP0 | RFE | RFOF | PORT | PVER | TYPE3 | TYPE2 | TYPE1 | TYPE0));
        p_ctrl->p_app_ptp_rx_desc = p_ctrl->p_app_ptp_rx_desc->p_next;
    }

    int8_t restart_status = HW_PTPEDMAC_RestartReceiveRequest(p_ptpedmac_reg);
    if (restart_status)
    {
        /** If receive operation of the PTPEDMAC was disabled, make PTPEDMAC to receive enable */
        HW_PTPEDMAC_EnableReceiveRequest(p_ptpedmac_reg, 1U);
    }

    return SSP_SUCCESS;
} /* End of function r_ptpedmac_read_zc2_buf_release() */

/***********************************************************************************************************************
@brief Initialize PTPEDMAC descriptors and the driver buffers.
 * @param[in]       p_ctrl              Pointer to PTPEDMAC instance structure
 *
 * @retval void
 ***********************************************************************************************************************/
static void ptpedmac_init_descriptors(ptpedmac_instance_ctrl_t * const p_ctrl)
{
    ptpedmac_descriptor_t * descriptor = NULL;
    uint32_t i = 0U;

    /* Initialize the receive descriptors */
    for(i = 0U; i < (uint32_t)PTPEDMAC_NUM_RX_DESCRIPTORS; i++)
    {
        descriptor = &(p_ctrl->p_rx_descriptors[i]);
        descriptor->p_buffer = &(p_ctrl->p_ptpedmac_buffer.buffer[i][0]);
        descriptor->bufsize = (uint16_t)PTPEDMAC_BUFFER_SIZE;
        descriptor->size = 0U;
        descriptor->status = RACT;
        descriptor->p_next = &(p_ctrl->p_rx_descriptors[i + 1]);
    }

    /* Boundary condition for receive descriptors allocation */
    descriptor->status |= RDLE;
    descriptor->p_next = &(p_ctrl->p_rx_descriptors[0]);

    /* Initialize receive descriptors pointer to which application allocated */
    p_ctrl->p_app_ptp_rx_desc  = &(p_ctrl->p_rx_descriptors[0]);
} /* End of function ptpedmac_init_descriptors() */


/***********************************************************************************************************************
@brief Configure PTP Direct Memory Access controller (PTPEDMAC).

 * @param[in]       p_ctrl              Pointer to PTPEDMAC instance structure
 *
 * @retval void
 ***********************************************************************************************************************/
static void ptpedmac_config_ethernet(ptpedmac_instance_ctrl_t * const p_ctrl)
{
    R_PTPEDMAC_Type * p_ptpedmac_reg = (R_PTPEDMAC_Type *) p_ctrl->p_reg;

    /* Enable error interrupt */
    /* MACE, RFOF, RDE, TFUF, TDE, ADE and RFCOF */
    HW_PTPEDMAC_EnableInterruptRequest(p_ptpedmac_reg, PTPEDMAC_ERROR_INTERRUPT);

    /* Enable frame received interrupt (FR) */
    HW_PTPEDMAC_EnableFrameReceiveInterrupt(p_ptpedmac_reg, true);

    /* Enable frame transmit complete interrupt (TC) */
    HW_PTPEDMAC_EnableFrameTransmitInterrupt(p_ptpedmac_reg, true);

    /* Set little endian mode */
    HW_PTPEDMAC_SetEndianMode(p_ptpedmac_reg, true);

    /* Initialize receive descriptor list address */
    HW_PTPEDMAC_InitializeReceiveDescriptors(p_ptpedmac_reg, (uint32_t)p_ctrl->p_app_ptp_rx_desc);

    /* Threshold of transmit FIFO */
    /* To prevent a transmit underflow, setting the initial value (store and forward modes) is recommended. */
    HW_PTPEDMAC_SetTransmitFIFOthreshold(p_ptpedmac_reg, 0U);

    /* Transmit FIFO is 2048 byte and receive FIFO is 4096 byte */
    HW_PTPEDMAC_SetTransmitFIFOdepth(p_ptpedmac_reg, PTPEDMAC_FIFO_DEPTH);

    /* Configure receiving method */
    HW_PTPEDMAC_ReceiveRequest(p_ptpedmac_reg, true);

} /* End of function ptpedmac_config_ethernet() */

/***********************************************************************************************************************
 @brief PINT Interrupt handler
 *
 * @param[in]   p_ptpedmac_reg           Pointer to R_PTPEDMAC_Type register
 ***********************************************************************************************************************/
void ptpedmac_isr(R_PTPEDMAC_Type * p_ptpedmac_reg)
{
    uint32_t status_eesr = HW_PTPEDMAC_GetEESR(p_ptpedmac_reg);

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    ptpedmac_instance_ctrl_t * p_ctrl = (ptpedmac_instance_ctrl_t *)*(p_vector_info->pp_ctrl);

    ptpedmac_callback_args_t args;

    if (NULL != p_ctrl->p_callback)
    {
        args.channel = (uint16_t)((status_eesr >> 7U) & 0x1);
        args.ether_frame_type = status_eesr & 0xF;

        /* Judge the interrupt element */
        if (status_eesr & (uint32_t)(PTPEDMAC_MACE_INT | PTPEDMAC_RFOF_INT | PTPEDMAC_RDE_INT | PTPEDMAC_TFUF_INT | PTPEDMAC_TDE_INT
                | PTPEDMAC_ADE_INT | PTPEDMAC_RFCOF_INT))
        {
            /** Setup parameters for the user-supplied callback function. */
            /* MAC address, Receive buffer overflow, Lack of receiving descriptor error, Transmit buffer overflow,
              Lack of transmitting descriptor, Address or The number of received frame overflow error */
            args.event = PTPEDMAC_EVENT_ERR;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
        if (status_eesr & (uint32_t)PTPEDMAC_FR_INT)
        {
            /* Frame received successfully */
            args.event = PTPEDMAC_EVENT_READ;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }

        if (status_eesr & (uint32_t)PTPEDMAC_TC_INT)
        {
            /* Frame transmitted successfully */
            args.event = PTPEDMAC_EVENT_WRITE;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
    }

    /* Clear PTPEDMAC status bits */
    HW_PTPEDMAC_SetEESR(p_ptpedmac_reg, status_eesr);

} /* End of function ptpedmac_isr() */

/*******************************************************************************************************************//**
 * @brief   PTPEDMAC interrupt handler
 *
 *  Saves context if RTOS is used, clears the interrupt flag, calls callback if one was provided in the open function,
 *  and restores context if RTOS is used.
 ***********************************************************************************************************************/
void eptpc_pint_isr(void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE
    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    ptpedmac_instance_ctrl_t * p_ctrl = (ptpedmac_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /* Interrupt handler of PTPEDMAC. */
    ptpedmac_isr(p_ctrl->p_reg);
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
} /* End of function eptpc_pint_isr() */

