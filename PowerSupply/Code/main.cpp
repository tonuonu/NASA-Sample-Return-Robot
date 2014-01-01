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
//#include "async.h"
#include "usb_hal.h"
#include "usb_cdc.h"
#include "usb_cdc_app.h"
#include "adc12repeat.h"
#include "low_voltage_detection.h"


int main() {
    HardwareSetup();
    __enable_interrupt();
    Init_OLED(); // Make sure noone else calls OLED calls in interrupts before init
    Init_PWM();
    // Init_VoltageDetect();
    Init_ADC12Repeat();
    Init_RTC();
    // Init_UART();  
#if 1
    USBCDC_Init();  
#endif

    __delay_cycles(96UL*2000UL); // 2000us delay    
    OLED_Fill_RAM(0x00);				   // Clear Screen
    OLED_Show_String(  1, "Battery statuses", 0, 0*8);

    while(1) {
        __wait_for_interrupt();
    }

}

