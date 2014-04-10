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

struct twobyte_st tmprecv[4] = {0,0,0,0,0,0,0,0};

volatile unsigned char motor_load[4] = {0,0,0,0};
volatile struct twobyte_st ticks[4] = {0,0,0,0,0,0,0,0};
volatile struct twobyte_st speed[4] = {0,0,0,0,0,0,0,0};
volatile struct twobyte_st voltage[4] = {0,0,0,0,0,0,0,0};
volatile struct twobyte_st acceleration[4] = {0,0,0,0,0,0,0,0};

static void
receive_ticks(void) {
  
    complete_pretx();
    M0TX=M1TX=M2TX=M3TX = 0;
    complete_tx();
    tmprecv[0].u.byte[1]=u0rb & 0xff;
    tmprecv[1].u.byte[1]=u3rb & 0xff;
    tmprecv[2].u.byte[1]=u4rb & 0xff;
    tmprecv[3].u.byte[1]=u6rb & 0xff;

    M0TX=M1TX=M2TX=M3TX = 0;
    complete_tx();
    tmprecv[0].u.byte[0]=u0rb & 0xff;
    tmprecv[1].u.byte[0]=u3rb & 0xff;
    tmprecv[2].u.byte[0]=u4rb & 0xff;
    tmprecv[3].u.byte[0]=u6rb & 0xff;

    CS0=CS3=CS4=CS6 = 1;
    
    for(int i=0;i<=3;i++) {
        int8_t y=tmprecv[i].u.int16 >> 9;
        if(y & 0x40)   // if   x1xx xxxx 
            y |= 0x80; // then 11xx xxxx
        motor_load[i] = abs(y); 
        /* make sure ticks[i].u.int16 does not change while we work */
        __disable_interrupt();
        
        int16_t kala2 = tmprecv[i].u.int16;
        int16_t kala1 = kala2 & 0x01ff; // keep 9 bits only
        if (kala1 & 0x0100)       // if   xxxx xxx1 xxxx xxxx
            kala1 |= 0xff00 ;     // then 1111 1111 xxxx xxxx

        int32_t x = (uint32_t)ticks[i].u.int16 + (uint32_t)kala1; 
        /* Check for possible overflow of INT16 and lit red LED */
        if(x > INT16_MAX || x < INT16_MIN) {
            LED5=1;
        } else {
            ticks[i].u.int16=x;
        }
        __enable_interrupt();
    }   
}

static void 
set_acceleration() {
  
    if(RESET5 == 0) {
        return;
    }
    complete_tx(); // make sure we are not transmitting garbage already

    CS0=CS3=CS4=CS6 = 0;
    M0TX=CMD_ACCELERATION | 0;
    M1TX=CMD_ACCELERATION | 1;
    M2TX=CMD_ACCELERATION | 2;
    M3TX=CMD_ACCELERATION | 3;
    
    /* 
     * Use temporary variable to ensure interrupts to not overwrite
     * value while we send it. 
     */
    struct twobyte_st tmp[4];
    tmp[0].u.int16=acceleration[0].u.int16;
    tmp[1].u.int16=acceleration[1].u.int16;
    tmp[2].u.int16=acceleration[2].u.int16;
    tmp[3].u.int16=acceleration[3].u.int16;

    complete_pretx();
    M0TX=tmp[0].u.byte[1];
    M1TX=tmp[1].u.byte[1];
    M2TX=tmp[2].u.byte[1];
    M3TX=tmp[3].u.byte[1]; 

    complete_pretx();
    M0TX=tmp[0].u.byte[0];
    M1TX=tmp[1].u.byte[0];
    M2TX=tmp[2].u.byte[0];
    M3TX=tmp[3].u.byte[0];
    
    receive_ticks();
}


static void 
set_speed() {
    if(RESET5 == 0) {
        return;
    }
    complete_tx(); // make sure we are not transmitting garbage already

    CS0=CS3=CS4=CS6 = 0;
    M0TX=CMD_SPEED | 0;
    M1TX=CMD_SPEED | 1;
    M2TX=CMD_SPEED | 2;
    M3TX=CMD_SPEED | 3;
    
    /* 
     * Use temporary variable to ensure interrupts to not overwrite
     * value while we send it. 
     */
    struct twobyte_st tmp[4];
    tmp[0].u.int16=speed[0].u.int16;
    tmp[1].u.int16=speed[1].u.int16;
    tmp[2].u.int16=speed[2].u.int16;
    tmp[3].u.int16=speed[3].u.int16;

    complete_pretx();
    M0TX=tmp[0].u.byte[1];
    M1TX=tmp[1].u.byte[1];
    M2TX=tmp[2].u.byte[1];
    M3TX=tmp[3].u.byte[1]; 

    complete_pretx();
    M0TX=tmp[0].u.byte[0];
    M1TX=tmp[1].u.byte[0];
    M2TX=tmp[2].u.byte[0];
    M3TX=tmp[3].u.byte[0];
    
receive_ticks();


}

static void 
get_voltage(void) {
  
    if(RESET5 == 0) {
        return;
    }
    complete_tx();
    CS0=CS3=CS4=CS6 = 0;

    M0TX=CMD_GET_VOLTAGE | 0;
    M1TX=CMD_GET_VOLTAGE | 1;
    M2TX=CMD_GET_VOLTAGE | 2;
    M3TX=CMD_GET_VOLTAGE | 3;
    complete_pretx();
    
    M0TX=M1TX=M2TX=M3TX=0;
    complete_tx();

    tmprecv[0].u.byte[1]=u0rb & 0xff;
    tmprecv[1].u.byte[1]=u3rb & 0xff;
    tmprecv[2].u.byte[1]=u4rb & 0xff;
    tmprecv[3].u.byte[1]=u6rb & 0xff;

    M0TX=M1TX=M2TX=M3TX=0;
    complete_tx();

    tmprecv[0].u.byte[0]=u0rb & 0xff;
    tmprecv[1].u.byte[0]=u3rb & 0xff;
    tmprecv[2].u.byte[0]=u4rb & 0xff;
    tmprecv[3].u.byte[0]=u6rb & 0xff;

    CS0=CS3=CS4=CS6 = 1;

    for(int i=0;i<=3;i++) {
        voltage[i].u.int16 = tmprecv[i].u.int16;
    }
}

static const unsigned char fpga_image[] = {
#include "motor-control-fpga-image.hpp"
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  
};

int
main(void) {
    HardwareSetup();
    // All CS* and RESET* are already pulled up in HW setups.
    // Start reading SPI5 for data
    volatile unsigned short dummy=u5rb;
    while(1) {      
        /* If any of motor controllers is not ready, reset everything */
        if(!CDONE0 || !CDONE1 || !CDONE2 || !CDONE3) {
            /* We ignore input bytes until FPGA is loaded */
            __disable_interrupt(); 
            RESET0=RESET1=RESET2=RESET3 = 0;
            CS0=CS3=CS4=CS6 = 0; 
            udelay(1000); // at least 800us 
            RESET0=RESET1=RESET2=RESET3 = 1;
            udelay(1000); // at least 800us 
            for(int i=0;i<sizeof(fpga_image);i++) {
                complete_pretx();
                M0TX=M1TX=M2TX=M3TX=fpga_image[i];
            }
            complete_tx();
            udelay(1000);
            CS0=CS3=CS4=CS6 = 1;
            udelay(1000);            
        } else {
            __enable_interrupt();
            udelay(3000);
            set_speed(); 
            get_voltage(); 
            //set_acceleration(); 
        }
        LED1 = CDONE0;
        LED2 = CDONE1;
        LED3 = CDONE2;
        LED4 = CDONE3;
    }
}
