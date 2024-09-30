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

#ifndef R_PTP_PRIVATE_API_H
#define R_PTP_PRIVATE_API_H

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Private Instance API Functions. DO NOT USE! Use functions through Interface API structure instead.
 **********************************************************************************************************************/
ssp_err_t R_PTP_VersionGet (ssp_version_t * const p_version);
ssp_err_t R_PTP_Open (ptp_ctrl_t * const p_ctrl, ptp_cfg_t const * const p_cfg);
ssp_err_t R_PTP_Configure (ptp_ctrl_t * const p_ctrl,
        uint32_t * p_ip_address,
        uint32_t * p_physical_address_msw,
        uint32_t * p_physical_address_lsw);
ssp_err_t R_PTP_Close (ptp_ctrl_t * const p_ctrl);
ssp_err_t R_PTP_Reset (ptp_ctrl_t * const p_ctrl);
ssp_err_t R_PTP_SetExtPromiscuous (ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, bool is_set);
ssp_err_t R_PTP_SetLocalClock (ptp_ctrl_t * const p_ctrl, ptp_timestamp_t * p_clock);
ssp_err_t R_PTP_GetLocalClock (ptp_ctrl_t * const p_ctrl, ptp_timestamp_t * p_clock, uint32_t wait_option);
ssp_err_t R_PTP_GetMasterPortID (ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, uint32_t * p_clock, uint16_t * p_port);
ssp_err_t R_PTP_SetMasterPortID (ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, uint32_t * p_clock, uint16_t * p_port);
ssp_err_t R_PTP_GetSyncInfo (ptp_ctrl_t * const p_ctrl, uint8_t ptp_channel, ptp_timeInterval_t * p_master_offset, ptp_timeInterval_t * p_path_delay);
ssp_err_t R_PTP_Start (ptp_ctrl_t * const p_ctrl, uint32_t wait_option);
ssp_err_t R_PTP_Stop (ptp_ctrl_t * const p_ctrl, uint32_t wait_option);
ssp_err_t R_PTP_SetWorst10Values (ptp_ctrl_t * const p_api_ctrl, uint8_t interval);
ssp_err_t R_PTP_CheckWorst10Values (ptp_ctrl_t * const p_api_ctrl, uint32_t wait_option);
ssp_err_t R_PTP_GetWorst10Values (ptp_ctrl_t * const p_api_ctrl,
        uint32_t * p_positive_worst10,
        uint32_t * p_negative_worst10,
        uint32_t wait_option);
ssp_err_t R_PTP_SetGradientLimit (ptp_ctrl_t * const p_api_ctrl, uint32_t * p_positive_limit, uint32_t * p_negative_limit);
ssp_err_t R_PTP_UpdateClockID (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, int8_t * p_clock_id);
ssp_err_t R_PTP_UpdateDomainNumber (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, uint8_t domain_num);
ssp_err_t R_PTP_UpdateAnnounceFlags (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, ptp_announce_flag_t * p_flag);
ssp_err_t R_PTP_UpdateAnnounceMsgs (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, ptp_announce_message_t * p_message);
ssp_err_t R_PTP_UpdateSyncAnnounceMsgInterval (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, int8_t * p_sync_interval, int8_t * p_announce_interval);
ssp_err_t R_PTP_UpdateDelayMsgInterval (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, int8_t * p_interval, uint32_t * p_timeout);
ssp_err_t R_PTP_GetMessageReceptionConfig (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, ptp_message_reception_t * p_ptp_message_reception);
ssp_err_t R_PTP_SetMessageReceptionConfig (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, ptp_message_reception_t * p_ptp_message_reception);
ssp_err_t R_PTP_DisableTimer (ptp_ctrl_t * const p_api_ctrl, ptp_stca_timer_channel_t timer_channel);
ssp_err_t R_PTP_IndicateEvent (ptp_ctrl_t * const p_api_ctrl,
        ptp_stca_timer_channel_t timer_channel,
        ptp_stca_timer_pulse_edge_t timer_pulse_edge,
        bool is_set);
ssp_err_t R_PTP_AutoClearEvent (ptp_ctrl_t * const p_api_ctrl,
        ptp_stca_timer_channel_t timer_channel,
        ptp_stca_timer_pulse_edge_t timer_pulse_edge,
        bool is_set);
ssp_err_t R_PTP_SetTimer (ptp_ctrl_t * const p_api_ctrl, uint8_t timer_channel, UInt64_t event, uint32_t cycle, uint32_t pulse);
ssp_err_t R_PTP_SetMINTevent (ptp_ctrl_t * const p_api_ctrl, ptp_event_t ptp_reg, uint32_t event, bool is_set);
ssp_err_t R_PTP_EnableINFABTnotification (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel);
ssp_err_t R_PTP_DisableINFABTnotification (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel);
ssp_err_t R_PTP_CheckINFABTstatus (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel, uint8_t * p_status);
ssp_err_t R_PTP_ClearINFABTstatus (ptp_ctrl_t * const p_api_ctrl, uint8_t ptp_channel);

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_PTP_PRIVATE_H */
