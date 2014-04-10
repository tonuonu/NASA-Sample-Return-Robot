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

#include "intrinsics.h"
#include "stdint.h"
void Int_Init(void);


static inline void 
complete_pretx(void) {
    while((ti_u0c1 == 0) ||
          (ti_u3c1 == 0) ||
          (ti_u4c1 == 0) ||
          (ti_u6c1 == 0));
}


static inline void 
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

enum {
    FPGA_LOADING,
    FPGA_LOADED
};

enum {
    CMD_NONE=0, 
    CMD_SPEED=0x04, 
    CMD_ACCELERATION=0x08, 
    CMD_GET_CUR_TARGET_SPEED=0x10,
    CMD_GET_VOLTAGE=0x18
};

/*
 * This union is used in every context where we need to receive two separate 
 * bytes into single short int (16 bit signed)
 */
struct twobyte_st {
    union {
        int8_t byte[2];
        int16_t int16;
    } u;
};

extern volatile struct twobyte_st voltage[4];
extern volatile struct twobyte_st speed[4];
extern volatile struct twobyte_st acceleration[4];

extern volatile unsigned char fpga_in;
extern volatile unsigned char recv_bytenum;

extern volatile struct twobyte_st ticks[4];
extern volatile unsigned char motor_load[4];

// On 48 Mhz we do 48 000 000 cycles per second
// or 48 cycles per microsecond
#define udelay(a) __delay_cycles(48UL*a);


