/*
 *  Copyright (c) 2014 Tonu Samuel
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


#define RGB_YELLOW 1,1,0
#define RGB_RED    1,0,0
#define RGB_GREEN  0,1,0
#define RGB_BLUE   0,0,1
#define RGB_PINK   1,0,1
#define RGB_WHITE  1,1,1
#define RGB_NONE   0,0,0

#define LED_ON      (1)
#define LED_OFF      (0)

#define  LED_GRN      PORT1.PODR.BIT.B3
#define  LED_BLU      PORT1.PODR.BIT.B5
#define  LED_RED      PORT1.PODR.BIT.B7

#ifdef __cplusplus
extern "C" {
#endif

void LED_RGB_set(int r, int g, int b);
void LED_RGB_blink(void);

#ifdef __cplusplus
}
#endif
