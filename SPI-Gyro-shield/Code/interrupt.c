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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "hwsetup.h"
#include "intrinsics.h"

void
Int_Init(void) {
    ifsr00=1; // INT0 in both edges. RESET input from Atmega2560 for motors
    ifsr02=1; // INT2 in both edges. CS pin for us
    pol_int0ic  = 0; // This should be 0, "falling edge" to make both edges work
    pol_int2ic  = 0; // This should be 0, "falling edge" to make both edges work
    ilvl_int0ic = 6; // level 6 int, very high
    ilvl_int2ic = 6; // level 6 int, very high
    lvs_int0ic  = 0; // edge sensitive
    lvs_int2ic  = 0; // edge sensitive
}

#pragma vector = INT0 // RESET pin is connected here
__interrupt void _int0(void) {
    recv_bytenum=0;
   
    if(RESET5==1) {
        /* If reset just came up but CS is already low,
         * FPGA code will be loaded in. 
         */
        if(CS5==0) {
            LED5 = 1;
            fpga_in=FPGA_LOADING;       
        }
    } else { // RESET is LOW, flush everything
        fpga_in=FPGA_EMPTY;
    }

    // Copy RESET5 pin to all four motor outputs
    RESET0=RESET1=RESET2=RESET3 = RESET5;
    /* Clear the interrupt flag. */
    ir_int0ic = 0;
}

#pragma vector = INT2 // CS pin is connected here
__interrupt void _int2(void) {
    recv_bytenum=0;

    /*
     * TXEPT (TX buffer EmPTy)
     * 0: Data held in the transmit shift
     * register (transmission in progress)
     * 1: No data held in the transmit shift
     * register (transmission completed)
     */
    while(txept_u0c0 == 0);
    while(txept_u6c0 == 0);
    while(txept_u3c0 == 0);
    while(txept_u4c0 == 0);

    if(fpga_in != FPGA_LOADED) {
        // Copy CS5 pin to all four motor outputs
        CS0=CS6=CS3=CS4 = CS5;
    }
 
    
    /* If we load FPGA bytes in and then CS goes up, code is loaded */
    if(fpga_in==FPGA_LOADING && CS5==1) {
        LED5 = 0;
        fpga_in=FPGA_LOADED;
    }

    /* Clear the interrupt flag. */
    ir_int2ic = 0;
}
