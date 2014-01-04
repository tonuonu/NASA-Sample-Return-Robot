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

int main() {
    HardwareSetup();
    __enable_interrupt();
    Init_OLED(); // Make sure noone else calls OLED calls in interrupts before init
    Init_PWM();
    // Init_VoltageDetect();
    Init_ADC12Repeat();
    Init_RTC();
    // Init_SPI();
    // Init_UART();  
    // USBCDC_Init();  
    /* Initialise the SPI unit */
  
  
    //MasterSend_SPI(sizeof(gTransmitMessage), gTransmitMessage);
  
    __delay_cycles(96UL*2000UL); // 2000us delay    
    OLED_Fill_RAM(0x00);				   // Clear Screen
    OLED_Show_String(  1, "Battery statuses", 0, 0*8);


#if 0
      // Chip reset
  _SPI_write(MPUREG_PWR_MGMT_1, BIT_H_RESET);
  
  
  __delay_cycles(100UL*100000UL); // 100ms delay    

  

  // Wake up device and select GyroZ clock (better performance)
  _SPI_write(MPUREG_PWR_MGMT_1, MPU_CLK_SEL_PLLGYROZ);
  // Disable I2C bus (recommended on datasheet)
  _SPI_write(MPUREG_USER_CTRL, BIT_I2C_IF_DIS);
  // SAMPLE RATE
  _SPI_write(MPUREG_SMPLRT_DIV,0x04);     // Sample rate = 200Hz    Fsample= 1Khz/(4+1) = 200Hz     
  // FS & DLPF   FS=2000?/s, DLPF = 42Hz (low pass filter)
  _SPI_write(MPUREG_CONFIG, BITS_DLPF_CFG_42HZ);  // BITS_DLPF_CFG_20HZ BITS_DLPF_CFG_42HZ BITS_DLPF_CFG_98HZ
  _SPI_write(MPUREG_GYRO_CONFIG,BITS_FS_2000DPS);  // Gyro scale 2000?/s
  _SPI_write(MPUREG_ACCEL_CONFIG,BITS_FS_2G);           // Accel scele 2g (g=8192)  
  // INT CFG => Interrupt on Data Ready
  _SPI_write(MPUREG_INT_ENABLE,BIT_RAW_RDY_EN);         // INT: Raw data ready
  _SPI_write(MPUREG_INT_PIN_CFG,BIT_INT_ANYRD_2CLEAR);  // INT: Clear on any read
#endif
    
    while(1) {
        __wait_for_interrupt();
#if 0
        /* Load message byte into transmit container variable */
        uint8_t tx_byte = 0x55;
    
        /* Write to data register */
        RSPI1.SPDR.LONG = (uint32_t) 0xA5000000 | (tx_byte << 16);
    
        /* Wait until transmission is complete */
        while(RSPI1.SPSR.BIT.IDLNF);
    
        /* Increment loop counter */
  
        /* Send a dummy transmission to receive the final byte from the slave */
        RSPI1.SPDR.LONG = 0xA3000000;
  
        /* Wait until transmission is complete */
        while(RSPI1.SPSR.BIT.IDLNF);
#endif
    }

}

