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

#include <stdlib.h>
#include "intrinsics.h"
#define PI 3.1415926535897932384626433832795028841971693993751058

enum {FPGA_EMPTY,FPGA_LOADING,FPGA_LOADED};
enum {CMD_NONE=0, CMD_STEADY_SPEED=0x04, CMD_TARGET_ACCELERATION=0x08, CMD_GET_CUR_TARGET_SPEED=0x10 };

extern volatile unsigned char steady_speed[2];
extern volatile unsigned char target_acceleration[2];

extern volatile unsigned char recv_buf;
//extern volatile unsigned char recv_flag;
extern volatile unsigned char fpga_in;
extern volatile unsigned short ticks;

/*
 * IAR-HEW compatibility 
 */
#define _asm asm

/*
 * 1 cycle delay 
 */
#define	NOP()			{_asm("nop");}

static inline void
uDelay(unsigned char l) {
    while (l--)
        NOP();

}

static inline void udelay(unsigned int usec) {
    // On 48 Mhz we do 48 000 000 cycles per second
    // or 48 cycles per microsecond
    __delay_cycles(48UL*(unsigned long)usec);
}

#define ENABLE_IRQ   	{_asm("FSET I");}
#define DISABLE_IRQ	{_asm("FCLR I");}

void
Delay(unsigned char n);

size_t __write(int, unsigned const char*, size_t);
size_t __read(int Handle, unsigned char *Buf, size_t BufSize);


