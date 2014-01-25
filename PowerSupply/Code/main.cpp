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

#define ENABLE_PWR                  PORT1.PODR.BIT.B2
#define ENABLE_PWR_DIR              PORT1.PDR.BIT.B2

int main() {
    BAT0_EN = MAX1614_OFF; // Make sure battery inputs are NOT enabled here.
    BAT1_EN = MAX1614_OFF; // They may have problems like low voltage, high voltage etc.,  
    BAT2_EN = MAX1614_OFF; // they later get checked and enabled if possible
    BAT3_EN = MAX1614_OFF; 

    BAT0_EN_DIR = 1; // Make output
    BAT1_EN_DIR = 1;
    BAT2_EN_DIR = 1; 
    BAT3_EN_DIR = 1;

    logerror("Boot");
    HardwareSetup();
    Init_OLED(); // Make sure noone else calls OLED calls in interrupts before init
    
    ENABLE_PWR=1; /* Enable TPS51222. 
                   * Basically 5V is enabled to ACS712 current sensors
                   */
    ENABLE_PWR_DIR=1; // Make it output AFTER to avoid power glitch.
    
    OUT1_EN = MAX1614_OFF; // Configure all power supply main outputs   
    OUT2_EN = MAX1614_OFF; 
    OUT3_EN = MAX1614_OFF;
    OUT1_EN_DIR = 1;
    OUT2_EN_DIR = 1;
    OUT3_EN_DIR = 1;
    
    OUT4_EN = 1; // Configure TPS51222 DC DC outputs
    OUT5_EN = 1; // 1 is enable, 0 is disable
    OUT4_EN_DIR = 1; 
    OUT5_EN_DIR = 1; 
    
    Init_PWM();
    // Init_VoltageDetect();
    Init_RTC();
    Init_ADC12Repeat();
    Init_SPI();
    Init_Gyros();

    // Init_UART();  
    USBCDC_Init();  
    __enable_interrupt();  
    __delay_cycles(96UL*2000UL); // 2000us delay    
    OLED_Fill_RAM(0x00);				   // Clear Screen
    while(1) {
        // __wait_for_interrupt();
        static bool done=false;
        if(USBCDC_IsConnected()) {
            LED6=LED_ON;
            if(!done) {
                done=true;
                USBCDC_WriteString("\r\nRobot!\r\n");
                /*Start a Read*/
                USBCDC_Read_Async(BUFFER_SIZE, g_pBuffEmpty, CBDoneRead);
                /*This continues in the CBDoneRead function...*/
            }
        } else {
            LED6=LED_OFF;
            done=false;
        }
        
    }
}

