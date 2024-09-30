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
 * File Name    : cac_example.c
 * Description  : Examples of CAC interface.
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_cac.h"
#include "r_cgc_api.h"

#if defined(BSP_MCU_GROUP_S3A7) || defined(BSP_MCU_GROUP_S7G2)

#include "r_cac_api.h"
#include "r_cac_cfg.h"
#include "r_cgc.h"
#include <stdio.h>          // for semihosting debug window

/*******************************************************************************************************************//**
 * @addtogroup CAC
 * @{
 **********************************************************************************************************************/

//#define CAC_EXAMPLE
#ifdef CAC_EXAMPLE

/***********************************************************************************************************************
 * Private global variables and functions
 **********************************************************************************************************************/
static cac_api_t const      * p_cac_ptr;
cac_ctrl_t                  ctrl;
ssp_err_t                   err;
static volatile bool        interrupt_flag = false;
static volatile cac_event_t result;
uint8_t                     cac_status;
uint16_t                    cac_counter;
bool                        ovff  = false;
bool                        ferrf = false;
bool                        mendf = false;

cgc_clock_cfg_t             clock_config;

static void cac_callback (cac_callback_args_t * cb_data)
{
    interrupt_flag = true;
    switch (cb_data->event)
    {
        case CAC_EVENT_FREQUENCY_ERROR:
            result = CAC_EVENT_FREQUENCY_ERROR;
            break;

        case CAC_EVENT_MEASUREMENT_COMPLETE:
            result = CAC_EVENT_MEASUREMENT_COMPLETE;
            break;

        case CAC_EVENT_COUNTER_OVERFLOW:
            result = CAC_EVENT_COUNTER_OVERFLOW;
            break;
    }
}

static cac_cfg_t g_cac_example_cfg =
{
    .p_callback                = cac_callback,
    .p_context               = 0,
    .p_extend                = NULL,
    .continuous_mode         = false,                 ///< true if measurement continuously restarts after completing.
    .mei_interrupt_enabled   = false,                 ///< true if Measurement Complete interrupt is enabled
    .ovf_interrupt_enabled   = false,                 ///< true if Overflow interrupt is enabled
    .ferr_interrupt_enabled  = false,                 ///< true if Frequency Error interrupt is enabled
    .cac_ref_clock.digfilter = CAC_REF_DIGITAL_FILTER_OFF,      ///< No digital filter
    .cac_ref_clock.edge      = CAC_REF_EDGE_RISE,     ///< Rising edge detect
    .cac_meas_clock.clock    = CAC_CLOCK_SOURCE_HOCO, ///< We want to measure HOCO (24 MHz)
    .cac_meas_clock.divider  = CAC_MEAS_DIV_1,        ///< No divisor on the measurement clock
    .cac_ref_clock.clock     = CAC_CLOCK_SOURCE_LOCO, ///< Our reference clock will LOCO (32.768 kHz)
    .cac_ref_clock.divider   = CAC_REF_DIV_32,        ///< Minimum divider is 32, so effective freq  = 1024 Hz
    .cac_lower_limit         = 21094,                 ///< Expected count = 24000000/1024 = 23438
    .cac_upper_limit         = 25781,                 ///< We'll allow a 10% tolerance, or an acceptable range of 21094
                                                      // - 25781
};

extern void      initialise_monitor_handles (void);

/***********************************************************************************************************************
 * Function Name: TEST_SETUP
 * Description  : Tear down for these unit tests
 * Arguments    : none
 * Return Value : none
 **********************************************************************************************************************/
void main (void)
{
    initialise_monitor_handles();

    //tx_kernel_enter();

    err = R_CGC_Init();
    if (err != SSP_SUCCESS)
    {
        printf("cgc open failed\r\n");
    }

    err = R_CGC_ClockStart(CGC_CLOCK_SUBCLOCK, &clock_config);
    err = R_CGC_ClockStart(CGC_CLOCK_HOCO, &clock_config);
    err = R_CGC_ClockStart(CGC_CLOCK_LOCO, &clock_config);

    // We have to wait for the clocks to stabilize...Check for LOCO stablization is not supported..
    do
    {
        if (R_CGC_ClockCheck(CGC_CLOCK_HOCO) == SSP_ERR_STABILIZED)
        {
            break;
        }
    } while (1);

    // Setup for CAC

    // According to the spec this should work for both S3A7 and S7G2
    // This should make P204 (S3A7 Eval board CN15 pin 11) as the CACREF input
    R_PMISC->PWPR                             = 0;    //  Enable writing to PSFE
    R_PMISC->PWPR                             = 0x40; //  Set the PFSWE bit

    R_PFS->P204PFS_b.PDR                      = 0;    // Make this an input
    R_PFS->P204PFS_b.PSEL                     = 0x0A; // Select CACREF_A as the peripheral function
    R_PFS->P204PFS_b.PMR                      = 1;    // Peripheral pin

    R_PMISC->PWPR                             = 0x80; //  Disable writing to PSFE

    p_cac_ptr                                 = &g_cac_on_cac;

    g_cac_example_cfg.mei_interrupt_enabled   = true;
    g_cac_example_cfg.ovf_interrupt_enabled   = true;
    g_cac_example_cfg.ferr_interrupt_enabled  = true;
    g_cac_example_cfg.cac_meas_clock.clock    = CAC_CLOCK_SOURCE_HOCO;     // We want to measure HOCO (24 MHz)
    g_cac_example_cfg.cac_meas_clock.divider  = CAC_MEAS_DIV_1;            // No divisor on the measurement clock
    g_cac_example_cfg.cac_ref_clock.clock     = CAC_CLOCK_SOURCE_EXTERNAL; // Our reference clock will be an external
                                                                           // 1kHz clock
    g_cac_example_cfg.cac_ref_clock.divider   = CAC_REF_DIV_32;            // External clock has no divider, so
                                                                           // effective freq  = 24MHz/1kHz = 24000
    g_cac_example_cfg.cac_ref_clock.digfilter = CAC_REF_DIGITAL_FILTER_OFF;          // No digital filter
    g_cac_example_cfg.cac_ref_clock.edge      = CAC_REF_EDGE_RISE;         // Rising edge detect

    g_cac_example_cfg.cac_lower_limit         = 21600;                     // Expected count = 24000
    g_cac_example_cfg.cac_upper_limit         = 26400;                     // We'll allow a 10% tolerance, or an
                                                                           // acceptable range of 21600 - 26400

    g_cac_example_cfg.mei_interrupt_enabled  = false;
    g_cac_example_cfg.ovf_interrupt_enabled  = false;
    g_cac_example_cfg.ferr_interrupt_enabled = false;

    /** Configure and open the CAC API */
    err = p_cac_ptr->open(&ctrl, &g_cac_example_cfg);

    printf("open\r\n");

    interrupt_flag = false;


    err = p_cac_ptr->startMeasurement(&ctrl);

    // Wait for completed measurement or error
    do
    {
        err =  p_cac_ptr->read(&ctrl, &cac_status,  &cac_counter);
        err =  p_cac_ptr->reset(&ctrl);
        err = p_cac_ptr->startMeasurement(&ctrl);
    } while ((cac_status == 0) && (SSP_SUCCESS == err));

    //    while (1);

    ovff                                  = (bool) (cac_status >> 2);
    ferrf                                 = (bool) (cac_status & 0x1);
    mendf                                 = (bool) ((cac_status & 0x2) >> 1);

    err                                   = p_cac_ptr->close(&ctrl);

    g_cac_example_cfg.cac_ref_clock.clock = CAC_CLOCK_SOURCE_SUBCLOCK,     ///< Our reference clock will Sub-clock
                                                                           // (32.768 kHz)

    /** Configure and open the CAC API */
    err = p_cac_ptr->open(&ctrl, &g_cac_example_cfg);

    printf("open\r\n");

    interrupt_flag = false;
    err            =  p_cac_ptr->read(&ctrl, &cac_status,  &cac_counter);

    err            = p_cac_ptr->startMeasurement(&ctrl);

    // Wait for completed measurement or error
    do
    {
        err =  p_cac_ptr->read(&ctrl, &cac_status,  &cac_counter);
    } while ((cac_status == 0) && (SSP_SUCCESS == err));

    ovff  = (bool) (cac_status >> 2);
    ferrf = (bool) (cac_status & 0x1);

    err   = p_cac_ptr->close(&ctrl);

    do
    {
        do
        {
        } while (interrupt_flag == false);

        err            =  p_cac_ptr->read(&ctrl, &cac_status,  &cac_counter);
        interrupt_flag = false;
    } while (1);
}

#endif /* ifdef CAC_EXAMPLE */
#endif /* if defined(BSP_MCU_GROUP_S3A7) || defined(BSP_MCU_GROUP_NPP4) */

/*******************************************************************************************************************//**
 * @} (end addtogroup CAC)
 **********************************************************************************************************************/
