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

#pragma vector = UART2_RX
__interrupt void _uart2_receive(void) {

    LED1 = 1; 
//    u2tb=0xaa;
//    __delay_cycles(1ULL);
    LED1 = 0; 
   /* Clear the 'reception complete' flag.	*/
    ir_s2ric = 0;
}

#pragma vector = UART2_TX
__interrupt void _uart2_transmit(void) {

    LED2 = 1; 
    u2tb=0xaa;
    //__delay_cycles(1ULL);
    LED2 = 0; 
   /* Clear the 'reception complete' flag.	*/
    ir_s2tic = 0;
}


void
SPI2_Init(void) {
    u2brg =  (unsigned char)(((base_freq)/(2*100000))-1);

    CS2d = PD_OUTPUT;
    CS2=1;
    CLOCK2d = PD_OUTPUT;
    CLOCK2s = PF_UART;
    TX2d = PD_OUTPUT;
    TX2s = PF_UART;
    RX2s = PF_UART;

    smd0_u2mr  = 1;                                        // \ 
    smd1_u2mr  = 0;                                        //  | Synchronous Serial Mode
    smd2_u2mr  = 0;                                        // /

    ckdir_u2mr = 0;                                        // 0=internal clock   
    stps_u2mr  = 0;                                        // 0=1 stop bit, 0 required
    pry_u2mr   = 0;                                        // Parity, 0=odd, 0 required 
    prye_u2mr  = 0;                                        // Parity Enable? 0=disable, 0 required 
    iopol_u2mr = 0;                                        // IO Polarity, 0=not inverted, 0 required

    clk0_u2c0 = 0;                                         // Clock source f1 for u4brg
    clk1_u2c0 = 0;                                         // 
    txept_u2c0 = 0;                                        // Transmit register empty flag 
    crd_u2c0 = 1;                                          // CTS disabled when 1
    nch_u2c0 = 0;                                          // 0=Output mode "push-pull" for TXD and CLOCK pin 
    ckpol_u2c0 = 1;                                        // CLK Polarity 0 rising edge, 1 falling edge
    uform_u2c0 = 1;                                        // 1=MSB first

    te_u2c1 = 1;                                           // 1=Transmission Enable
    ti_u2c1 = 0;                                           // Must be 0 to send or receive
    re_u2c1 = 1;                                           // Reception Enable when 1
    ri_u2c1 = 0;                                           // Receive complete flag - U2RB is empty.
    u2irs_u2c1 = 1;                                        // Interrupt  when transmission is completed. 
    u2rrm_u2c1 = 0;                                        // Continuous receive mode off
    u2lch_u2c1 = 0;                                        // Logical inversion off 

    u2smr = 0x00;
    u2smr2 = 0x00;

    sse_u2smr3 = 0;                                        // SS is disabled when 0
    ckph_u2smr3 = 1;                                       // Non clock delayed 
    dinc_u2smr3 = 0;                                       // Master mode when 0
    nodc_u2smr3 = 0;                                       // Select a clock output  mode "push-pull" when 0 
    err_u2smr3 = 0;                                        // Error flag, no error when 0 
    dl0_u2smr3 = 0;                                        // Set 0 for no  delay 
    dl1_u2smr3 = 0;                                        // Set 0 for no  delay 
    dl2_u2smr3 = 0;                                        // Set 0 for no  delay 

    u2smr4 = 0x00;

    DISABLE_IRQ
    /* 
     * Lowest interrupt priority
     * we do not care about speed
     */
    ilvl_s2ric =1;
    ir_s2ric   =0;            
    ilvl_s2tic =1;
    ir_s2tic   =0;            
    ENABLE_IRQ
}


