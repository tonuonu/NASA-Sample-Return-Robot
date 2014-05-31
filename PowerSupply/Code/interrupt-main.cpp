/*
 *  Copyright (c) 2013, 2014 Tonu Samuel
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

#include "intrinsics.h"
#include "rtc.h"
#include "pwm.h"
#include "oled.h"
#include "hwsetup.h"
#include "spi.h"
#include "usb_hal.h"
#include "usb_cdc.h"
#include "usb.h"
#include "log.h"
#include "adc12repeat.h"
#include "interrupt-main.h"
#include "low_voltage_detection.h"
#include "rskrx630def.h"
#include "iorx630.h"
#include "stdbool.h"
#include "led.h"

/* Dclare a variable to hold the periodic delay specified */
volatile uint32_t gPeriodic_Delay;
/* Declare a variable to store the global delay count value */
volatile uint32_t gDelay_Counter = 0;

/******************************************************************************
* Outline    : Timer_Delay
* Description  : Function used to create delays in milliseconds or 
*          microseconds depending on the user selection for controlling 
*          the debug LCD function's calls. 
* Argument      : uint32_t   -   _Delay_Period
*          uint8   -   Unit  
* Return value  : none
******************************************************************************/
void Timer_Delay(uint32_t user_delay, uint8_t unit, uint8_t timer_mode) {
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

        /* Store a copy of the user delay value to gPeriodic_Delay */
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

#pragma vector=VECT_CMT2_CMI2
__interrupt void Excep_CMTU1_CMT2(void) {
    LED7 =LED_ON;
    /*
     * Resistor divider made from 33k and 100k 
     * Actual voltage is 133/33=4.03 times higher than ADC reading.
     * So when reference is 3.3V, maximum ADC reading is 133k/33k*3.3V=13.3V
     * Because we have 12bit ADC, formula for voltage is 13.3/(2^12-1)*reading
     * 13.3/(2^12)*4095=13.3V etc
     */
#define VCOEFF (((100.0+33.0)/33.0*3.3)/(4095.0))
    adc[0] =  (float)(S12AD.ADDR0>>2)*VCOEFF; // VBAT0_AD 
    adc[1] =  (float)(S12AD.ADDR1>>2)*VCOEFF; // VBAT1_AD
    adc[2] =  (float)(S12AD.ADDR2>>2)*VCOEFF; // VBAT2_AD
    adc[3] =  (float)(S12AD.ADDR3>>2)*VCOEFF; // VBAT3_AD
  
    /*
     * ACS712 20A version outputs 100mV for each A
     * Center is at VCC/2, so 5V/2=2.5V
     * Theoretical maximum current we can measure is 2.5/0.1=25A?
     * Zero current point is (2^12-1)/3.3*2.5
     * 4095/3.3*2.5 = ~3102 
     * For every volt 4095/3.3=1240.90909091 units
     * For every amper change is 4095/3.3*0.1=~124.09 units on ADC
     */
#define ICOEFF (4095.0/3.3*   0.1*-1)
#define IZBASE (   4095.0/3.3*2.53  )
    adc[4] = 0.8*adc[4] + 0.2*((float)(S12AD.ADDR4>>2)-IZBASE)/ICOEFF; // IBAT0_AD
    adc[5] = 0.8*adc[5] + 0.2*((float)(S12AD.ADDR5>>2)-IZBASE)/ICOEFF; // IBAT1_AD
    adc[6] = 0.8*adc[6] + 0.2*((float)(S12AD.ADDR6>>2)-IZBASE)/ICOEFF; // IBAT2_AD
    adc[7] = 0.8*adc[7] + 0.2*((float)(S12AD.ADDR7>>2)-IZBASE)/ICOEFF; // IBAT3_AD
    /*
     * Every volt on AD input 1023.0/3.3 
     * Resistor divider 680k and 100k makes (680.0+100.0)/100.0
     */
#define VCOEFF2 (((680.0+100.0)/100.0*3.3)/(1023.0))
    // AN0 is external adapter input
    adapter = (float) AD.ADDRA*VCOEFF2;
    
    /*
     * IMON1 and IMON2 are current measuring outputs of TPS51222
     * The measure voltage drop over output inductor and amplyify it by 50.
     * For Bourns SRP1250-4R7M actual DCR is ~10 milliohm (15 mohm max by 
     * datasheet) or 0.008 ohm. Ohm's law states I = U/R.
     * In our ADC maximum reading of 1023 means 3.3V. 3.3V/50= 0.066V and this  
     * is maximum drop we can measure on inductor. 
     * I = 0.066V / 0.008ohm = 8.25A 
     * Additionally there is a resistor divider on board from 10k and 3k12. 
     * This means we need to multiply value by ~1.0312
     * Measured current is not exact as inductor actualy drop may differ.
     * PGOOD indicates if power supply actually works. If not, current always
     * shows maximum value.
     */
    if(PGOOD1) {
        imon1 = (float) AD.ADDRC / 1023.0 * (0.066 / 0.010) * ((10.0+3.12)/3.12);
        imon1max = imon1 > imon1max ? imon1 : imon1max;
    } else {
        imon1 = 0.0f;
    }

    if(PGOOD2) {
        imon2 = (float) AD.ADDRD / 1023.0 * (0.066 / 0.010) * ((10.0+3.12)/3.12);
        imon2max = imon2 > imon2max ? imon2 : imon2max;
        if(imon2 > 10.0) { // Excess current on steering power supply
            //LED_GRN = LED_OFF;
            //LED_RED = LED_ON;
            static int da=0;
            //da= da == 0 ? 0xffff : 0;
            da ^= 0xffff;
            DA.DADR1=da;
        } else {
            //LED_RED = LED_OFF;
        }
    } else {
        imon2 = 0.0f;
    }
    
#define CURRENT_MAX     16.0
#define CURRENT_MIN     -10.0
#define VOLTAGE_MAX     12.6 * 1.1 // Allow some overhead before forcing shut off
#define VOLTAGE_MIN     9.0  * 0.9

    int BAT0_error=0;
    int BAT1_error=0;
    int BAT2_error=0;
    int BAT3_error=0;

    char buf[65];
#if 1
        
    /* check for voltages */   
    if(adc[0] > VOLTAGE_MAX || adc[0] < VOLTAGE_MIN) { // BAT0
        uint8_t flag=BAT0_EN;
        BAT0_EN=MAX1614_OFF; // Turn off MOSFET
        BAT0_error=1;
        if(flag==MAX1614_ON) {
            snprintf(buf,sizeof(buf),"E:BAT0:%.2fV",adc[0]);
            logerror(buf);
        }
    }
    if(adc[1] > VOLTAGE_MAX || adc[1] < VOLTAGE_MIN) { // BAT1
        uint8_t flag=BAT1_EN;
        BAT1_EN=MAX1614_OFF; // Turn off MOSFET
        BAT1_error=1;
        if(flag==MAX1614_ON) {
            snprintf(buf,sizeof(buf),"E:BAT1:%.2fV",adc[1]);
            logerror(buf);
        }
    }
    if(adc[2] > VOLTAGE_MAX || adc[2] < VOLTAGE_MIN) { // BAT2
        uint8_t flag=BAT2_EN;
        BAT2_EN=MAX1614_OFF; // Turn off MOSFET
        BAT2_error=1;
        if(flag==MAX1614_ON) {
            snprintf(buf,sizeof(buf),"E:BAT2:%.2fV",adc[2]);
            logerror(buf);
        }
    }
    if(adc[3] > VOLTAGE_MAX || adc[3] < VOLTAGE_MIN) { // BAT3
        uint8_t flag=BAT3_EN;
        BAT3_EN=MAX1614_OFF; // Turn off MOSFET
        BAT3_error=1;
        if(flag==MAX1614_ON) {
            snprintf(buf,sizeof(buf),"E:BAT3:%.2fV",adc[3]);
            logerror(buf);
        }
    }

    /* Find out which is maximum current on active batteries. */
    float maxcurrent=0.0;
    maxcurrent=maxcurrent < adc[4] ? adc[4] : maxcurrent; 
    maxcurrent=maxcurrent < adc[5] ? adc[5] : maxcurrent; 
    maxcurrent=maxcurrent < adc[6] ? adc[6] : maxcurrent; 
    maxcurrent=maxcurrent < adc[7] ? adc[7] : maxcurrent; 

    // make sure consumption is big enough to rule out too small values
    if(maxcurrent > 1.0f) {
        if(adc[4] < 0.5f) // Input is enabled but battery is not connected
            BAT0_EN=MAX1614_OFF; // Turn off MOSFET
        if(adc[5] < 0.5f) // Input is enabled but battery is not connected
            BAT1_EN=MAX1614_OFF; // Turn off MOSFET
        if(adc[6] < 0.5f) // Input is enabled but battery is not connected
            BAT2_EN=MAX1614_OFF; // Turn off MOSFET
        if(adc[7] < 0.5f) // Input is enabled but battery is not connected
            BAT3_EN=MAX1614_OFF; // Turn off MOSFET
    }
    
    /* check for battery currents */   
    if(adc[4] > CURRENT_MAX || adc[4] < CURRENT_MIN) { // BAT0
        uint8_t flag=BAT0_EN;
        BAT0_EN=MAX1614_OFF; // Turn off MOSFET
        BAT0_error=1;
        if(flag==MAX1614_ON) {
            snprintf(buf,sizeof(buf),"E:BAT0:%.2fA",adc[4]);
            logerror(buf);
        }
    }
    if(adc[5] > CURRENT_MAX || adc[5] < CURRENT_MIN) { // BAT1
        uint8_t flag=BAT1_EN;
        BAT1_EN=MAX1614_OFF; // Turn off MOSFET
        BAT1_error=1;
        if(flag==MAX1614_ON) {
            snprintf(buf,sizeof(buf),"E:BAT1:%.2fA",adc[5]);
            logerror(buf);
        }
    }
    if(adc[6] > CURRENT_MAX || adc[6] < CURRENT_MIN) { // BAT2
        uint8_t flag=BAT2_EN;
        BAT2_EN=MAX1614_OFF; // Turn off MOSFET
        BAT2_error=1;
        if(flag==MAX1614_ON) {
            snprintf(buf,sizeof(buf),"E:BAT2:%.2fA",adc[6]);
            logerror(buf);
        }
    }
    if(adc[7] > CURRENT_MAX || adc[7] < CURRENT_MIN) { // BAT3
        uint8_t flag=BAT3_EN;
        BAT3_EN=MAX1614_OFF; // Turn off MOSFET
        BAT3_error=1;
        if(flag==MAX1614_ON) {
            snprintf(buf,sizeof(buf),"E:BAT3:%.2fA",adc[7]);
            logerror(buf);
        }
    }    
#endif
      
    /* Enable inputs which had no problems */
    if(!BAT0_error && BAT0_EN==MAX1614_OFF) {
        snprintf(buf,sizeof(buf),"I:BAT0:%.2fV",adc[0]);
        logerror(buf);
        BAT0_EN=MAX1614_ON;
    }
    if(!BAT1_error && BAT1_EN==MAX1614_OFF) {
        snprintf(buf,sizeof(buf),"I:BAT1:%.2fV",adc[1]);
        logerror(buf);
        BAT1_EN=MAX1614_ON;
    }
    if(!BAT2_error && BAT2_EN==MAX1614_OFF) {
        snprintf(buf,sizeof(buf),"I:BAT2:%.2fV",adc[2]);
        logerror(buf);
        BAT2_EN=MAX1614_ON;
    }
    if(!BAT3_error && BAT3_EN==MAX1614_OFF) {
        snprintf(buf,sizeof(buf),"I:BAT3:%.2fV",adc[3]);
        logerror(buf);
        BAT3_EN=MAX1614_ON;
    }
    
    
#if 0
    static int PWM0=0,PWM1=0,PWM2=0,PWM3=0;
    /* Check if adapter voltage exceeds any battery voltage. 
       If yes, we can charge! */
    
    if(adapter > adc[0] && adc[0] > 8.0f /* 8V = if any battery at all */) {
        /* 
         * Intensify charging is voltage is under 12.6V and current is low.
         * Charging current is negative, this is why ">" is ok 
         */
        LED_RGB_set(RGB_BLUE);
        if(adc[0] >= VOLTAGE_MAX) {
            PWM3=0;
            LED_RGB_set(RGB_YELLOW);
        } else if(adc[4] > CURRENT_MIN) { 
            PWM3++; // Increase charging
            LED_RGB_set(RGB_RED);
        } else if(adc[4] < CURRENT_MIN) { // Current is high, reduce PWM
            PWM3--; // 
            LED_RGB_set(RGB_GREEN);
        }
    } else {
    //  LED_RGB_set(RGB_PINK);
    }

    
    if(adapter > adc[1] && adc[1] > 8.0f /* 8V = is any battery at all */) {
        /* 
         * Intensify charging is voltage is under 12.6V and current is low.
         * Charging current is negative, this is why ">" is ok 
         */
        LED_RGB_set(RGB_BLUE);
        if(adc[1] >= VOLTAGE_MAX) {
            PWM3=0;
            LED_RGB_set(RGB_YELLOW);
        } else if(adc[5] > CURRENT_MIN) { 
            PWM3++; // Increase charging
            LED_RGB_set(RGB_RED);
        } else if(adc[5] < CURRENT_MIN) { // Current is high, reduce PWM
            PWM3--; // 
            LED_RGB_set(RGB_GREEN);
        }
    } else {
    //  LED_RGB_set(RGB_PINK);
    }

    
    if(adapter > adc[2] && adc[2] > 8.0f /* 8V = is any battery at all */) {
        /* 
         * Intensify charging is voltage is under 12.6V and current is low.
         * Charging current is negative, this is why ">" is ok 
         */
        LED_RGB_set(RGB_BLUE);
        if(adc[2] >= VOLTAGE_MAX) {
            PWM3=0;
            LED_RGB_set(RGB_YELLOW);
        } else if(adc[6] > CURRENT_MIN) { 
            PWM3++; // Increase charging
            LED_RGB_set(RGB_RED);
        } else if(adc[6] < CURRENT_MIN) { // Current is high, reduce PWM
            PWM3--; // 
            LED_RGB_set(RGB_GREEN);
        }
    } else {
//      LED_RGB_set(RGB_PINK);
    }


    if(adapter > adc[3] && adc[3] > 8.0f /* 8V = is any battery at all */) {
        /* 
         * Intensify charging is voltage is under 12.6V and current is low.
         * Charging current is negative, this is why ">" is ok 
         */
        LED_RGB_set(RGB_BLUE);
        if(adc[3] >= VOLTAGE_MAX) {
            PWM3=0;
            LED_RGB_set(RGB_YELLOW);
        } else if(adc[7] > CURRENT_MIN) { 
            PWM3++; // Increase charging
            LED_RGB_set(RGB_RED);
        } else if(adc[7] < CURRENT_MIN) { // Current is high, reduce PWM
            PWM3--; // 
            LED_RGB_set(RGB_GREEN);
        }
    } else {
  //    LED_RGB_set(RGB_PINK);
    }
    
    PWM0 = PWM0 > 10 ? 10 : PWM0;
    PWM1 = PWM1 > 10 ? 10 : PWM1;
    PWM2 = PWM2 > 10 ? 10 : PWM2;
    PWM3 = PWM3 > 10 ? 10 : PWM3;

    PWM0 = PWM0 < 0 ? 0 : PWM0;
    PWM1 = PWM1 < 0 ? 0 : PWM1;
    PWM2 = PWM2 < 0 ? 0 : PWM2;
    PWM3 = PWM3 < 0 ? 0 : PWM3;

    /* 
     * if PWM is enabled, enable CSD97374Q4M chip by applying 0 to !SKIPx pin
     * If PWM is disabled, make !SKIPx pin into high impendance (input).
     */
    SKIP0=0;
    SKIP1=0;
    SKIP2=0;
    SKIP3=0;
    SKIP0_DIR= PWM0 ? 1 : 0;
    SKIP1_DIR= PWM1 ? 1 : 0;
    SKIP2_DIR= PWM2 ? 1 : 0;
    SKIP3_DIR= PWM3 ? 1 : 0;
        
    TPU3.TGRA = 10-PWM3;
    TPU3.TGRB = 10;

    TPU2.TGRA = 20-PWM2;
    TPU2.TGRB = 20;

    TPU1.TGRA = 30-PWM1;
    TPU1.TGRB = 30;

    TPU0.TGRA = 40-PWM0;
    TPU0.TGRB = 40;

#endif
    /* update gyroscope and accelerometer values */ 
    read_gyro();
    static int cnt=0;
    static int da=0;
    if(cnt++<1000) {
        da= da == 0 ? 0xffff : 0;
        DA.DADR1=da;
    } else {
        if(cnt>10000)
            cnt=0;
    }
    
//    LED_RGB_set(RGB_BLUE);
    LED7 =LED_OFF;
}
