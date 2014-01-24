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
SPI2_Init(void) {
    u6brg =  (unsigned char)(((base_freq)/(1*MOTORS_SPI_SPEED))-1);
//#define CLOCK2       p7_2
//    pu62=1; // pullup for p7_2 and p7_3. CLOCK2 may need it
    CS6d = PD_OUTPUT;
    CS6=1;
    CLOCK6d = PD_OUTPUT;
    CLOCK6s = PF_UART;
    TX6d = PD_OUTPUT;
    TX6s = PF_UART;
    RX6s = PF_UART;

    smd0_u6mr  = 1;                                        // \ 
    smd1_u6mr  = 0;                                        //  | Synchronous Serial Mode
    smd2_u6mr  = 0;                                        // /

    ckdir_u6mr = 0;                                        // 0=internal clock   
    stps_u6mr  = 0;                                        // 0=1 stop bit, 0 required
    pry_u6mr   = 0;                                        // Parity, 0=odd, 0 required 
    prye_u6mr  = 0;                                        // Parity Enable? 0=disable, 0 required 
    iopol_u6mr = myIOPOL;                                        // IO Polarity, 0=not inverted, 0 required

    clk0_u6c0 = 0;                                         // Clock source f1 for u6brg
    clk1_u6c0 = 0;                                         // 
    txept_u6c0 = 0;                                        // Transmit register empty flag 
    crd_u6c0 = 1;                                          // CTS disabled when 1
    nch_u6c0 = 0;                                          // 0=Output mode "push-pull" for TXD and CLOCK pin 
    ckpol_u6c0 = myCKPOL;                                        // CLK Polarity 0 rising edge, 1 falling edge
    uform_u6c0 = 1;                                        // 1=MSB first

    te_u6c1 = 1;                                           // 1=Transmission Enable
    ti_u6c1 = 0;                                           // Must be 0 to send or receive
    re_u6c1 = 1;                                           // Reception Enable when 1
    ri_u6c1 = 0;                                           // Receive complete flag - u6RB is empty.
    u6irs_u6c1 = 1;                                        // Interrupt  when transmission is completed. 
    u6rrm_u6c1 = 0;                                        // Continuous receive mode off
    u6lch_u6c1 = 1;                                        // Logical inversion off 

    u6smr = 0x00;
    u6smr2 = 0x00;

    sse_u6smr3 = 0;                                        // SS is disabled when 0
    ckph_u6smr3 = myCKPH;                                       // Non clock delayed 
    dinc_u6smr3 = 0;                                       // Master mode when 0
    nodc_u6smr3 = 0;                                       // Select a clock output  mode "push-pull" when 0 
    err_u6smr3 = 0;                                        // Error flag, no error when 0 
    dl0_u6smr3 = 0;                                        // Set 0 for no  delay 
    dl1_u6smr3 = 0;                                        // Set 0 for no  delay 
    dl2_u6smr3 = 0;                                        // Set 0 for no  delay 

    u6smr4 = 0x00;

    DISABLE_IRQ
    /* 
     * Lowest interrupt priority
     * we do not care about speed
     */
    ilvl_s6ric =0;
    ir_s6ric   =0;            
    ilvl_s6tic =0;
    ir_s6tic   =0;            
    ENABLE_IRQ
}


