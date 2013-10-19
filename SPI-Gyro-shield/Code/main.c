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
 *  along with TYROS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "ior32c111.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "hwsetup.h"
#include "gyro.h"
#include "locking.h"


extern int alarm;
volatile unsigned short ticks;
volatile unsigned short ticks2;

float bat=0.0f;
float capacitor=0.0f;
        

int
main(void) {
    int j;
    //int autocharge=0;
    HardwareSetup();    
    LED1=1;
    u0tb=0;
    //Delay(2);
//    PANDA=1;
    /* 
     * Position sensors interrupt-driven state machine start  
     */
    CS4=0;
    CS7=0;
    // 300uS needed. On 48Mhz each cycle is ~21nS, so
    // 300 000nS/21=~1200
    for(j=0;j<2;j++) {
        uDelay(255); 
    }
    u4tb=0xAA;
    u7tb=0xAA;
    /* 
     * Gyroscopic sensor interrupt-driven state machine start  
     */
#if 0
    CS6=0;
    u6tb=L3G4200D_WHOAMI | 0x80;
#endif
    while(1) {
        __wait_for_interrupt();
        
    }
}
