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
#include "rskrx630def.h"
#include "oled.h"
#include "switch.h"
#include "oled.h"
#include "log.h"
#include "rtc.h"
#include "stdio.h"
#include "string.h"


char errlog[MAXERRORS][SCREENWIDTH+1]={"","","","","",""};
volatile char errptr=0;


void logerror(char *buf) {
    readstime(errlog[errptr]);
    errlog[errptr][14]=' ';
    strcpy(errlog[errptr]+15,buf);
    errptr++;
    if(errptr==MAXERRORS){
        errptr=0;
    }
}
