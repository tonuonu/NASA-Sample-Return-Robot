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
    /*
     * Transmit Buffer Empty Flag 
     * 0: Data held in the UiTB register
     * 1: No data held in the UiTB register
     */
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

static inline void 
complete_rx(void) {
    /*
     * RI (Receive Complete Flag)
     * 0: No data held in the UiRB register 
     * 1: Data held in the UiRB register 
     */
    while((ri_u0c1 == 0) ||
          (ri_u3c1 == 0) ||
          (ri_u4c1 == 0) ||
          (ri_u6c1 == 0));
}

enum cmds_e {
    CMD_NONE=0, 
    CMD_SPEED=0x04, 
    CMD_ACCELERATION=0x08, 
    CMD_GET_CUR_TARGET_SPEED=0x10,
    CMD_GET_VOLTAGE=0x18,
    CMD_CALIBRATE=0x70
} ;

/*
 * This union is used in every context where we need to receive two separate 
 * bytes into single short int (16 bit signed)
 */
struct twobyte_st {
    union {
        uint8_t byte[2];
        int16_t int16;
    } u;
};

extern struct twobyte_st voltage[3][4];
extern struct twobyte_st cur_target_speed[3][4];

extern volatile struct twobyte_st cur_cmd_param[4];
extern volatile unsigned char cur_cmd[4];

extern volatile struct twobyte_st ticks[4];
extern volatile unsigned char motor_load[4];

extern volatile unsigned char fpga_in;
extern volatile unsigned char recv_bytenum;

int16_t calc_median3(const struct twobyte_st values[3][4],
											const unsigned int motor_idx);

// On 48 Mhz we do 48 000 000 cycles per second
// or 48 cycles per microsecond
#define udelay(a) __delay_cycles(48UL*a);
