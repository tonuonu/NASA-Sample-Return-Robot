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

/* SPI Master receive buffer */
volatile uint8_t gMasterRxBuffer[32];
/* Transmit message buffer variable (imported) */
extern const uint8_t gTransmitMessage[32];

/* Slave receive interrupt function prototypes */
void SlaveRxInterrupt_SPI(void);
/* Master receive interrupt function prototypes */
void MasterRxInterrupt_SPI(void);

void Init_SPI(void) { 

  /* Set pins from general IO to SPI */
  /* RSPI A */
  PORTC.PMR.BIT.B4=1; // CS
  PORTC.PMR.BIT.B5=1; // CLK
  PORTC.PMR.BIT.B6=1; // MOSI
  PORTC.PMR.BIT.B7=1; // MISO
  
  /* enable pullups */
  PORTC.PCR.BIT.B4=1; // CS
  PORTC.PCR.BIT.B5=1; // CLK
  PORTC.PCR.BIT.B6=1; // MOSI
  PORTC.PCR.BIT.B7=1; // MISO
  
  /* RSPI B */
  PORTE.PMR.BIT.B1=1; // CLK
  PORTE.PMR.BIT.B4=1; // CS
  PORTE.PMR.BIT.B6=1; // MOSI
  PORTE.PMR.BIT.B7=1; // MISO

  /* enable pullups */
  PORTE.PCR.BIT.B1=1; // CLK
  PORTE.PCR.BIT.B4=1; // CS
  PORTE.PCR.BIT.B6=1; // MOSI
  PORTE.PCR.BIT.B7=1; // MISO
    
  /* Disable MPC register protection */
  MPC.PWPR.BIT.B0WI = 0;
  MPC.PWPR.BIT.PFSWE = 1;
  
  /* Disable PFS register protection */
  SYSTEM.PRCR.WORD = 0xA50B;  
  
  /* Configure channel A SPI pins */
  MPC.PC4PFS.BYTE = 0x0D; // 01101b is RSPI mode
  MPC.PC5PFS.BYTE = 0x0D;
  MPC.PC6PFS.BYTE = 0x0D;
  MPC.PC7PFS.BYTE = 0x0D;
  
  /* Configure channel B SPI pins */
  MPC.PE1PFS.BYTE = 0x0E; // 01110b for this one only
  MPC.PE4PFS.BYTE = 0x0D; // 01101b again
  MPC.PE6PFS.BYTE = 0x0D;
  MPC.PE7PFS.BYTE = 0x0D;  

  /* Enable RSPI0 and 1 peripherals */
  MSTP_RSPI0 = 0;
  MSTP_RSPI1 = 0;

  /* Re-enable PFS register protection */
  SYSTEM.PRCR.WORD = 0xA500u;
  
  /* Configure SPI to operate in 3-wire mode. CONFUSING! This is not MOSI-MISO on same wire!  */
  RSPI0.SPCR.BIT.SPMS = 1;
  RSPI1.SPCR.BIT.SPMS = 1;
  /* Full duplex mode */
  RSPI0.SPCR.BIT.TXMD = 0;
  RSPI1.SPCR.BIT.TXMD = 0;
  
  /* Set SPI master */
  RSPI0.SPCR.BIT.MSTR = 1;
  RSPI1.SPCR.BIT.MSTR = 1;
  
  /* Enable receive interrupt for both channels */
//  ICU.IER[4].BIT.IEN7 = 1;
//  ICU.IER[5].BIT.IEN2 = 1;
//  RSPI1.SPCR.BIT.SPRIE = 1;
//  RSPI0.SPCR.BIT.SPRIE = 1;
  
  /* Set receive interrupt priority for both channels */
//  ICU.IPR[39].BYTE = 5u;
//  ICU.IPR[42].BYTE = 6u;
      
  /* Set bit rate */
  RSPI0.SPBR = 255u;
  RSPI1.SPBR = 255u;
  
  /* Set number of transmit frames to 1 for both channels */
  RSPI1.SPDCR.BIT.SPFC = 0;
  RSPI0.SPDCR.BIT.SPFC = 0;
      
  /* Set command register 0 data length to 16bits */
  RSPI1.SPCMD0.BIT.SPB = 0xF;
  RSPI0.SPCMD0.BIT.SPB = 0xF;
  
  /* Set command register 0 bit rate divisor to 0 */
  RSPI1.SPCMD0.BIT.BRDV = 0;
  RSPI0.SPCMD0.BIT.BRDV = 0;

  /* Empty the transmit buffer for both channels */
  RSPI1.SPDR.LONG = 0x00000000;
  RSPI0.SPDR.LONG = 0x00000000;
    
  /* Enable both SPI channels */
  RSPI1.SPCR.BIT.SPE = 1;
  RSPI0.SPCR.BIT.SPE = 1;
}

#if 0
void MasterSend_SPI(uint16_t num_bytes, uint8_t * message) {
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
  
}
#endif

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
