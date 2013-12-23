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

volatile unsigned char recv_buf;
volatile unsigned char recv_flag=0;

#if 1
__fast_interrupt void _uart5_receive(void) {
#else
#pragma vector = UART5_RX
__interrupt void _uart5_receive(void) {
#endif
    LED3 = 1; 
    recv_buf=u5rb & 0xff;
    recv_flag=1;
    LED3 = 0; 
    /* Clear the 'reception complete' flag.	*/
    ir_s5ric = 0;
}

#pragma vector = UART5_TX
__interrupt void _uart5_transmit(void) {
    /* Clear the 'reception complete' flag.	*/
    ir_s5tic = 0;
}

#pragma vector = INT0 // RESET pin is connected here
__interrupt void _int0(void) {
//   LED1 ^= 1;
   recv_flag = 0; // clear any receive buffers   
   /*
    * TXEPT (TX buffer EmPTy)
    * 0: Data held in the transmit shift
    * register (transmission in progress)
    * 1: No data held in the transmit shift
    * register (transmission completed)
    */
    while(txept_u0c0 == 0);
    while(txept_u2c0 == 0);
    while(txept_u3c0 == 0);
    while(txept_u4c0 == 0);

    // Copy RESET5 pin to all four motor outputs
    RESET0=RESET1=RESET2=RESET3 = RESET5;
    /* Clear the interrupt flag. */
    ir_int0ic = 0;
}

#pragma vector = INT2 // CS pin is connected here
__interrupt void _int2(void) {
   recv_flag = 0; // clear any receive buffers 
   /*
    * TXEPT (TX buffer EmPTy)
    * 0: Data held in the transmit shift
    * register (transmission in progress)
    * 1: No data held in the transmit shift
    * register (transmission completed)
    */
    while(txept_u0c0 == 0);
    while(txept_u2c0 == 0);
    while(txept_u3c0 == 0);
    while(txept_u4c0 == 0);

    // Copy CS5 pin to all four motor outputs
    LED1=CS0=CS2=CS3=CS4 = CS5;
   
    /* Clear the interrupt flag. */
    ir_int2ic = 0;
}

void
SPI5_Init(void) {
    // u5brg =  (unsigned char)(((base_freq)/(1*MOTORS_SPI_SPEED))-1);
    // u5brg does not matter because clock is external
  
  //CS5d = PD_OUTPUT; Input!!
    //CS5=1;
    //CLOCK5d = PD_OUTPUT;
    
    //p7_1 (miso) has hardware pullup on board;
    pu27=1; // Enable pullup to avoid floating pin noise on p7_7 (clock)
    CLOCK5s = PF_UART;
    TX5d = PD_OUTPUT;
    TX5s = PF_UART;
    RX5s = PF_UART;

    smd0_u5mr  = 1;                                        // \ 
    smd1_u5mr  = 0;                                        //  | Synchronous Serial Mode
    smd2_u5mr  = 0;                                        // /

    ckdir_u5mr = 1;                                        // 1=external clock   
    stps_u5mr  = 0;                                        // 0=1 stop bit, 0 required
    pry_u5mr   = 0;                                        // Parity, 0=odd, 0 required 
    prye_u5mr  = 0;                                        // Parity Enable? 0=disable, 0 required 
    iopol_u5mr = myIOPOL;                                  // IO Polarity, 0=not inverted, 0 required

    clk0_u5c0 = 0;                                         // Clock source f1 for u5brg
    clk1_u5c0 = 0;                                         // 
    txept_u5c0 = 0;                                        // Transmit register empty flag 
    crd_u5c0 = 1;                                          // CTS disabled when 1
    nch_u5c0 = 0;                                          // 0=Output mode "push-pull" for TXD and CLOCK pin 
    ckpol_u5c0 = myCKPOL;                                  // CLK Polarity 0 rising edge, 1 falling edge
    uform_u5c0 = 1;                                        // 1=MSB first

    te_u5c1 = 1;                                           // 1=Transmission Enable
    ti_u5c1 = 0;                                           // Must be 0 to send or receive
    re_u5c1 = 1;                                           // Reception Enable when 1
    ri_u5c1 = 0;                                           // Receive complete flag - U5RB is empty.
    u5irs_u5c1 = 1;                                        // Interrupt when transmission is completed.
    u5rrm_u5c1 = 1;                                        // Continuous receive mode off
    u5lch_u5c1 = 1;                                        // Logical inversion off 

    u5smr = 0x00;
    u5smr2 = 0x00;

    sse_u5smr3 = 0;                                        // SS is disabled when 0
    ckph_u5smr3 = myCKPH;                                  // Non clock delayed 
    dinc_u5smr3 = 0;                                       // Slave mode when 1. Unsure if should be unless SS is used too
    nodc_u5smr3 = 0;                                       // Select a clock output  mode "push-pull" when 0 
    err_u5smr3 = 0;                                        // Error flag, no error when 0 
    dl0_u5smr3 = 0;                                        // Set 0 for no  delay 
    dl1_u5smr3 = 0;                                        // Set 0 for no  delay 
    dl2_u5smr3 = 0;                                        // Set 0 for no  delay 

    u5smr4 = 0x00;

    DISABLE_IRQ
    /* 
     * Middle interrupt priority
     */
#if 1
    fsit_ripl1 = 1;
    fsit_ripl2 = 1;
    __set_VCT_register((unsigned long)&_uart5_receive);
    ilvl_s5ric =7; // fast interrupt
#else
    ilvl_s5ric =5; 
#endif
    ir_s5ric   =0;            
    ilvl_s5tic =4;
    ir_s5tic   =0;            
    ENABLE_IRQ
}


