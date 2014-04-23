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

#define IDEAL_VOLTAGE_CODE		((int16_t)(48 + 12*67.65))	// corresponds to 12.0V

struct twobyte_st tmprecv[4] = {0,0,0,0,0,0,0,0};

volatile unsigned char motor_load[4] = {0,0,0,0};
volatile struct twobyte_st ticks[4] = {0,0,0,0,0,0,0,0};
volatile struct twobyte_st cur_cmd_param[4] = {0,0,0,0,0,0,0,0};
volatile unsigned char cur_cmd[4] = {CMD_SPEED,CMD_SPEED,CMD_SPEED,CMD_SPEED};

static int measurement_idx=0;
struct twobyte_st voltage[3][4]=
				{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
struct twobyte_st cur_target_speed[3][4]=
				{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};

static unsigned int UART_to_motor_id[4]={0,1,2,3};
static unsigned int motor_online[4]={0,0,0,0};

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

    complete_tx();
    CS0=CS1=CS2=CS3 = 1;

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
        /* Check for possible overflow of INT16 and lit red LED */
        if(x > INT16_MAX || x < INT16_MIN)
            LED5=1;
        else {
            __disable_interrupt();
            ticks[i].u.int16+=ticks_increment;
            __enable_interrupt();
        }
    }   
}

static void 
send_cur_cmd(const int force_cmd=-1,const int force_param=-1) {
    if(RESET5 == 0) {
        return;
    }
    complete_tx(); // make sure we are not transmitting garbage already

    { volatile unsigned char dummy=M0RX; }
    { volatile unsigned char dummy=M1RX; }
    { volatile unsigned char dummy=M2RX; }
    { volatile unsigned char dummy=M3RX; }

    CS0=CS1=CS2=CS3 = 0;
    M0TX=(force_cmd >= 0 ? force_cmd : cur_cmd[UART_to_motor_id[0]]) |
													UART_to_motor_id[0];
    M1TX=(force_cmd >= 0 ? force_cmd : cur_cmd[UART_to_motor_id[1]]) |
													UART_to_motor_id[1];
    M2TX=(force_cmd >= 0 ? force_cmd : cur_cmd[UART_to_motor_id[2]]) |
													UART_to_motor_id[2];
    M3TX=(force_cmd >= 0 ? force_cmd : cur_cmd[UART_to_motor_id[3]]) |
													UART_to_motor_id[3];

    /* 
     * Use temporary variable to ensure interrupts to not overwrite
     * value while we send it. 
     */
    struct twobyte_st tmp[4];
    tmp[0].u.int16=(force_cmd >= 0 ? force_param : cur_cmd_param[0].u.int16);
    tmp[1].u.int16=(force_cmd >= 0 ? force_param : cur_cmd_param[1].u.int16);
    tmp[2].u.int16=(force_cmd >= 0 ? force_param : cur_cmd_param[2].u.int16);
    tmp[3].u.int16=(force_cmd >= 0 ? force_param : cur_cmd_param[3].u.int16);

    complete_rx();

    { volatile unsigned char dummy=M0RX; }
    { volatile unsigned char dummy=M1RX; }
    { volatile unsigned char dummy=M2RX; }
    { volatile unsigned char dummy=M3RX; }

    complete_pretx();

    M0TX=tmp[UART_to_motor_id[0]].u.byte[1];
    M1TX=tmp[UART_to_motor_id[1]].u.byte[1];
    M2TX=tmp[UART_to_motor_id[2]].u.byte[1];
    M3TX=tmp[UART_to_motor_id[3]].u.byte[1]; 

    complete_rx();

    tmprecv[UART_to_motor_id[0]].u.byte[1]=M0RX & 0xff;
    tmprecv[UART_to_motor_id[1]].u.byte[1]=M1RX & 0xff;
    tmprecv[UART_to_motor_id[2]].u.byte[1]=M2RX & 0xff;
    tmprecv[UART_to_motor_id[3]].u.byte[1]=M3RX & 0xff;

    complete_pretx();

    M0TX=tmp[UART_to_motor_id[0]].u.byte[0];
    M1TX=tmp[UART_to_motor_id[1]].u.byte[0];
    M2TX=tmp[UART_to_motor_id[2]].u.byte[0];
    M3TX=tmp[UART_to_motor_id[3]].u.byte[0];

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
        tmprecv[0].u.int16=0;
    if (!CDONE1)
        tmprecv[1].u.int16=0;
    if (!CDONE2)
        tmprecv[2].u.int16=0;
    if (!CDONE3)
        tmprecv[3].u.int16=0;

    for(int i=0;i<=3;i++)
        voltage[measurement_idx][i].u.int16 = tmprecv[i].u.int16;
}

static const unsigned char fpga_image[] = {
#include "motor-control-fpga-image.hpp"
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  
};

int
main(void) {
    HardwareSetup();
    __enable_interrupt();
    // All CS* and RESET* are already pulled up in HW setups.

    // Enable SPI5 receive by reading from u5rb
    volatile unsigned short dummy=u5rb;

    unsigned int milliseconds_since_last_reset=0xffffffffU;

    while(1) {
        LED1 = motor_online[0];
        LED2 = motor_online[1];
        LED3 = motor_online[2];
        LED4 = motor_online[3];

        /* If any of motor controllers is not ready, reset everything */
        if((!motor_online[0] || !motor_online[1] ||
								!motor_online[2] || !motor_online[3]) &&
								milliseconds_since_last_reset > 5000) {
			milliseconds_since_last_reset=0;

            /*!!! Should reset only these controllers that are not ready! */

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

            udelay(1000);
            CS0=CS1=CS2=CS3 = 1;
            udelay(1000);
            LED1=LED2=LED3=LED4=0;

                // Wait until all motors report a valid voltage

            { for (int iteration=0;iteration < 300/4;iteration++) {
				unsigned int penalty[4][4];	// Penalty for various
											//   UART_to_motor_id[] values

				for (unsigned int combination_idx=0;combination_idx < 4;
														combination_idx++) {
					{ for (unsigned int i=0;i < 4;i++)
						UART_to_motor_id[i]=(i + combination_idx) & (4-1); }

	                measurement_idx=0;
			        send_cur_cmd(CMD_SPEED,0);	// Work around motor controller
												//  bug that causes problems
												//  after sending FPGA image

	                get_voltage();

					{ for (unsigned int motor_id=0;motor_id < 4;motor_id++) {
						const int16_t voltage=
										voltage[measurement_idx][i].u.int16;
						const int UART_idx=(4 + i - combination_idx) & (4-1);
						penalty[UART_idx][motor_id]=
								is_voltage_valid(voltage) ?
										(unsigned int)abs(
											voltage-IDEAL_VOLTAGE_CODE) :
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
						if ((used_motor_id_bitmask & mask) == 0)
							UART_to_motor_id[UART_idx]=motor_id;
							used_motor_id_bitmask|=mask;
							break;
					}
				}}
            }}
            udelay(30*1000);
            continue;
        }

        udelay(3000);
        milliseconds_since_last_reset+=3;

        get_voltage();
        send_cur_cmd();

        measurement_idx++;
        if (measurement_idx >= 3)
            measurement_idx=0;

        motor_online[0]=CDONE0 && is_voltage_valid(calc_median3(voltage,0));
        motor_online[1]=CDONE1 && is_voltage_valid(calc_median3(voltage,1));
        motor_online[2]=CDONE2 && is_voltage_valid(calc_median3(voltage,2));
        motor_online[3]=CDONE3 && is_voltage_valid(calc_median3(voltage,3));
    }
}
