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

#include <stdbool.h>
#include <stdint.h>

#ifndef SWITCH_H
#define SWITCH_H

/* Switch 1 pressed flag mask */
#define SWITCHPRESS_1    0x80u
/* Switch 1 held down flag mask */
#define SWITCHHOLD_1    0x08u
/* Switch 2 pressed flag mask */
#define SWITCHPRESS_2    0x40u
/* Switch 2 held down flag mask */
#define SWITCHHOLD_2    0x04u
/* Switch 3 pressed flag mask */
#define SWITCHPRESS_3    0x20u
/* Switch 3 held down flag mask */
#define SWITCHHOLD_3    0x02u
/* Switch 4 pressed flag mask */
#define SWITCHPRESS_4    0x10u
/* Switch 4 held down flag mask */
#define SWITCHHOLD_4    0x01u

/* Any switch pressed flag mask */
#define SWITCHPRESS_ALL    0xF0u
/* Any switch held down flag mask */
#define SWITCHHOLD_ALL    0x0Fu

/* Defines the CMT compare match value for the short switch debounce */
#define DEBOUNCE_SHORT    0x080u
/* Defines the CMT compare match value for the long switch debounce */
#define DEBOUNCE_LONG    0x500u

/* Define switch interrupt priority level */
#define SWITCH_IPL      7u
  
/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Switch standby ready global flag */
extern volatile bool gSwitchStandbyReady;
/* Switch flag global variable. Switch status bits:
    b7 : Switch 1 press complete flag*
    b6 : Switch 2 press complete flag*
    b5 : Switch 3 press complete flag*
    b4 : Switch 4 press complete flag*
    b3 : Switch 1 held-down status flag
    b2 : Switch 2 held-down status flag
    b1 : Switch 3 held-down status flag
    b0 : Switch 4 held-down status flag
     * Switch press complete flags must be cleared manually     */
extern volatile uint8_t gSwitchFlag;
extern volatile uint16_t gSwitchFaultsDetected; // REMOVE ME LATER
/*******************************************************************************
* Global Function Prototypes
*******************************************************************************/
/* Switch initialisation function prototype */
void InitialiseSwitchInterrupts(void);
/* Switch enable/disable function prototype */
void ControlSwitchInterrupts(uint8_t);
/* Switch callback function initialisation function prototype */
void SetSwitchPressCallback(void(*callBack)(void));
/* Switch callback function initialisation function prototype */
void SetSwitchReleaseCallback(void(*callBack)(void));

#endif
