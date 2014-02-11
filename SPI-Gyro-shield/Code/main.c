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

union u16 recv_u_u0={0,0};
union u16 recv_u_u3={0,0};
union u16 recv_u_u4={0,0};
union u16 recv_u_u6={0,0};

short int recv_buf_u0=0;
short int recv_buf_u3=0;
short int recv_buf_u4=0;
short int recv_buf_u6=0;


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
          (txept_u3c0 == 0) ||
          (txept_u4c0 == 0) ||
          (txept_u6c0 == 0));
}

static void 
set_acceleration(unsigned char motor_idx) {
    complete_tx();
    CS0=CS3=CS4=CS6 = 0;

    u0tb=u3tb=u4tb=u6tb=CMD_ACCELERATION | motor_idx;
    complete_tx();

    u0tb=u3tb=u4tb=u6tb=acceleration[0];
    complete_tx();

    u0tb=u3tb=u4tb=u6tb=acceleration[1];
    complete_tx();

    u0tb=u3tb=u4tb=u6tb=0;
    complete_tx();

    recv_u_u0.ticks[0]=u0rb & 0xff;
    recv_u_u3.ticks[0]=u3rb & 0xff;
    recv_u_u4.ticks[0]=u4rb & 0xff;
    recv_u_u6.ticks[0]=u6rb & 0xff;

    u0tb=u3tb=u4tb=u6tb=0;
    complete_tx();

    recv_u_u0.ticks[1]=u0rb & 0xff;
    recv_u_u3.ticks[1]=u3rb & 0xff;
    recv_u_u4.ticks[1]=u4rb & 0xff;
    recv_u_u6.ticks[1]=u6rb & 0xff;

    recv_buf_u0+=recv_u_u0.x;
    recv_buf_u3+=recv_u_u3.x;
    recv_buf_u4+=recv_u_u4.x;
    recv_buf_u6+=recv_u_u6.x;
    
    CS0=CS3=CS4=CS6 = 1;
}

static void 
set_speed(unsigned char motor_idx) {
    complete_tx();
    CS0=CS3=CS4=CS6 = 0;

    u0tb=u3tb=u4tb=u6tb=CMD_SPEED | motor_idx;
    complete_tx();

    u0tb=u3tb=u4tb=u6tb=speed[0];
    complete_tx();

    u0tb=u3tb=u4tb=u6tb=speed[1];
    complete_tx();

    u0tb=u3tb=u4tb=u6tb=0;
    complete_tx();

    recv_u_u0.ticks[0]=u0rb & 0xff;
    recv_u_u3.ticks[0]=u3rb & 0xff;
    recv_u_u4.ticks[0]=u4rb & 0xff;
    recv_u_u6.ticks[0]=u6rb & 0xff;

    u0tb=u3tb=u4tb=u6tb=0;
    complete_tx();

    recv_u_u0.ticks[1]=u0rb & 0xff;
    recv_u_u3.ticks[1]=u3rb & 0xff;
    recv_u_u4.ticks[1]=u4rb & 0xff;
    recv_u_u6.ticks[1]=u6rb & 0xff;

    recv_buf_u0+=recv_u_u0.x;
    recv_buf_u3+=recv_u_u3.x;
    recv_buf_u4+=recv_u_u4.x;
    recv_buf_u6+=recv_u_u6.x;
    
    CS0=CS3=CS4=CS6 = 1;
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
