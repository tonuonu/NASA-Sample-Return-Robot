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

#define myCKPOL (0)
#define myCKPH (1)
#define myIOPOL (1)

#define PD_INPUT    (0)
#define PD_OUTPUT   (1)
#define PF_TIMER    (1)
#define PF_MOTOR    (2)
#define PF_UART     (3)
#define PF_ANALOG   (0x80)

void HardwareSetup(void);

#define LED1         p1_0
#define LED1d        pd1_0
#define LED2         p1_1
#define LED2d        pd1_1
#define LED3         p1_2
#define LED3d        pd1_2
#define LED4         p1_3
#define LED4d        pd1_3
#define LED5         p1_4
#define LED5d        pd1_4

#define RESET0       p0_3
#define RESET0d      pd0_3
#define RESET1       p0_2
#define RESET1d      pd0_2
#define RESET2       p0_1
#define RESET2d      pd0_1
#define RESET3       p0_0
#define RESET3d      pd0_0

#define CDONE0       p0_7
#define CDONE1       p0_4
#define CDONE2       p0_6
#define CDONE3       p0_5

#define MOTOR_ERROR   p5_6
#define MOTOR_ERRORd   pd5_6

#define RESET5       p8_2
#define RESET5d      pd8_2


// SPI0, motor 0
#define CS0d         pd6_0
#define CS0          p6_0
#define CS0s         p6_0s
#define CLOCK0       p6_1
#define CLOCK0d      pd6_1
#define CLOCK0s      p6_1s
#define RX0          p6_2
#define RX0s         p6_2s
#define TX0          p6_3
#define TX0s         p6_3s
#define TX0d         pd6_3

// SPI3, motor 1
#define CS1d         pd4_0
#define CS1          p4_0
#define CS1s         p4_0s
#define CLOCK3       p4_1
#define CLOCK3d      pd4_1
#define CLOCK3s      p4_1s
#define RX3          p4_2
#define RX3s         p4_2s
#define RX3d         pd4_2
#define TX3          p4_3
#define TX3s         p4_3s
#define TX3d         pd4_3

// SPI4, motor 2
#define CS2         p9_4
#define CS2d        pd9_4
#define CLOCK4      p9_5
#define CLOCK4d     pd9_5
#define CLOCK4s     p9_5s
#define RX4         p9_7
#define RX4s        p9_7s
#define TX4         p9_6
#define TX4s        p9_6s
#define TX4d        pd9_6

// SPI6, motor 3
#define CS3d         pd7_3
#define CS3          p7_3
#define CS3s         p7_3s
#define CLOCK6       p4_5
#define CLOCK6d      pd4_5
#define CLOCK6s      p4_5s
#define RX6          p4_6
#define RX6s         p4_6s
#define RX6d         pd4_6
#define TX6          p4_7
#define TX6s         p4_7s
#define TX6d         pd4_7

// Serial 5

#define CS5         p8_4
#define CS5d        pd8_4
#define CS5s        p8_4s
#define CLOCK5      p7_7
#define CLOCK5d     pd7_7
#define CLOCK5s     p7_7s

#define TX5         p7_6
#define TX5s        p7_6s
#define TX5d        pd7_6
#define RX5         p8_0
#define RX5d        pd8_0
#define RX5s        p8_0s
#define TX5         p7_6
#define TX5d        pd7_6
#define TX5s        p7_6s

#define M0TX        u0tb
#define M1TX        u3tb
#define M2TX        u4tb
#define M3TX        u6tb

#define M0RX        u0rb
#define M1RX        u3rb
#define M2RX        u4rb
#define M3RX        u6rb


#define SPI_DELAY (1)


// Serial 1 (debug output)

#define TX1         p6_7
#define TX1s        p6_7s
#define TX1d        pd6_7
#define RX1         p6_6
#define RX1d        pd6_6
#define RX1s        p6_6s


struct statuses {
    char sek_flag;
};
void Read_AD(void);

extern volatile struct statuses status;
extern unsigned int base_freq;

