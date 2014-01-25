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
#include "low_voltage_detection.h"
#include "rskrx630def.h"
#include "iorx630.h"


#pragma vector=VECT_CMT2_CMI2
__interrupt void Excep_CMTU1_CMT2(void) {
    LED7 =LED_ON;
  
    /* Resistor divider made from 33k and 100k 
     * Actual voltage is 133/33=4.03 times higher than ADC reading.
     * So when reference is 3.3V, maximum ADC reading is 133k/33k*3.3V=13.3V
     * Because we have 12bit ADC, formula for voltage is 13.3/(2^12-1)*reading
     * 13.3/(2^12)*4095=13.3V etc
     */
#define VCOEFF (((100.0+33.0)/33.0*3.3)/(4095.0))
  
    /* ACS712 5A version outputs 185mV for each A
     * Center is at VCC/2, so 2.5V
     * Maximum current we can measure is (3.3-2.5)/0.185=4.32A
     * ACS711 can do 3.3V. ACS711EEXLT-15AB-T3 90 mv/A
     * Zero current point is 2.5/3.3/(2^12-1)*reading
     * 2.5/3.3*4095 = 3102 is middle point
     * For every volt 4095/3.3=1240.90909091 units
     * For every amper change is 4095/3.3*0.185=229.568181818
     */
#define ICOEFF (4095.0/3.3*   0.185)
#define IZBASE (   2.5/3.3*4095.0  )
    adc[0] =  (float)(S12AD.ADDR0>>4)*VCOEFF; // VBAT0_AD 
    adc[1] =  (float)(S12AD.ADDR1>>4)*VCOEFF; // VBAT1_AD
    adc[2] =  (float)(S12AD.ADDR2>>4)*VCOEFF; // VBAT2_AD
    adc[3] =  (float)(S12AD.ADDR3>>4)*VCOEFF; // VBAT3_AD

    adc[4] =  ((float)(S12AD.ADDR4>>4)-IZBASE)/ICOEFF; // IBAT0_AD
    adc[5] =  ((float)(S12AD.ADDR5>>4)-IZBASE)/ICOEFF; // IBAT1_AD
    adc[6] =  ((float)(S12AD.ADDR6>>4)-IZBASE)/ICOEFF; // IBAT2_AD
    adc[7] =  ((float)(S12AD.ADDR7>>4)-IZBASE)/ICOEFF; // IBAT3_AD
#define VCOEFF2 (((680.0+100.0)/100.0*3.3)/(1023.0))
    // AN0 is external adapter input
    adapter = (float) AD.ADDRA*VCOEFF2;
    
    /*
     * IMON1 and IMON2 are current measuring outputs of TPS51222
     * The measure voltage drop over output inductor and amplyify it by 50.
     * For Bourns SRP1250-4R7M DCR is 15 milliohm (0.015 ohm) max.
     * Ohms law states I = U/R.
     * In our ADC maximum reading of 1023 means 3.3V. 3.3V/50= 0.066V and this  
     * is maximum drop we can measure on inductor. 
     *I = 0.066V / 0.015ohm = 4.4A 
     */
    imon1 = (float) AD.ADDRC / 1023.0 * 4.4;
    imon2 = (float) AD.ADDRD / 1023.0 * 4.4;
    
#define CURRENT_MAX     3.0
#define CURRENT_MIN     -3.0
#define VOLTAGE_MAX     12.6
#define VOLTAGE_MIN     9.5    

    int BAT0_error=0;
    int BAT1_error=0;
    int BAT2_error=0;
    int BAT3_error=0;

    char buf[65];

    /* check for voltages */   
    if(BAT0_EN==MAX1614_ON && (adc[0] > VOLTAGE_MAX || adc[0] < VOLTAGE_MIN)) { // BAT0
        BAT0_EN=MAX1614_OFF; // Turn off MOSFET
        BAT0_error=1;
        snprintf(buf,sizeof(buf),"E:BAT0:%.2fV",adc[0]);
        logerror(buf);
    }
    if(BAT1_EN==MAX1614_ON && (adc[1] > VOLTAGE_MAX || adc[1] < VOLTAGE_MIN)) { // BAT1
        BAT1_EN=MAX1614_OFF; // Turn off MOSFET
        BAT1_error=1;
        snprintf(buf,sizeof(buf),"E:BAT1:%.2fV",adc[1]);
        logerror(buf);
    }
    if(BAT2_EN==MAX1614_ON && (adc[2] > VOLTAGE_MAX || adc[2] < VOLTAGE_MIN)) { // BAT2
        BAT2_EN=MAX1614_OFF; // Turn off MOSFET
        BAT2_error=1;
        snprintf(buf,sizeof(buf),"E:BAT2:%.2fV",adc[2]);
        logerror(buf);
    }
    if(BAT3_EN==MAX1614_ON && (adc[3] > VOLTAGE_MAX || adc[3] < VOLTAGE_MIN)) { // BAT3
        BAT3_EN=MAX1614_OFF; // Turn off MOSFET
        BAT3_error=1;
        snprintf(buf,sizeof(buf),"E:BAT3:%.2fV",adc[3]);
        logerror(buf);
    }

    /* check for battery currents */   
    if(BAT0_EN==MAX1614_ON && (adc[4] > CURRENT_MAX || adc[4] < CURRENT_MIN)) { // BAT0
        BAT0_EN=MAX1614_OFF; // Turn off MOSFET
        BAT0_error=1;
        snprintf(buf,sizeof(buf),"E:BAT0:%.2fA",adc[4]);
        logerror(buf);
    }
    if(BAT1_EN==MAX1614_ON && (adc[5] > CURRENT_MAX || adc[5] < CURRENT_MIN)) { // BAT1
        BAT1_EN=MAX1614_OFF; // Turn off MOSFET
        BAT1_error=1;
        snprintf(buf,sizeof(buf),"E:BAT1:%.2fA",adc[5]);
        logerror(buf);
    }
    if(BAT2_EN==MAX1614_ON && (adc[6] > CURRENT_MAX || adc[6] < CURRENT_MIN)) { // BAT2
        BAT2_EN=MAX1614_OFF; // Turn off MOSFET
        BAT2_error=1;
        snprintf(buf,sizeof(buf),"E:BAT2:%.2fA",adc[6]);
        logerror(buf);
    }
    if(BAT3_EN==MAX1614_ON && (adc[7] > CURRENT_MAX || adc[7] < CURRENT_MIN)) { // BAT3
        BAT3_EN=MAX1614_OFF; // Turn off MOSFET
        BAT3_error=1;
        snprintf(buf,sizeof(buf),"E:BAT3:%.2fA",adc[7]);
        logerror(buf);
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

    /* update gyroscope and accelerometer values */ 
    read_gyro();

    LED7 =LED_OFF;
}
