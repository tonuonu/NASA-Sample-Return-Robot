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
    char * ctrlc=strchr((char *)shellbuf,3); // search for "Ctrl-C"
    char * bs=strchr((char *)shellbuf,3); // search for "backspace"
    char * error="";
    if(ctrlc) {
        mems_realtime=false;
        *ctrlc=0;
        shellbuf[0]=0;
    } else if(bs) {
        *bs=0x7f;
        //shellbuf[0]=0;
    } else if(enter) {
        char pnferr[80];
        *enter=0;
        logerror((char *)shellbuf);
        if(strcmp((char *)shellbuf,"")==0) {
            /* just empty enter 
             * Avoid syntax error with this empty check.
             */
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
        } else if(strncmp((char*)shellbuf,"setsteering ",12)==0) {
            int leftsteering,rightsteering;
            sscanf((char*)shellbuf+12,"%d %d",&leftsteering,&rightsteering);
                /* 
                 * http://en.wikipedia.org/wiki/Servo_control 
                 * recommends using 20ms (50Hz) cycle.
                 * 48Mhz/64/15000 == 50Hz
                 * Same page says 1.5ms high pulse keeps servo at middle 
                 * 15000/20*1.5=1125
                 * Side limits are 1 and 2ms (750 and 1500).
                 */
            if(abs(leftsteering) <= 100 && abs(rightsteering) <= 100 ) {
                TPU4.TGRA = (15000-1125)+ leftsteering*(750/2)/100; // left steering servo
                TPU5.TGRA = (15000-1125)+rightsteering*(750/2)/100; // right steering servo
            } else {
                error="Error in arguments. Usage: steering L R, where L and R are integers between -100 and 100\r\n";
            }
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