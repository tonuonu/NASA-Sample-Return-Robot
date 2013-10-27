/* Copyright (C) 2012 Renesas Electronics Corporation. All rights reserved.   */
/*******************************************************************************
* File Name : usb_cdc_app.h
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
********************************************************************************/
/********************************************************************************
* History     : 23.01.2012 Ver. 1.00 First Release
********************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
  
#ifndef USB_CDC_APP_H
#define USB_CDC_APP_H

/********************************************************************************
User Includes
********************************************************************************/

#include "usb_common.h"

/********************************************************************************
Function Prototypes
********************************************************************************/
void USB_CDC_APP_Main(void);

/* End of multiple inclusion prevention macro */
#endif


#ifdef __cplusplus
}
#endif
