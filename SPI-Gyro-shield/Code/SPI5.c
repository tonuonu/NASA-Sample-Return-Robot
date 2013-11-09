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

#pragma vector = UART5_RX
__interrupt void _uart5_receive(void) {

    /* Clear the 'reception complete' flag.	*/
    ir_s5ric = 0;
}

#pragma vector = UART5_TX
__interrupt void _uart5_send(void) {

    /* Clear the 'reception complete' flag.	*/
    ir_s5tic = 0;
}

void
SPI5_Init(void) { 
 
    CS5d = PD_OUTPUT;
    CS5=1;
    CLOCK5d = PD_OUTPUT;
    CLOCK5s = PF_UART;
    TX5d = PD_OUTPUT;
    TX5s = PF_UART;
    RX5s = PF_UART;

    
    smd0_u5mr = 1;                                        // \ 
    smd1_u5mr = 0;                                         // >    // Synchronous Serial Mode
    smd2_u5mr = 0;                                         // /
    ckdir_u5mr = 0;                                        // internal clock , 243
    stps_u5mr = 0;                                         // 0 required
    pry_u5mr = 0;                                          // 0 required
    prye_u5mr = 0;                                         // 0 required
    iopol_u5mr = 0;                                        // 0 required
    clk0_u5c0 = 0;                                         // \ Clock
    clk1_u5c0 = 0;                                         // /
    txept_u5c0 = 0;                                        // Transmit
    crd_u5c0 = 1;                                          // CTS disabled 
    nch_u5c0 = 0;                                          // Output mode
    ckpol_u5c0 = 0;                                        // CLK Polarity 0 rising edge, 1 falling edge (0 ok)
    uform_u5c0 = 1;                                        // MSB first
    te_u5c1 = 1;                                           // Transmission 
    ti_u5c1 = 0;                                           // Must be 0 to 
    re_u5c1 = 0;                                           // Reception is 
    ri_u5c1 = 0;                                           // Receive
    u5irs_u5c1 = 1;                                        // 1 when
    u5rrm_u5c1 = 1;                                        // Continuous
    u5lch_u5c1 = 0;                                        // Logical
    u5smr = 0x00;                                          // Set 0
    u5smr2 = 0x00;                                         // Set 0 
    sse_u5smr3 = 0;                                        // SS is
    ckph_u5smr3 = 0;                                       // Non clock
    dinc_u5smr3 = 0;                                       // Master mode
    nodc_u5smr3 = 0;                                       // Select a
    err_u5smr3 = 0;                                        // Error flag,
    dl0_u5smr3 = 0;                                        // Set 0 for no 
    dl1_u5smr3 = 0;                                        // Set 0 for no 
    dl2_u5smr3 = 0;                                        // Set 0 for no 
    u5smr4 = 0x00;                                         // Set 0 (page
    u5brg = 3;                                             
    s5tic = 0x0;
}

