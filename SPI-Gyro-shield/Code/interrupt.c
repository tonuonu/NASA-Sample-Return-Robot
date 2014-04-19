/*
 *  Copyright (c) 2013, 2014 Tonu Samuel
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
    /* Remove protection from PM0 register */
    prc1 = 1; 
    /* Make software reset of chip */
    pm03 = 1;   
    
    /* We should never get there. But return protection bit just in case */
    prc1 = 0; 
    
    /* Clear the interrupt flag. */
    ir_int0ic = 0;
}

int int2_count=0;	//!!!

#pragma vector = INT2 // CS pin is connected here
__interrupt void _int2(void) {
  
    //LED5=1;  
    recv_bytenum=0;    
    TX5d=PD_INPUT;

    int2_count++;	//!!!

    LED5=0;
    /* Clear the interrupt flag. */
    ir_int2ic = 0;
}
