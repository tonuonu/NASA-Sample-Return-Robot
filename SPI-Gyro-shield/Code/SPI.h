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


/* Four motor ports */
void SPI0_Init(void); 
void SPI2_Init(void); 
void SPI3_Init(void); 
void SPI4_Init(void);

/* Interface to Arduino */
void SPI5_Init(void);


#if 0

void
gyro_send(unsigned char c);

unsigned short 
SPI2_receive(void);

void
SPI3_send_data(unsigned char c);

void
SPI3_send_cmd(unsigned char c);

void
SPI4_send(unsigned short c);

short unsigned
SPI4_receive(void);

void
SPI6_send(unsigned short c);

short unsigned
SPI6_receive(void);

void
SPI7_send(unsigned short c);
#endif