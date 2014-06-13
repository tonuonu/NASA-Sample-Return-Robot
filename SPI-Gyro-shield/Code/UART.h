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



/* Debug ports */
void UART1_Init(void); 
void UART1_Char(char c);
void UART1_Dump(struct twobyte_st w1[4], struct twobyte_st w2[4], struct twobyte_st w3[4], struct twobyte_st w4[4], struct twobyte_st w5[4]);
void UART1_DumpLog(int16_t w1, unsigned char w2[4], struct twobyte_st w3[4], char w4);