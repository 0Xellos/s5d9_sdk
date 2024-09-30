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
* File Name    : r_slcdc_api.h
* Description  : Segment LCD Controller (SLCDC) Module API header file.
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup SLCDC_API SLCDC Interface
 * @brief Interface for Segment LCD controllers.
 *
 * @section SLCDC_API_SUMMARY Summary
 * This driver uses the Segment LCD controller (SLCDC) to display data on a
 * Segment LCD.
 *
 * Implemented by:
 * @ref SLCDC
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * SLCDC Interface description: @ref HALSLCDCInterface
 * @{
 **********************************************************************************************************************/

#ifndef DRV_SLCDC_API_H
#define DRV_SLCDC_API_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
/** Register definitions, common services and error codes. */
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define SLCDC_API_VERSION_MAJOR   (2U)
#define SLCDC_API_VERSION_MINOR   (0U)

/*----------------------------------------------------------------------------*/
/*   The range of voltage                                                     */
/*----------------------------------------------------------------------------*/
#define SLCDC_VOL_MIN           (4)
#define SLCDC_VOL_MAX           (19)
#define SLCDC_VOL_MAX_4BIAS     (10)

/* Reference voltage (contrast Adjustment) select (VLCD bit).
   - VL1 voltage (reference voltage).
   4 = 1.00V
   5 = 1.05V
   6 = 1.10V
   7 = 1.15V
   8 = 1.20V
   9 = 1.25V
   10 = 1.30V
   11 = 1.35V
   12 = 1.40V
   13 = 1.45V
   14 = 1.50V
   15 = 1.55V
   16 = 1.60V
   17 = 1.65V
   18 = 1.70V
   19 = 1.75V

   - VL4 voltage (1/3 bias method).
   4 = 3.00V
   5 = 3.15V
   6 = 3.30V
   7 = 3.45V
   8 = 3.60V
   9 = 3.75V
   10 = 3.90V
   11 = 4.05V
   12 = 4.20V
   13 = 4.35V
   14 = 4.50V
   15 = 4.65V
   16 = 4.80V
   17 = 4.95V
   18 = 5.10V
   19 = 5.20V

   - VL4 voltage (1/4 bias method).
   4 = 4.00V
   5 = 4.20V
   6 = 4.40V
   7 = 4.60V
   8 = 4.80V
   9 = 5.00V
   10 = 5.20V
*/
#define SLCDC_CFG_REF_VCC (12)

#define SLCDC_BOOST_COUNTER  (5)  /* The number of times of performing waiting for 100 ms */
#define SLCDC_BOOST_WAIT     (uint32_t)(100000) /* Waiting for 100ms  */
#define SLCDC_SETUP_WAIT     (5)   /* Waiting for 5ms    */

#define MAX_NUM_SEG           (51U)

/**********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/** Size definition for slcdc */
typedef uint8_t slcdc_size_t;

/** Display interface operation state */
typedef enum e_slcdc_display_state
{
    SLCDC_DISPLAY_STATE_CLOSED     = 0,	///< Display closed
    SLCDC_DISPLAY_STATE_OPENED     = 1,	///< Display opened
} slcdc_display_state_t;

/** LCD display bias method. */
typedef enum e_slcd_bias_method
{
	SLCDC_BIAS_2 = 0,                                    ///< 1/2 bias method
	SLCDC_BIAS_3,                                        ///< 1/3 bias method
	SLCDC_BIAS_4,                                        ///< 1/4 bias method
} slcdc_bias_method_t;

/** Time slice of LCD display. */
typedef enum e_slcd_time_slice
{
	SLCDC_STATIC  = 0,                                     ///< Static
	SLCDC_SLICE_2 = 1,                                     ///< 2-time slice
	SLCDC_SLICE_3 = 2,                                     ///< 3-time slice
	SLCDC_SLICE_4 = 3,                                     ///< 4-time slice
	SLCDC_SLICE_8 = 5,           						  ///< 8-time slice
} slcdc_time_slice_t;

/** LCD display waveform select. */
typedef enum e_slcd_wave_form
{
	SLCDC_WAVE_A  = 0,                                     ///< Waveform A
	SLCDC_WAVE_B,                                          ///< Waveform B
} slcdc_wave_form_t;

/** LCD Drive Voltage Generator Select.*/
typedef enum e_slcd_drive_volt_gen
{
	SLCDC_VOLT_EXTERNAL  = 0,                              ///< External resistance division method
	SLCDC_VOLT_INTERNAL,                                   ///< Internal voltage boosting method
	SLCDC_VOLT_CAPACITOR,                                  ///< Capacitor split method
} slcdc_drive_volt_gen_t;

/** Display Data Area Control*/
typedef enum e_slcd_display_area_control_blink
{
	SLCDC_NOT_BLINKING  = 0,                                ///< Alternately displaying A-pattern and B-pattern area data
	SLCDC_BLINKING,                                         ///< Displaying an A-pattern or B-pattern area data
} slcdc_display_area_control_blink_t;

/** Display Area data */
typedef enum e_slcd_display_area_control_disp
{
	SLCDC_DISP_A  = 0,                                       ///< Displaying an A-pattern area data
	SLCDC_DISP_B,                                            ///< Displaying an B-pattern area data
	SLCDC_DISP_BLINK,                                        ///< Alternatively displaying A-pattern and B-pattern area data
} slcdc_display_area_t;

/** LCD Display Enable/Disable*/
typedef enum e_slcd_display_on_off
{
	SLCDC_DISP_OFF  = 0,                                       ///< Display off
	SLCDC_DISP_ON,                                             ///< Display on
} slcdc_display_on_off_t;

/** LCD Display output enable */
typedef enum e_slcd_display_enable_disable
{
	SLCDC_DISP_DISABLE  = 0,                                   ///< Output ground level to segment/common pin
	SLCDC_DISP_ENABLE,                                         ///< Output enable
} slcdc_display_enable_disable_t;

/** LCD Display clock selection */
typedef enum e_slcd_display_clock
{
    SLCDC_CLOCK_LOCO      = 0x00,                              ///<  Display clock source LOCO
	SLCDC_CLOCK_SOSC      = 0x01,                              ///<  Display clock source SOSC
	SLCDC_CLOCK_MOSC      = 0x02,                              ///<  Display clock source MOSC
	SLCDC_CLOCK_HOCO      = 0x03,                              ///<  Display clock source HOCO
} slcdc_display_clock_t;

/* LCD Clock Control Register 0 (LCDC0 bit).
   - LCD source clock is the main clock, HOCO clock or LOCO clock.
   17 = 1/256
   18 = 1/512
   19 = 1/1024
   20 = 1/2048
   21 = 1/4096
   22 = 1/8192
   23 = 1/16384
   24 = 1/32768
   25 = 1/65536
   26 = 1/131072
   27 = 1/262144
   43 = 1/524288

   - LCD source clock is the sub-clock.
   1 = 1/4
   2 = 1/8
   3 = 1/16
   4 = 1/32
   5 = 1/64
   6 = 1/128
   7 = 1/256
   8 = 1/512
   9 = 1/1024

   - LCD source clock is the IWDT-dedicated on-chip oscillator.
   - The following selections are prohibition when IWDT-dedicated is chosen as LCD clock sauce.
   1 = 1/4
   2 = 1/8
   3 = 1/16
   4 = 1/32
   5 = 1/64
   6 = 1/128
   7 = 1/256
*/
//#define SLCDC_CFG_CLOCK_DIV (23)

/** LCD clock settings */
typedef enum e_slcdc_clk_div
{
    SLCDC_CLK_DIVISOR_LOCO_4  = 1,                          ///< LOCO Clock/4
	SLCDC_CLK_DIVISOR_LOCO_8,							    ///< LOCO Clock/8
	SLCDC_CLK_DIVISOR_LOCO_16,                              ///< LOCO Clock/16
	SLCDC_CLK_DIVISOR_LOCO_32,                              ///< LOCO Clock/32
	SLCDC_CLK_DIVISOR_LOCO_64,                              ///< LOCO Clock/64
	SLCDC_CLK_DIVISOR_LOCO_128,                             ///< LOCO Clock/128
	SLCDC_CLK_DIVISOR_LOCO_256,                             ///< LOCO Clock/256
	SLCDC_CLK_DIVISOR_LOCO_512,                             ///< LOCO Clock/512
	SLCDC_CLK_DIVISOR_LOCO_1024,                            ///< LOCO Clock/1024

	SLCDC_CLK_DIVISOR_HOCO_256 = 17,    					///< HOCO Clock/256
	SLCDC_CLK_DIVISOR_HOCO_512,                             ///< HOCO Clock/512
	SLCDC_CLK_DIVISOR_HOCO_1024,                            ///< HOCO Clock/1024
	SLCDC_CLK_DIVISOR_HOCO_2048,                            ///< HOCO Clock/2048
	SLCDC_CLK_DIVISOR_HOCO_4096,                            ///< HOCO Clock/4096
	SLCDC_CLK_DIVISOR_HOCO_8192,                            ///< HOCO Clock/8192
	SLCDC_CLK_DIVISOR_HOCO_16384,                           ///< HOCO Clock/16384
	SLCDC_CLK_DIVISOR_HOCO_32768,                           ///< HOCO Clock/32768
	SLCDC_CLK_DIVISOR_HOCO_65536,                           ///< HOCO Clock/65536
	SLCDC_CLK_DIVISOR_HOCO_131072,                          ///< HOCO Clock/131072
	SLCDC_CLK_DIVISOR_HOCO_262144,                          ///< HOCO Clock/262144
    SLCDC_CLK_DIVISOR_HOCO_524288,                          ///< HOCO Clock/524288
} slcdc_clk_div_t;

/** SLCDC configuration block */
typedef struct st_slcdc_cfg
{
slcdc_display_clock_t        slcdc_clock;               ///< LCD clock source (LCDSCKSEL)
slcdc_clk_div_t              slcdc_clock_setting;       ///< LCD clock setting (LCDC0)
slcdc_bias_method_t          bias_method;               ///< LCD display bias method select (LBAS bit).
slcdc_time_slice_t           time_slice;                ///< Time slice of LCD display select (LDTY bit)
slcdc_wave_form_t            wave_form;                 ///< LCD display waveform select (LWAVE bit).
slcdc_drive_volt_gen_t       drive_volt_gen;            ///< LCD Drive Voltage Generator Select (MDSTET bit).

} slcdc_cfg_t;

/** SLCDC control block.  Allocate an instance specific control block to pass into the SLCDC API calls.
 * @par Implemented as
 * - slcdc_instance_ctrl_t
 */
/** SLCDC control block */
typedef void slcdc_ctrl_t;

/** SLCDC functions implemented at the HAL layer will follow this API. */
typedef struct st_slcdc_api
{
    /** Open SLCD device.
     * @par Implemented as
     * - R_SLCDC_Open()
     * @param[in,out]  p_ctrl        Pointer to display interface control block. Must be declared by user. Value set
     *                               here.
     * @param[in]      p_cfg         Pointer to display configuration structure. All elements of this structure must be
     *                               set by user.
     */
   ssp_err_t (*open)       (slcdc_ctrl_t * const p_ctrl, slcdc_cfg_t  const * const p_cfg);

   /** Write data to SLCD segment.
    * Specifies the initial display data. Except for 8-time slice, store values in the lower 4 bits when writing to the A-pattern area, and in the upper 4 bits when writing to the B-pattern area.
    * The display data is stored in the display data register.
    * @par Implemented as
    * - R_SLCDC_Write()
    * @param[in]  p_ctrl           Pointer to display interface control block.
    * @param[in]  start_segment    Specify the start segment number to be written.
    * @param[in]  p_data           pointer to the display data to be written to the specified segments
    * @param[in]  segment_count    Number of segments to be written
    */
   ssp_err_t (*write)      (slcdc_ctrl_t * const p_ctrl, slcdc_size_t const start_segment, slcdc_size_t const * const p_data, slcdc_size_t const segment_count);

   /** Rewrite data in the SLCD segment.
    * Rewrites the LCD display data in 1-bit units. If a bit is not specified for rewriting, the value stored in the bit is held as it is.
    * Specifies the data to rewrite
    * @par Implemented as
    * - R_SLCDC_Modify()
    * @param[in]  p_ctrl           Pointer to display interface control block.
    * @param[in]  seg              Specify the segment to be written.
    * @param[in]  data_mask        Mask the data being displayed. Set 0 to the bit to be rewritten and set 1 to the other bits. Multiple bits can be rewritten.
    * Setting value of data_mask,
    * Bit 3 - 0xf7
    * Bit 2 - 0xfb
    * Bit 1 - 0xfd
    * Bit 0 - 0xfe
    * @param[in]  data             Specify display data to rewrite to the specified segment.
    *
    */
   ssp_err_t (*modify)     (slcdc_ctrl_t * const p_ctrl, slcdc_size_t const segment, slcdc_size_t const data_mask, slcdc_size_t const data);

   /** Enable display on the SLCD.
    * Displays the specified data on the LCD. Before that data should be written to the segments.
    * @par Implemented as
    * - R_SLCDC_Start()
    * @param[in]  p_ctrl           Pointer to display interface control block.
    */
   ssp_err_t (*start)      (slcdc_ctrl_t * const p_ctrl);

   /** Disable display on the SLCD.
    * Stops displaying data on the SLCD.
    * @par Implemented as
    * - R_SLCDC_Stop()
    * @param[in]  p_ctrl           Pointer to display interface control block.
    */
   ssp_err_t (*stop)       (slcdc_ctrl_t * const p_ctrl);

   /** Increase the display contrast. Increase by 1 unit.
    * This function can be selected when the internal voltage boosting method is used for the drive voltage generator
    * @par Implemented as
    * - R_SLCDC_ContrastIncrease()
    * @param[in]  p_ctrl           Pointer to display interface control block.
    */
   ssp_err_t (*contrastIncrease)  (slcdc_ctrl_t * const p_ctrl);

   /** Decrease the display contrast. Decrease by 1 unit.
    * This function can be selected when the internal voltage boosting method is used for the drive voltage generator
    * @par Implemented as
    * - R_SLCDC_ContrastDecrease()
    * @param[in]  p_ctrl           Pointer to display interface control block.
    */
   ssp_err_t (*contrastDecrease)  (slcdc_ctrl_t * const p_ctrl);

   /** Set LCD display area.
       * This function sets a specified display area, A-pattern or B-pattern. This function can be used to set blink on
       * where A-pattern and B-pattern area data will be alternately displayed.
       *
       * When using blinking, the RTC is required to operate before this function is executed. To configure the RTC, follow the steps below.
       *  1) Open RTC
       *  2) Set Periodic interrupt request, 1/2 second
       *  3) Start RTC counter
       *  4) Enable IRQ, RTC_EVENT_PERIODIC_IRQ
       *  Refer to the User’s Manual: Hardware for the detailed procedure.
       *
       * @par Implemented as
       * - R_SLCDC_SetdisplayArea()
       * @param[in]  p_ctrl           Pointer to display interface control block.
       * @param[in]  display_area     Display area to be used, A-pattern or B-pattern area.
       */
   ssp_err_t (*setdisplayArea) (slcdc_ctrl_t * const p_ctrl, slcdc_display_area_t const display_area);

   /** Close display device.
    * @par Implemented as
    * - R_SLCDC_Close()
    * @param[in]     p_ctrl   Pointer to display interface control block.
    */
   ssp_err_t (*close)       (slcdc_ctrl_t * const p_ctrl);

   /** Get version.
    * @par Implemented as
    * - R_SLCDC_VersionGet()
    * @param[in]   p_version  Pointer to the memory to store the version information.
    */
   ssp_err_t (*versionGet)  (ssp_version_t * p_version);

} slcdc_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_slcdc_instance
{
    slcdc_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    slcdc_cfg_t const * p_cfg;     ///< Pointer to the configuration structure for this instance
    slcdc_api_t const * p_api;     ///< Pointer to the API structure for this instance
} slcdc_instance_t;

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_SLCDC_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup SLCDC_API)
***********************************************************************************************************************/
