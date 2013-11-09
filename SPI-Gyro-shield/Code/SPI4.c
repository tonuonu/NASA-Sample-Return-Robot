/*
 *  Copyright (c) 2013 Tonu Samuel
 *  All rights reserved.
 *
 *  This file is part of robot "Kuukulgur".
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ior32c111.h"
#include <intrinsics.h>
#include "hwsetup.h"
#include "main.h"
#include "SPI.h"

#pragma vector = UART4_RX
__interrupt void _uart4_receive(void) {

    /* Clear the 'reception complete' flag.	*/
    ir_s4ric = 0;
}

#pragma vector = UART4_TX
__interrupt void _uart4_send(void) {

    /* Clear the 'reception complete' flag.	*/
    ir_s4tic = 0;
}

void
SPI4_Init(void) { 
 
    CS4d = PD_OUTPUT;
    CS4=1;
    CLOCK4d = PD_OUTPUT;
    CLOCK4s = PF_UART;
    TX4d = PD_OUTPUT;
    TX4s = PF_UART;
    RX4s = PF_UART;

    
    smd0_u4mr = 1;                                        // \ 
    smd1_u4mr = 0;                                         // >    // Synchronous Serial Mode
    smd2_u4mr = 0;                                         // /
    ckdir_u4mr = 0;                                        // internal clock , 243
    stps_u4mr = 0;                                         // 0 required
    pry_u4mr = 0;                                          // 0 required
    prye_u4mr = 0;                                         // 0 required
    iopol_u4mr = 0;                                        // 0 required
    clk0_u4c0 = 0;                                         // \ Clock
    clk1_u4c0 = 0;                                         // /
    txept_u4c0 = 0;                                        // Transmit
    crd_u4c0 = 1;                                          // CTS disabled 
    nch_u4c0 = 0;                                          // Output mode
    ckpol_u4c0 = 0;                                        // CLK Polarity 0 rising edge, 1 falling edge (0 ok)
    uform_u4c0 = 1;                                        // MSB first
    te_u4c1 = 1;                                           // Transmission 
    ti_u4c1 = 0;                                           // Must be 0 to 
    re_u4c1 = 0;                                           // Reception is 
    ri_u4c1 = 0;                                           // Receive
    u4irs_u4c1 = 1;                                        // 1 when
    u4rrm_u4c1 = 1;                                        // Continuous
    u4lch_u4c1 = 0;                                        // Logical
    u4smr = 0x00;                                          // Set 0
    u4smr2 = 0x00;                                         // Set 0 
    sse_u4smr3 = 0;                                        // SS is
    ckph_u4smr3 = 0;                                       // Non clock
    dinc_u4smr3 = 0;                                       // Master mode
    nodc_u4smr3 = 0;                                       // Select a
    err_u4smr3 = 0;                                        // Error flag,
    dl0_u4smr3 = 0;                                        // Set 0 for no 
    dl1_u4smr3 = 0;                                        // Set 0 for no 
    dl2_u4smr3 = 0;                                        // Set 0 for no 
    u4smr4 = 0x00;                                         // Set 0 (page
    u4brg = 3;                                             
    s4tic = 0x0;
}

