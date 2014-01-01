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
* Description   : Defines interrupt service routine functions for the switch
*          interrupts. De-bounces switch and sets a key press flag. 
*******************************************************************************/

#include <stdint.h>
#include <string.h>
#include "rskrx630def.h"
#include "iorx630.h"
#include "switch.h"

/* Switch flag global variable */
volatile uint8_t gSwitchFlag = 0x00;
/* Switch standby ready global variable */
volatile bool gSwitchStandbyReady = true;
/* Detected switch faults counter variable */
volatile uint16_t gSwitchFaultsDetected = 0;
/* Switch press callback pointer declaration */
void (*gSwitchPressCallbackFunc)(void) = NULL;
/* Switch release callback pointer declaration */
void (*gSwitchReleaseCallbackFunc)(void) = NULL;
/* Start debounce timer function prototype */ 
void StartDebounceTimer(uint16_t);
/* Switch debounce timer callback function prototype */
void SwitchDebounceCB(void);

/*******************************************************************************
* Description   : Configures the IRQ pins connected to switches SW1-SW4 to
*          detect switch presses, and invoke an ISR.
*******************************************************************************/
void InitialiseSwitchInterrupts(void)
{    
  /* Reset the switch flag */
  gSwitchFlag = 0x00;
  
  /* Reset the switch standby ready flag */
  gSwitchStandbyReady = 0x1;
  
  /* Enable write to PFSWE bit */
  MPC.PWPR.BYTE = 0x00;
  /* Disable write protection to PFS registers */
  MPC.PWPR.BYTE = 0x40;
  
  /* Set IRQ0 on Port3 as an input pin */
  MPC.P30PFS.BIT.ISEL = 0x1;
  /* Set IRQ1 on Port3 as an input pin */
  MPC.P31PFS.BIT.ISEL = 0x1;
  /* Set IRQ2 on Port3 as an input pin */
  MPC.P32PFS.BIT.ISEL = 0x1;
  /* Set IRQ3 on Port3 as an input pin */
  MPC.P33PFS.BIT.ISEL = 0x1;

  /* Select the pin function for the lower 5 bits */
  MPC.P30PFS.BIT.PSEL = 0x0;
  MPC.P31PFS.BIT.PSEL = 0x0;
  MPC.P32PFS.BIT.PSEL = 0x0;
  MPC.P33PFS.BIT.PSEL = 0x0;
    
  /* Enable write protection to PFS registers */
  MPC.PWPR.BYTE = 0x80;
  
  /* Disable digital pin filtering for all IRQ pins */
  ICU.IRQFLTE1.BYTE = 0;
  
  /* Configure switch SW1..SW4 (IRQ0..IRQ3) */
  /* Clear SW1 interrupt requests */
  IEN(ICU, IRQ0) = 0x0;  
  IEN(ICU, IRQ1) = 0x0;  
  IEN(ICU, IRQ2) = 0x0;  
  IEN(ICU, IRQ3) = 0x0;  
  /* Set P3_x (SWx) as an input */
  PORT3.PDR.BIT.B0 = 0;
  PORT3.PDR.BIT.B1 = 0;
  PORT3.PDR.BIT.B2 = 0;
  PORT3.PDR.BIT.B3 = 0;

  /* Set detection direction as falling edge */
  ICU.IRQCR[0].BIT.IRQMD = 0x1;  
  ICU.IRQCR[1].BIT.IRQMD = 0x1;  
  ICU.IRQCR[2].BIT.IRQMD = 0x1;  
  ICU.IRQCR[3].BIT.IRQMD = 0x1;  
  /* Clear IR flag */
  IR(ICU, IRQ0) = 0x0;            
  IR(ICU, IRQ1) = 0x0;            
  IR(ICU, IRQ2) = 0x0;            
  IR(ICU, IRQ3) = 0x0;            
  /* Set interrupt priority level to 7 */
  IPR(ICU, IRQ0) = SWITCH_IPL;  
  IPR(ICU, IRQ1) = SWITCH_IPL;  
  IPR(ICU, IRQ2) = SWITCH_IPL;  
  IPR(ICU, IRQ3) = SWITCH_IPL;  
  /* Enable SW1 interrupt requests */
  IEN(ICU, IRQ0) = 0x1;
  IEN(ICU, IRQ1) = 0x1;
  IEN(ICU, IRQ2) = 0x1;
  IEN(ICU, IRQ3) = 0x1;
}

/******************************************************************************
* Description   : Takes a pointer to a function, and sets it as the callback
*          function for the switch interrupts. The passed function is
*          executed every time any switch is pressed down.
* Argument    : pointer to callback function (set to NULL to disable)
******************************************************************************/
void SetSwitchPressCallback(void(*callback)(void))
{
  /* Store the callback function pointer into the global variable */
  gSwitchPressCallbackFunc = callback;
}

/******************************************************************************
* Description   : Takes a pointer to a function, and sets it as the callback
*          function for the switch interrupts. The passed function is
*          executed every time any switch is released.
* Argument    : pointer to callback function (set to NULL to disable)
******************************************************************************/
void SetSwitchReleaseCallback(void(*callback)(void))
{
  /* Store the callback function pointer into the global variable */
  gSwitchReleaseCallbackFunc = callback;
}

/******************************************************************************
* Description   : Enables or disables the switch IRQ interrupts, based on the
*          input variable, control.
* Argument    : uint8_t - 0x1  : Enables switch interrupts.
*                0x0  : Disables switch interrupts.
******************************************************************************/
void ControlSwitchInterrupts(uint8_t control)
{  
  /* Check if control input is 0x1 */
  if(control)
  {
    /* Enable SW1 interrupt requests */
    IEN(ICU, IRQ0) = 0x1;
    /* Enable SW2 interrupt requests */
    IEN(ICU, IRQ1) = 0x1;  
    /* Enable SW3 interrupt requests */
    IEN(ICU, IRQ2) = 0x1;
    /* Enable SW4 interrupt requests */
    IEN(ICU, IRQ3) = 0x1;
  }
  /* Control input is 0x0 */
  else
  {  
    /* Disable SW1 interrupts */
    IEN(ICU, IRQ0) = 0x0;
    /* Disable SW2 interrupts */
    IEN(ICU, IRQ1) = 0x0;
    /* Disable SW3 interrupts */
    IEN(ICU, IRQ2) = 0x0;
    /* Disable SW4 interrupts */
    IEN(ICU, IRQ3) = 0x0;
  }
}  
/******************************************************************************
* Description   : Switch 1 callback ISR function. The function disables switch
*          interrupts, then uses the debounce timer to re-enable them
*          after the debounce period finishes.
******************************************************************************/
#pragma vector=VECT_ICU_IRQ0
__interrupt void Excep_IRQ0(void)
{    
  /* Disable switch 1 interrupts */
  IEN(ICU, IRQ0) = 0x0;
  
  /* Set standby ready flag as false */
  gSwitchStandbyReady = false;
            
  /* Check if interrupt was generated by falling edge */
  if(0x1 == ICU.IRQCR[0].BIT.IRQMD)
  {
    /* Start debounce timer */
    StartDebounceTimer(DEBOUNCE_SHORT);    
    
    /* Set detection direction as rising edge */
    ICU.IRQCR[0].BIT.IRQMD = 0x2;
        
    /* Set global switch flag to indicate SW1 is held down */
    gSwitchFlag |= SWITCHHOLD_1;
  }
  else
  {  
    /* Start debounce timer */
    StartDebounceTimer(DEBOUNCE_LONG);    
      
    /* Set detection direction to falling edge */
    ICU.IRQCR[0].BIT.IRQMD = 0x1;
    
    /* Clear SW1 held-down flag bit in switch flag */
    gSwitchFlag &= (0xF0 | ~SWITCHHOLD_1);
    
    /* Set global switch flag to indicate SW1 press complete */
    gSwitchFlag |= SWITCHPRESS_1;
  
    /* Check if switch release callback function is not NULL */
    if(gSwitchReleaseCallbackFunc)
    {
      /* Execute user callback function */
      gSwitchReleaseCallbackFunc();
    }
  }
}
/******************************************************************************
* Description   : Switch 2 callback ISR function. The function disables switch
*          interrupts, then uses the debounce timer to re-enable them
*          after the debounce period finishes.
******************************************************************************/
#pragma vector=VECT_ICU_IRQ1
__interrupt void Excep_IRQ1(void)
{    
  /* Disable switch 1 interrupts */
  IEN(ICU, IRQ1) = 0x0;
  
  /* Set standby ready flag as false */
  gSwitchStandbyReady = false;
            
  /* Check if interrupt was generated by falling edge */
  if(0x1 == ICU.IRQCR[1].BIT.IRQMD)
  {
    /* Start debounce timer */
    StartDebounceTimer(DEBOUNCE_SHORT);    
    
    /* Set detection direction as rising edge */
    ICU.IRQCR[1].BIT.IRQMD = 0x2;
        
    /* Set global switch flag to indicate SW2 is held down */
    gSwitchFlag |= SWITCHHOLD_2;
  }
  else
  {  
    /* Start debounce timer */
    StartDebounceTimer(DEBOUNCE_LONG);    
      
    /* Set detection direction to falling edge */
    ICU.IRQCR[1].BIT.IRQMD = 0x1;
    
    /* Clear SW2 held-down flag bit in switch flag */
    gSwitchFlag &= (0xF0 | ~SWITCHHOLD_2);
    
    /* Set global switch flag to indicate SW2 press complete */
    gSwitchFlag |= SWITCHPRESS_2;
  
    /* Check if switch release callback function is not NULL */
    if(gSwitchReleaseCallbackFunc)
    {
      /* Execute user callback function */
      gSwitchReleaseCallbackFunc();
    }
  }
}
/******************************************************************************
* Description   : Switch 3 callback ISR function. The function disables switch
*          interrupts, then uses the debounce timer to re-enable them
*          after the debounce period finishes.
******************************************************************************/
#pragma vector=VECT_ICU_IRQ2
__interrupt void Excep_IRQ2(void)
{    
  /* Disable switch 1 interrupts */
  IEN(ICU, IRQ2) = 0x0;
  
  /* Set standby ready flag as false */
  gSwitchStandbyReady = false;
            
  /* Check if interrupt was generated by falling edge */
  if(0x1 == ICU.IRQCR[2].BIT.IRQMD)
  {
    /* Start debounce timer */
    StartDebounceTimer(DEBOUNCE_SHORT);    
    
    /* Set detection direction as rising edge */
    ICU.IRQCR[2].BIT.IRQMD = 0x2;
        
    /* Set global switch flag to indicate SW1 is held down */
    gSwitchFlag |= SWITCHHOLD_3;
  }
  else
  {  
    /* Start debounce timer */
    StartDebounceTimer(DEBOUNCE_LONG);    
      
    /* Set detection direction to falling edge */
    ICU.IRQCR[2].BIT.IRQMD = 0x1;
    
    /* Clear SW1 held-down flag bit in switch flag */
    gSwitchFlag &= (0xF0 | ~SWITCHHOLD_3);
    
    /* Set global switch flag to indicate SW1 press complete */
    gSwitchFlag |= SWITCHPRESS_3;
  
    /* Check if switch release callback function is not NULL */
    if(gSwitchReleaseCallbackFunc)
    {
      /* Execute user callback function */
      gSwitchReleaseCallbackFunc();
    }
  }
}
  
/******************************************************************************
* Description   : Switch 4 callback ISR function. The function disables switch
*          interrupts, then uses the debounce timer to re-enable them
*          after the debounce period finishes.
******************************************************************************/
#pragma vector=VECT_ICU_IRQ3
__interrupt void Excep_IRQ3(void)
{    
  /* Disable switch 1 interrupts */
  IEN(ICU, IRQ3) = 0x0;
  
  /* Set standby ready flag as false */
  gSwitchStandbyReady = false;
            
  /* Check if interrupt was generated by falling edge */
  if(0x1 == ICU.IRQCR[3].BIT.IRQMD)
  {
    /* Start debounce timer */
    StartDebounceTimer(DEBOUNCE_SHORT);    
    
    /* Set detection direction as rising edge */
    ICU.IRQCR[3].BIT.IRQMD = 0x2;
        
    /* Set global switch flag to indicate SW1 is held down */
    gSwitchFlag |= SWITCHHOLD_4;
  }
  else
  {  
    /* Start debounce timer */
    StartDebounceTimer(DEBOUNCE_LONG);    
      
    /* Set detection direction to falling edge */
    ICU.IRQCR[3].BIT.IRQMD = 0x1;
    
    /* Clear SW1 held-down flag bit in switch flag */
    gSwitchFlag &= (0xF0 | ~SWITCHHOLD_4);
    
    /* Set global switch flag to indicate SW1 press complete */
    gSwitchFlag |= SWITCHPRESS_4;
  
    /* Check if switch release callback function is not NULL */
    if(gSwitchReleaseCallbackFunc)
    {
      /* Execute user callback function */
      gSwitchReleaseCallbackFunc();
    }
  }
}


/******************************************************************************
* Description   : Switch debounce timer callback function. Function is executed
*          by the CMT channel ISR. Function re-enables all switch all
*          switch interrupts; then verifies a correct switch press was
*          detected, and calls the user switch press callback function.
******************************************************************************/
void SwitchDebounceCB(void)
{  
#if 0
  /* Clear IRQ2 interrupt flag */
  IR(ICU, IRQ2) = 0;
  
  /* Re-enable switch 1 interrupts */
  IEN(ICU, IRQ2) = 0x1;

  /* Clear IRQ12 interrupt flag */
  IR(ICU, IRQ12) = 0;

  /* Re-enable switch 2 interrupts */
  IEN(ICU, IRQ12) = 0x1;

  /* Clear IRQ15 interrupt flag */
  IR(ICU, IRQ15) = 0;

  /* Re-enable switch 3 interrupts */
  IEN(ICU, IRQ15) = 0x1;
  
  /* Check if switch 1 hold flag is set */ 
  if((gSwitchFlag & 0x0F) & SWITCHHOLD_1)
  {
    /* Check if switch 1 pin level is high (switch press duration
       less than debounce, invalid switch press) */
    if(PORT3.PIDR.BIT.B2)
    {
      /* Reset detection direction to falling edge */
      ICU.IRQCR[2].BIT.IRQMD = 0x1;
      
      /* Clear switch press flag */
      gSwitchFlag &= (0xF0 | ~SWITCHHOLD_1);
      
      /* Increment detected switch faults counter */
      gSwitchFaultsDetected++;
    }
    /* Switch 1 pin level is low (valid switch press) */
    else
    {
      /* Check if switch press callback function is not NULL */
      if(gSwitchPressCallbackFunc)
      {
        /* Execute user callback function */
        gSwitchPressCallbackFunc();
      }
    }
  }
  
  /* Check if switch 2 hold flag is set */   
  if((gSwitchFlag & 0x0F) & SWITCHHOLD_2)
  {
    /* Check if switch 2 pin level is high (switch press duration
       less than debounce, invalid switch press) */
    if(PORT4.PIDR.BIT.B4)
    {
      /* Reset detection direction to falling edge */
      ICU.IRQCR[12].BIT.IRQMD = 0x1;
      
      /* Clear switch press flag */
      gSwitchFlag &= (0xF0 | ~SWITCHHOLD_2);
      
      /* Increment detected switch faults counter */
      gSwitchFaultsDetected++;
    }
    /* Switch 2 pin level is low (valid switch press) */
    else
    {
      /* Check if switch press callback function is not NULL */
      if(gSwitchPressCallbackFunc)
      {
        /* Execute user callback function */
        gSwitchPressCallbackFunc();
      }
    }
  }
  
  /* Check if switch 3 hold flag is set */ 
  if((gSwitchFlag & 0x0F) & SWITCHHOLD_3)
  {
    /* Check if switch 3 pin level is high (switch press duration
       less than debounce, invalid switch press) */
    if(PORT0.PIDR.BIT.B7)
    {
      /* Reset detection direction to falling edge */
      ICU.IRQCR[15].BIT.IRQMD = 0x1;
      
      /* Clear switch press flag */
      gSwitchFlag &= (0xF0 | ~SWITCHHOLD_3);
      
      /* Increment detected switch faults counter */
      gSwitchFaultsDetected++;
    }
    /* Switch 3 pin level is low (valid switch press) */
    else
    {
      /* Check if switch press callback function is not NULL */
      if(gSwitchPressCallbackFunc)
      {
        /* Execute user callback function */
        gSwitchPressCallbackFunc();
      }
    }
  }
  
  /* Check if any switches are held down */
  if(0x00 == (gSwitchFlag & 0x0F))
  {    
    gSwitchStandbyReady = true;
  }
#endif
}

/******************************************************************************
* Description   : Function initialises the CMT timer (the first time the
*          function is called), and starts the CMT timer to perform
*          generate the switch debounce interrupt.
* Argument      : uint16_t compare_match  : compare match value to trigger
*                        *interrupt at.
******************************************************************************/
void StartDebounceTimer(uint16_t compare_match)
{  
  /* Declare local static variable to track if the CMT timer has been 
    initialised yet */
  static bool timer_initialised = false;
  
  /* Check if the CMT timer is not initialised (first time function has been
     called). */
  if(!timer_initialised)
  {  
    /* Disable register protection */
    SYSTEM.PRCR.WORD = 0xA50B;  
  
    /* Enable the CMT0 module */
    MSTP_CMT0 = 0;
  
    /* Re-enable register protection */
    SYSTEM.PRCR.BIT.PRKEY = 0xA5u;
    SYSTEM.PRCR.WORD &= 0xFF00;  
  
    /* Set CMT0 clock source as PLCK/512 */
    CMT0.CMCR.BIT.CKS = 0x2;
   
    /* Enable compare match interrupt */
    CMT0.CMCR.BIT.CMIE = 1;
  
    /* Enable CMT0 interrupt request */
    IEN(CMT0, CMI0) = 1;
  
    /* Set interrupt priority to 12 */
    IPR(CMT0, CMI0) = 12u;
    
    /* Set the timer inititalised flag */
    timer_initialised = true;
  }
  
  /* Set compare match to to generate debounce period */
  CMT0.CMCOR = compare_match;
  
  /* Reset count to zero */
  CMT0.CMCNT = 0x0000;
    
  /* Start timer */
  CMT.CMSTR0.BIT.STR0 = 1;
}

/******************************************************************************
* Description   : Interupt service routine for the CMT channel 0 compare
*          interrupt. Fun
* Argument      : uint16_t compare_match  : compare match value to trigger
*                        *interrupt at.
******************************************************************************/
#pragma vector=VECT_CMT0_CMI0
__interrupt void Excep_CMTU0_CMT0(void)
{
  /* Stop the CMT0 timer running */
  CMT.CMSTR0.BIT.STR0 = 0;
  
  /* Call the switch debounce callback function */
  //SwitchDebounceCB();
  /* Clear the interrupt flag */
  ICU.IR[IR_CMT0_CMI0].BIT.IR = 0;

}
