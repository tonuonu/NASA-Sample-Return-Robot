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
#include <stdlib.h>
#include "intrinsics.h"
#define PI 3.1415926535897932384626433832795028841971693993751058

extern volatile unsigned short ticks;

void uart0_init(void);
void SPI3_send_cmd(unsigned char c);
void SPI3_send_data(unsigned char c);

short unsigned SPI6_receive(void) ;
extern double twist[6];
extern int pwm[2];
extern int pwmtarget[2];
extern int accok, accwhoami; 
extern int accstatus;

extern unsigned char gyrowhoami;
extern signed int gyrox,gyroy,gyroz;
extern signed int gyrorawx,gyrorawy,gyrorawz;
extern int gyromaxx,gyromaxy,gyromaxz;
extern int gyrominx,gyrominy,gyrominz;

extern signed char gyrotemp;
extern int gyrowhoamistatus;
extern int buzzer;
extern float bat, capacitor, leftmotorcurrent , rightmotorcurrent ;

extern signed int MLXaccumulatorL;
extern signed int MLXaccumulatorR;
extern float turnsincetwist[3];
extern char text[8][150];
extern int lineno;
extern int textpos;

enum {
  MODE_MANUAL,
  MODE_COMPETITION,
  MODE_DEBUG_ACCSENSOR,
  MODE_DEBUG_POSSENSOR,
  MODE_DEBUG_GYRO,
};
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

extern int twistcmdage;
//extern int xxx;
/* 
 * Next constant is from 
 * http://www.wolframalpha.com/input/?i=8.75millidegrees+per+second
 */
#define GYRORATE (0.0001527)
extern float yaw;

