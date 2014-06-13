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

#include "ior32c111.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "hwsetup.h"
#include "UART.h"
#include "SPI.h"

#define IDEAL_VOLTAGE_CODE		((int16_t)(48 + 12*67.65))	// corresponds to 12.0V

//define DEBUG_MOTO
#define LOG_LEN 3072

struct twobyte_st tmprecv[4] = {0,0,0,0,0,0,0,0};

volatile unsigned char motor_load[4] = {0,0,0,0};
volatile struct twobyte_st ticks[4] = {0,0,0,0,0,0,0,0};
volatile struct twobyte_st cur_cmd_param[4] = {0,0x80,0,0x80,0,0x80,0,0x80};
volatile unsigned char cur_cmd[4] = {CMD_ACCELERATION,CMD_ACCELERATION,CMD_ACCELERATION,CMD_ACCELERATION};
//volatile struct twobyte_st cur_cmd_param[4] = {0,0,0,0,0,0,0,0};
//volatile unsigned char cur_cmd[4] = {CMD_SPEED,CMD_SPEED,CMD_SPEED,CMD_SPEED};

static int measurement_idx=0;
struct twobyte_st voltage[3][4]=
				{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
struct twobyte_st cur_target_speed[3][4]=
				{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};

static unsigned int UART_to_motor_id[4]={0,1,2,3};
static unsigned int motor_online[4]={0,0,0,0};

struct twobyte_st dbg_pid[4] = {0,0,0,0};
struct twobyte_st dbg_switch[4] = {0,0,0,0};
struct twobyte_st dbg_curr[4] = {0,0,0,0};
struct twobyte_st calib_data[64][4];
static int calib_i[4][3] = {{-15,16,-4},{5,-1,-6},{8,12,15},{-6,-16,-7}};//Kõu
//static int calib_i[4][3] = {{9,-16,-6},{1,-15,-10},{-7,-6,-16},{-3,-16,-11}};//Öö
//static int calib_i[4][3] = {{-16,6,-16},{-16,-16,-16},{-16,-11,-16},{15,-16,-16}};//Säde
static int calib_t[4] = {33,78,96,111};
unsigned char log_cmd[LOG_LEN][4];
struct twobyte_st log_param[LOG_LEN][4];
static int16_t log_time[LOG_LEN];
static char log_state[LOG_LEN];
int16_t cur_time = 0;
int log_ptr = 0;
int log_cnt = 0;
char log_active = 0;

int16_t calc_median3(const struct twobyte_st values[3][4],
											const unsigned int motor_idx) {
    const int16_t v[3]={values[0][motor_idx].u.int16,
						values[1][motor_idx].u.int16,
						values[2][motor_idx].u.int16};

    unsigned int median_value_idx=(v[0] < v[1]) ? 1 : 0;

    if (v[2] >= v[median_value_idx])
        return v[median_value_idx];
    if (v[2] <= v[1-median_value_idx])
        return v[1-median_value_idx];
    return v[2];
}

static int is_voltage_valid(const int16_t voltage) {
    return (voltage && voltage != (int16_t)0xffff);
}

static void
receive_ticks(void) {
  
    complete_pretx();
    M0TX=M1TX=M2TX=M3TX = 0;

    complete_rx();
    tmprecv[UART_to_motor_id[0]].u.byte[1]=M0RX & 0xff;
    tmprecv[UART_to_motor_id[1]].u.byte[1]=M1RX & 0xff;
    tmprecv[UART_to_motor_id[2]].u.byte[1]=M2RX & 0xff;
    tmprecv[UART_to_motor_id[3]].u.byte[1]=M3RX & 0xff;

    complete_pretx();
    M0TX=M1TX=M2TX=M3TX = 0;

    complete_rx();
    tmprecv[UART_to_motor_id[0]].u.byte[0]=M0RX & 0xff;
    tmprecv[UART_to_motor_id[1]].u.byte[0]=M1RX & 0xff;
    tmprecv[UART_to_motor_id[2]].u.byte[0]=M2RX & 0xff;
    tmprecv[UART_to_motor_id[3]].u.byte[0]=M3RX & 0xff;

#ifdef DEBUG_MOTO
    complete_pretx();
    M0TX=M1TX=M2TX=M3TX = 0;

    complete_rx();
    dbg_pid[UART_to_motor_id[0]].u.byte[1]=M0RX & 0xff;
    dbg_pid[UART_to_motor_id[1]].u.byte[1]=M1RX & 0xff;
    dbg_pid[UART_to_motor_id[2]].u.byte[1]=M2RX & 0xff;
    dbg_pid[UART_to_motor_id[3]].u.byte[1]=M3RX & 0xff;

    complete_pretx();
    M0TX=M1TX=M2TX=M3TX = 0;

    complete_rx();
    dbg_pid[UART_to_motor_id[0]].u.byte[0]=M0RX & 0xff;
    dbg_pid[UART_to_motor_id[1]].u.byte[0]=M1RX & 0xff;
    dbg_pid[UART_to_motor_id[2]].u.byte[0]=M2RX & 0xff;
    dbg_pid[UART_to_motor_id[3]].u.byte[0]=M3RX & 0xff;

    complete_pretx();
    M0TX=M1TX=M2TX=M3TX = 0;

    complete_rx();
    dbg_switch[UART_to_motor_id[0]].u.byte[1]=M0RX & 0xff;
    dbg_switch[UART_to_motor_id[1]].u.byte[1]=M1RX & 0xff;
    dbg_switch[UART_to_motor_id[2]].u.byte[1]=M2RX & 0xff;
    dbg_switch[UART_to_motor_id[3]].u.byte[1]=M3RX & 0xff;    

    complete_pretx();
    M0TX=M1TX=M2TX=M3TX = 0;

    complete_rx();
    dbg_switch[UART_to_motor_id[0]].u.byte[0]=M0RX & 0xff;
    dbg_switch[UART_to_motor_id[1]].u.byte[0]=M1RX & 0xff;
    dbg_switch[UART_to_motor_id[2]].u.byte[0]=M2RX & 0xff;
    dbg_switch[UART_to_motor_id[3]].u.byte[0]=M3RX & 0xff;    
    complete_pretx();
    M0TX=M1TX=M2TX=M3TX = 0;

    complete_rx();
    dbg_curr[UART_to_motor_id[0]].u.byte[1]=M0RX & 0xff;
    dbg_curr[UART_to_motor_id[1]].u.byte[1]=M1RX & 0xff;
    dbg_curr[UART_to_motor_id[2]].u.byte[1]=M2RX & 0xff;
    dbg_curr[UART_to_motor_id[3]].u.byte[1]=M3RX & 0xff;    

    complete_pretx();
    M0TX=M1TX=M2TX=M3TX = 0;

    complete_rx();
    dbg_curr[UART_to_motor_id[0]].u.byte[0]=M0RX & 0xff;
    dbg_curr[UART_to_motor_id[1]].u.byte[0]=M1RX & 0xff;
    dbg_curr[UART_to_motor_id[2]].u.byte[0]=M2RX & 0xff;
    dbg_curr[UART_to_motor_id[3]].u.byte[0]=M3RX & 0xff;    
#endif
    
    complete_tx();
    CS0=CS1=CS2=CS3 = 1;

#ifdef DEBUG_MOTO
    UART1_Dump(cur_target_speed[measurement_idx], tmprecv, dbg_pid, dbg_switch, dbg_curr);
#endif

    if (!motor_online[0])
        tmprecv[0].u.int16=0;
    if (!motor_online[1])
        tmprecv[1].u.int16=0;
    if (!motor_online[2])
        tmprecv[2].u.int16=0;
    if (!motor_online[3])
        tmprecv[3].u.int16=0;

    for(int i=0;i<=3;i++) {
        motor_load[i] = (tmprecv[i].u.int16 >> 9);

        int16_t ticks_increment = tmprecv[i].u.int16 & 0x01ff; // keep 9 bits only
        if (ticks_increment & 0x0100)       // if   xxxx xxx1 xxxx xxxx
            ticks_increment |= 0xff00 ;     // then 1111 1111 xxxx xxxx

        const int32_t x = (uint32_t)ticks[i].u.int16 + (uint32_t)ticks_increment;
        /* Check for possible overflow of INT16 and light red LED */
        if(x > INT16_MAX || x < INT16_MIN)
            LED5=1;
        else {
            __disable_interrupt();
            ticks[i].u.int16+=ticks_increment;
            __enable_interrupt();
        }
    }   
}

void
dump_log(void) {
  int ptr = log_ptr;
  char buf[64];

  sprintf(buf, "\n%d:\n", cur_time);
  for (int j=0; j<strlen(buf); j++)
    UART1_Char(buf[j]);

  for (int i=0; i<log_cnt; i++) {
    UART1_DumpLog(log_time[ptr], log_cmd[ptr], log_param[ptr], log_state[ptr]);
    if (++ptr >= LOG_LEN)
      ptr = 0;
  }
}

static int
send_cur_cmd(const int force_cmd,const int force_param) {
    if(RESET5 == 0) {
        return 0;
    }
    char motor_map = motor_online[0] | (motor_online[1]<<1) | (motor_online[2]<<2) | (motor_online[3]<<3);
    if (log_active || (motor_map==0xf)) {
      for (int i=0; i<4; i++) {
        log_cmd[log_ptr][i] = cur_cmd[i];
        log_param[log_ptr][i].u.int16 = cur_cmd_param[i].u.int16;
        log_time[log_ptr] = cur_time;
        log_state[log_ptr] = motor_map;
      }
      if (++log_ptr >= LOG_LEN)
        log_ptr = 0;
      if (log_cnt < LOG_LEN)
        log_cnt++;
    }
    log_active = motor_map==0xf;
    cur_time++;
    
    complete_tx(); // make sure we are not transmitting garbage already

    { volatile unsigned char dummy=M0RX; }
    { volatile unsigned char dummy=M1RX; }
    { volatile unsigned char dummy=M2RX; }
    { volatile unsigned char dummy=M3RX; }

    /* 
     * Use temporary variable to ensure interrupts do not overwrite
     * value while we send it. 
     */
    struct twobyte_st tmp[4];
	unsigned char cmds[4];

	int all_motors_stopped=1;
	{ for (int i=0;i < 4;i++) {
		cmds[i]=(force_cmd >= 0 ? force_cmd : cur_cmd[UART_to_motor_id[i]]);
	    tmp[i].u.int16=(force_cmd >= 0 ? force_param :
							cur_cmd_param[UART_to_motor_id[i]].u.int16);
		if (cmds[i] != CMD_SPEED || tmp[i].u.int16 != 0)
			all_motors_stopped=0;
		cmds[i]|=UART_to_motor_id[i];
		}}

    CS0=CS1=CS2=CS3 = 0;

    M0TX=cmds[0];
    M1TX=cmds[1];
    M2TX=cmds[2];
    M3TX=cmds[3];

    complete_rx();

    { volatile unsigned char dummy=M0RX; }
    { volatile unsigned char dummy=M1RX; }
    { volatile unsigned char dummy=M2RX; }
    { volatile unsigned char dummy=M3RX; }

    complete_pretx();

    M0TX=tmp[0].u.byte[1];
    M1TX=tmp[1].u.byte[1];
    M2TX=tmp[2].u.byte[1];
    M3TX=tmp[3].u.byte[1]; 

    complete_rx();

    tmprecv[UART_to_motor_id[0]].u.byte[1]=M0RX & 0xff;
    tmprecv[UART_to_motor_id[1]].u.byte[1]=M1RX & 0xff;
    tmprecv[UART_to_motor_id[2]].u.byte[1]=M2RX & 0xff;
    tmprecv[UART_to_motor_id[3]].u.byte[1]=M3RX & 0xff;

    complete_pretx();

    M0TX=tmp[0].u.byte[0];
    M1TX=tmp[1].u.byte[0];
    M2TX=tmp[2].u.byte[0];
    M3TX=tmp[3].u.byte[0];

    complete_rx();

    tmprecv[UART_to_motor_id[0]].u.byte[0]=M0RX & 0xff;
    tmprecv[UART_to_motor_id[1]].u.byte[0]=M1RX & 0xff;
    tmprecv[UART_to_motor_id[2]].u.byte[0]=M2RX & 0xff;
    tmprecv[UART_to_motor_id[3]].u.byte[0]=M3RX & 0xff;

    if (!motor_online[0])
        tmprecv[0].u.int16=0;
    if (!motor_online[1])
        tmprecv[1].u.int16=0;
    if (!motor_online[2])
        tmprecv[2].u.int16=0;
    if (!motor_online[3])
        tmprecv[3].u.int16=0;

    for(int i=0;i<=3;i++)
        cur_target_speed[measurement_idx][i].u.int16 = tmprecv[i].u.int16;

	receive_ticks();
	return all_motors_stopped;
}

static void 
get_voltage(void) {
  
    if(RESET5 == 0) {
        return;
    }
    complete_tx();
    CS0=CS1=CS2=CS3 = 0;

    M0TX=CMD_GET_VOLTAGE | UART_to_motor_id[0];
    M1TX=CMD_GET_VOLTAGE | UART_to_motor_id[1];
    M2TX=CMD_GET_VOLTAGE | UART_to_motor_id[2];
    M3TX=CMD_GET_VOLTAGE | UART_to_motor_id[3];
    complete_pretx();
    
    M0TX=M1TX=M2TX=M3TX=0;
    complete_tx();

    tmprecv[UART_to_motor_id[0]].u.byte[1]=M0RX & 0xff;
    tmprecv[UART_to_motor_id[1]].u.byte[1]=M1RX & 0xff;
    tmprecv[UART_to_motor_id[2]].u.byte[1]=M2RX & 0xff;
    tmprecv[UART_to_motor_id[3]].u.byte[1]=M3RX & 0xff;

    M0TX=M1TX=M2TX=M3TX=0;
    complete_tx();

    tmprecv[UART_to_motor_id[0]].u.byte[0]=M0RX & 0xff;
    tmprecv[UART_to_motor_id[1]].u.byte[0]=M1RX & 0xff;
    tmprecv[UART_to_motor_id[2]].u.byte[0]=M2RX & 0xff;
    tmprecv[UART_to_motor_id[3]].u.byte[0]=M3RX & 0xff;

    CS0=CS1=CS2=CS3 = 1;

    if (!CDONE0)
        tmprecv[UART_to_motor_id[0]].u.int16=0;
    if (!CDONE1)
        tmprecv[UART_to_motor_id[1]].u.int16=0;
    if (!CDONE2)
        tmprecv[UART_to_motor_id[2]].u.int16=0;
    if (!CDONE3)
        tmprecv[UART_to_motor_id[3]].u.int16=0;

    for(int i=0;i<=3;i++)
        voltage[measurement_idx][i].u.int16 = tmprecv[i].u.int16;
}

static void 
calibrate(void) {
  
    if(RESET5 == 0) {
        return;
    }
    
    for (int i=0; i<16; i++)
      for (int j=0; j<4; j++)
        for (int m=0; m<4; m++)
          calib_data[4*i+j][m].u.int16 = 1024*((i&7)>5 ? 0 : calib_i[m][(i>>1)&3])+calib_t[j];
    
    complete_tx();
    CS0=CS1=CS2=CS3 = 0;

    M0TX=CMD_CALIBRATE | UART_to_motor_id[0];
    M1TX=CMD_CALIBRATE | UART_to_motor_id[1];
    M2TX=CMD_CALIBRATE | UART_to_motor_id[2];
    M3TX=CMD_CALIBRATE | UART_to_motor_id[3];
    complete_pretx();
    
    for (int i=0; i<64; i++) {
      M0TX=calib_data[i][UART_to_motor_id[0]].u.byte[1];
      M1TX=calib_data[i][UART_to_motor_id[1]].u.byte[1];
      M2TX=calib_data[i][UART_to_motor_id[2]].u.byte[1];
      M3TX=calib_data[i][UART_to_motor_id[3]].u.byte[1];
      complete_pretx();
      M0TX=calib_data[i][UART_to_motor_id[0]].u.byte[0];
      M1TX=calib_data[i][UART_to_motor_id[1]].u.byte[0];
      M2TX=calib_data[i][UART_to_motor_id[2]].u.byte[0];
      M3TX=calib_data[i][UART_to_motor_id[3]].u.byte[0];
      complete_pretx();
    }
    complete_tx();
    { volatile unsigned char dummy=M0RX; }
    { volatile unsigned char dummy=M1RX; }
    { volatile unsigned char dummy=M2RX; }
    { volatile unsigned char dummy=M3RX; }
    CS0=CS1=CS2=CS3 = 1;
}

static const unsigned char fpga_image[] = {
#include "motor-control-fpga-image.hpp"
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  
};

static void 
init_fpga(int nr) {
  switch (nr) {
    case 0:
      // Controller in socket "0"
      u0brg =  (unsigned char)(((base_freq)/(1*MOTORS_PROG_SPEED))-1);
      LED1=0;
      RESET0=0;
      CS0=0;
      udelay(2000); // at least 800us
      RESET0=1;
      udelay(2000); // at least 800us
      LED1=1;
      { for(int i=0;i<sizeof(fpga_image);i++) {
          while (ti_u0c1 == 0);
          M0TX=fpga_image[i];
      } }
      complete_tx();
      udelay(100);
      CS0=1;
      udelay(100);
      LED1=0;
      
      u0brg =  (unsigned char)(((base_freq)/(1*MOTORS_SPI_SPEED))-1);
      for (int i=0; i<4; i++) {
        complete_tx(); // make sure we are not transmitting garbage already
        { volatile unsigned char dummy=M0RX; }
        CS0=0;
        M0TX=CMD_ACCELERATION | (i&3);
        complete_pretx();
        M0TX=0x80;
        complete_pretx();
        M0TX=0;
        complete_tx();
        { volatile unsigned char dummy=M0RX; }
        CS0=1;
      }
      break;
    case 1:
      // Controller in socket "1"
      u3brg =  (unsigned char)(((base_freq)/(1*MOTORS_PROG_SPEED))-1);
      LED2=0;
      RESET1=0;
      CS1=0;
      udelay(1000); // at least 800us
      RESET1=1;
      udelay(1000); // at least 800us
      LED2=1;
      { for(int i=0;i<sizeof(fpga_image);i++) {
          while (ti_u3c1 == 0);
          M1TX=fpga_image[i];
      } }
      complete_tx();
      udelay(100);
      CS1=1;
      udelay(100);
      LED2=0;
      
      u3brg =  (unsigned char)(((base_freq)/(1*MOTORS_SPI_SPEED))-1);
      for (int i=0; i<4; i++) {
        complete_tx(); // make sure we are not transmitting garbage already
        { volatile unsigned char dummy=M0RX; }
        CS1=0;
        M1TX=CMD_ACCELERATION | (i&3);
        complete_pretx();
        M1TX=0x80;
        complete_pretx();
        M1TX=0;
        complete_tx();
        { volatile unsigned char dummy=M1RX; }
        CS1=1;
      }
      break;
    case 2:
      // Controller in socket "2"
      u4brg =  (unsigned char)(((base_freq)/(1*MOTORS_PROG_SPEED))-1);
      LED3=0;
      RESET2=0;
      CS2=0;
      udelay(2000); // at least 800us
      RESET2=1;
      udelay(2000); // at least 800us
      LED3=1;
      { for(int i=0;i<sizeof(fpga_image);i++) {
          while (ti_u4c1 == 0);
          M2TX=fpga_image[i];
      } }
      complete_tx();
      udelay(100);
      CS2=1;
      udelay(100);
      LED3=0;
      
      u4brg =  (unsigned char)(((base_freq)/(1*MOTORS_SPI_SPEED))-1);
      for (int i=0; i<4; i++) {
        complete_tx(); // make sure we are not transmitting garbage already
        { volatile unsigned char dummy=M0RX; }
        CS1=0;
        M2TX=CMD_ACCELERATION | (i&3);
        complete_pretx();
        M2TX=0x80;
        complete_pretx();
        M2TX=0;
        complete_tx();
        { volatile unsigned char dummy=M2RX; }
        CS2=1;
      }
      break;
    case 3:
      // Controller in socket "3"
      u6brg =  (unsigned char)(((base_freq)/(1*MOTORS_PROG_SPEED))-1);
      LED4=0;
      RESET3=0;
      CS3=0;
      udelay(2000); // at least 800us
      RESET3=1;
      udelay(2000); // at least 800us
      LED4=1;
      { for(int i=0;i<sizeof(fpga_image);i++) {
          while (ti_u6c1 == 0);
          M3TX=fpga_image[i];
      } }
      complete_tx();
      udelay(100);
      CS3=1;
      udelay(100);
      LED4=0;
      
      u6brg =  (unsigned char)(((base_freq)/(1*MOTORS_SPI_SPEED))-1);
      for (int i=0; i<4; i++) {
        complete_tx(); // make sure we are not transmitting garbage already
        { volatile unsigned char dummy=M0RX; }
        CS3=0;
        M3TX=CMD_ACCELERATION | (i&3);
        complete_pretx();
        M3TX=0x80;
        complete_pretx();
        M3TX=0;
        complete_tx();
        { volatile unsigned char dummy=M3RX; }
        CS3=1;
      }
      break;
  }
}

  
int
main(void) {
    HardwareSetup();
    __enable_interrupt();
    // All CS* and RESET* are already pulled up in HW setups.

    // Enable SPI5 receive by reading from u5rb
    volatile unsigned short dummy=u5rb;

    unsigned int milliseconds_since_last_reset=0xffffffffU;
	int all_motors_stopped=1;

    while(1) {
        LED1 = motor_online[0];
        LED2 = motor_online[1];
        LED3 = motor_online[2];
        LED4 = motor_online[3];

        /* If any of motor controllers is not ready, reset everything */
        if ((!motor_online[0] || !motor_online[1] ||
								!motor_online[2] || !motor_online[3]) &&
								milliseconds_since_last_reset > 30*1000 &&
								all_motors_stopped) {
			milliseconds_since_last_reset=0;
            dump_log();

            /*!!! Should reset only these controllers that are not ready!

            LED1=LED2=LED3=LED4=0;
            RESET0=RESET1=RESET2=RESET3 = 0;
            CS0=CS1=CS2=CS3 = 0;
            udelay(1000); // at least 800us
            RESET0=RESET1=RESET2=RESET3 = 1;
            udelay(1000); // at least 800us

            LED1=LED2=LED3=LED4=1;
            for(int i=0;i<sizeof(fpga_image);i++) {
                complete_pretx();
                M0TX=M1TX=M2TX=M3TX = fpga_image[i];
            }
            complete_tx();

            udelay(100);
            CS0=CS1=CS2=CS3 = 1;
            udelay(100);
            LED1=LED2=LED3=LED4=0;

            // Send setspeed 0 to all controllers, without thinking
            
			for (unsigned int combination_idx=0;combination_idx < 4;
													combination_idx++) {
				{ for (unsigned int i=0;i < 4;i++)
					UART_to_motor_id[i]=(i + combination_idx) & (4-1); }
            
                measurement_idx=0;
		        send_cur_cmd(CMD_SPEED,0);	// Work around motor controller
                                            //  bug that causes problems
											//  after sending FPGA image
            } */
            
            init_fpga(0);
            init_fpga(1);
            init_fpga(2);
            init_fpga(3);
            udelay(1800);
            
            // Wait until all motors report a valid voltage

            { for (int iteration=0;iteration < 300/4;iteration++) {
				unsigned int penalty[4][4];	// Penalty for various
											//   UART_to_motor_id[] values

				for (unsigned int combination_idx=0;combination_idx < 4;
														combination_idx++) {
					{ for (unsigned int i=0;i < 4;i++)
						UART_to_motor_id[i]=(i + combination_idx) & (4-1); }

	                measurement_idx=0;
			        //send_cur_cmd(CMD_SPEED,0);	// Work around motor controller
												//  bug that causes problems
												//  after sending FPGA image

	                get_voltage();

					{ for (unsigned int motor_id=0;motor_id < 4;motor_id++) {
						const int16_t v=
								voltage[measurement_idx][motor_id].u.int16;
						const int UART_idx=
									(4 + motor_id - combination_idx) & (4-1);
						penalty[UART_idx][motor_id]=is_voltage_valid(v) ?
									(unsigned int)abs(v-IDEAL_VOLTAGE_CODE) :
									0xffffU;
					}}

	                udelay(1000);
				}

					// Set UART_to_motor_id[] based on penalty[][]

				unsigned int used_motor_id_bitmask=0;

				{ for (unsigned int UART_idx=0;UART_idx < 4;UART_idx++) {
					UART_to_motor_id[UART_idx]=0xff;

					unsigned int best_penalty=0xffffU;
					for (unsigned int motor_id=0;motor_id < 4;motor_id++) {

						if ((used_motor_id_bitmask & (1U << motor_id)) != 0)
							continue;

						if (best_penalty > penalty[UART_idx][motor_id]) {
							best_penalty = penalty[UART_idx][motor_id];
							UART_to_motor_id[UART_idx]=motor_id;
						}
					}

					if (UART_to_motor_id[UART_idx] < 4)
						used_motor_id_bitmask|=
										1U << UART_to_motor_id[UART_idx];
				}}

				if (used_motor_id_bitmask == (1U << 4)-1)
					break;

					// Set fallback UART_to_motor_id[] for UARTs that appear
					//   to have no working motor controller attached

				{ for (unsigned int UART_idx=0;UART_idx < 4;UART_idx++) {
					if (UART_to_motor_id[UART_idx] < 4)
						continue;	// UART_to_motor_id[] is already set

					for (unsigned int motor_id=0;motor_id < 4;motor_id++) {
						const unsigned int mask=(1U << motor_id);
						if ((used_motor_id_bitmask & mask) == 0) {
							UART_to_motor_id[UART_idx]=motor_id;
							used_motor_id_bitmask|=mask;
							break;
						}
					}
				}}
            }}
            calibrate();
            udelay(30*1000);
            continue;
        }

        udelay(3000);
        milliseconds_since_last_reset+=2*3;	// compensate for 2x slowdown due to ISRs

        get_voltage();
        all_motors_stopped=send_cur_cmd(-1,-1);

        measurement_idx++;
        if (measurement_idx >= 3)
            measurement_idx=0;

		{ for (int i=0;i < 4;i++)
	        motor_online[i]=is_voltage_valid(calc_median3(voltage,i)); }

		if (!CDONE0)
			motor_online[UART_to_motor_id[0]]=0;
		if (!CDONE1)
			motor_online[UART_to_motor_id[1]]=0;
		if (!CDONE2)
			motor_online[UART_to_motor_id[2]]=0;
		if (!CDONE3)
			motor_online[UART_to_motor_id[3]]=0;
    }
}
