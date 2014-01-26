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

/* Following header file provides standard integer type definitions. */
#include <stdint.h>
#include <stdio.h>
/* Following header file provides string type definitions. */
#include <string.h>

#include "rskrx630def.h"
#include "iorx630.h"
#include "switch.h"
#include "pwm.h"
#include <stdint.h>

/* Callback function prototype for switch presses */
void CB_Switch_Press(void);

/* Variable for storing the event count */
volatile uint16_t gDutyPercentage;
/* Variable for halting updates and displaying the duty cycle */
//volatile bool gVaryDutyCycle = true;


void Init_PWM(void) {
#if 0
    /* 
     * RGB LED is connected to P13 (blue), P15 (green) and P17 (red). We use PWM 
     * to drive them
     */
    /* Configure pin 7 of port 1 as an output */
    PORT1.PDR.BIT.B3 = 1;
    PORT1.PDR.BIT.B5 = 1;
    PORT1.PDR.BIT.B7 = 1;
  
    /* Configure pin 7 of port 1 for peripheral function */
    PORT1.PMR.BIT.B3 = 1;
    PORT1.PMR.BIT.B5 = 1;
    PORT1.PMR.BIT.B7 = 1;
#endif  
    /* Enable write to PFSWE bit */
    MPC.PWPR.BYTE = 0x00;
  
    /* Disable write protection to PFS registers */
    MPC.PWPR.BYTE = 0x40;
#if 0  
    /* Configure pin 7 of port 1 for MTIOC3A outputs */
    MPC.P13PFS.BIT.PSEL = 0x1;
    MPC.P15PFS.BIT.PSEL = 0x1;
    MPC.P17PFS.BIT.PSEL = 0x1;
#endif  
    /* Enable write protection to PFS registers */
    MPC.PWPR.BYTE = 0x80;
  
    /* Protection off */
    SYSTEM.PRCR.WORD = 0xA503;
  
    /* Cancel the MTU3 module clock stop mode */
    MSTP_MTU3 = 0x0;
    
    /* Protection on */
    SYSTEM.PRCR.WORD = 0xA500;
              
    /* Set MTU3 TGIA3 interrupt priority level to 7 */  
    IPR(MTU3,TGIA3) = 0x7;
    /* Enable MTU3 TGIA3 interrupts */
    IEN(MTU3,TGIA3) = 0x1;
    /* Clear MTU3 TGIA3 interrupt flag */
    IR(MTU3,TGIA3) = 0x0;
  
    /* Set MTU3 TGIB3 interrupt priority level to 7 */  
    IPR(MTU3,TGIB3) = 0x7;
    /* Enable MTU3 TGIB3 interrupts */
    IEN(MTU3,TGIB3) = 0x1;
    /* Clear MTU3 TGIB3 interrupt flag */
    IR(MTU3,TGIB3) = 0x0;
  
    /* Clock PCLK/1=48MHz, count at falling edge, 
       TCNT cleared by TGRA compare match */
    MTU3.TCR.BYTE = 0x28;
    
    /* Operate timers in pwm mode */
    MTU3.TMDR.BYTE = 0x02;
  
    /* Initial MTIOC3A output is 1 and toggled at 
       TGRA compare match. TRGB output is disabled. */
    MTU3.TIORH.BYTE = 0x37;
    
    /* Enable TGIEA interrupts */
    MTU3.TIER.BYTE = 0x01;
  
    /* Enable access to protected MTU registers */
    MTU.TRWER.BIT.RWE = 0x1;
  
    /* Use TGRA to set the pwm cycle to 1KHz*/
    MTU3.TGRA = 48000;
  
    /* TGRB used to set the pwm duty cycle to 90% */
    MTU3.TGRB = 4800;
  
    /* Set a switch press callback function */
    SetSwitchReleaseCallback(CB_Switch_Press);
//LED5=LED_ON;
    /* Start the TCNT counter */  
    MTU.TSTR.BIT.CST3 = 0x1;

  /***************/
  
    /*
     * Battery charger PWMs init
     */
    SYSTEM.PRCR.WORD = 0xA502;
    SYSTEM.MSTPCRA.BIT.MSTPA13=0; // Get module TPU0...TPU5 out of stop mode

    /* Configure pins of port E for peripheral function */
    PORTA.PMR.BIT.B0 = 1;
    PORTA.PMR.BIT.B4 = 1;
    PORTA.PMR.BIT.B6 = 1;
    PORTB.PMR.BIT.B0 = 1;

    /* Configure pins of port E as an output */
    PORTA.PDR.BIT.B0 = 1;
    PORTA.PDR.BIT.B4 = 1;
    PORTA.PDR.BIT.B6 = 1;
    PORTB.PDR.BIT.B0 = 1;

    MPC.PWPR.BIT.B0WI=0; // Enable writing to PFSWE
    MPC.PWPR.BIT.PFSWE=1;// Enable writing to PFS registers

    /* Configure pins of ports for TIOCAx outputs */
    MPC.PA0PFS.BIT.PSEL = 0x3; // TIOCA0
    MPC.PA4PFS.BIT.PSEL = 0x3; // TIOCA1
    MPC.PA6PFS.BIT.PSEL = 0x3; // TIOCA2
    MPC.PB0PFS.BIT.PSEL = 0x3; // TIOCA3

    MPC.PWPR.BIT.PFSWE=0;// Enable writing to PFS registers
    MPC.PWPR.BIT.B0WI=1; // Enable writing to PFSWE
  
    /* Protection off */
    SYSTEM.PRCR.WORD = 0xA503;

    TPUA.TSTR.BIT.CST0=0;// stop clock
    TPUA.TSTR.BIT.CST1=0;// stop clock
    TPUA.TSTR.BIT.CST2=0;// stop clock
    TPUA.TSTR.BIT.CST3=0;// stop clock
  
    /* Protection on */
    SYSTEM.PRCR.WORD = 0xA500;

    TPUA.TSYR.BIT.SYNC0=1; // TCNT performs synchronous operation
    TPUA.TSYR.BIT.SYNC1=1; // --""--
    TPUA.TSYR.BIT.SYNC2=1; // --""--
    TPUA.TSYR.BIT.SYNC3=1; // --""--
  
    TPU0.TCR.BIT.TPSC=1; // input PCLK/4
    TPU1.TCR.BIT.TPSC=1; // --""--
    TPU2.TCR.BIT.TPSC=1; // --""--
    TPU3.TCR.BIT.TPSC=1; // --""--

    TPU0.TCR.BIT.CKEG=0; // Count only falling edges
    TPU1.TCR.BIT.CKEG=0; // --""--
    TPU2.TCR.BIT.CKEG=0; // --""--
    TPU3.TCR.BIT.CKEG=0; // --""--

    // page 757
    TPU0.TCR.BIT.CCLR=2; // TCNT counter cleared by TGRB compare match/input capture
    TPU1.TCR.BIT.CCLR=3; // TCNT counter cleared by counter clearing for another channel performing synchronous clearing/synchronous operation
    TPU2.TCR.BIT.CCLR=3; // --""--
    TPU3.TCR.BIT.CCLR=3; // --""--

    /* Read hardware manual chapters "24.3.5 PWM Modes" and 
     * "Table 24.24 PWM Output Registers and Output Pins" for poor explanation
     */
    TPU0.TMDR.BIT.MD = 2; // PWM mode 1  
    TPU1.TMDR.BIT.MD = 2; // PWM mode 1  
    TPU2.TMDR.BIT.MD = 2; // PWM mode 1
    TPU3.TMDR.BIT.MD = 2; // PWM mode 1
  
    /* 
     * In PWM mode 1 we use both TPUx.TGRA and TPUx.TGRB to trigger waveform 
     * of TIACAx pin, not TIACBx as you may think after reading hardware manual
     * about TGRB register function.
     * TPUx.TIORH.BIT.IOA=2; 
     * TIOCAx function, output compare of TPUx.TGRA: 
     * Initial output is low output; high output at compare match. 
     * TPUx.TIORH.BIT.IOB=5; 
     * TIOCAx function, output compare of TPUx.TGRB: 
     * Initial output is high output; low output at compare match
     */
    TPU0.TIORH.BIT.IOA=2; 
    TPU0.TIORH.BIT.IOB=5;
  
    TPU1.TIOR.BIT.IOA=2;
    TPU1.TIOR.BIT.IOB=5;
  
    TPU2.TIOR.BIT.IOA=2;
    TPU2.TIOR.BIT.IOB=5;
  
    TPU3.TIORH.BIT.IOA=2;
    TPU3.TIORH.BIT.IOB=5;

    // TPU0.TIER.BIT.TCIEU; // Cool interrupt stuff
  
    // 48Mhz/4/400 == 30000Hz
    /* Set a period */
   
#define length0 1
#define length1 0
#define length2 1
#define length3 0
  
#if 1 // we just play around now
    TPU3.TGRA = 100-length3;
    TPU3.TGRB = 100;

    TPU2.TGRA = 200-length2;
    TPU2.TGRB = 200;

    TPU1.TGRA = 300-length1;
    TPU1.TGRB = 300;

    TPU0.TGRA = 400-length0;
    TPU0.TGRB = 400;
#else // this will be in final code for safety
    TPU3.TGRA = 0;
    TPU3.TGRB = 0;

    TPU2.TGRA = 0;
    TPU2.TGRB = 0;

    TPU1.TGRA = 0;
    TPU1.TGRB = 0;

    TPU0.TGRA = 400;
    TPU0.TGRB = 400;
#endif
  
    SYSTEM.PRCR.WORD = 0xA503;
    TPUA.TSTR.BIT.CST0=1; // start counter
    TPUA.TSTR.BIT.CST1=1; // start counter
    TPUA.TSTR.BIT.CST2=1; // start counter
    TPUA.TSTR.BIT.CST3=1; // start counter
    /* Protection on */
    SYSTEM.PRCR.WORD = 0xA500;

    /*
     * Servo PWMs init
     */
    /* Configure pins of port E for peripheral function */

#if 1
    PORTB.PMR.BIT.B4 = 1;
    PORTB.PMR.BIT.B6 = 1;

    /* Configure pins of port E as an output */
    PORTB.PDR.BIT.B4 = 1;
    PORTB.PDR.BIT.B6 = 1;

    MPC.PWPR.BIT.B0WI=0; // Enable writing to PFSWE
    MPC.PWPR.BIT.PFSWE=1;// Enable writing to PFS registers

    /* Configure pins of ports for TIOCxA outputs */
    MPC.PB4PFS.BIT.PSEL = 0x3;
    MPC.PB6PFS.BIT.PSEL = 0x3;
#endif

    MPC.PWPR.BIT.PFSWE=0;// Enable writing to PFS registers
    MPC.PWPR.BIT.B0WI=1; // Enable writing to PFSWE
    /* Protection off */
    SYSTEM.PRCR.WORD = 0xA503;

    TPUA.TSTR.BIT.CST4=0;// stop clock TPU4
    TPUA.TSTR.BIT.CST5=0;// stop clock TPU5
  
    /* Protection on */
    SYSTEM.PRCR.WORD = 0xA500;
  
    TPU4.TCR.BIT.TPSC=3; // input PCLK/64
    TPU5.TCR.BIT.TPSC=3; // --""--

    TPU4.TCR.BIT.CKEG=0; // Count only falling edges
    TPU5.TCR.BIT.CKEG=0; // --""--

    // page 757
    TPU4.TCR.BIT.CCLR=2; // TCNT counter cleared by TGRB compare match/input capture
    TPU5.TCR.BIT.CCLR=2; // TCNT counter cleared by TGRB compare match/input capture

    /* Read hardware manual chapters "24.3.5 PWM Modes" and 
     * "Table 24.24 PWM Output Registers and Output Pins" for poor explanation
     */
    TPU4.TMDR.BIT.MD = 2; // PWM mode 1  
    TPU5.TMDR.BIT.MD = 2; // PWM mode 1  
  
    /* 
     * In PWM mode 1 we use both TPUx.TGRA and TPUx.TGRB to trigger waveform 
     * of TIACAx pin, not TIACBx as you may think after reading hardware manual
     * about TGRB register function.
     * TPUx.TIORH.BIT.IOA=2; 
     * TIOCAx function, output compare of TPUx.TGRA: 
     * Initial output is low output; high output at compare match. 
     * TPUx.TIORH.BIT.IOB=5; 
     * TIOCAx function, output compare of TPUx.TGRB: 
     * Initial output is high output; low output at compare match
     */
    TPU4.TIOR.BIT.IOA=2; 
    TPU4.TIOR.BIT.IOB=5;
  
    TPU5.TIOR.BIT.IOA=2;
    TPU5.TIOR.BIT.IOB=5;
    
    /* 
     * http://en.wikipedia.org/wiki/Servo_control 
     * recommends using 20ms (50Hz) cycle.
     * 48Mhz/64/15000 == 50Hz
     * Same page says 1.5ms high pulse keeps servo at middle 
     * 15000/20*1.5=1125
     * Side limits are 1 and 2ms (750 and 1500).
     */
    TPU4.TGRA = 15000-1125;
    TPU4.TGRB = 15000;

    TPU5.TGRA = 15000-1125;
    TPU5.TGRB = 15000;

    SYSTEM.PRCR.WORD = 0xA503;
    TPUA.TSTR.BIT.CST4=1; // start counter
    TPUA.TSTR.BIT.CST5=1; // start counter
    /* Protection on */
    SYSTEM.PRCR.WORD = 0xA500;
}

/*******************************************************************************
* Outline      : CB_Switch_Press
* Description  : Switch Press callback function. This function checks which 
*         switch was pressed. The second line of the debud LCD is cleared 
*         if switch SW1 is pressed and if the gClearLCD flag is set to 
*         true, allowing the duty cycle to be varied. If the flag isn't 
*         set but SW1 has been pressed, duty cycle variation is stopped 
*         and the duty is displayed as a 
*         percentage.   
* Argument     : none
* Return value : none
*******************************************************************************/
void CB_Switch_Press(void) {  
//  uint32_t calc_duty_percent = 0;
//LED4=LED_ON;
#if 0  
  /* Has switch SW1 been pressed and LCD clearance allowed? */
  if(!(gSwitchFlag & SWITCHPRESS_1)) {  
    /* Allow duty cycle to be updated */
    //gVaryDutyCycle  = true;
  }

  /* Set the gVaryDutyCycle to false and display the duty 
     cycle as a percentage if SW1 has been pressed */
  if((gSwitchFlag & SWITCHPRESS_1)) {
    /* Set flag to indicate stoppage of duty cycle updates. */
    //gVaryDutyCycle  = false;
            
    /* Calculate the percentage */
    //calc_duty_percent = (uint32_t)((gDutyPercentage * 100) / 48000); 
  }
  /* Clear the switch press flags */
  //gSwitchFlag &= SWITCHHOLD_ALL;
#endif
}
  
/*******************************************************************************
* Outline      : Excep_MTU3_TGIA3
* Description  : MTU3 TGIA3 interrupt handler. This function updates the duty 
*         cycle by 1%, resets the duty to 10% if it reaches 90%, and
*         automatically toggles the MTIOC3A pin on every TGRA output 
*         compare match. 
* Argument     : none
* Return value : none
*******************************************************************************/

#pragma vector=VECT_MTU3_TGIA3
__interrupt void Excep_MTU3_TGIA3(void)
{  
  static uint16_t counter=0;
  /* Stop the TCNT counter */  
  MTU.TSTR.BIT.CST3 = 0x0;
  /* Reset the duty cyccle to 10% */
  MTU3.TGRB = 4800;
  switch(counter) {
  case 0:
//      LED0 ^=1;
//      LED_RED =1;
      LED_GRN ^=1;
//      LED_BLU =1;

      counter++;
      break;
  case 2:
      counter++;
      break;
  case 999:
      counter=0;
      break;
  default:
      counter++;
  }
  if(counter >= 999)
     counter=0;

  /* Start the TCNT counter */  
  MTU.TSTR.BIT.CST3 = 0x1;    
  /* Clear the interrupt flag */
  ICU.IR[IR_MTU3_TGIA3].BIT.IR = 0;

}

#pragma vector=VECT_MTU3_TGIB3
__interrupt void Excep_MTU3_TGIB3(void) {
  
  /* Clear the interrupt flag */
  ICU.IR[IR_MTU3_TGIB3].BIT.IR = 0;

}