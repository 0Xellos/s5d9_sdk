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
 * File Name    : r_ptpedmac_api.h
 * Description  : Interface file for PTPEDMAC(PTP host interface) HAL API
 **********************************************************************************************************************/

#ifndef R_PTPEDMAC_API_H
#define R_PTPEDMAC_API_H

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup PTPEDMAC_API PTPEDMAC driver Interface
 *
 * @brief Interface for PTPEDMAC functions.
 *
 * @section PTPEDMAC_API_SUMMARY Summary
 * The PTPEDMAC interface supports PTP host interface to receive PTP message.
 *
 * The PTPEDMAC interface can be implemented by:
 * - @ref PTPEDMAC
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * PTPEDMAC Interface description: @ref HALPTPEDMACInterface
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/* Includes board and MCU related header files. */
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* Version Number of API. */
#define PTPEDMAC_API_VERSION_MAJOR   (2U)
#define PTPEDMAC_API_VERSION_MINOR   (0U)

/* Bit definition of interrupt factor of Ethernet interrupt */
#define PTPEDMAC_TYPE_INT      (0xFU)
#define PTPEDMAC_PVER_INT      (1UL << 4)
#define PTPEDMAC_RPORT_INT     (1UL << 7)
#define PTPEDMAC_MACE_INT      (1UL << 8)  /* MAC address error */
#define PTPEDMAC_RFOF_INT      (1UL << 16) /* Receive buffer overflow error */
#define PTPEDMAC_RDE_INT       (1UL << 17) /* Lack of receiving descriptor error */
#define PTPEDMAC_FR_INT        (1UL << 18) /* Frame receive complete */
#define PTPEDMAC_TFUF_INT      (1UL << 19) /* Transmit buffer overflow error */
#define PTPEDMAC_TDE_INT       (1UL << 20) /* Lack of transmitting descriptor error */
#define PTPEDMAC_TC_INT        (1UL << 21) /* Frame transmit complete */
#define PTPEDMAC_ADE_INT       (1UL << 23) /* Address error */
#define PTPEDMAC_RFCOF_INT     (1UL << 24) /* The number of received frame overflow error */
#define PTPEDMAC_TABT_INT      (1UL << 26)
#define PTPEDMAC_TWB_INT       (1UL << 30)

/* Bit definitions of status member of Descriptor */
#define  TACT               (0x80000000U)
#define  RACT               (0x80000000U)
#define  TDLE               (0x40000000U)
#define  RDLE               (0x40000000U)
#define  TFP1               (0x20000000U)
#define  RFP1               (0x20000000U)
#define  TFP0               (0x10000000U)
#define  RFP0               (0x10000000U)
#define  TFE                (0x08000000U)
#define  RFE                (0x08000000U)
#define  RFOF               (0x00000200U)
#define  PORT               (0x00000080U)
#define  PVER               (0x00000010U)
#define  TYPE3              (0x00000008U)
#define  TYPE2              (0x00000004U)
#define  TYPE1              (0x00000002U)
#define  TYPE0              (0x00000001U)
#define  TWBI               (0x04000000U)
#define  TFS0_MACE          (0x00000001U)

#define PTPEDMAC_ERROR_INTERRUPT (0x019B0100U)
#define PTPEDMAC_FIFO_DEPTH      (0x0000070FU)
#define PTPEDMAC_STATUS_FLAG     (0x45BF019FU)
#define PTPEDMAC_RECEIVE_PORT    (0x00000007U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** PTPEDMAC interrupt event definitions */
typedef enum
{
    PTPEDMAC_EVENT_READ = 0, ///< Frame reception interrupt (FR)
    PTPEDMAC_EVENT_WRITE,    ///< Frame transmission interrupt (TC)
    PTPEDMAC_EVENT_ERR,      ///< Error interrupt (MACE, RFOF, RDE, TFUF, TDE, ADE and RFCOF)
} ptpedmac_event_t;

/** Enumeration to specify the status of transfer flag */
typedef enum
{
    PTPEDMAC_TRANS_FLAG_OFF = 0,  ///< Transfer disable
    PTPEDMAC_TRANS_FLAG_ON  = 1   ///< Transfer enable
} ptpedmac_trans_t;

/** PTPEDMAC descriptor structure */
typedef struct st_ptpedmac_descriptor
{
    uint32_t                       status;
    uint16_t                       size;
    uint16_t                       bufsize;
    uint8_t                        *p_buffer;
    struct st_ptpedmac_descriptor  *p_next;
}ptpedmac_descriptor_t;

/** PTPEDMAC control block.  Allocate an instance specific control block to pass into the PTPEDMAC API calls.
 * @par Implemented as
 * - ptpedmac_instance_ctrl_t
 */
typedef void ptpedmac_ctrl_t;

/** PTPEDMAC callback arguments definition */
typedef struct st_ptpedmac_callback_arg
{
    uint16_t         channel;                  ///< Device channel number
    uint32_t         ether_frame_type;         ///< Ethernet PTP message type
    void const     * p_context;                ///< Context provided to user during callback
    ptpedmac_event_t event;                    ///< The event can be used to identify what caused the callback
} ptpedmac_callback_args_t;

/** PTPEDMAC configuration block.  Allocate an instance specific control block to pass into the PTPEDMAC API calls. */
typedef struct st_ptpedmac_cfg
{
    ssp_err_t (* p_callback)(ptpedmac_callback_args_t * p_args);      ///< Pointer to interrupt callback function
    void const * p_context;                                           ///< User defined context passed into callback function
    uint8_t      irq_ipl;                                             ///< PINT interrupt IRQ number
} ptpedmac_cfg_t;

/** PTPEDMAC functions implemented at the HAL layer will follow this API. */
typedef struct st_ptpedmac_api
{
    /** Open the PTPEDMAC driver module for reception of PTP messages.
     * @par Implemented as
     * - R_PTPEDMAC_Open()
     *
     * @param[in] p_ctrl               Pointer to the control structure
     * @param[in] p_cfg                Pointer to configuration structure
     **/
    ssp_err_t (* open)(ptpedmac_ctrl_t * const p_ctrl, ptpedmac_cfg_t const * const p_cfg);

    /** Sets host interface to transfer PTP messages
     * @par Implemented as
     * - R_PTPEDMAC_LinkProcess()
     * @param[in] p_ctrl               Pointer to the control structure
     **/
    ssp_err_t(* linkProcess)(ptpedmac_ctrl_t * const p_api_ctrl);

    /** Checks host interface communication status
     * @par Implemented as
     * - R_PTPEDMAC_CheckLink()
     * @param[in] p_ctrl               Pointer to the control structure
     **/
    ssp_err_t (* linkCheck)(ptpedmac_ctrl_t * const p_ctrl);

    /** Receives PTP message
     * @par Implemented as
     * - R_PTPEDMAC_Read()
     * @param[in] p_ctrl                 Pointer to the control structure
     * @param[out] p_channel             Pointer to received channel
     * @param[out] p_buffer              Pointer to received data buffer
     * @param[out] p_num_received        Pointer to number of received data
     **/
    ssp_err_t (* read)(ptpedmac_ctrl_t * const p_ctrl,
            uint32_t * p_channel,
            void * const p_buffer,
            int32_t * p_num_received);

    /** Close the PTPEDMAC driver module.
     * @par Implemented as
     * - R_PTPEDMAC_Close()
     * @param[in] p_ctrl               Pointer to the control structure
     **/
    ssp_err_t (* close)(ptpedmac_ctrl_t * const p_ctrl);

    /** Get the driver version based on compile time macros.
     * @par Implemented as
     * - R_PTPEDMAC_VersionGet()
     * @param[out]  p_version  Code and API version used.
     **/
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);

} ptpedmac_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_ptpedmac_instance
{
    ptpedmac_ctrl_t       * p_ctrl;    ///< Pointer to the control structure for this instance
    ptpedmac_cfg_t  const * p_cfg;     ///< Pointer to the configuration structure for this instance
    ptpedmac_api_t  const * p_api;     ///< Pointer to the API structure for this instance
} ptpedmac_instance_t;

/*******************************************************************************************************************//**
 * @} (end defgroup PTPEDMAC_API)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_PTPEDMAC_API_H */
