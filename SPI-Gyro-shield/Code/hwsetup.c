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

#if 0
static void 
PWM_Init(void) {
    /* 
     * Removes Protection for INVC0 & INVC1 Registers 
     */
    prc1 = 1;

    /* 
     * Three-Phase PWM Control Register0 - 00001100b b1:b0 INV01:INV00
     * ICTB2 Count Condition: Underflow of timer B2 b2 INV02 Use
     * Three-Phase Motor Control Timers b3 INV03 Enable Three-Phase Motor 
     * Control Timer Output b4 INV04 Ignore Simultaneous turn-on Signal
     * output b5 INV05 Simultaneous Conduction Not Detected b6 INV06
     * Triangular Wave Modulation b7 INV07 Software Trigger Select Bit 
     */
    invc0 = 0x1C;

    prc1 = 1;
    /* 
     * Three-Phase PWM Control Register1 - 00110000b b0 INV10 Timers A1,
     * A2 & A4 Trigger : Underflow of Timer B2 b1 INV11 Timers A1-1, A2-1 
     * & A4-1 Control : Thee-phase mode 0 b2 INV12 Dead Time Timer Count
     * Source : f1 b3 INV13 Timer A reload control signal is 0 b4 INV14
     * Active High output b5 INV15 Dead Time Disable b6 INV16 Dead Time
     * Timer Trigger : Rising edge of the three-phase output shift
     * register b7 - Reserved 
     */
    invc1 = 0x40;

    /* 
     * Three-Phase Output Use pins U, U, V, V, W, and W of port P3 
     */
    tbsout = 1;

    /* 
     * Protects INVC0 & INVC1 Registers 
     */
    prc1 = 0;

    /* 
     * Timer B2 Reload every time an underflow occure 
     */
    tb2sc = 0x00;

    /* 
     * Three-Phase Output Buffer Register0 - 00111111b b0 DU0 U-Phase
     * Output Buffer0 ON b1 DUB0 U'-Phase Output Buffer0 ON b2 DV0
     * V-Phase Output Buffer0 ON b3 DVB0 V'-Phase Output Buffer0 ON
     * b4 DW0 W-Phase Output Buffer0 ON b5 DWB0 W'-Phase Output
     * Buffer0 OFF b7:b6 - Reserved 
     */
    idb0 = 0x1A;

    /* 
     * Three-Phase Output Buffer Register1 - 00000000b b0 DU1 U-Phase
     * Output Buffer1 OFF b1 DUB1 U'-Phase Output Buffer1 OFF b2 DV1 
     * V-Phase Output Buffer1 OFF b3 DVB1 V'-Phase Output Buffer1 OFF 
     * b4 , DW1 W-Phase Output Buffer1 OFF b5 DWB1 W'-Phase Output
     * Buffer1 OFF b7:b6 - Reserved 
     */
    idb1 = 0x25;

    /* 
     * Timer A4 Mode Register - 00010001b b1:b0 TMOD0:TMOD1 One shot
     * mode b2 MR0 Reserved b3 MR1 External Trigger Select Bit b4
     * MR2 Trigger selected by TRGSR register b5 MR3 Reserved b6:b7
     * TCK0:TCK1 Count Source f1 
     */
    ta4mr = 0x13; // Toon! Mootori reziim, t2itetegurit muudetakse

    /* 
     * Timer A1 Mode Register - 00010001b b1:b0 TMOD0:TMOD1 One shot
     * mode b2 MR0 Reserved b3 MR1 External Trigger Select Bit b4
     * MR2 Trigger selected by TRGSR register b5 MR3 Reserved b6:b7
     * TCK0:TCK1 Count Source f1 
     */
    ta1mr = 0x12;

    /* 
     * Timer A2 Mode Register - 00010001b b1:b0 TMOD0:TMOD1 One shot
     * mode b2 MR0 Reserved b3 MR1 External Trigger Select Bit b4
     * MR2 Trigger selected by TRGSR register b5 MR3 Reserved b6:b7
     * TCK0:TCK1 Count Source f1 
     */
    ta2mr = 0x12;

    /* 
     * Timer B2 Mode Register - 00000000b b1:b0 TMOD0:TMOD1 Timer
     * Mode b2:b3 mr0:mr1 Reserved b4 mr2 Reserved b5 mr3 Reserved
     * b6:b7 TCK0:TCK1 Count Source f1 
     */
    tb2mr = 0x00;

    /* 
     * Trigger Select Register - 01000101b b1:b0 TA1TGL:TA1TGH Timer
     * A1 Trigger the underflow of TB2 b2:b3 TA2TGL:TA2TGH Timer A2
     * Trigger the underflow of TB2 b4:b5 TA3TGL:TA3TGH Timer A3
     * Trigger : input to TA3IN pin b6:b7 TA4TGL:TA4TGH Timer A4
     * Trigger the underflow of TB2 
     */
    trgsr = 0x45;

    /* 
     * Configures 3-Phase motor control timers 
     */
    /* 
     * Loading the register 
     */
    tb2 = TIMERB2COUNT;

    /* 
     * Timer A4 register 
     */
    ta1 = 0;
    ta2 = 0;
    ta4 = 0;

    /* 
     * Setting timer A4, timer A1, timer A2 and timer B2 start flag 
     */
    tabsr = 0x96;

    /* Configure outputs */
#if 0    
    LEFT_PWMd    = PD_OUTPUT;
    LEFT_PWMs    = PF_TIMER;
    RIGHT_PWMd   = PD_OUTPUT;
    RIGHT_PWMs   = PF_TIMER;
    LEFT_PWM     = 0;
    RIGHT_PWM    = 0;
#endif
    // FIXME, timer3 start into right place
    // TABSR_bit.TA3S = 1;
}


static void 
Heartbeat_Init(void) {
    // Init_TMRB5 1 mS timer
    ticks = 0;
    tb5mr = 0x80;                       // timer mode,fc/8 = 1,0 MHz
    tb5 = 5000;                         // 1MHz/5000 ; Fi = 200Hz
    tb5ic = 1;                          // level 1 interrupt
    tb5s = 1;
}

static void
Oneshot_Init(void) {
    // Timer 0 for ..?
  
    tmod0_ta0mr = 0; // One shot timer mode
    tmod1_ta0mr = 1; // One shot timer mode
    tck0_ta0mr  = 0; // f1 clock source    
    tck1_ta0mr  = 1; // f1 clock source    
    mr2_ta0mr   = 0; // Start on ta0os bit
    ta0         = 1;
    ta0s        = 5; // start counter
    DISABLE_IRQ
    /* 
     * Lowest interrupt priority
     * we do not care about speed
     */
    //ilvl_ta0ic  = 1; 
    ir_ta0ic    = 0;            
    ENABLE_IRQ
      
    // Timer 3 for ...?
    tmod0_ta3mr = 0; // One shot timer mode
    tmod1_ta3mr = 1; // One shot timer mode
    tck0_ta3mr  = 0; // f1 clock source    
    tck1_ta3mr  = 1; // f1 clock source    
    mr2_ta3mr   = 0; // Start on ta0os bit
    ta3         = 1;
    ta3s        = 5; // start counter
    DISABLE_IRQ
    /* 
     * Lowest interrupt priority
     * we do not care about speed
     */
    //ilvl_ta3ic  = 1;
    ir_ta3ic    = 0;            
    ENABLE_IRQ
}
#endif

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

    //PWM_Init();
    //Oneshot_Init();
}

