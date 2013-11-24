/* Copyright (C) 2012 Renesas Electronics Corporation. All rights reserved.    */
/* Description   : Defines macros relating to the RX630 user LEDs and switches
*******************************************************************************/
/* Multiple inclusion prevention macro */
#ifndef _RSKRX630_H
#define _RSKRX630_H

/* Defines RX630 port registers */
#include "iorx630.h"

/* General Values */
#define LED_ON      (0)
#define LED_OFF      (1)
#define SET_BIT_HIGH  (1)
#define SET_BIT_LOW    (0)
#define SET_BYTE_HIGH  (0xFF)
#define SET_BYTE_LOW  (0x00)

/* Switch port pins state settings */
#define  SW1          PORT3.PIDR.BIT.2
#define  SW2          PORT4.PIDR.BIT.4
#define  SW3          PORT0.PIDR.BIT.7

/* LED ouptut pin settings */
#define  LED0      PORT0.PODR.BIT.B5
#define  LED1      PORT0.PODR.BIT.B7
//#define  LED2      PORTC.PODR.BIT.B2
//#define  LED3      PORT1.PODR.BIT.B7

/* LED data direction */
#define LED0_PORT_DIR  PORT0.PDR.BIT.B5
#define LED1_PORT_DIR  PORT0.PDR.BIT.B7
//#define LED2_PORT_DIR  PORTC.PDR.BIT.B2
//#define LED3_PORT_DIR  PORT1.PDR.BIT.B7

/* Delay units */
#define uS        'u'
#define mS        'm'

/* End of multiple inclusion prevention macro */
#endif

