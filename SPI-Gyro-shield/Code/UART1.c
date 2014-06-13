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
#include <stdio.h>
#include <string.h>
#include "hwsetup.h"
#include "main.h"
#include "UART.h"

void UART1_Char(char c) {
  while (ti_u1c1 == 0);
  u1tb = c;
}

void UART1_Dump(struct twobyte_st w1[4], struct twobyte_st w2[4], struct twobyte_st w3[4], struct twobyte_st w4[4], struct twobyte_st w5[4]) {
  for (int i=0; i<4; i++) {
    if (i>0)
      UART1_Char(' ');
    
    char buf[64];
    sprintf(buf, "%02x%02x %02x%02x %02x%02x %02x%02x", w1[i].u.byte[1], w1[i].u.byte[0], w2[i].u.byte[1], w2[i].u.byte[0],
                                                        w3[i].u.byte[1], w3[i].u.byte[0], w4[i].u.byte[1], w4[i].u.byte[0] );
    for (int j=0; j<strlen(buf); j++)
      UART1_Char(buf[j]);
  }
  for (int i=0; i<4; i++) {
    char buf[64];
    sprintf(buf, " %02x%02x", w5[i].u.byte[1], w5[i].u.byte[0] );
    for (int j=0; j<strlen(buf); j++)
      UART1_Char(buf[j]);
  }
  UART1_Char('\n');
}

void UART1_DumpLog(int16_t w1, unsigned char w2[4], struct twobyte_st w3[4], char w4) {
  char buf[64];
  
  sprintf(buf, "%d:", w1 );
  for (int j=0; j<strlen(buf); j++)
    UART1_Char(buf[j]);
  
  for (int i=0; i<4; i++) {
    sprintf(buf, " %02x %02x%02x", w2[i], w3[i].u.byte[1], w3[i].u.byte[0] );
    for (int j=0; j<strlen(buf); j++)
      UART1_Char(buf[j]);
  }
  
  sprintf(buf, " %x%x%x%x\n", (w4>>3)&1, (w4>>2)&1, (w4>>1)&1, w4&1 );
  for (int j=0; j<strlen(buf); j++)
    UART1_Char(buf[j]);
}

void
UART1_Init(void) {
    u1brg =  0;                                             // 3Mbps

    TX1d = PD_OUTPUT;
    TX1s = PF_UART;
    RX1s = PF_UART;

    smd0_u1mr  = 1;                                        // \ 
    smd1_u1mr  = 0;                                        //  | UART Mode 
    smd2_u1mr  = 1;                                        // /

    ckdir_u1mr = 0;                                        // 0=internal clock   
    stps_u1mr  = 0;                                        // 0=1 stop bit, 0 required
    pry_u1mr   = 0;                                        // Parity, 0=odd, 0 required 
    prye_u1mr  = 0;                                        // Parity Enable? 0=disable, 0 required 
    iopol_u1mr = 0;                                        // IO Polarity, 0=not inverted, 0 required

    clk0_u1c0 = 0;                                         // Clock source f1 for u0brg
    clk1_u1c0 = 0;                                         // 
    txept_u1c0 = 0;                                        // Transmit register empty flag 
    crd_u1c0 = 1;                                          // CTS disabled when 1
    nch_u1c0 = 0;                                          // 0=Output mode "push-pull" for TXD and CLOCK pin 
    ckpol_u1c0 = myCKPOL;                                  // CLK Polarity 0 rising edge, 1 falling edge
    uform_u1c0 = 0;                                        // 1=LSB first

    te_u1c1 = 1;                                           // 1=Transmission Enable
    ti_u1c1 = 0;                                           // Must be 0 to send or receive
    re_u1c1 = 1;                                           // Reception Enable when 1
    ri_u1c1 = 0;                                           // Receive complete flag - U2RB is empty.
    u1irs_u1c1 = 1;                                        // Interrupt  when transmission is completed. 
    u1rrm_u1c1 = 0;                                        // Continuous receive mode off
    u1lch_u1c1 = 0;                                        // Logical inversion off 

    u1smr = 0x00;
    u1smr2 = 0x00;

    sse_u1smr3 = 0;                                        // SS is disabled when 0
    ckph_u1smr3 = myCKPH;                                  // Non clock delayed 
    dinc_u1smr3 = 0;                                       // Master mode when 0
    nodc_u1smr3 = 0;                                       // Select a clock output  mode "push-pull" when 0 
    err_u1smr3 = 0;                                        // Error flag, no error when 0 
    dl0_u1smr3 = 0;                                        // Set 0 for no  delay 
    dl1_u1smr3 = 0;                                        // Set 0 for no  delay 
    dl2_u1smr3 = 0;                                        // Set 0 for no  delay 

    u1smr4 = 0x00;
}
