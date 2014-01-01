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
  
  /* Use the AN000 (Potentiometer) pin as an I/O for peripheral functions */
//  PORT4.PMR.BYTE = 0x01;
  
  /* ADC clock = PCLK/8, continuous scan mode */
  S12AD.ADCSR.BYTE |= 0x40;
  
  /* Selects AN000..AN007 */
  S12AD.ADANS0.WORD = 0x00FF;

  /* Set the sampling cycle to 255 states (approximately 5 us) */
  S12AD.ADSSTR01.WORD = 0x14FF;

  /* Start ADC */
  S12AD.ADCSR.BIT.ADST = 1;

  /* Configure a 10 ms periodic delay used 
     to update the ADC result on to the LCD */
  Timer_Delay(1, 'm', PERIODIC_MODE);
#if 0
  AD.ADCSR.BIT.ADST = 0; // stop conversion
  AD.ADCSR.BIT.CH   = 3; // enable AN3 only
  AD.ADCR.BIT.MODE  = 3; // continuous scanning mode
  AD.ADCR.BIT.CKS   = 0; // clock is PCLK/8
  AD.ADCR.BIT.TRGS  = 0; // software trigger
  //AD.ADCSR.BIT.ADST = 1; // start conversion
#endif
  S12AD.ADCER.BIT.ACE=1;
  S12AD.ADEXICR.BIT.TSS = 1;//?
  S12AD.ADADC.BIT.ADC=0;// how many times to convert
  S12AD.ADSSTR23.BIT.SST2=20;//sampling interval
  TEMPS.TSCR.BIT.TSEN=1;// Start sensor
  TEMPS.TSCR.BIT.TSOE=1;// Enable temperature sensor
}
/*******************************************************************************
* End of function Init_ADC12Repeat
*******************************************************************************/

/*******************************************************************************
* Outline      : Init_Timer
* Description  : This function initialises the CMT channel 2.
* Argument     : none
* Return value : none
*******************************************************************************/
static void Init_Timer(void)
{
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
/*******************************************************************************
* End of function Init_Timer
*******************************************************************************/

/*******************************************************************************
* Outline      : uint16_ToString
* Description  : Function converts a 16 bit integer into a character string, 
*         inserts it into the array via the pointer passed at execution.
* Argument     : * output_string : Pointer to uint8_t array that will hold 
*                   character string.
*               pos : uint8_t number, element number to begin 
*                   inserting the character string from (offset).
*          input_number : 16 bit integer to convert into a string.
* Return value : none
* Note       : No input validation is used, so output data can overflow the
*         array passed.
*******************************************************************************/
void uint16_ToString(uint8_t *output_string, uint8_t pos, 
      uint16_t input_number)
{
  /* Declare temporary character storage variable, and bit_shift variable */  
  uint8_t a = 0x00, bit_shift = 12u;
  
  /* Declare 16bit mask variable */
  uint16_t mask = 0xF000;
  
  /* Loop through until each hex digit is converted to an ASCII character */
  while(bit_shift < 30u)
  {
    /* Mask and shift the hex digit, and store in temporary variable, a */ 
    a = (uint8_t)((input_number & mask) >> bit_shift);
    
    /* Convert the hex digit into an ASCII character, and store in output
       string */
    output_string[pos] = (uint8_t)((a < 0x0A) ? (a + 0x30) : (a + 0x37));
    
    /* Shift the bit mask 4 bits to the right, to convert the next digit */
    mask = (uint16_t) (mask >> 4u);
    
    /* Decrement the bit_shift counter by 4 (bits in a each digit) */
    bit_shift -= 4u;
    
    /* Increment the output string location */
    pos++;
  }
}
/*******************************************************************************
* End of function uint16_ToString
*******************************************************************************/

/******************************************************************************
* Outline    : Timer_Delay
* Description  : Function used to create delays in milliseconds or 
*          microseconds depending on the user selection for controlling 
*          the debug LCD function's calls. 
* Argument      : uint32_t   -   _Delay_Period
*          uint8   -   Unit  
* Return value  : none
******************************************************************************/
static void Timer_Delay(uint32_t user_delay, uint8_t unit, uint8_t timer_mode)
{
  /* Clear the timer's count */
  gDelay_Counter = 0;
    
  /* Check if microseconds delay is required */
  if(unit == 'u')
  {  
    /* Select the PCLK clock division as PCLK/8 = 6MHz */ 
    CMT2.CMCR.BIT.CKS = 0x0;
    
    /* Store a copy of the user delay value to gPeriodic_Delay */
    gPeriodic_Delay = user_delay * 6;
  
    /* Specify the timer period */
    CMT2.CMCOR = gPeriodic_Delay;
  }
  
  /* Check if milliseconds delay is required */
  if(unit == 'm')
  {
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
  if((timer_mode != PERIODIC_MODE))
  {
    /* Wait for the timer to timeout */
    while((gDelay_Counter != gPeriodic_Delay))
    {
      /* Wait */
    }  

    /* Stop CMT2 count */
    CMT.CMSTR1.BIT.STR2 = 0;
  }      
}
/******************************************************************************
* End of function Timer_Delay
******************************************************************************/

/******************************************************************************
* Regular ADC interrupt. 
******************************************************************************/

#pragma vector=VECT_CMT2_CMI2
__interrupt void Excep_CMTU1_CMT2(void) {
    LED1 =1;

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

    adc[0] =  (float)S12AD.ADDR0*VCOEFF; // VBAT0_AD 
    adc[1] =  (float)S12AD.ADDR1*VCOEFF; // VBAT1_AD
    adc[2] =  (float)S12AD.ADDR2*VCOEFF; // VBAT2_AD
    adc[3] =  (float)S12AD.ADDR3*VCOEFF; // VBAT3_AD

    adc[4] = ((float)S12AD.ADDR4-IZBASE)/ICOEFF; // IBAT0_AD
    adc[5] = ((float)S12AD.ADDR5-IZBASE)/ICOEFF; // IBAT1_AD
    adc[6] = ((float)S12AD.ADDR6-IZBASE)/ICOEFF; // IBAT2_AD
    adc[7] = ((float)S12AD.ADDR7-IZBASE)/ICOEFF; // IBAT3_AD

    S12AD.ADCSR.BIT.ADST =1;
    // AN3 is external adapter input
    adapter = (float) AD.ADDRD;
    // T = (Vs – V1)/Slope + T1
    // Slope = 4.1 mV/C
    // voltage =1.26
    
#define degrees25C  (16384.0/3.3*1.25)
    temperature = (float)(S12AD.ADTSDR/(16384.0/3.3)-degrees25C)/4.1+25.0;

#define CURRENT_MAX     1.0
#define CURRENT_MIN     -1.0
    
#define BAT0_EN PORTE.PODR.BIT.B0
#define BAT1_EN PORTE.PODR.BIT.B1
#define BAT2_EN PORTE.PODR.BIT.B2
#define BAT3_EN PORTE.PODR.BIT.B3

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
    
    /* Voltages are less critical */
    
    
    
    /* Display the contents of the local string lcd_buffer */
    LED1 =0;
}
