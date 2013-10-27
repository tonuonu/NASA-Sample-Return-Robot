/* Copyright (C) 2012 Renesas Electronics Corporation. All rights reserved.   */
/*******************************************************************************
* File Name    : hwsetup.c
* Version     : 1.00
* Device     : R5F5630E
* Tool-Chain   : Renesas RX Toolchain 1.2.0.0
* H/W Platform  : RSKRX630
* Description   : Defines the hardware initialisation routines used each time 
*          the MCU is restarted. 
*******************************************************************************/
/*******************************************************************************
* History     : 23 Jan. 2012  Ver. 1.00 First Release
*******************************************************************************/

/*******************************************************************************
System Includes
*******************************************************************************/
/* Includes assembley level definitions */
#include    <machine.h>
/* Following header file provides C99 type definitions. */
#include <stdint.h>

/*******************************************************************************
User Includes (Project Level Includes)
*******************************************************************************/
/* Defines RX630 port registers */
#include "iorx630.h"
/* Defines macros relating to the RX630 user LEDs and switches */
#include "rskrx630def.h"
/* LCD controlling function prototypes & macro definitions */
#include "oled.h"
/* Switch handler function definitions */
#include "switch.h"
/* Contains declarations for the functions defined in this file */
#include "hwsetup.h"

/*******************************************************************************
* Outline     : HardwareSetup
* Description   : Contains all the setup functions called at device restart
* Argument      : none
* Return value  : none
*******************************************************************************/
void HardwareSetup(void)
{
  ConfigureOperatingFrequency();
  ConfigureOutputPorts();
  ConfigureInterrupts();
  EnablePeripheralModules();
}
/*******************************************************************************
* End of function HardwareSetup
*******************************************************************************/

/*******************************************************************************
* Outline     : ConfigureOperatingFrequency
* Description   : Configures the clock settings for each of the device clocks
* Argument      : none
* Return value  : none
*******************************************************************************/
void ConfigureOperatingFrequency(void)
{      
  /* Declare and initialise a loop count variable */
  uint16_t i = 0;
    
  /* Protection off */
  SYSTEM.PRCR.WORD = 0xA503;      
  
  /* Stop sub-clock */
  SYSTEM.SOSCCR.BYTE = 0x01;      
                    
  /* Specify a wait state greater than 10ms at 12MHz (10.92 msec) */ 
  SYSTEM.MOSCWTCR.BYTE = 0x0D;    
    
  /* 4194304 state (default)*/
  /* wait over 12ms  @PLL=192MHz(12MHz*16) */                
  SYSTEM.PLLWTCR.BYTE = 0x0F;      
  
  /* x16 @PLL */
  SYSTEM.PLLCR.WORD = 0x0F00;  
  
  /* Configure the EXTAL pin */
  PORT3.PMR.BIT.B6 = 0;
  PORT3.PDR.BIT.B6 = 0;

  /* Configure the XTAL pin */
  PORT3.PMR.BIT.B7 = 0;
  PORT3.PDR.BIT.B7 = 0;    

  /* Turn on EXTAL */
  SYSTEM.MOSCCR.BYTE = 0x00;  
      
  /* Turn on the PLL and enable writing to the PLL control register */
  SYSTEM.PLLCR2.BYTE = 0x00;  
  
  /* Wait over 12ms */
  for(i=0; i<2075; i++)
  {
    nop();
  }
    
  /* Configure the clocks as follows -
  Clock Description              Frequency
  ----------------------------------------
  PLL Clock frequency...............192MHz
  System Clock Frequency.............96MHz
  Peripheral Module Clock B..........48MHz   
  FlashIF Clock......................48MHz
  External Bus Clock.................48MHz */       
  SYSTEM.SCKCR.LONG = 0x21821211;    
  
  /* Configure the clocks as follows -
  Clock Description              Frequency
  ----------------------------------------
  USB Clock..........................48MHz    
  IEBus Clock........................24MHz */ 
  SYSTEM.SCKCR2.WORD = 0x0033;
  
  /* Set the clock source to PLL */
  SYSTEM.SCKCR3.WORD = 0x0400;
  
  /* Protection on */
  SYSTEM.PRCR.WORD = 0xA500;    
}
/*******************************************************************************
* End of function ConfigureOperatingFrequency
*******************************************************************************/

/*******************************************************************************
* Outline     : ConfigureOutputPorts
* Description   : Configures the port and pin direction settings, and sets the
*          pin outputs to a safe level.
* Argument    : none
* Return value  : none
*******************************************************************************/
void ConfigureOutputPorts(void)
{  

    /* Configure the LED pins (LED0-LED3) as outputs */
  LED0_PORT_DIR = 0x1;
  LED1_PORT_DIR = 0x1;
//  LED2_PORT_DIR = 0x1;
//  LED3_PORT_DIR = 0x1;

  /* Set LED pin outputs as high (all LEDs off) */
  LED0 = LED_OFF;
  LED1 = LED_OFF;
//  LED2 = LED_OFF;
//  LED3 = LED_OFF;
#if 0  
    /* Configure the debug LCD data pins (DLCD7-DLCD4) as outputs */
  LCD_DATA_DIR_PORT = 0xF0;
      
  /* Set debug LCD data pin (DLCD7-DLCD4) outputs as low */
  LCD_DATA_PORT = 0x00;
  
  /* Configure the debug LCD control pin (DLCDRS) and enable pin (DLCDE)
     as output */
  RS_PIN_DIR = 0x1;
  E_PIN_DIR  = 0x1;
      
  /* Set the debug LCD control pin (DLCDRS) and enable pin (DLCDE) as output
     low */
  RS_PIN = 0x0;
  E_PIN  = 0x0;  
#endif
}
/*******************************************************************************
* End of function ConfigureOutputPorts
*******************************************************************************/

/*******************************************************************************
* Outline     : ConfigureInterrupts
* Description   : Configures the interrupts used
* Argument    : none
* Return value  : none
*******************************************************************************/
void ConfigureInterrupts(void)
{
  /* Configure switch interrupts */
  InitialiseSwitchInterrupts();
}
/*******************************************************************************
* End of function ConfigureInterrupts
*******************************************************************************/

/*******************************************************************************
* Outline     : EnablePeripheralModules
* Description   : Enables and configures peripheral devices on the MCU
* Argument    : none
* Return value  : none
*******************************************************************************/
void EnablePeripheralModules(void)
{
  /* Peripherals initialised in specific initialisation functions */
}
/*******************************************************************************
* End of function EnablePeripheralModules
*******************************************************************************/
