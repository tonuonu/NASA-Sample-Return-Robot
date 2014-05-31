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

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "intrinsics.h"

#include "switch.h"
#include "shell.h"
#include "usb_hal.h"
#include "usb_cdc.h"
#include "usb.h"
#include "log.h"
#include "led.h"
#include "rskrx630def.h"



/*Delay between repeating initial message*/
#define DELAY_VALUE 0x00500000UL

/*Flags*/
//static volatile bool g_bEcho = false;

/*Data Buffers*/
uint8_t g_Buffer1[BUFFER_SIZE];
uint8_t g_Buffer2[BUFFER_SIZE];
uint8_t* g_pBuffEmpty = g_Buffer1;
uint8_t* g_pBuffFull = g_Buffer2;

/* Initialises this modules data. */
//static void InitialiseData(void);
/* Callback called when we have read some data. */
void CBDoneRead(USB_ERR _err, uint32_t _NumBytes);
/* Callback called when we have written some data. */
//static void CBDoneWrite(USB_ERR _err);

#if 0
void USB_CDC_APP_Main(void)
{  
  /* LCD */
//  Init_LCD();
  
  /* Display splash screen on the debug LCD */
//  Display_LCD(LCD_LINE1,"Renesas ");
//  Display_LCD(LCD_LINE2,"USB CDC ");
    
  DEBUG_MSG_LOW( ("\r\n*** CDC App Starting ***\r\n"));
  
  /*Initialise the USB CDC Class*/
  USBCDC_Init();
  
  __enable_interrupt();
  
  while(1)
  {
    volatile uint32_t DelayPreWrite = DELAY_VALUE_INITIAL;
    
    /*Wait for USB cable to be connected */
    while(false == USBCDC_IsConnected());
    
    /*Reset the application data for a new connection */
    InitialiseData();
    
    /*Not necessary but to avoid confusion when reading the log -
    Wait for enmeration to happen before starting to send data*/
    while(0 != DelayPreWrite){DelayPreWrite--;}
  
    /*Keep putting out a message that will be seen when a terminal connects
    to the virtual com port this has just produced.
    The message says press SW1 - so continue until SW1 is pressed.*/
    while(0 == (SWITCHPRESS_1 & gSwitchFlag))
    {
      volatile uint32_t Delay;
      USBCDC_WriteString("\r\nRenesas USB CDC Sample, Press Switch SW1.\r\n");
      Delay = DELAY_VALUE;
      while(0 != Delay--)
      {
        if(SWITCHPRESS_1 == gSwitchFlag)
        {
          Delay = 0;
        }
        
        /*If a user presses another switch (not SW1) at this time, then
        immediatly repeat the instruction to press SW1*/
        if(SWITCHPRESS_2 & gSwitchFlag)
        {
          Delay = 0;
          gSwitchFlag = 0;
        }
        if(SWITCHPRESS_3 & gSwitchFlag)
        {
          Delay = 0;
          gSwitchFlag = 0;
        }
      }
    }
  
    /*Wait for switch interrupts to set flags.*/
    while(true == USBCDC_IsConnected())
    {
      /*Has SW1 been pressed*/
      if(SWITCHPRESS_1 & gSwitchFlag)
      {
        /*Write main instrutions out*/
        USBCDC_WriteString(szWelcomeMsg1);
        USBCDC_WriteString(szWelcomeMsg2);
        USBCDC_WriteString(szWelcomeMsg3);
        USBCDC_WriteString(szWelcomeMsg4);
        USBCDC_WriteString(szWelcomeMsg5);
      
        /*Reset the switch pressed flag*/
        gSwitchFlag = 0;
      }
    
      /*Has SW2 been pressed*/
      if(SWITCHPRESS_2 & gSwitchFlag)
      {
        /*Reset the switch pressed flag*/
        gSwitchFlag = 0;
      
        /*Make sure we are not already in echo mode*/
        if(false == g_bEcho)
        {
          /*Start Echo mode*/
          g_bEcho = true;
      
          USBCDC_WriteString("\r\nStarting Echo:-\r\n");
      
          /*Start a Read*/
          USBCDC_Read_Async(BUFFER_SIZE, g_pBuffEmpty, CBDoneRead);
      
          /*This continues in the CBDoneRead function...*/
        }
      }
  
      /*Has SW3 been pressed*/
      if(SWITCHPRESS_3 & gSwitchFlag)
      {
        /*Reset the switch pressed flag*/
        gSwitchFlag = 0;
      
        /*Make sure we are in echo mode*/
        if(true == g_bEcho)
        {
          /*Stop echo mode*/
          g_bEcho = false;
        
          /*Stop the CDC layer waiting for a read*/
          USBCDC_Cancel();
          USBCDC_WriteString("\r\nFinished Echo.\r\n");
        }
      }  
    }
  }
}

#endif
/***********************************************************************************
* Outline     : CBDoneRead 
* Description   : Callback called when a USBCDC_Read_Async request
*          has completed. i.e. Have read some data.
*          If in echo mode then write data just read back out.
* Argument    : _err : Error code.
*          _NumBytes : Number of bytes read.
* Return value  : none
***********************************************************************************/
void 
CBDoneRead(USB_ERR _err, uint32_t _NumBytes) {  
    LED2=LED_ON;  
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
  //logerror((char *) g_pBuffFull);
      
  /* Echo what was read back */
  if(USB_ERR_OK == _err) {
      LED3=LED_ON;
      shell(_NumBytes, g_pBuffFull);
  }
  /*Setup another read*/
  USBCDC_Read_Async(BUFFER_SIZE, g_pBuffEmpty, CBDoneRead);
}


/***********************************************************************************
* Outline     : CBDoneWrite 
* Description   : Callback called when a USBCDC_Write_Async request
*          has completed. i.e. Have written some data.
* Argument    : none
* Return value  : none
***********************************************************************************/
void CBDoneWrite(USB_ERR _err)
{
  assert(USB_ERR_OK == _err);
  /*Write has completed*/
  /*Nothing to do*/
}


#if 0
void InitialiseData(void)
{
  g_bEcho = false;
  gSwitchFlag = 0;
}
#endif
/***********************************************************************************
End of function InitialiseData
***********************************************************************************/
