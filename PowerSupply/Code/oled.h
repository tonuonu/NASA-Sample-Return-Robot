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

#include "iorx630.h"

/* OLED ouptut pin settings */
#define  OLED_CS      PORT5.PODR.BIT.B5
#define  OLED_DC      PORT5.PODR.BIT.B4
#define  OLED_RD      PORT5.PODR.BIT.B2
#define  OLED_RESET   PORT5.PODR.BIT.B1
#define  OLED_WR      PORT5.PODR.BIT.B0
#define  OLED_DATA_PORT   PORTD.PODR.BYTE

/* OLED data direction */
#define OLED_CS_PORT_DIR     PORT5.PDR.BIT.B5
#define OLED_DC_PORT_DIR     PORT5.PDR.BIT.B4
#define OLED_RD_PORT_DIR     PORT5.PDR.BIT.B2
#define OLED_RESET_PORT_DIR  PORT5.PDR.BIT.B1
#define OLED_WR_PORT_DIR     PORT5.PDR.BIT.B0
#define OLED_DATA_PORT_DIR   PORTD.PDR.BYTE
 

void Init_OLED(void);
#ifdef __cplusplus
extern "C" {
#endif

void OLED_Show_String(unsigned char a,  char *Data_Pointer, unsigned char b, unsigned char c);
void OLED_Fill_RAM(unsigned char Data);


/* modes */
#define E_MAINMENU (0)
#define E_BAT      (1)
#define E_ERR      (2)
#define E_GYRO     (3)
/* Update this to largest from previous defines */
#define MAX_MODE   (3)

extern volatile char mode;
extern volatile char mode_just_changed;
void readtime(char *buf);
void readstime(char *buf);
#define MAXERRORS 6
#define SCREENWIDTH    42
extern char errlog[MAXERRORS][SCREENWIDTH+1];


#ifdef __cplusplus
}
#endif