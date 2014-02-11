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

#include "ior32c111.h"
#include <intrinsics.h>
#include "hwsetup.h"
#include "main.h"
#include "SPI.h"

static unsigned char tmp_speed;
static unsigned char tmp_acceleration;
static volatile signed short int ticks;
static volatile unsigned char recv_buf_ardu;
static volatile unsigned char command=CMD_NONE;
static union u16 ticks_u = {0,0};

volatile unsigned char recv_bytenum=0;
volatile unsigned char fpga_in=FPGA_EMPTY;
volatile unsigned char speed[2]={0,0};
volatile unsigned char acceleration[2]={0,0};

__fast_interrupt void _uart5_receive(void) {    
    recv_buf_ardu=u5rb & 0xff;
    
    /* Process this only if FPGA is loaded */
    if(fpga_in == FPGA_LOADING) {
LED4=1;
        /* bypass byte transparently */
        u0tb=
        u6tb=
        u3tb=
        u4tb=recv_buf_ardu;
LED4=0;
    } else { // Probably FPGA_LOADED
        switch(recv_bytenum) {
        case 0:
LED1=1;
            command=recv_buf_ardu & 0xFC;
            switch(command) {
            case CMD_SPEED:
                break;
            case CMD_ACCELERATION:
                break;
            case CMD_GET_CUR_TARGET_SPEED:
                break;
            default:
                LED5=1; // ERROR!!!
                break;
            }
LED1=0;
            break;
        case 1:
LED2=1;
            switch(command) {
            case CMD_SPEED:
                tmp_speed=recv_buf_ardu;
                break;
            case CMD_ACCELERATION:
                tmp_acceleration=recv_buf_ardu;
                break;
            case CMD_GET_CUR_TARGET_SPEED:
                /* ? */
                break;
            default:
                LED5=1; // ERROR!!!
                break;
            }
LED2=0;
            break;
        case 2:
LED3=1;
            switch(command) {
            case CMD_SPEED:
                speed[0]=tmp_speed ;
                speed[1]=recv_buf_ardu;
                break;
            case CMD_ACCELERATION:
                acceleration[0]=tmp_acceleration;
                acceleration[1]=recv_buf_ardu;
                break;
            case CMD_GET_CUR_TARGET_SPEED:
                /* ? */
                break;
            default:
                LED5=1; // ERROR!!!
                break;
            }
LED3=0;
            break;
        case 3:
            ticks_u.ticks[0]=recv_buf_ardu;
            break;
        case 4:
            ticks_u.ticks[1]=recv_buf_ardu;
            ticks=ticks_u.x;
            break;
        default:
            LED5=1;
            __no_operation();
            __no_operation();
            __no_operation();
            __no_operation();
            LED5=0; 
        }
        recv_bytenum++;
    }
    
    /* Clear the 'reception complete' flag.	*/
    ir_s5ric = 0;
}

void
SPI5_Init(void) {
    pu27=1; // Enable pullup to avoid floating pin noise on p7_7 (clock)
    CLOCK5s = PF_UART;
    TX5d = PD_OUTPUT;
    TX5s = PF_UART;
    RX5s = PF_UART;

    smd0_u5mr  = 1;                                        // \ 
    smd1_u5mr  = 0;                                        //  | Synchronous Serial Mode
    smd2_u5mr  = 0;                                        // /

    ckdir_u5mr = 1;                                        // 1=external clock   
    stps_u5mr  = 0;                                        // 0=1 stop bit, 0 required
    pry_u5mr   = 0;                                        // Parity, 0=odd, 0 required 
    prye_u5mr  = 0;                                        // Parity Enable? 0=disable, 0 required 
    iopol_u5mr = myIOPOL;                                  // IO Polarity, 0=not inverted, 0 required

    clk0_u5c0 = 0;                                         // Clock source f1 for u5brg
    clk1_u5c0 = 0;                                         // 
    txept_u5c0 = 0;                                        // Transmit register empty flag 
    crd_u5c0 = 1;                                          // CTS disabled when 1
    nch_u5c0 = 0;                                          // 0=Output mode "push-pull" for TXD and CLOCK pin 
    ckpol_u5c0 = myCKPOL;                                  // CLK Polarity 0 rising edge, 1 falling edge
    uform_u5c0 = 1;                                        // 1=MSB first

    te_u5c1 = 1;                                           // 1=Transmission Enable
    ti_u5c1 = 0;                                           // Must be 0 to send or receive
    re_u5c1 = 1;                                           // Reception Enable when 1
    ri_u5c1 = 0;                                           // Receive complete flag - U5RB is empty.
    u5irs_u5c1 = 1;                                        // Interrupt when transmission is completed.
    u5rrm_u5c1 = 1;                                        // Continuous receive mode off
    u5lch_u5c1 = 1;                                        // Logical inversion off 

    u5smr = 0x00;
    u5smr2 = 0x00;

    sse_u5smr3 = 0;                                        // SS is disabled when 0
    ckph_u5smr3 = myCKPH;                                  // Non clock delayed 
    dinc_u5smr3 = 0;                                       // Slave mode when 1. Unsure if should be unless SS is used too
    nodc_u5smr3 = 0;                                       // Select a clock output  mode "push-pull" when 0 
    err_u5smr3 = 0;                                        // Error flag, no error when 0 
    dl0_u5smr3 = 0;                                        // Set 0 for no  delay 
    dl1_u5smr3 = 0;                                        // Set 0 for no  delay 
    dl2_u5smr3 = 0;                                        // Set 0 for no  delay 

    u5smr4 = 0x00;

    __disable_interrupt();
    /* 
     * Fastest interrupt priority
     */
    fsit_ripl1 = 1;
    fsit_ripl2 = 1;
    __set_VCT_register((unsigned long)&_uart5_receive);
    ilvl_s5ric =7; // fast interrupt
    ir_s5ric   =0;            
    ilvl_s5tic =0;
    ir_s5tic   =0;            
    __enable_interrupt();
}

