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

#ifndef R_SLCDC_PRIVATE_API_H
#define R_SLCDC_PRIVATE_API_H

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Private Instance API Functions. DO NOT USE! Use functions through Interface API structure instead.
 **********************************************************************************************************************/

ssp_err_t R_SLCDC_Open (slcdc_ctrl_t * const p_ctrl,
		                slcdc_cfg_t const * const p_cfg);

ssp_err_t R_SLCDC_Write (slcdc_ctrl_t * const p_ctrl,
		                 slcdc_size_t const start_segment,
						 slcdc_size_t const * const p_data,
						 slcdc_size_t const segment_count);

ssp_err_t R_SLCDC_Modify (slcdc_ctrl_t * const p_ctrl,
                          slcdc_size_t const segment_number,
                          slcdc_size_t const data_mask,
                          slcdc_size_t const data);

ssp_err_t R_SLCDC_Start (slcdc_ctrl_t * const p_ctrl);

ssp_err_t R_SLCDC_Stop (slcdc_ctrl_t * const p_ctrl);

ssp_err_t R_SLCDC_ContrastIncrease (slcdc_ctrl_t * const p_ctrl);

ssp_err_t R_SLCDC_ContrastDecrease (slcdc_ctrl_t * const p_ctrl);

ssp_err_t R_SLCDC_SetDisplayArea (slcdc_ctrl_t * const p_ctrl,
		                       slcdc_display_area_t const display_area);

ssp_err_t R_SLCDC_Close (slcdc_ctrl_t * const p_ctrl);

ssp_err_t R_SLCDC_VersionGet (ssp_version_t * p_version);

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_SLCDC_PRIVATE_API_H */
