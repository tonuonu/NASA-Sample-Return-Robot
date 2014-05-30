/* Copyright (C) 2012 Renesas Electronics Corporation. All rights reserved.    */
/* Description   : Defines macros relating to the RX630 user LEDs and switches
*******************************************************************************/
/* Multiple inclusion prevention macro */
#ifndef _RSKRX630_H
#define _RSKRX630_H

/* Defines RX630 port registers */
#include "iorx630.h"

/* General Values */
#define SET_BIT_HIGH  (1)
#define SET_BIT_LOW    (0)
#define SET_BYTE_HIGH  (0xFF)
#define SET_BYTE_LOW  (0x00)

/* Switch port pins state settings */
#define  SW1          PORT3.PIDR.BIT.3
#define  SW2          PORT3.PIDR.BIT.2
#define  SW3          PORT3.PIDR.BIT.1
#define  SW4          PORT3.PIDR.BIT.0

/* LED ouptut pin settings */
#define  LED0      PORT2.PODR.BIT.B0
#define  LED1      PORT2.PODR.BIT.B1
#define  LED2      PORT2.PODR.BIT.B2
#define  LED3      PORT2.PODR.BIT.B3
#define  LED4      PORT2.PODR.BIT.B4
#define  LED5      PORT2.PODR.BIT.B5
#define  LED6      PORT2.PODR.BIT.B6
#define  LED7      PORT2.PODR.BIT.B7

/* LED data direction */
#define LED0_PORT_DIR  PORT2.PDR.BIT.B0
#define LED1_PORT_DIR  PORT2.PDR.BIT.B1
#define LED2_PORT_DIR  PORT2.PDR.BIT.B2
#define LED3_PORT_DIR  PORT2.PDR.BIT.B3
#define LED4_PORT_DIR  PORT2.PDR.BIT.B4
#define LED5_PORT_DIR  PORT2.PDR.BIT.B5
#define LED6_PORT_DIR  PORT2.PDR.BIT.B6
#define LED7_PORT_DIR  PORT2.PDR.BIT.B7

#define LED_GRN_PORT_DIR  PORT1.PDR.BIT.B3
#define LED_BLU_PORT_DIR  PORT1.PDR.BIT.B5
#define LED_RED_PORT_DIR  PORT1.PDR.BIT.B7

#define BAT0_EN PORTA.PODR.BIT.B7
#define BAT1_EN PORTB.PODR.BIT.B1
#define BAT2_EN PORTB.PODR.BIT.B2
#define BAT3_EN PORTB.PODR.BIT.B3
#define MAX1614_OFF  0
#define MAX1614_ON   1

#define BAT0_EN_DIR PORTA.PDR.BIT.B7
#define BAT1_EN_DIR PORTB.PDR.BIT.B1
#define BAT2_EN_DIR PORTB.PDR.BIT.B2
#define BAT3_EN_DIR PORTB.PDR.BIT.B3

#define PGOOD1_DIR PORT1.PDR.BIT.B9
#define PGOOD2_DIR PORTC.PDR.BIT.B2

#define PGOOD1 PORTC.PIDR.BIT.B3
#define PGOOD2 PORTC.PIDR.BIT.B2

#define OUT1_EN_DIR PORTB.PDR.BIT.B5
#define OUT2_EN_DIR PORTB.PDR.BIT.B7
#define OUT3_EN_DIR PORTJ.PDR.BIT.B3
#define OUT4_EN_DIR PORT5.PDR.BIT.B3
#define OUT5_EN_DIR PORT0.PDR.BIT.B7

#define OUT1_EN PORTB.PODR.BIT.B5
#define OUT2_EN PORTB.PODR.BIT.B7
#define OUT3_EN PORTJ.PODR.BIT.B3
#define OUT4_EN PORT5.PODR.BIT.B3
#define OUT5_EN PORT0.PODR.BIT.B7


/* Delay units */
#define uS        'u'
#define mS        'm'

/* End of multiple inclusion prevention macro */
#endif

