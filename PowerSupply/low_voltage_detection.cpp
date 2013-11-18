/*
 *  Copyright (c) 2011-2013 Tonu Samuel
 *  All rights reserved.
 *
 *  This file is part of TYROS.
 *
 *  TYROS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TYROS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TYROS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "delay.h"
#include "hwsetup.h"
#include "oled.h"
#include "tnroman.h"
/* Defines RX630 port registers */
#include "iorx630.h"
/* Defines macros relating to the RX630 user LEDs and switches */
#include "rskrx630def.h"
#include "low_voltage_detection.h"

bool gVCCAboveVdet = true;


void Init_VoltageDetect(void) {	
	/* Disable write protection of the system registers */
	SYSTEM.PRCR.WORD = 0xA508;
	
	/* Set vdet1 to 2.95V */
	SYSTEM.LVDLVLR.BYTE = 0x0Au;
	
	/* Disable digital filter */
	SYSTEM.LVD1CR0.BIT.LVD1DFDIS = 1u;

	/* Enable vdet1 interrupt */
	SYSTEM.LVD1CR0.BIT.LVD1RI = 0;
	
	/* Set vdet1 to trigger when VCC is below trigger value */
	SYSTEM.LVD1CR1.BYTE = 0x02u;
	
	/* Enable output comparison */
	SYSTEM.LVD1CR0.BIT.LVD1CMPE = 1u;
	
	/* Clear the vdet1 flag */
	SYSTEM.LVD1SR.BIT.LVD1DET = 0;
	
	/* Ensure vdet flag is cleared */
	while(SYSTEM.LVD1SR.BIT.LVD1DET);
			
	/* Enable vdet1 interrupt */		
	SYSTEM.LVD1CR0.BIT.LVD1RIE = 1u;	
			
	/* Enable vdet1 */
	SYSTEM.LVCMPCR.BIT.LVD1E = 1u;		
			
	/* Enable vdet1 NMI interrupt */
	ICU.NMIER.BIT.LVD1EN = 1u;

	/* Re-enable write protection of the system registers */
	SYSTEM.PRCR.BIT.PRC3 = 0xA50u;
}

/*******************************************************************************
* Outline 		: NonMaskableInterrupt
* Description 	: NMI interrupt handler. The function is called when an NMI 
*				  interrupt is triggered (voltage detection interrupt triggers
*				  an NMI). If VCC < VDET, the function sets the voltage detect
*				  point to above VDET and vice versa. The function also updates
*				  the gVCCAboveVdet flag.
* Argument 	 	: none
* Return value  : none
*******************************************************************************/
void NonMaskableInterrupt(void)
{
	/* Clear vdet1 passage detection flag */
	SYSTEM.LVD1SR.BIT.LVD1DET = 0;
	
	/* Clear NMI interrupt flag */
	ICU.NMICLR.BIT.LVD1CLR = 1;
	
	/* Check if VCC is below trigger voltage */
	if(SYSTEM.LVD1SR.BIT.LVD1MON == 0)
	{
		/* Set vdet to trigger when VCC > trigger voltage */
		SYSTEM.LVD1CR1.BYTE = 0x00u;
		
		/* Set global vdet status flag */
		gVCCAboveVdet = false;	
	}
	/* VCC is above trigger voltage */
	else
	{
		/* Set vdet to trigger VCC < trigger voltage */
		SYSTEM.LVD1CR1.BYTE = 0x02u;	
		
		/* Set global vdet status flag */
		gVCCAboveVdet = true;
		
		/* Turn off LED3 */
		//LED3 = LED_OFF;
	}	
}
/*******************************************************************************
* End of function NonMaskableInterrupt
*******************************************************************************/
#if 0
/*******************************************************************************
* Outline 		: LedFlash_VoltageDetect
* Description 	: This function toggles all the user LEDs if gVCCAboveVdet flag
*				  is true, or turns LED3 and the other LEDs off if the flag is
*				  false. The function also waits in a while loop to make the 
*				  LED toggle a visible flash.
* Argument 	 	: none
* Return value  : none
*******************************************************************************/
void LedFlash_VoltageDetect(void)
{
	/* Check if gVCCAboveVdet is true (VCC above trigger voltage) */
	if(gVCCAboveVdet)
	{
		/* Toggle all user LEDs */
		LED0 = ~LED0;
		LED1 = ~LED1;
		LED2 = ~LED2;
		LED3 = ~LED3;
		
		/* Write VCC status to LCD */
		Display_LCD(LCD_LINE2, "VCC>VDET");
	}
	/* gVCCAboveVdet is false (VCC below trigger voltage) */
	else
	{
		/* Turn off LED0 to LED2 */
		LED0 = LED_OFF;
		LED1 = LED_OFF;
		LED2 = LED_OFF;
		
		/* Turn on LED3 */
		LED3 = LED_ON;
		
		/* Write VCC status to LCD */
		Display_LCD(LCD_LINE2, "VCC<VDET");
	}
	
	/* Define wait count variable */
	uint32_t wait_count = 0x000FFFFF;
	
	/* Decrement the count variable to create delay */
	while(wait_count--);
}

#endif
