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
#include "intrinsics.h"
#include "oled.h"
#include "switch.h"
#include "shell.h"
#include "log.h"
#include "rtc.h"
#include "stdio.h"
#include "string.h"
#include "usb_hal.h"
#include "usb_cdc.h"
#include "usb.h"

#define SHELLBUFLEN 1024

static uint8_t shellbuf[SHELLBUFLEN]="";
static uint8_t *path="/";

volatile bool mems_realtime=false;

void
shell(uint32_t _NumBytes, const uint8_t* _Buffer) {
    strncat((char *)shellbuf,(char *)_Buffer,_NumBytes);
    char * enter=strchr((char *)shellbuf,13); // search for "Enter"
    char * error="";
    if(enter) {
        char pnferr[80];
        *enter=0;
        logerror((char *)shellbuf);
        if(strcmp((char *)shellbuf,"")==0) {
            /* just emty enter */
            //USBCDC_Write_Async(strlen((char *)path),path , CBDoneWrite);
        } else if(strncmp((char *)shellbuf,"cd ",3)==0) {
            if(strcmp((char *)shellbuf+3,"/mems")==0) {
                path="/mems";
            } else if(strcmp((char const *)shellbuf+3,"/power")==0) {
                path="/power";
            } else if(strcmp((char const *)shellbuf+3,"/steer")==0) {
                path="/steer";
            } else if(strcmp((char const *)shellbuf+3,"/log")==0) {
                path="/log";
            } else if(strcmp((char const *)shellbuf+3,"/rtc")==0) {
                path="/rtc";
            } else if(strcmp((char const *)shellbuf+3,"/")==0) {
                path="/";
            } else {
               sprintf(pnferr,"\r\nPath '%s' not found\r\n",shellbuf+3);
               error=pnferr;
            }
        } else if(strcmp((char*)shellbuf,"cat /mems/realtime")==0) {
            mems_realtime=true;
        } else {
            error="Syntax error\r\n";
        }
        if(!mems_realtime) {
          shellbuf[0]=0;
          char buf[80];
          sprintf(buf,"\r\n%srobot:%s# ",error,path);
          USBCDC_Write_Async(strlen((char*)buf),(uint8_t*)buf , CBDoneWrite);
        }
    } else {
      USBCDC_Write_Async(_NumBytes,_Buffer , CBDoneWrite);
    }
}