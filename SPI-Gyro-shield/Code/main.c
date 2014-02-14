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

struct twobyte_st tmprecv[4]={0,0,0,0,0,0,0,0 };

volatile struct twobyte_st ticks[4] = {0,0,0,0,0,0,0,0};
volatile struct twobyte_st speed={0,0};
volatile struct twobyte_st acceleration={0,0};

static void 
set_acceleration(unsigned char motor_idx) {
    complete_tx();
    CS0=CS3=CS4=CS6 = 0;

    u0tb=u3tb=u4tb=u6tb=CMD_ACCELERATION | motor_idx;
    complete_tx();

    /* 
     * Use temporary variable to ensure interrupts to not overwrite
     * value while we send it 
     */
    struct twobyte_st tmp;
    tmp.u.int16=acceleration.u.int16;
    u0tb=u3tb=u4tb=u6tb=tmp.u.byte[0];
    complete_tx();

    u0tb=u3tb=u4tb=u6tb=tmp.u.byte[1];
    complete_tx();

    u0tb=u3tb=u4tb=u6tb=0;
    complete_tx();

    tmprecv[0].u.byte[0]=u0rb & 0xff;
    tmprecv[1].u.byte[0]=u3rb & 0xff;
    tmprecv[2].u.byte[0]=u4rb & 0xff;
    tmprecv[3].u.byte[0]=u6rb & 0xff;

    u0tb=u3tb=u4tb=u6tb=0;
    complete_tx();

    tmprecv[0].u.byte[1]=u0rb & 0xff;
    tmprecv[1].u.byte[1]=u3rb & 0xff;
    tmprecv[2].u.byte[1]=u4rb & 0xff;
    tmprecv[3].u.byte[1]=u6rb & 0xff;

    ticks[0].u.int16+=tmprecv[0].u.int16;
    ticks[1].u.int16+=tmprecv[1].u.int16;
    ticks[2].u.int16+=tmprecv[2].u.int16;
    ticks[3].u.int16+=tmprecv[3].u.int16;
    
    CS0=CS3=CS4=CS6 = 1;
    udelay(1); // make sure high CS is noticed 
}

static void 
set_speed(unsigned char motor_idx) {
    complete_tx();
    CS0=CS3=CS4=CS6 = 0;

    u0tb=u3tb=u4tb=u6tb=CMD_SPEED | motor_idx;
    complete_tx();
    
    /* 
     * Use temporary variable to ensure interrupts to not overwrite
     * value while we send it 
     */
    struct twobyte_st tmp;
    tmp.u.int16=speed.u.int16;
    u0tb=u3tb=u4tb=u6tb=tmp.u.byte[0];
    complete_tx();

    u0tb=u3tb=u4tb=u6tb=tmp.u.byte[1];
    complete_tx();

    u0tb=u3tb=u4tb=u6tb=0;
    complete_tx();

    tmprecv[0].u.byte[0]=u0rb & 0xff;
    tmprecv[1].u.byte[0]=u3rb & 0xff;
    tmprecv[2].u.byte[0]=u4rb & 0xff;
    tmprecv[3].u.byte[0]=u6rb & 0xff;

    u0tb=u3tb=u4tb=u6tb=0;
    complete_tx();

    tmprecv[0].u.byte[1]=u0rb & 0xff;
    tmprecv[1].u.byte[1]=u3rb & 0xff;
    tmprecv[2].u.byte[1]=u4rb & 0xff;
    tmprecv[3].u.byte[1]=u6rb & 0xff;

    ticks[0].u.int16+=tmprecv[0].u.int16;
    ticks[1].u.int16+=tmprecv[1].u.int16;
    ticks[2].u.int16+=tmprecv[2].u.int16;
    ticks[3].u.int16+=tmprecv[3].u.int16;
    
    CS0=CS3=CS4=CS6 = 1;
    udelay(1); // make sure high CS is noticed 
}

int
main(void) {
    HardwareSetup();
    // All CS* and RESET* are already pulled up in HW setups.
    volatile unsigned short dummy=u5rb & 0xff;
    
    while(1) {
        /* Code hangs here until some interrupt is done */
 //       __wait_for_interrupt();

        if(fpga_in==FPGA_LOADED) {
            set_acceleration(0); 
            set_acceleration(1);
            set_acceleration(2);
            set_acceleration(3);
            set_speed(0); 
            set_speed(1);
            set_speed(2);
            set_speed(3);
        }

    }
}
