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

#include "spi.h"
#include "iorx630.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "oled.h"
#include "usb_hal.h"
#include "usb.h"
#include "usb_cdc.h"


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

#if 0
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
#endif

//extern volatile bool mems_realtime;

void 
read_gyro(void) {
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

void
Init_Gyros(void) {
    /* 
     * In some weird reason this reset breaks everything 
     * Fix if you can.
     */ 
    // sendspi16cmd(MPUREG_PWR_MGMT_1,0x80); // reset
    // __delay_cycles(100UL*100000UL); // 100ms delay from GYRO reset is mandatory to wait

    sendspi16cmd(MPUREG_PWR_MGMT_1,0x00); // wake up    
    sendspi16cmd(MPUREG_USER_CTRL, BIT_I2C_IF_DIS); // Disable I2C. 
    sendspi16cmd(MPUREG_PWR_MGMT_1,0x03); // wake up and Z axis is reference.
    sendspi16cmd(MPUREG_PWR_MGMT_2,0x00); // ?
    sendspi16cmd(MPUREG_GYRO_CONFIG,BITS_FS_500DPS); 
    sendspi16cmd(MPUREG_ACCEL_CONFIG,BITS_FS_4G);
    sendspi16cmd(MPUREG_SMPLRT_DIV,0); // maximum sample rate. Why not :)
                                       // Fsample= 1khz/(0+1) = 1kHz     
    // DLPF = 42Hz (low pass filter)
    // sendspi16cmd(MPUREG_CONFIG, BITS_DLPF_CFG_42HZ);  // BITS_DLPF_CFG_20HZ BITS_DLPF_CFG_42HZ BITS_DLPF_CFG_98HZ
  
}




/* SPI Master receive buffer */
volatile uint8_t gMasterRxBuffer[32];
/* Transmit message buffer variable (imported) */
extern const uint8_t gTransmitMessage[32];

/* Slave receive interrupt function prototypes */
void SlaveRxInterrupt_SPI(void);
/* Master receive interrupt function prototypes */
void MasterRxInterrupt_SPI(void);

void Init_SPI(void) { 

  SYSTEM.PRCR.WORD = 0xA502;
  /* Enable RSPI0 and 1 peripherals */
  MSTP_RSPI0 = 0;
  MSTP_RSPI1 = 0;
  
  SYSTEM.PRCR.WORD = 0xA500;

  RSPI0.SSLP.BIT.SSL0P=0; // SSL0 is active low
  RSPI1.SSLP.BIT.SSL0P=0; // SSL0 is active low

  /* Set number of transmit frames to 1 for both channels */
  RSPI0.SPDCR.BIT.SPFC = 0;
  RSPI1.SPDCR.BIT.SPFC = 0;
  
  RSPI0.SPDCR.BIT.SPRDTD = 0; // 
  RSPI1.SPDCR.BIT.SPRDTD = 0; // 
  
  RSPI0.SPDCR.BIT.SPLW = 1;
  RSPI1.SPDCR.BIT.SPLW = 1;
  
  RSPI0.SPCR2.BIT.SPPE=0;
  RSPI1.SPCR2.BIT.SPPE=0;
  
  RSPI0.SPCR2.BIT.SPIIE=0;
  RSPI1.SPCR2.BIT.SPIIE=0;
  
  RSPI0.SPCR2.BIT.PTE=0;
  RSPI1.SPCR2.BIT.PTE=0;
      
  RSPI0.SPCMD0.BIT.CPHA=0;
  RSPI1.SPCMD0.BIT.CPHA=0;

  RSPI0.SPCMD0.BIT.CPOL=0;
  RSPI1.SPCMD0.BIT.CPOL=0;

  RSPI0.SPBR=0;
  RSPI1.SPBR=0;

  RSPI0.SPCMD0.BIT.BRDV=0;
  RSPI1.SPCMD0.BIT.BRDV=0;
  
  RSPI0.SPCMD0.BIT.SSLA=0;
  RSPI1.SPCMD0.BIT.SSLA=0;
  
  RSPI0.SPCMD0.BIT.SSLKP=0;
  RSPI1.SPCMD0.BIT.SSLKP=0;
  
  RSPI0.SPCMD0.BIT.SPB=0xF; // 16 bits data length
  RSPI1.SPCMD0.BIT.SPB=0xF;
  
  RSPI0.SPCMD0.BIT.LSBF=0;
  RSPI1.SPCMD0.BIT.LSBF=0;

  RSPI0.SPCMD0.BIT.SPNDEN=0;
  RSPI1.SPCMD0.BIT.SPNDEN=0;
  
  RSPI0.SPCMD0.BIT.SLNDEN=0;
  RSPI1.SPCMD0.BIT.SLNDEN=0;
  
  RSPI0.SPCMD0.BIT.SCKDEN=0;
  RSPI1.SPCMD0.BIT.SCKDEN=0;
   
  /* Set pins from general IO to SPI */
  /* RSPI A */
  PORTC.PMR.BIT.B4=0; // CS
  PORTC.PMR.BIT.B5=0; // CLK
  PORTC.PMR.BIT.B6=0; // MOSI
  PORTC.PMR.BIT.B7=0; // MISO
    
  /* RSPI B */
  PORTE.PMR.BIT.B1=0; // CLK
  PORTE.PMR.BIT.B4=0; // CS
  PORTE.PMR.BIT.B6=0; // MOSI
  PORTE.PMR.BIT.B7=0; // MISO
  
  /* Disable MPC register protection */
  MPC.PWPR.BIT.B0WI = 0; // Enable writing to PFSWE bit
  MPC.PWPR.BIT.PFSWE = 1; // Enable writing to PFS register
    
  /* Configure channel A SPI pins */
  MPC.PC4PFS.BIT.PSEL=0xD; // 01101b is RSPI mode
  MPC.PC5PFS.BIT.PSEL=0xD;
  MPC.PC6PFS.BIT.PSEL=0xD;
  MPC.PC7PFS.BIT.PSEL=0xD;
  
  /* Configure channel B SPI pins */
  MPC.PE1PFS.BYTE = 0x0E; // 01110b for this pineonly
  MPC.PE4PFS.BYTE = 0x0D; // 01101b again
  MPC.PE6PFS.BYTE = 0x0D;
  MPC.PE7PFS.BYTE = 0x0D;  

  MPC.PWPR.BIT.PFSWE = 0; // Disable writing to PFS register
  MPC.PWPR.BIT.B0WI = 1; // Disable writing to PFSWE bit

  /* Set pins from general IO to SPI */
  /* RSPI A */
  PORTC.PMR.BIT.B4=1; // CS
  PORTC.PMR.BIT.B5=1; // CLK
  PORTC.PMR.BIT.B6=1; // MOSI
  PORTC.PMR.BIT.B7=1; // MISO
  /* RSPI B */
  PORTE.PMR.BIT.B1=1; // CLK
  PORTE.PMR.BIT.B4=1; // CS
  PORTE.PMR.BIT.B6=1; // MOSI
  PORTE.PMR.BIT.B7=1; // MISO
  
  /* Configure SPI to operate in 4-wire mode. CONFUSING! This is not MOSI-MISO on same wire!  */
  RSPI0.SPCR.BIT.SPMS = 0;
  RSPI1.SPCR.BIT.SPMS = 0;

  /* Full duplex mode */
  RSPI0.SPCR.BIT.TXMD = 0;
  RSPI1.SPCR.BIT.TXMD = 0;

  RSPI0.SPCR.BIT.MODFEN = 0; // Must be 0 for Single-Master Mode
  RSPI1.SPCR.BIT.MODFEN = 0; // Must be 0 for Single-Master Mode

  /* Set SPI master */
  RSPI0.SPCR.BIT.MSTR = 1;
  RSPI1.SPCR.BIT.MSTR = 1;

  RSPI0.SPCR.BIT.SPEIE = 0;
  RSPI1.SPCR.BIT.SPEIE = 0;

  RSPI0.SPCR.BIT.SPTIE = 0;
  RSPI1.SPCR.BIT.SPTIE = 0;

  RSPI0.SPCR.BIT.SPE = 0;
  RSPI1.SPCR.BIT.SPE = 0;

  RSPI0.SPCR.BIT.SPRIE = 0;
  RSPI1.SPCR.BIT.SPRIE = 0;
  
  /* Empty the transmit buffer for both channels */
  RSPI0.SPDR.LONG = 0x00000000;
  RSPI1.SPDR.LONG = 0x00000000;
    
  /* Enable both SPI channels */
  RSPI0.SPCR.BIT.SPE = 1;
  RSPI1.SPCR.BIT.SPE = 1;
}

/*******************************************************************************
* Outline     : Excep_RSPI0_SPRI0
* Description   : Slave receive interrupt handler. Function is triggered when
*          SPI channel 0 receives data, and calls the 
*          SlaveRxInterrupt_SPI function.
* Argument      : none
* Return value  : none
*******************************************************************************/
#pragma vector=VECT_RSPI0_SPRI0
__interrupt void Excep_RSPI0_SPRI0(void) {
  /* Call the slave RX interrupt function */
  SlaveRxInterrupt_SPI();
}

/*******************************************************************************
* Outline     : Excep_RSPI1_SPRI1
* Description   : Slave receive interrupt handler. Function is triggered when
*          SPI channel 1 receives data, and calls the 
*          MasterRxInterrupt_SPI function.
* Argument      : none
* Return value  : none
*******************************************************************************/
#pragma vector=VECT_RSPI1_SPRI1
__interrupt void Excep_RSPI1_SPRI1(void) {
  /* Call the master RX interrupt function */
  MasterRxInterrupt_SPI();
}

void SlaveRxInterrupt_SPI(void) {
  /* Declare temporary slave rx frame buffer, and fetch value from SPI unit */
  uint16_t slave_rx = (uint16_t) RSPI0.SPDR.LONG;
  
  /* Declare temporary slave tx frame buffer */
  uint16_t slave_tx = 0;
  
  /* Check if master is requesting data */
  if((slave_rx & 0xFF00) == 0xA500)
  {
    /* Resend the last byte received with 0xC5 in front */
    slave_tx = (uint16_t) (0xC500 | (slave_rx & 0x00FF));
    
    /* Copy slave transmit data to the SPI unit */
    RSPI0.SPDR.LONG = (uint32_t) slave_tx << 16;
  }
  /* Master is requesting final byte */
  else if((slave_rx & 0xFF00) == 0xA300)
  {
    /* Resend the last byte received with 0xC3 in front */
    slave_tx = (uint16_t) (0xC300 | (slave_rx & 0x00FF));
    
    /* Copy slave transmit data to the SPI unit */
    RSPI0.SPDR.LONG = (uint32_t) slave_tx << 16;
  }
  /* Master is not sending valid request */
  else
  {
    /* Empty the transmit buffer */
    RSPI0.SPDR.LONG = 0x00000000;
  }
}

void MasterRxInterrupt_SPI(void) {
  /* Declare static frame count variable */
  static uint16_t master_rx_count = 0;
  
  /* Declare temporary master rx frame buffer, and fetch value from SPI unit */
  uint16_t master_rx = (uint16_t) RSPI1.SPDR.LONG;
      
  /* Check if slave is sending valid frame */
  if((master_rx  & 0xFF00) == 0xC500)
  {
    /* Extract byte from received data frame, append to master receive buffer */
    gMasterRxBuffer[master_rx_count] = (uint8_t) master_rx;
  
    /* Increment the number of received frames */
    master_rx_count++;
  }
  /* Check if the slave is sending a final frame */
  else if((master_rx  & 0xFF00) == 0xC300)
  {
    /* Extract final byte from received data frame, append to master receive buffer */
    gMasterRxBuffer[master_rx_count] = (uint8_t) master_rx;
     
    /* Reset the receive count to 0 */
    master_rx_count = 0;
  }
  /* Invalid data frame received from slave */
  else
  {
    /* Do nothing */
  }
}
