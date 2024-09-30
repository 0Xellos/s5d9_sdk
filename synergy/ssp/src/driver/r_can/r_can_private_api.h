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
 * File Name    : r_can_private_api.h
 * Description  : CAN Private Interface definition
 **********************************************************************************************************************/

#ifndef R_CAN_PRIVATE_API_H_
#define R_CAN_PRIVATE_API_H_


/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER


typedef union
{
    uint32_t  status;
    struct st_int_status_b
    {
        uint32_t  bus_error        : 1;     ///< bus_error.
        uint32_t  error_warning    : 1;     ///< error_warning.
        uint32_t  error_passive    : 1;     ///< error_passive.
        uint32_t  bus_off_entry    : 1;     ///< bus_off_entry.
        uint32_t  bus_off_recovery : 1;     ///< bus_off_recovery
        uint32_t  receive_overrun  : 1;     ///< receive_overrun.
        uint32_t  overload_frame   : 1;     ///< overload_frame.
        uint32_t  bus_lock         : 1;     ///< bus_lock.
        uint32_t  reserved_3       : 24;    ///< Reserved bits.
    }  int_status_b;
} can_interrrupt_status_t;

/***********************************************************************************************************************
 * Private Instance API Functions. DO NOT USE! Use functions through Interface API structure instead.
 **********************************************************************************************************************/
ssp_err_t R_CAN_Open(can_ctrl_t * const p_ctrl, can_cfg_t const * const p_cfg);

ssp_err_t R_CAN_Close(can_ctrl_t * const p_ctrl);

ssp_err_t R_CAN_Read(can_ctrl_t * const p_ctrl, uint32_t const mailbox, can_frame_t * const p_frame);

ssp_err_t R_CAN_Write(can_ctrl_t * const p_ctrl, uint32_t const mailbox, can_frame_t * const p_frame);

ssp_err_t R_CAN_Control(can_ctrl_t * const p_ctrl, can_command_t const command, void * p_data);

ssp_err_t R_CAN_InfoGet(can_ctrl_t * const p_ctrl, can_info_t * const p_info);

ssp_err_t R_CAN_VersionGet(ssp_version_t * const version);

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_CAN_PRIVATE_API_H_ */
