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
#include "adc12repeat.h"

#define SHELLBUFLEN 1024

static uint8_t shellbuf[SHELLBUFLEN]="";
static uint8_t *path="/";

volatile bool mems_realtime=false;

void
shell(uint32_t _NumBytes, const uint8_t* _Buffer) {
    strncat((char *)shellbuf,(char *)_Buffer,_NumBytes);
    char * enter=strchr((char *)shellbuf,13); // search for "Enter"
    char * ctrlc=strchr((char *)shellbuf,3); // search for "Ctrl-C"
    char * bs   =strchr((char *)shellbuf,3); // search for "backspace"
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
        } else if(strcmp((char*)shellbuf,"cat /switch")==0) {
            char *buf= (PORT3.PIDR.BIT.B4==1) ? "1\n" : "0\n";
            USBCDC_Write_Async(strlen((char*)buf),(uint8_t*)buf , CBDoneWrite);
        } else if(strcmp((char*)shellbuf,"cat /power/status")==0) {
//            char *buf= (PORT3.PIDR.BIT.B4==1) ? "1\n" : "0\n";
           char buf[256];
           snprintf(buf,sizeof(buf),"\r\n"
                    "BAT0:%.2fV %.2fA\r\n"
                    "BAT1:%.2fV %.2fA\r\n"
                    "BAT2:%.2fV %.2fA\r\n"
                    "BAT3:%.2fV %.2fA\r\n"
                    ,adc[0],adc[4]
                    ,adc[1],adc[5]
                    ,adc[2],adc[6]
                    ,adc[3],adc[7]);
           USBCDC_Write_Async(strlen((char*)buf),(uint8_t*)buf , CBDoneWrite);
        } else if(strncmp((char*)shellbuf,"date ",5)==0) {
            int day,month,year,weekday;
            int hour,minute,second;
            sscanf((char*)shellbuf+5,"%2x/%2x/%2x %1x %2x:%2x:%2x",&day,&month,&year,&weekday,&hour,&minute,&second);

            RTC.RSECCNT.BYTE = second;
            RTC.RMINCNT.BYTE = minute;
            RTC.RHRCNT.BYTE = hour;
  
            RTC.RDAYCNT.BYTE = day;
            RTC.RMONCNT.BYTE = month;
            RTC.RYRCNT.WORD = year;
            RTC.RWKCNT.BYTE = weekday; // Saturday - 6, Sunday 7
            
            char buf[80];
            sprintf(buf,"\r\nSet date to: %02x/%02x/20%02x %x %02x:%02x:%02x\r\n",RTC.RDAYCNT.BYTE,RTC.RMONCNT.BYTE,RTC.RYRCNT.WORD,RTC.RWKCNT.BYTE,RTC.RHRCNT.BYTE,RTC.RMINCNT.BYTE,RTC.RSECCNT.BYTE);
            USBCDC_Write_Async(strlen((char*)buf),(uint8_t*)buf , CBDoneWrite);
            
        } else if(strncmp((char*)shellbuf,"setsteering off",15)==0) {
                SYSTEM.PRCR.WORD = 0xA503;
                TPUA.TSTR.BIT.CST4=0; // stop counter
                TPUA.TSTR.BIT.CST5=0; // stop counter
                /* Protection on */
                SYSTEM.PRCR.WORD = 0xA500;
        } else if(strncmp((char*)shellbuf,"setpower ",9)==0) {
            int power1,power2,power3;
            sscanf((char*)shellbuf+9,"%d %d %d",&power1,&power2,&power3);
            OUT1_EN = power1 ? MAX1614_ON : MAX1614_OFF;
            OUT2_EN = power2 ? MAX1614_ON : MAX1614_OFF;
            OUT3_EN = power3 ? MAX1614_ON : MAX1614_OFF;
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
             * UPDATE: We want wider! Here we use +-500 from center
             */
            if(abs(leftsteering) <= 100 && abs(rightsteering) <= 100 ) {
                TPU5.TGRA = (15000-1125)+ leftsteering*(1000/2)/100; // left steering servo
                TPU4.TGRA = (15000-1125)+rightsteering*(1000/2)/100; // right steering servo
                
                SYSTEM.PRCR.WORD = 0xA503;
                TPUA.TSTR.BIT.CST4=1; // start counter
                TPUA.TSTR.BIT.CST5=1; // start counter
                /* Protection on */
                SYSTEM.PRCR.WORD = 0xA500;

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