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
//#include "async.h"
#include "usb_hal.h"
#include "usb_cdc.h"
#include "usb_cdc_app.h"
#include "adc12repeat.h"
#include "low_voltage_detection.h"
#include "rskrx630def.h"

   
void _SPI_write(uint8_t reg, uint8_t data) {
    /* Write to data register */
    RSPI0.SPDR.LONG = (uint32_t) 0x00000000 | (reg << 24) | (data << 16);   
    RSPI1.SPDR.LONG = (uint32_t) 0x00000000 | (reg << 24) | (data << 16);
    /* Wait until transmission is complete */
    while(RSPI0.SPSR.BIT.IDLNF);
    while(RSPI1.SPSR.BIT.IDLNF);
}

// Configuration bits MPU6000
#define BIT_SLEEP                   0x40
#define BIT_H_RESET                 0x80
#define BITS_CLKSEL                 0x07
#define MPU_CLK_SEL_PLLGYROX        0x01
#define MPU_CLK_SEL_PLLGYROZ        0x03
#define MPU_EXT_SYNC_GYROX          0x02
#define BITS_FS_250DPS              0x00
#define BITS_FS_500DPS              0x08
#define BITS_FS_1000DPS             0x10
#define BITS_FS_2000DPS             0x18
#define BITS_FS_2G                  0x00
#define BITS_FS_4G                  0x08
#define BITS_FS_8G                  0x10
#define BITS_FS_16G                 0x18
#define BITS_FS_MASK                0x18
#define BITS_DLPF_CFG_256HZ_NOLPF2  0x00
#define BITS_DLPF_CFG_188HZ         0x01
#define BITS_DLPF_CFG_98HZ          0x02
#define BITS_DLPF_CFG_42HZ          0x03
#define BITS_DLPF_CFG_20HZ          0x04
#define BITS_DLPF_CFG_10HZ          0x05
#define BITS_DLPF_CFG_5HZ           0x06
#define BITS_DLPF_CFG_2100HZ_NOLPF  0x07
#define BITS_DLPF_CFG_MASK          0x07
#define BIT_INT_ANYRD_2CLEAR        0x10
#define BIT_RAW_RDY_EN              0x01
#define BIT_I2C_IF_DIS              0x10    
#define MPUREG_SMPLRT_DIV           0x19
#define MPUREG_CONFIG               0x1A
#define MPUREG_GYRO_CONFIG          0x1B
#define MPUREG_ACCEL_CONFIG         0x1C
#define MPUREG_INT_PIN_CFG          0x37
#define MPUREG_INT_ENABLE           0x38
#define MPUREG_USER_CTRL            0x6A
#define MPUREG_PWR_MGMT_1           0x6B
#define MPUREG_WHOAMI               0x75
#define MPUREG_ACCEL_XOUT           0x3b
#define MPUREG_TEMP_OUT             0x41
#define MPUREG_GYRO_XOUT            0x43
#define ENABLE_PWR                  PORT1.PODR.BIT.B2
#define ENABLE_PWR_DIR              PORT1.PDR.BIT.B2
#include "iorx630.h"


volatile uint32_t hello0 = 0xAAAAAAAA;
volatile uint32_t hello1 = 0xAAAAAAAA;


void
sendspi16(uint8_t cmd,uint8_t data) {

        RSPI0.SPCMD0.BIT.SPB=0xF; // 16 bits data length
        RSPI1.SPCMD0.BIT.SPB=0xF;

        /* Write to data register */
        RSPI0.SPDR.LONG = (uint32_t) 0x00000000 | (cmd << 8) | data;
        RSPI1.SPDR.LONG = (uint32_t) 0x00000000 | (cmd << 8) | data;
        while(RSPI0.SPSR.BIT.IDLNF); /* Wait until transmission is complete */
        while(RSPI1.SPSR.BIT.IDLNF); /* Wait until transmission is complete */

}

void
sendspi32(uint8_t cmd) {

        RSPI0.SPCMD0.BIT.SPB=0x3; // 32 bits data length
        RSPI1.SPCMD0.BIT.SPB=0x3;

        /* Write to data register */
        RSPI0.SPDR.LONG = (uint32_t) 0x00000000 | (cmd << 24);
        RSPI1.SPDR.LONG = (uint32_t) 0x00000000 | (cmd << 24);
        while(RSPI0.SPSR.BIT.IDLNF); /* Wait until transmission is complete */
        while(RSPI1.SPSR.BIT.IDLNF); /* Wait until transmission is complete */

}


int main() {
    BAT0_EN = MAX1614_OFF; // Make sure battery inputs are NOT enabled here.
    BAT1_EN = MAX1614_OFF; // They may have problems like low voltage, high voltage etc.,  
    BAT2_EN = MAX1614_OFF; // they later get checked and enabled if possible
    BAT3_EN = MAX1614_OFF; 

    BAT0_EN_DIR = 1; // Make output
    BAT1_EN_DIR = 1;
    BAT2_EN_DIR = 1; 
    BAT3_EN_DIR = 1;

    HardwareSetup();
    __enable_interrupt();
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
    // Init_UART();  
    // USBCDC_Init();  
    /* Initialise the SPI unit */
  
//sendspi16(MPUREG_PWR_MGMT_1,0x80); // reset
  
    //MasterSend_SPI(sizeof(gTransmitMessage), gTransmitMessage);
  
    __delay_cycles(96UL*2000UL); // 2000us delay    
    OLED_Fill_RAM(0x00);				   // Clear Screen
//    OLED_Show_String(  1, "Battery statuses", 0, 0*8);



    __delay_cycles(100UL*100000UL); // 100ms delay from GYRO reset is mandatory to wait
    sendspi16(MPUREG_PWR_MGMT_1,0x03); // wake up
        hello0=RSPI0.SPDR.LONG ; 
        hello1=RSPI1.SPDR.LONG ; 
  
  
    sendspi16(MPUREG_USER_CTRL, BIT_I2C_IF_DIS);
        hello0=RSPI0.SPDR.LONG ; 
        hello1=RSPI1.SPDR.LONG ; 
#if 1
    // SAMPLE RATE
    sendspi16(MPUREG_SMPLRT_DIV,0x04);     // Sample rate = 200Hz    Fsample= 1Khz/(4+1) = 200Hz     
        hello0=RSPI0.SPDR.LONG ; 
        hello1=RSPI1.SPDR.LONG ; 
    // FS & DLPF   FS=2000?/s, DLPF = 42Hz (low pass filter)
    sendspi16(MPUREG_CONFIG, BITS_DLPF_CFG_42HZ);  // BITS_DLPF_CFG_20HZ BITS_DLPF_CFG_42HZ BITS_DLPF_CFG_98HZ
        hello0=RSPI0.SPDR.LONG ; 
        hello1=RSPI1.SPDR.LONG ; 
    sendspi16(MPUREG_GYRO_CONFIG,BITS_FS_2000DPS);  // Gyro scale 2000?/s
        hello0=RSPI0.SPDR.LONG ; 
        hello1=RSPI1.SPDR.LONG ; 
    sendspi16(MPUREG_ACCEL_CONFIG,BITS_FS_2G);           // Accel scele 2g (g=8192)  
        hello0=RSPI0.SPDR.LONG ; 
        hello1=RSPI1.SPDR.LONG ; 
#endif
    while(1) {
        // __wait_for_interrupt();
        LED6=LED_ON;
        sendspi16(MPUREG_GYRO_XOUT | 0x80 /* read bit*/, 0x00);
//        sendspi16(MPUREG_WHOAMI | 0x80 /* read bit*/, 0x00);
        // sendspi32(MPUREG_TEMP_OUT | 0x80 /* read bit*/);
        hello0=RSPI0.SPDR.LONG ; 
        hello1=RSPI1.SPDR.LONG ; 
        LED6=LED_OFF;
    }

}

