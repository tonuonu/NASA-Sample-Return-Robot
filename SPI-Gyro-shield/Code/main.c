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

static void 
complete_tx(void) {
    /*
     * TXEPT (TX buffer EmPTy)
     * 0: Data held in the transmit shift
     * register (transmission in progress)
     * 1: No data held in the transmit shift
     * register (transmission completed)
     */
    while((txept_u0c0 == 0) ||
          (txept_u6c0 == 0) ||
          (txept_u3c0 == 0) ||
          (txept_u4c0 == 0));
}

void 
set_steady_speed(unsigned char motor_idx) {
    complete_tx();
    CS0=CS6=CS3=CS4 = 0;

    u0tb=u6tb=u3tb=u4tb=CMD_STEADY_SPEED | motor_idx;
    complete_tx();

    u0tb=u6tb=u3tb=u4tb=steady_speed[0];
    complete_tx();
    
    //    u0tb=u6tb=u3tb=u4tb=100;
    u0tb=u6tb=u3tb=u4tb=steady_speed[1];
    complete_tx();

    u0tb=u6tb=u3tb=u4tb=0;
    complete_tx();

    u0tb=u6tb=u3tb=u4tb=0;
    complete_tx();

    CS0=CS6=CS3=CS4 = 1;
}


int
main(void) {
    HardwareSetup();
    // All CS* and RESET* are pulled up before in HW setups.
    volatile unsigned short dummy=u5rb & 0xff;
    
    while(1) {
        /* Code hangs here until some interrupt is done */
 //       __wait_for_interrupt();
#if 1        
        if(fpga_in==FPGA_LOADED) {
            set_steady_speed(0);
            set_steady_speed(1);
            set_steady_speed(2);
            set_steady_speed(3);
        }
#endif
    }
}
