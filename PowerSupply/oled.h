/*
 *  Copyright (c) 2011-2013 Tonu Samuel
 *  All rights reserved.
 *
 *  This file is part of TYROS.
 *
 *  TYROS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TYROS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TYROS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "iorx630.h"


/* OLED ouptut pin settings */
#define  OLED_CS      PORTC.PODR.BIT.B6
#define  OLED_DC      PORTC.PODR.BIT.B7
#define  OLED_WR      PORT5.PODR.BIT.B0
#define  OLED_RESET   PORT5.PODR.BIT.B1
#define  OLED_DATA_PORT   PORTD.PODR.BYTE


/* OLED data direction */
#define OLED_CS_PORT_DIR     PORTC.PDR.BIT.B6
#define OLED_DC_PORT_DIR     PORTC.PDR.BIT.B7
#define OLED_WR_PORT_DIR     PORT5.PDR.BIT.B0
#define OLED_RESET_PORT_DIR  PORT5.PDR.BIT.B1
#define OLED_DATA_PORT_DIR   PORTD.PDR.BYTE

void Init_OLED(void);

