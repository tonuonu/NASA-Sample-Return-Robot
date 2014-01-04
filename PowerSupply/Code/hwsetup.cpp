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

#include <machine.h>
#include <stdint.h>

#include "iorx630.h"
#include "rskrx630def.h"
#include "oled.h"
#include "switch.h"
#include "hwsetup.h"

void HardwareSetup(void) {
  ConfigureOperatingFrequency();
  ConfigureOutputPorts();
  ConfigureInterrupts();
  EnablePeripheralModules();
}

void ConfigureOperatingFrequency(void) {      
  uint16_t i = 0;
    
  /* Protection off */
  SYSTEM.PRCR.WORD = 0xA503;      

  /* Stop sub-clock */
  SYSTEM.SOSCCR.BYTE = 0x01;      

  /* Specify a wait state greater than 10ms at 12MHz (10.92 msec) */ 
  SYSTEM.MOSCWTCR.BYTE = 0x0D;    

  /* 4194304 state (default)*/
  /* wait over 12ms  @PLL=192MHz(12MHz*16) */                
  SYSTEM.PLLWTCR.BYTE = 0x0F;      

  /* x16 @PLL */
  SYSTEM.PLLCR.WORD = 0x0F00;  

  /* Configure the EXTAL pin */
  PORT3.PMR.BIT.B6 = 0;
  PORT3.PDR.BIT.B6 = 0;

  /* Configure the XTAL pin */
  PORT3.PMR.BIT.B7 = 0;
  PORT3.PDR.BIT.B7 = 0;    

  /* Turn on EXTAL */
  SYSTEM.MOSCCR.BYTE = 0x00;  
      
  /* Turn on the PLL and enable writing to the PLL control register */
  SYSTEM.PLLCR2.BYTE = 0x00;  
  
  /* Wait over 12ms */
  for(i=0; i<2075; i++) {
    nop();
  }
    
  /* Configure the clocks as follows -
  Clock Description              Frequency
  ----------------------------------------
  PLL Clock frequency...............192MHz
  System Clock Frequency.............96MHz
  Peripheral Module Clock B..........48MHz   
  FlashIF Clock......................48MHz
  External Bus Clock.................48MHz */       
  SYSTEM.SCKCR.LONG = 0x21821211; 
//  SYSTEM.SCKCR.BIT.BCK=6; // Change bus speed to lower, /64 divisor 
//  SYSTEM.SCKCR.BIT.PSTOP1=1; // Disable BCLK pin, it is useless for us;
  
  /* Configure the clocks as follows -
  Clock Description              Frequency
  ----------------------------------------
  USB Clock..........................48MHz    
  IEBus Clock........................24MHz */ 
  SYSTEM.SCKCR2.WORD = 0x0033;
  
  /* Set the clock source to PLL */
  SYSTEM.SCKCR3.WORD = 0x0400;
  
  /* Protection on */
  SYSTEM.PRCR.WORD = 0xA500;    
}

void ConfigureOutputPorts(void) {  

    /* Configure the LED pins (LED0-LED7) as outputs */
    LED0_PORT_DIR = 0x1;
    LED1_PORT_DIR = 0x1;
    LED2_PORT_DIR = 0x1;
    LED3_PORT_DIR = 0x1;
    LED4_PORT_DIR = 0x1;
    LED5_PORT_DIR = 0x1;
    LED6_PORT_DIR = 0x1;
    LED7_PORT_DIR = 0x1;

    LED_BLU_PORT_DIR = 0x1;
    LED_GRN_PORT_DIR = 0x1;
    LED_RED_PORT_DIR = 0x1;

    
    /* Set LED pin outputs as high (all LEDs off) */
    LED0 = LED_OFF;
    LED1 = LED_OFF;
    LED2 = LED_OFF;
    LED3 = LED_OFF;
    LED4 = LED_OFF;
    LED5 = LED_OFF;
    LED6 = LED_OFF;
    LED7 = LED_OFF;
    LED_RED = LED_OFF;
    LED_GRN = LED_OFF;
    LED_BLU = LED_OFF;
}

void ConfigureInterrupts(void) {
  /* Configure switch interrupts */
  InitialiseSwitchInterrupts();
}

void EnablePeripheralModules(void) {
  /* Peripherals initialised in specific initialisation functions */
}
