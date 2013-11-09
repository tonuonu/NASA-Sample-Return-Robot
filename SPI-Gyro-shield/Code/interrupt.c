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
#include "SPI.h"
#include "intrinsics.h"

/*
 * Timer A0 is used in "one shot" mode
 * to provide long delay waiting service for
 * SPI7 with rotational position sensor attached.
 */
#pragma vector=TIMER_A0
__interrupt void
oneshot1(void) {
#if 0
    switch(mlx2whoamistatus) {
    case 6+1:
        CS7=0;
        uDelay(6); // t6, 10+uS on scope, 6.9 required
        u7tb=0xAA;
        mlx2whoamistatus=0;
        break;
    default:
        u7tb=0xFF;
        break;
    }
#endif   
    ir_ta0ic = 0;
}

/*
 * Timer A3 is used in "one shot" mode
 * to provide long delay waiting service for
 * SPI4 with rotational position sensor attached.
 */
#pragma vector=TIMER_A3
__interrupt void
oneshot2(void) {
#if 0
  switch(mlx1whoamistatus) {
        case 6+1:
            CS4=0;
            uDelay(6); // t6, 10+uS on scope, 6.9 required
            u4tb=0xAA;
            mlx1whoamistatus=0;
            break;
        default:
            u4tb=0xFF;
            break;
    }   
#endif
    ir_ta3ic = 0;
}

#pragma vector=TIMER_B5
__interrupt void
s_int(void) {
    /* 
     * This interrupt gets called 100 times per second
     */
    ticks++;
    /*
     * Here are lot of activities we want to do once per second
     * We try to keep interrupt run time lower by distributing
     * these activites to different 1/100th-s of second.
     */
    switch(ticks % 100) {
        case 0:
            LED5 ^= 1; 

            break;
        case 99:
           /*
            * Reset the counter to prevent overroll later
            */
           ticks=99;
           break;
        default:
           break;
    }
}
