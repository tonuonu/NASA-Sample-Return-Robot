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
#include "hwsetup.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include "main.h"
#include "SPI.h"


volatile struct statuses status;
unsigned int base_freq;

static void
ConfigureOperatingFrequency(char mode) {
    unsigned short i;

    prr = 0xAA;
    ccr = 0x1F; // 0001 1111
    prr = 0x00;
    prc0 = 1;
    pm3 = 0x60; // peripheral clock 48MHz
    prc0 = 0;
    prc2 = 1;
//    *(unsigned short *) &plc0 = 0x0226; // 48MHz, PLL = 96MHz
    *(unsigned short *) &plc0 = 0x0104; // 48MHz, PLL = 96MHz
    prc2 = 0;
    base_freq = 25000000;

    for (i = 0; i < 0x8000u; i++);                         /* Add delay
                                                            * for PLL to
                                                            * stabilise. */
    /* 
     * Disable the port pins 
     */
    pd8_7 = 0;
    pd8_6 = 0;

    /* 
     * Disable the pull-up resistor 
     */
    pu25 = 0;

    /* 
     * Enable writing to CM0 
     */
    prc0 = 1;

    /* 
     * Start the 32KHz crystal 
     */
    cm04 = 1;

    /* 
     * Disable writing to CM0 
     */
    prc0 = 0;

    /* 
     * Enable writing to PM2 
     */
    prc1 = 1;
    /* 
     * Disable clock changes 
     */
    pm21 = 1;
    pm26 = 1;                                              
    /* 
     * Disable writing to PM2 
     */
    prc1 = 0;
    cst_tcspr = 0;
    tcspr = 0x08;
    cst_tcspr = 1;                                        
    cnt0_tcspr = 1; // f2n is 24Mhz / 16
    cnt1_tcspr = 1; // f2n is 24Mhz / 16
    cnt2_tcspr = 1; // f2n is 24Mhz / 16
    cnt3_tcspr = 1; // f2n is 24Mhz / 16
}

static void 
Led_Init(void) {
    LED1d  = PD_OUTPUT;
    LED2d  = PD_OUTPUT;
    LED3d  = PD_OUTPUT;
    LED4d  = PD_OUTPUT;
    LED5d  = PD_OUTPUT;
    LED1=LED2=LED3=LED4=0;
}

static void 
MotorIO_Init(void) {
    RESET0d = PD_OUTPUT;
    RESET1d = PD_OUTPUT;
    RESET2d = PD_OUTPUT;
    RESET3d = PD_OUTPUT;

    RESET0=1;
    RESET1=1;
    RESET2=1;
    RESET3=1;
}

void
HardwareSetup(void) {
    /* 
     * Configures CPU clock 
     */
    DISABLE_IRQ;
    ConfigureOperatingFrequency(1);
    ENABLE_IRQ;
    pu26=1; // Just to make sure unused P9_1 and P9_3 are not floating
    Led_Init();
    MotorIO_Init(); // Reset and DONE pins. SPI is separate
    //Heartbeat_Init();
    
    ifsr00=1; // INT0 in both edges. RESET input from Atmega2560 for motors
    ifsr02=1; // INT2 in both edges. CS pin for us
    pol_int0ic  = 0; // This should be 0, "falling edge" to make both edges work
    pol_int2ic  = 0; // This should be 0, "falling edge" to make both edges work
    ilvl_int0ic = 6; // level 6 int, very high
    ilvl_int2ic = 6; // level 6 int, very high
    lvs_int0ic  = 0; // edge sensitive
    lvs_int2ic  = 0; // edge sensitive

    /* Four motor ports, masters */
    SPI0_Init(); 
    SPI2_Init(); 
    SPI3_Init(); 
    SPI4_Init();

    /* Interface to Arduino, slave */
    SPI5_Init();

}
