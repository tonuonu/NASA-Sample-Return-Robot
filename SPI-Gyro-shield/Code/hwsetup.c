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
#include "gyro.h"


volatile struct statuses status;
unsigned int base_freq;

static void
ConfigureOperatingFrequency(char mode) {
    unsigned short i;

    prr = 0xAA;
    ccr = 0x1F;
    prr = 0x00;
    prc0 = 1;
    pm3 = 0x40;                                        // peripheral clock 24MHz
    prc0 = 0;
    prc2 = 1;
    *(unsigned short *) &plc0 = 0x0226;                // 48MHz, PLL = 96MHz
    prc2 = 0;
    base_freq = 24000000;

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

/*
void 
OLED_On(void) {
}
*/

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
    
    LEFT_INAd    = PD_OUTPUT;
    LEFT_INBd    = PD_OUTPUT;
    RIGHT_INAd   = PD_OUTPUT;
    RIGHT_INBd   = PD_OUTPUT;

    LEFT_INA    = 0;
    LEFT_INB    = 0;
    RIGHT_INA   = 0;
    RIGHT_INB   = 0;

    LEFT_ENAd  = PD_OUTPUT;
    LEFT_ENBd  = PD_OUTPUT;
    RIGHT_ENAd = PD_OUTPUT;
    RIGHT_ENBd = PD_OUTPUT;

    LEFT_ENA   = 0;
    LEFT_ENB   = 0;
    RIGHT_ENA  = 0;
    RIGHT_ENB  = 0;

    LEFT_PWMd    = PD_OUTPUT;
    LEFT_PWMs    = PF_TIMER;
    RIGHT_PWMd   = PD_OUTPUT;
    RIGHT_PWMs   = PF_TIMER;
    LEFT_PWM     = 0;
    RIGHT_PWM    = 0;

    // FIXME, timer3 start into right place
//    TABSR_bit.TA3S = 1;
}

static void 
Heartbeat_Init(void) {
    // Init_TMRB5 1 mS timer
    tb5mr = 0x80;                       // timer mode,fc/8 = 1,0 MHz
    tb5 = 5000;                         // 1MHz/5000 ; Fi = 200Hz
    tb5ic = 1;                          // level 1 interrupt
    tb5s = 1;
    ticks = 0;
}
#if 0
static void 
Buzzer_Init(void) {
    BUZZERs = PF_MOTOR;
    BUZZERd = PD_OUTPUT; 
}

static void 
Panda_Init(void) {
    PANDAd = PD_OUTPUT; 
    PANDA  = 0;
}

static void 
Coilgun_Init(void) {
    KICKd = PD_OUTPUT; 
    KICK = 1; // Drive it up for normal mode
    pu22 = 1; // ball detect p7_3 pullup on
}

static void 
CapacitorCharger_Init(void) {
    CHARGE = 0;
    CHARGEd = PD_OUTPUT; 
}

static void 
Analog_Init(void) {
    ch0_ad0con0  = 0; // does not matter in sweep mode       
    ch1_ad0con0  = 0; // does not matter in sweep mode           
    ch2_ad0con0  = 0; // does not matter in sweep mode
    md0_ad0con0  = 1; // repeat sweep mode          
    md1_ad0con0  = 1; // repeat sweep mode
    trg_ad0con0  = 0; // software tells when to start, not HW            
    adst_ad0con0 = 1; // we set this later to 1 when we start AD conversion to start
    cks0_ad0con0 = 0; // divide by 8   
    
    scan0_ad0con1 = 1; // read AN_0 to AN_3
    scan1_ad0con1 = 1; // read AN_0 to AN_3   
    bits_ad0con1  = 1; // 10 bit resolution
    vcut_ad0con1  = 1; // Disable reference voltage to save power
    md2_ad0con1   = 1; // repeat sweep mode         
    cks1_ad0con1  = 0; // divide by 8
    opa0_ad0con1  = 0; // External op amp not used
    opa1_ad0con1  = 0; // External op amp not used

    smp_ad0con2  = 0; // with sample and hold
    aps0_ad0con2 = 0; // an_0 to an_7
    aps1_ad0con2 = 0; // an_0 to an_7
    trg0_ad0con2 = 0; // does not matter

    dus_ad0con3    = 0; // no DMA
    mss_ad0con3    = 0; // not multi-port sweep mode
    cks2_ad0con3   = 1; // divide by 8
    msf0_ad0con3   = 0; // multi port sweep status flag. Disabled.
    msf1_ad0con3   = 0; // multi port sweep status flag. Disabled.

    ad0con4 = 0;     
    AN00s = PF_ANALOG;
    AN01s = PF_ANALOG;
    AN02s = PF_ANALOG;
    AN03s = PF_ANALOG;

    AN00d = PD_INPUT;
    AN01d = PD_INPUT;
    AN02d = PD_INPUT;
    AN03d = PD_INPUT;

    AN00 = 0;
    AN01 = 0;
    AN02 = 0;
    AN03 = 0;

}

static void 
Joy_Init(void) {
    pu02 = 1; // P1_0 to P1_3 Pull-up Control Bit
    pu03 = 1; // P1_4 to P1_7 Pull-up Control Bit
}
#endif

static void
Oneshot_Init(void) {
    // Timer 0 for SPI7
  
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
    ilvl_ta0ic  = 1; 
    ir_ta0ic    = 0;            
    ENABLE_IRQ
      
    // Timer 3 for SPI4
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
    ilvl_ta3ic  = 1;
    ir_ta3ic    = 0;            
    ENABLE_IRQ
}

void
HardwareSetup(void) {
    /* 
     * Configures CPU clock 
     */
    DISABLE_IRQ;
    ConfigureOperatingFrequency(1);
    LED1d  = PD_OUTPUT;
    LED2d  = PD_OUTPUT;
    LED3d  = PD_OUTPUT;
    LED4d  = PD_OUTPUT;
    LED5d  = PD_OUTPUT;

    //Buzzer_Init();
    Heartbeat_Init();
//    ERRORLEDd=PD_OUTPUT; // error LED
//    MELEXIS_ENd=PD_OUTPUT;
//    MELEXIS_EN=0; // Low is ON, This is 5V LDO
    //CapacitorCharger_Init();
    //Coilgun_Init();
    //Panda_Init();
    //Joy_Init();
    // OLED init must be before gyro or OLED does not init properly
    // Pullup pm p4_4, pu11 tries to  make it high while initializing gyro 
    // and messes up our OLED init later.    OLED_Init(); 

#if 0
    SPI2_Init();  // Accel sensor
    SPI3_Init();  // OLED
    SPI4_Init();  // Melexis sensor left
    SPI6_Init();  // gyro
    SPI7_Init();  // Melexis sensor right
#endif
    ENABLE_IRQ;

    //Analog_Init();
    PWM_Init();
    Oneshot_Init();
}

