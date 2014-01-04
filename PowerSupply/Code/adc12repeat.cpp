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
#include "oled.h"
#include "adc12repeat.h"

volatile float adc[8] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
volatile float temperature = 1.0;
volatile float adapter = 1.0;

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
  /* Protection on */
  SYSTEM.PRCR.WORD = 0xA500;

  /* Enable write to PFSWE bit */
  MPC.PWPR.BIT.B0WI=0;
  /* Disable write protection to PFS registers */
  MPC.PWPR.BIT.PFSWE=1;

  MPC.P40PFS.BIT.ASEL=1;
  MPC.P41PFS.BIT.ASEL=1;
  MPC.P42PFS.BIT.ASEL=1;
  MPC.P43PFS.BIT.ASEL=1;
  MPC.P44PFS.BIT.ASEL=1;
  MPC.P45PFS.BIT.ASEL=1;
  MPC.P46PFS.BIT.ASEL=1;
  MPC.P47PFS.BIT.ASEL=1;

  MPC.PWPR.BIT.PFSWE=0;
  MPC.PWPR.BIT.B0WI=1;
  
  /* ADC clock = PCLK/8, continuous scan mode */
  // S12AD.ADCSR.BYTE |= 0x40;
  S12AD.ADCSR.BIT.CKS=0; // PCLK/8
  S12AD.ADCSR.BIT.ADCS=1; // 0 single scan, 1 continuous
  
  /* Selects AN000..AN007 */
  S12AD.ADANS0.WORD = 0x00FF;
  S12AD.ADANS0.BIT.ANS0=0x00FF;

  /* Set the sampling cycle to 255 states (approximately 5 us) */
  //S12AD.ADSSTR01.WORD = 0x14FF;
//  S12AD.ADSSTR01.BIT.SST1=255;
  S12AD.ADSSTR01.BIT.SST1=25;
  S12AD.ADSSTR23.BIT.SST2=25;

  /* Start ADC */
  S12AD.ADCSR.BIT.ADST = 1;
  S12AD.ADCER.BIT.ACE = 1;

  /* Configure a 10 ms periodic delay used 
     to update the ADC result on to the LCD */
  Timer_Delay(100, 'm', PERIODIC_MODE);
#if 0
  AD.ADCSR.BIT.ADST = 0; // stop conversion
  AD.ADCSR.BIT.CH   = 3; // enable AN3 only
  AD.ADCR.BIT.MODE  = 3; // continuous scanning mode
  AD.ADCR.BIT.CKS   = 0; // clock is PCLK/8
  AD.ADCR.BIT.TRGS  = 0; // software trigger
  AD.ADCSR.BIT.ADST = 1; // start conversion
#endif

#if 0
  S12AD.ADCER.BIT.ACE=1;
  S12AD.ADEXICR.BIT.TSS = 1;// temperature sensor
  S12AD.ADADC.BIT.ADC=0;// how many times to convert
  S12AD.ADSSTR23.BIT.SST2=20;//sampling interval
  TEMPS.TSCR.BIT.TSEN=1;// Start sensor
  TEMPS.TSCR.BIT.TSOE=1;// Enable temperature sensor
#endif
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

volatile float test=9.0;

#include "assert.h"
/******************************************************************************
* Regular ADC interrupt. 
******************************************************************************/
#pragma vector=VECT_CMT2_CMI2
__interrupt void Excep_CMTU1_CMT2(void) {
    LED7 =LED_ON;

    /* Declare temporary character string */
    // uint8_t lcd_buffer[9] = "=H\'WXYZ\0";
  
    /* Fetch 12 bit ADC value */
    // Resistor divider made from 33k and 100k 
    // Actual voltage is 133/33=4.03 times higher than ADC reading.
    // So when reference is 3.3V, maximum ADC reading is 133k/33k*3.3V=13.3V
    // Because we have 12bit ADC, formula for voltage is 13.3/(2^12-1)*reading
    // 13.3/(2^12)*4095=13.3V etc
#define VCOEFF ((133.0/33.0*3.3)/(4095.0))
  
    // ACS712 5A version outputs 185mV for each A
    // Center is at VCC/2, so 2.5V
    // Maximum current we can measure is (3.3-2.5)/0.185=4.32A
    // ACS711 can do 3.3V. ACS711EEXLT-15AB-T3 90 mv/A
    // Zero current point is 2.5/3.3/(2^12-1)*reading
    // 2.5/3.3*4095 = 3102 is middle point
    // For every volt 4095/3.3=1240.90909091 units
    // For every amper change is 4095/3.3*0.185=229.568181818
#define ICOEFF (4095.0/3.3*   0.185)
#define IZBASE (   2.5/3.3*4095.0  )
    //while(S12AD.ADCSR.BIT.ADST);
LED6=LED_ON;
    //test =  (float)S12AD.ADDR0*VCOEFF+3.0; // VBAT0_AD 
    adc[0] =  3.0; // VBAT0_AD 
    adc[1] =  (float)S12AD.ADDR1*VCOEFF+3.0; // VBAT1_AD
    adc[2] =  (float)S12AD.ADDR2*VCOEFF+3.0; // VBAT2_AD
    adc[3] =  (float)S12AD.ADDR3*VCOEFF+3.0; // VBAT3_AD

    adc[4] = ((float)S12AD.ADDR4-IZBASE)/ICOEFF+3.0; // IBAT0_AD
    adc[5] = ((float)S12AD.ADDR5-IZBASE)/ICOEFF+3.0; // IBAT1_AD
    adc[6] = ((float)S12AD.ADDR6-IZBASE)/ICOEFF+3.0; // IBAT2_AD
    adc[7] = ((float)S12AD.ADDR7-IZBASE)/ICOEFF+3.0; // IBAT3_AD
LED6=LED_OFF;

    S12AD.ADCSR.BIT.ADST = 1;
#if 0 //////////////////////
    // AN3 is external adapter input
    adapter = (float) AD.ADDRD;
    // T = (Vs – V1)/Slope + T1
    // Slope = 4.1 mV/C
    // voltage =1.26
    
#define degrees25C  (16384.0/3.3*1.25)
    temperature = (float)(S12AD.ADTSDR/(16384.0/3.3)-degrees25C)/4.1+25.0;

#define CURRENT_MAX     1.0
#define CURRENT_MIN     -1.0
    
#define BAT0_EN PORTA.PODR.BIT.B7
#define BAT1_EN PORTB.PODR.BIT.B1
#define BAT2_EN PORTB.PODR.BIT.B2
#define BAT3_EN PORTB.PODR.BIT.B3

    int BAT0_error=0;
    int BAT1_error=0;
    int BAT2_error=0;
    int BAT3_error=0;
    
    /* First check for battery currents */
    
    if(adc[4] > CURRENT_MAX || adc[4] < CURRENT_MIN) { // BAT0
        BAT0_EN=0; // Turn off MOSFET
        BAT0_error=1;
    }
    if(adc[5] > CURRENT_MAX || adc[5] < CURRENT_MIN) { // BAT0
        BAT1_EN=0; // Turn off MOSFET
        BAT1_error=1;
    }
    if(adc[6] > CURRENT_MAX || adc[6] < CURRENT_MIN) { // BAT0
        BAT2_EN=0; // Turn off MOSFET
        BAT2_error=1;
    }
    if(adc[7] > CURRENT_MAX || adc[7] < CURRENT_MIN) { // BAT0
        BAT3_EN=0; // Turn off MOSFET
        BAT3_error=1;
    }

    int PWM0,PWM1,PWM2,PWM3;
    /* Check if adapter voltage exceeds any battery voltage. 
       If yes, we can charge! */
    if(adapter > adc[0] || adapter > adc[1] || adapter > adc[2] || adapter > adc[3] ) {
        if(adapter > adc[0]) {
            if(adc[4] > (CURRENT_MAX*0.9) && adc[0] < 4.20) { // ">" is right because charging current is negative
                PWM0+=1; // Increase charging
            } else if(adc[4] > (CURRENT_MAX*0.9)) { // Current is high, reduce PWM
                PWM0-=1; // 
            }
        }
        if(adapter > adc[1]) {
            if(adc[5] > (CURRENT_MAX*0.9) && adc[1] < 4.20) { // ">" is right because charging current is negative
                PWM1+=1; // Increase charging
            } else if(adc[5] > (CURRENT_MAX*0.9)) { // Current is high, reduce PWM
                PWM1-=1; // 
            }
        }
        if(adapter > adc[2]) {
            if(adc[6] > (CURRENT_MAX*0.9) && adc[2] < 4.20) { // ">" is right because charging current is negative
                PWM2+=1; // Increase charging
            } else if(adc[6] > (CURRENT_MAX*0.9)) { // Current is high, reduce PWM
                PWM2-=1; // 
            }
        }
        if(adapter > adc[3]) {
            if(adc[7] > (CURRENT_MAX*0.9) && adc[3] < 4.20) { // ">" is right because charging current is negative
                PWM3+=1; // Increase charging
            } else if(adc[7] > (CURRENT_MAX*0.9)) { // Current is high, reduce PWM
                PWM3-=1; // 
            }
        }
    }
#endif //////////////
    /* Voltages are less critical */        
    LED7 =LED_OFF;
}
