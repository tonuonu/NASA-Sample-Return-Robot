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
/*******************************************************************************
* File Name : usb_cdc_app.c
* Version : 1.00
* Device     : R5F5630EDDFP
* Tool Chain : RX Family C Compiler
* H/W Platform   : RSKRX630
* Description : Application that uses the USB CDC class.
       
        This means the device will appear as a virtual COM port to the host.
        When the host connects to this virtual COM port using a terminal
        program such as MS HyperTerminal instructions will appear.
        A menu is provided that can be controlled by pressing the RSK switches.
    
        Demonstrates CDC usage by providing an echo test mode,
        where any characters read will be echoed back.
    
        For more details see 'USB Sample Code User's Manual'.
    
        See file async.c for baud rate for serial debug output.
***********************************************************************************/
/***********************************************************************************
* History     : 23.01.2012 Ver. 1.00 First Release
***********************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************
System Includes
***********************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "intrinsics.h"

/***********************************************************************************
User Includes
***********************************************************************************/
#include "oled.h"
#include "switch.h"
#include "usb_hal.h"
#include "usb_cdc.h"
#include "usb_cdc_app.h"
#include "adc12repeat.h"

/***********************************************************************************
Defines
***********************************************************************************/
/*Delay between cable connecting and starting and starting repeating message.
This allows enumeration to complete before we try and communicate.*/
#define DELAY_VALUE_INITIAL 0x01000000UL

/*Delay between repeating initial message*/
#define DELAY_VALUE 0x00500000UL

/*Size of buffer - Lets use the packet size*/
#define BUFFER_SIZE BULK_OUT_PACKET_SIZE 

/***********************************************************************************
Variables
***********************************************************************************/

/*Flags*/
static volatile bool g_bEcho = false;

/*Data Buffers*/
static uint8_t g_Buffer1[BUFFER_SIZE];
static uint8_t g_Buffer2[BUFFER_SIZE];
static uint8_t* g_pBuffEmpty = g_Buffer1;
static uint8_t* g_pBuffFull = g_Buffer2;

/***********************************************************************************
Private Function Prototypes
***********************************************************************************/
/* Initialises this modules data. */
static void InitialiseData(void);
/* Callback called when we have read some data. */
static void CBDoneRead(USB_ERR _err, uint32_t _NumBytes);
/* Callback called when we have written some data. */
static void CBDoneWrite(USB_ERR _err);

/***********************************************************************************
End of function USB_CDC_APP_Main
***********************************************************************************/

/***********************************************************************************
* Outline     : CBDoneRead 
* Description   : Callback called when a USBCDC_Read_Async request
*          has completed. i.e. Have read some data.
*          If in echo mode then write data just read back out.
* Argument    : _err : Error code.
*          _NumBytes : Number of bytes read.
* Return value  : none
***********************************************************************************/

static void CBDoneRead(USB_ERR _err, uint32_t _NumBytes)
{    
  /*Toggle buffers - as now the empty buffer has been filled
  by this read completing*/
  if(g_pBuffEmpty == g_Buffer2)
  {
    g_pBuffEmpty = g_Buffer1;
    g_pBuffFull = g_Buffer2;
  }
  else
  {
    g_pBuffEmpty = g_Buffer2;
    g_pBuffFull = g_Buffer1;
  }
  
  if(true == g_bEcho)
  {
    /*Setup another read*/
    USBCDC_Read_Async(BUFFER_SIZE, g_pBuffEmpty, CBDoneRead);
      
    /*Echo what was read back*/
    if(USB_ERR_OK == _err)
    {
      USBCDC_Write_Async(_NumBytes, g_pBuffFull, CBDoneWrite);
    }
  }
}
/***********************************************************************************
End of function CBDoneRead
***********************************************************************************/

/***********************************************************************************
* Outline     : CBDoneWrite 
* Description   : Callback called when a USBCDC_Write_Async request
*          has completed. i.e. Have written some data.
* Argument    : none
* Return value  : none
***********************************************************************************/

static void CBDoneWrite(USB_ERR _err)
{
  assert(USB_ERR_OK == _err);
  /*Write has completed*/
  /*Nothing to do*/
}
/***********************************************************************************
End of function CBDoneWrite
***********************************************************************************/

/***********************************************************************************
* Outline     : InitialiseData
* Description   : Initialise this modules data.
*          Put into a function so it can be called each
*          time a USB cable is connected.
* Argument    : none
* Return value  : none
***********************************************************************************/
static void InitialiseData(void) {
  g_bEcho = false;
  gSwitchFlag = 0;
}
/***********************************************************************************
End of function InitialiseData
***********************************************************************************/
#ifdef __cplusplus
}
#endif
