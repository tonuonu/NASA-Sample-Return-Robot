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


#include "iorx630.h"
#include "rskrx630def.h"
#include "oled.h"
#include "switch.h"
#include "log.h"
#include "adc12repeat.h"
#include "stdio.h"
#include "string.h"

volatile float adc[8] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
volatile float adapter = 0.0;
volatile float imon1 = 0.0;
volatile float imon2 = 0.0;

/* The variable gADC_Result is used to contain the value of the 12 bit ADC */
volatile uint16_t gADC_Result;

/* Declare a variable used to specify if the delay 
function should only generate the specific delay or not */
uint8_t gTimer_Mode_Flag = 0;
/* Dclare a variable to hold the periodic delay specified */
volatile uint32_t gPeriodic_Delay;
/* Declare a variable to store the global delay count value */
volatile uint32_t gDelay_Counter = 0;
/* Init_Timer function prototype */
static void Init_Timer(void);
/* 16 bit integer to character string function prototype declaration */
//static void uint16_ToString(uint8_t *, uint8_t, uint16_t);
/* Timer_Delay function prototype */
static void Timer_Delay(uint32_t, uint8_t, uint8_t);

/*******************************************************************************
* Outline     : Init_ADC12Repeat
* Description   : ADC initialisation function. This function configures the ADC
*          unit for continuous scan operation, then configures a timer  
*          for a periodic interrupt every 300 milliseconds.
*******************************************************************************/
void Init_ADC12Repeat(void) {
  /* Initialise CMT2 */
  Init_Timer();
  
  /* Protection off */
  SYSTEM.PRCR.WORD = 0xA503;  
  /* Cancel the S12AD module clock stop mode */
  MSTP_S12AD = 0;
  MSTP_AD = 0;
  /* Protection on */
  SYSTEM.PRCR.WORD = 0xA500;

  /* Enable write to PFSWE bit */
  MPC.PWPR.BIT.B0WI=0;
  /* Disable write protection to PFS registers */
  MPC.PWPR.BIT.PFSWE=1;

  MPC.P40PFS.BIT.ASEL=1; // AN000
  MPC.P41PFS.BIT.ASEL=1;
  MPC.P42PFS.BIT.ASEL=1;
  MPC.P43PFS.BIT.ASEL=1;
  MPC.P44PFS.BIT.ASEL=1;
  MPC.P45PFS.BIT.ASEL=1;
  MPC.P46PFS.BIT.ASEL=1;
  MPC.P47PFS.BIT.ASEL=1; // AN007

  MPC.PE2PFS.BIT.ASEL=1; // AN0
  MPC.PE3PFS.BIT.ASEL=1; // AN1
  MPC.PE5PFS.BIT.ASEL=1; // AN3
  
  MPC.PWPR.BIT.PFSWE=0;
  MPC.PWPR.BIT.B0WI=1;
  
  /* ADC clock = PCLK/8, continuous scan mode */
  // S12AD.ADCSR.BYTE |= 0x40;
  S12AD.ADCSR.BIT.CKS=0; /*  0: PCLK/8
  1: PCLK/4
  2: PCLK/2
  3: PCLK
  */
  S12AD.ADCSR.BIT.ADCS=1; // 0 single scan, 1 continuous
  
  /* Selects AN000..AN007 */
  S12AD.ADANS0.WORD = 0x00FF;
  S12AD.ADANS0.BIT.ANS0=0x00FF;

  /* Set the sampling cycle to 255 states (approximately 5 us) */
  S12AD.ADSSTR01.BIT.SST1=255;
  S12AD.ADSSTR23.BIT.SST2=255;

  S12AD.ADADS0.BIT.ADS0=0x00FF; // AN000...AN007 addition mode on
  S12AD.ADADS1.BIT.ADS1=0x0000; // AN016... we no not care
  S12AD.ADADC.BIT.ADC=3;// convert and add 4 times
  S12AD.ADCER.BIT.ACE = 1; // Clear automatically after data is read
  
  /* Start ADC */
  S12AD.ADCSR.BIT.ADST = 1;

  AD.ADCSR.BIT.ADST = 0; // stop conversion
  AD.ADCSR.BIT.CH   = 3; // enable AN0..AN3 
  AD.ADCR.BIT.MODE  = 2; // continuous scan mode
  AD.ADCR.BIT.CKS   = 0; // clock is PCLK/8
  AD.ADCR.BIT.TRGS  = 0; // software trigger  
  AD.ADCR2.BIT.DPSEL = 0; // Flush-right
  AD.ADCSR.BIT.ADST = 1; // start conversion

  /* Configure a 10 ms periodic delay used 
     to update the ADC and Gyro results and check for safe numbers */
  Timer_Delay(10, 'm', PERIODIC_MODE);

}

static void Init_Timer(void) {
  /* Protection off */
  SYSTEM.PRCR.WORD = 0xA503;
  
  /* Cancel the CMT2 module clock stop mode */
  MSTP_CMT2 = 0;
  
  /* Protection on */
  SYSTEM.PRCR.WORD = 0xA500;

  /* Set CMT2 interrupt priority level to 3 */  
  IPR(CMT2,CMI2) = 0x3;
  /* Enable CMT2 interrupts */
  IEN(CMT2,CMI2) = 0x1;
  /* Clear CMT2 interrupt flag */
  IR(CMT2,CMI2) = 0x0;  
}

/******************************************************************************
* Outline    : Timer_Delay
* Description  : Function used to create delays in milliseconds or 
*          microseconds depending on the user selection for controlling 
*          the debug LCD function's calls. 
* Argument      : uint32_t   -   _Delay_Period
*          uint8   -   Unit  
* Return value  : none
******************************************************************************/
static void Timer_Delay(uint32_t user_delay, uint8_t unit, uint8_t timer_mode) {
  /* Clear the timer's count */
  gDelay_Counter = 0;
    
  /* Check if microseconds delay is required */
  if(unit == 'u') {
    /* Select the PCLK clock division as PCLK/8 = 6MHz */ 
    CMT2.CMCR.BIT.CKS = 0x0;
    
    /* Store a copy of the user delay value to gPeriodic_Delay */
    gPeriodic_Delay = user_delay * 6;
  
    /* Specify the timer period */
    CMT2.CMCOR = gPeriodic_Delay;
  }
  
  /* Check if milliseconds delay is required */
  if(unit == 'm') {
    /* Select the PCLK clock division as PCLK/128 = 375KHz */ 
    CMT2.CMCR.BIT.CKS = 0x2;

    /* Stor a copy of the user delay value to gPeriodic_Delay */
    gPeriodic_Delay = user_delay * 375;

    /* Specify the timer period */
    CMT2.CMCOR = gPeriodic_Delay;
  }
  
  /* Enable the compare match interrupt */
  CMT2.CMCR.BIT.CMIE = 1;

  /* Start CMT2 count */
  CMT.CMSTR1.BIT.STR2 = 1;

  /* Skip the following instructions if a periodic timer is required */
  if((timer_mode != PERIODIC_MODE)) {
    /* Wait for the timer to timeout */
    while((gDelay_Counter != gPeriodic_Delay)) {
      /* Wait */
    }  

    /* Stop CMT2 count */
    CMT.CMSTR1.BIT.STR2 = 0;
  }      
}

