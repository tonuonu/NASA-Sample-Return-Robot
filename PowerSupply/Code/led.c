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

#include "iorx630.h"
#include "rskrx630def.h"
#include "led.h"

static int _LED_RED=0;
static int _LED_GRN=0;
static int _LED_BLU=0;

void 
LED_RGB_set(int r, int g, int b) {
    _LED_RED = r ? LED_ON : LED_OFF;
    _LED_GRN = g ? LED_ON : LED_OFF;
    _LED_BLU = b ? LED_ON : LED_OFF;
}

void 
LED_RGB_blink(void) {
    if(_LED_RED)
        LED_RED ^=1;
    else
        LED_RED = LED_OFF;
    
    if(_LED_GRN)
        LED_GRN ^=1;
    else
        LED_GRN = LED_OFF;
    
    if(_LED_BLU)
        LED_BLU ^=1;
    else
        LED_BLU = LED_OFF;
}