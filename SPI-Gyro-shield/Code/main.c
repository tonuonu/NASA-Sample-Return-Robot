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

extern int alarm;
volatile unsigned short ticks;

extern volatile unsigned char recv_buf;
extern volatile unsigned char recv_flag;

int
main(void) {
    HardwareSetup();
    // All CS* and RESET* are pulled up before in HW setups.
//    volatile unsigned short kala;
    volatile unsigned short dummy=u5rb & 0xff;
    
    while(1) {
        /* Code hangs here until some interrupt is done */
        __wait_for_interrupt();
        LED2=p0_4; // CDONE pin
        /* Check if some data is held in recv_buf. Main shield sent us byte!!          
         * Also see if CS is down and RESET is up
         */
        
        if(recv_flag && !CS5 && RESET5) { 
          recv_flag=0; // clear it
          LED4=1;
          u0tb=recv_buf;          
          u2tb=recv_buf;          
          u3tb=recv_buf;          
          u4tb=recv_buf;          
          LED4=0;
        }
    }
}
