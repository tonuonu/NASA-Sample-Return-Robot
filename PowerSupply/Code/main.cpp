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

#define MPUREG_TEMP_OUT             0x41

#define MPUREG_GYRO_XOUT            0x43
#define MPUREG_GYRO_YOUT            0x45
#define MPUREG_GYRO_ZOUT            0x47

#define MPUREG_ACCEL_XOUT            0x3B
#define MPUREG_ACCEL_YOUT            0x3D
#define MPUREG_ACCEL_ZOUT            0x3F

#define ENABLE_PWR                  PORT1.PODR.BIT.B2
#define ENABLE_PWR_DIR              PORT1.PDR.BIT.B2
#include "iorx630.h"

volatile int16_t  gyro0[3] = {0,0,0};
volatile int16_t  gyro1[3] = {0,0,0};
volatile int16_t accel0[3] = {0,0,0};
volatile int16_t accel1[3] = {0,0,0};

void
sendspi16cmd(uint8_t cmd,uint8_t data) {
    RSPI0.SPCMD0.BIT.SPB=0xF; // 16 bits data length (8 register no + 8 data)
    RSPI1.SPCMD0.BIT.SPB=0xF;

    /* Turn full duplex mode off when we do not care about
     * returned data. This avoids dummy read like this:
     *  dummy=RSPI0.SPDR.LONG; 
     *  dummy=RSPI1.SPDR.LONG;     
     */
    RSPI0.SPCR.BIT.TXMD = 1; // duplex mode off
    RSPI1.SPCR.BIT.TXMD = 1;

    /* Write to data register */
    RSPI0.SPDR.LONG = (uint32_t) 0x00000000 | (cmd << 8) | data;
    RSPI1.SPDR.LONG = (uint32_t) 0x00000000 | (cmd << 8) | data;
    while(RSPI0.SPSR.BIT.IDLNF); /* Wait until transmission is complete */
    while(RSPI1.SPSR.BIT.IDLNF); /* Wait until transmission is complete */
}

void
sendspi24(uint8_t cmd) {
    RSPI0.SPCMD0.BIT.SPB=0x1; // 24 bits data length
    RSPI1.SPCMD0.BIT.SPB=0x1;
    RSPI0.SPCR.BIT.TXMD = 0; // duplex mode on
    RSPI1.SPCR.BIT.TXMD = 0;

    /* Write to data register */
    RSPI0.SPDR.LONG = (uint32_t) 0x00000000 | (cmd << 16);
    RSPI1.SPDR.LONG = (uint32_t) 0x00000000 | (cmd << 16);
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

extern "C" void logerror(char *buf);
void
Init_Gyros(void) {

    // sendspi16cmd(MPUREG_PWR_MGMT_1,0x80); // reset
    // __delay_cycles(100UL*100000UL); // 100ms delay from GYRO reset is mandatory to wait
    sendspi16cmd(MPUREG_PWR_MGMT_1,0x03); // wake up
    sendspi16cmd(MPUREG_USER_CTRL, BIT_I2C_IF_DIS);
    // SAMPLE RATE
    sendspi16cmd(MPUREG_SMPLRT_DIV,0x04);     // Sample rate = 200Hz    Fsample= 1Khz/(4+1) = 200Hz     
    // DLPF = 42Hz (low pass filter)
    sendspi16cmd(MPUREG_CONFIG, BITS_DLPF_CFG_42HZ);  // BITS_DLPF_CFG_20HZ BITS_DLPF_CFG_42HZ BITS_DLPF_CFG_98HZ
    sendspi16cmd(MPUREG_GYRO_CONFIG,BITS_FS_250DPS); 
    sendspi16cmd(MPUREG_ACCEL_CONFIG,BITS_FS_2G);
  
}


/*Size of buffer - Lets use the packet size*/
#define BUFFER_SIZE BULK_OUT_PACKET_SIZE 

/*Data Buffers*/
static uint8_t g_Buffer1[BUFFER_SIZE];
static uint8_t g_Buffer2[BUFFER_SIZE];
static uint8_t* g_pBuffEmpty = g_Buffer1;
static uint8_t* g_pBuffFull = g_Buffer2;

static void CBDoneRead(USB_ERR _err, uint32_t _NumBytes)
{    
  /*Toggle buffers - as now the empty buffer has been filled
  by this read completing*/
  if(g_pBuffEmpty == g_Buffer2)
  {
    g_pBuffEmpty = g_Buffer1;
    g_pBuffFull = g_Buffer2;
  }
  else
  {
    g_pBuffEmpty = g_Buffer2;
    g_pBuffFull = g_Buffer1;
  }
  logerror((char *) g_pBuffFull);
  //if(true == g_bEcho)
  {
    /*Setup another read*/
    USBCDC_Read_Async(BUFFER_SIZE, g_pBuffEmpty, CBDoneRead);
      
    /*Echo what was read back*/
    if(USB_ERR_OK == _err)
    {
   //   USBCDC_Write_Async(_NumBytes, g_pBuffFull, CBDoneWrite);
    }
  }
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
    /* Initialise the SPI unit */
  
  
    //MasterSend_SPI(sizeof(gTransmitMessage), gTransmitMessage);
  
    __delay_cycles(96UL*2000UL); // 2000us delay    
    OLED_Fill_RAM(0x00);				   // Clear Screen

    while(1) {
      // __wait_for_interrupt();
          static bool done=false;
           
      if(USBCDC_IsConnected()) {
        LED6=LED_ON;
       
          if(!done) {
          done=true;
            USBCDC_WriteString("\r\nRenesas USB CDC Sample, Press Switch SW1.\r\n");
              /*Start a Read*/
              USBCDC_Read_Async(BUFFER_SIZE, g_pBuffEmpty, CBDoneRead);
          }
              /*This continues in the CBDoneRead function...*/
      }     else        {
        LED6=LED_OFF;
          done=false;
      }
      
        sendspi24(MPUREG_GYRO_XOUT | 0x80 /* read bit*/);
        gyro0[0]=(int16_t)RSPI0.SPDR.LONG ; 
        gyro1[0]=(int16_t)RSPI1.SPDR.LONG ; 

        sendspi24(MPUREG_GYRO_YOUT | 0x80 /* read bit*/);
        gyro0[1]=(int16_t)RSPI0.SPDR.LONG ; 
        gyro1[1]=(int16_t)RSPI1.SPDR.LONG ; 

        sendspi24(MPUREG_GYRO_ZOUT | 0x80 /* read bit*/);
        gyro0[2]=(int16_t)RSPI0.SPDR.LONG ; 
        gyro1[2]=(int16_t)RSPI1.SPDR.LONG ; 

        sendspi24(MPUREG_ACCEL_XOUT | 0x80 /* read bit*/);
        accel0[0]=(int16_t)RSPI0.SPDR.LONG ; 
        accel1[0]=(int16_t)RSPI1.SPDR.LONG ; 

        sendspi24(MPUREG_ACCEL_YOUT | 0x80 /* read bit*/);
        accel0[1]=(int16_t)RSPI0.SPDR.LONG ; 
        accel1[1]=(int16_t)RSPI1.SPDR.LONG ; 

        sendspi24(MPUREG_ACCEL_ZOUT | 0x80 /* read bit*/);
        accel0[2]=(int16_t)RSPI0.SPDR.LONG ; 
        accel1[2]=(int16_t)RSPI1.SPDR.LONG ; 
        
        
    }

}

