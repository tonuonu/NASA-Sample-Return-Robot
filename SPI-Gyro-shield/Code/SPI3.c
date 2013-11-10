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

#pragma vector = UART3_RX
__interrupt void _uart3_receive(void) {

    LED1 = 1; 
//    u3tb=0xaa;
//    __delay_cycles(1ULL);
    LED1 = 0; 
   /* Clear the 'reception complete' flag.	*/
    ir_s3ric = 0;
}

#pragma vector = UART3_TX
__interrupt void _uart3_transmit(void) {

    LED2 = 1; 
    u3tb=0xaa;
    //__delay_cycles(1ULL);
    LED2 = 0; 
   /* Clear the 'reception complete' flag.	*/
    ir_s3tic = 0;
}


void
SPI3_Init(void) {
    u3brg =  (unsigned char)(((base_freq)/(2*MOTORS_SPI_SPEED))-1);

    CS3d = PD_OUTPUT;
    CS3=1;
    CLOCK3d = PD_OUTPUT;
    CLOCK3s = PF_UART;
    TX3d = PD_OUTPUT;
    TX3s = PF_UART;
    RX3s = PF_UART;

    smd0_u3mr  = 1;                                        // \ 
    smd1_u3mr  = 0;                                        //  | Synchronous Serial Mode
    smd2_u3mr  = 0;                                        // /

    ckdir_u3mr = 0;                                        // 0=internal clock   
    stps_u3mr  = 0;                                        // 0=1 stop bit, 0 required
    pry_u3mr   = 0;                                        // Parity, 0=odd, 0 required 
    prye_u3mr  = 0;                                        // Parity Enable? 0=disable, 0 required 
    iopol_u3mr = 0;                                        // IO Polarity, 0=not inverted, 0 required

    clk0_u3c0 = 0;                                         // Clock source f1 for u3brg
    clk1_u3c0 = 0;                                         // 
    txept_u3c0 = 0;                                        // Transmit register empty flag 
    crd_u3c0 = 1;                                          // CTS disabled when 1
    nch_u3c0 = 0;                                          // 0=Output mode "push-pull" for TXD and CLOCK pin 
    ckpol_u3c0 = 1;                                        // CLK Polarity 0 rising edge, 1 falling edge
    uform_u3c0 = 1;                                        // 1=MSB first

    te_u3c1 = 1;                                           // 1=Transmission Enable
    ti_u3c1 = 0;                                           // Must be 0 to send or receive
    re_u3c1 = 1;                                           // Reception Enable when 1
    ri_u3c1 = 0;                                           // Receive complete flag - U2RB is empty.
    u3irs_u3c1 = 1;                                        // Interrupt  when transmission is completed. 
    u3rrm_u3c1 = 0;                                        // Continuous receive mode off
    u3lch_u3c1 = 0;                                        // Logical inversion off 

    u3smr = 0x00;
    u3smr2 = 0x00;

    sse_u3smr3 = 0;                                        // SS is disabled when 0
    ckph_u3smr3 = 1;                                       // Non clock delayed 
    dinc_u3smr3 = 0;                                       // Master mode when 0
    nodc_u3smr3 = 0;                                       // Select a clock output  mode "push-pull" when 0 
    err_u3smr3 = 0;                                        // Error flag, no error when 0 
    dl0_u3smr3 = 0;                                        // Set 0 for no  delay 
    dl1_u3smr3 = 0;                                        // Set 0 for no  delay 
    dl2_u3smr3 = 0;                                        // Set 0 for no  delay 

    u3smr4 = 0x00;

    DISABLE_IRQ
    /* 
     * Lowest interrupt priority
     * we do not care about speed
     */
    ilvl_s3ric =1;
    ir_s3ric   =0;            
    ilvl_s3tic =1;
    ir_s3tic   =0;            
    ENABLE_IRQ
}


