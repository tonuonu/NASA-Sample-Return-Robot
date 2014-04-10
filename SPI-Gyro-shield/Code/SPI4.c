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

void
SPI4_Init(void) {
    u4brg =  (unsigned char)(((base_freq)/(1*MOTORS_SPI_SPEED))-1);

    prc2 = 1; // pd9 is protected. 
    CS4d = PD_OUTPUT;
    CS4=1;
    prc2 = 1; // pd9 is protected. 
    CLOCK4d = PD_OUTPUT;
    prc2 = 1; // P9d is protected. 
    TX4d = PD_OUTPUT;
    prc2 = 1; // P9_3s..p9_7s are protected. 
    CLOCK4s = PF_UART;
    prc2 = 1; // P9_3s..p9_7s are protected. 
    TX4s = PF_UART;
    prc2 = 1; // P9_3s..p9_7s are protected. 
    RX4s = PF_UART;
    // prc2 is automatically cleared. No need to set back to zero.
    
    smd0_u4mr  = 1;                                        // \ 
    smd1_u4mr  = 0;                                        //  | Synchronous Serial Mode
    smd2_u4mr  = 0;                                        // /

    ckdir_u4mr = 0;                                        // 0=internal clock   
    stps_u4mr  = 0;                                        // 0=1 stop bit, 0 required
    pry_u4mr   = 0;                                        // Parity, 0=odd, 0 required 
    prye_u4mr  = 0;                                        // Parity Enable? 0=disable, 0 required 
    iopol_u4mr = myIOPOL;                                        // IO Polarity, 0=not inverted, 0 required

    clk0_u4c0 = 0;                                         // Clock source f1 for u4brg
    clk1_u4c0 = 0;                                         // 
    txept_u4c0 = 0;                                        // Transmit register empty flag 
    crd_u4c0 = 1;                                          // CTS disabled when 1
    nch_u4c0 = 0;                                          // 0=Output mode "push-pull" for TXD and CLOCK pin 
    ckpol_u4c0 = myCKPOL;                                        // CLK Polarity 0 rising edge, 1 falling edge
    uform_u4c0 = 1;                                        // 1=MSB first

    te_u4c1 = 1;                                           // 1=Transmission Enable
    ti_u4c1 = 0;                                           // Must be 0 to send or receive
    re_u4c1 = 1;                                           // Reception Enable when 1
    ri_u4c1 = 0;                                           // Receive complete flag - U2RB is empty.
    u4irs_u4c1 = 1;                                        // Interrupt  when transmission is completed. 
    u4rrm_u4c1 = 0;                                        // Continuous receive mode off
    u4lch_u4c1 = 1;                                        // Logical inversion off 

    u4smr = 0x00;
    u4smr2 = 0x00;

    sse_u4smr3 = 0;                                        // SS is disabled when 0
    ckph_u4smr3 = myCKPH;                                       // Non clock delayed 
    dinc_u4smr3 = 0;                                       // Master mode when 0
    nodc_u4smr3 = 0;                                       // Select a clock output  mode "push-pull" when 0 
    err_u4smr3 = 0;                                        // Error flag, no error when 0 
    dl0_u4smr3 = 0;                                        // Set 0 for no  delay 
    dl1_u4smr3 = 0;                                        // Set 0 for no  delay 
    dl2_u4smr3 = 0;                                        // Set 0 for no  delay 

    u4smr4 = 0x00;
#if 0
    __disable_interrupt();
    /* 
     * No interrupts
     */
    ilvl_s4ric =0;
    ir_s4ric   =0;            
    ilvl_s4tic =0;
    ir_s4tic   =0;            
    __enable_interrupt();
#endif
}


