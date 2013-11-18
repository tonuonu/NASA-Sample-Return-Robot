/* Copyright (C) 2012 Renesas Electronics Corporation. All rights reserved.   */
/*******************************************************************************
* File Name    : pwm.c
* Version     : 1.00
* Device     : R5F5630E
* Tool-Chain   : Renesas RX Toolchain 1.2.0.0
* H/W Platform  : RSKRX630
* Description   : Defines MTU3 configuration functions
*******************************************************************************/
/* Following header file provides standard integer type definitions. */
#include <stdint.h>
/* Following header file provides string type definitions. */
#include <string.h>

/*******************************************************************************
User Includes (Project Level Includes)
*******************************************************************************/
/* Defines RX630 LEDs and switches */
#include "rskrx630def.h"
/* Defines RX630 port registers */
#include "iorx630.h"
/* LCD controlling function prototypes & macro defines */
#include "oled.h"
/* Switch handler function definitions */
#include "switch.h"
/* Declares function prototypes defined in this file */
#include "pwm.h"
#include <stdint.h>

/******************************************************************************
* Local Function Prototypes
******************************************************************************/
/* Callback function prototype for switch presses */
void CB_Switch_Press(void);
/* 16 bit integer to character string function prototype declaration */
static void uint16_ToBCDString(uint8_t *, uint8_t, uint16_t);


/******************************************************************************
Global Variables
******************************************************************************/
/* Variable for storing the event count */
volatile uint16_t gDutyPercentage;
/* Variable for halting updates and displaying the duty cycle */
volatile bool gVaryDutyCycle = true;
/* Variable for starting clearance of LCD's line 
   two only after the first SW1 press */
volatile bool gClearLCD = false;


/*******************************************************************************
* Outline     : Init_PWM
* Description   : Function configures the MTU's output pin MTIOC3A. It then 
*          initialises the MTU's channel 3 to output a 1KHz PWM signal,
*          with interrupt generations enabled.
* Argument    : none
* Return value  : none
*******************************************************************************/
void Init_PWM(void)
{  
  /* Configure pin 7 of port 1 as an output */
  PORT1.PDR.BIT.B7 = 1;
  
  /* Configure pin 7 of port 1 for peripheral function */
  PORT1.PMR.BIT.B7 = 1;
  
  /* Enable write to PFSWE bit */
  MPC.PWPR.BYTE = 0x00;
  
  /* Disable write protection to PFS registers */
  MPC.PWPR.BYTE = 0x40;
  
  /* Configure pin 7 of port 1 for MTIOC3A outputs */
  MPC.P17PFS.BIT.PSEL = 0x1;
  
  /* Enable write protection to PFS registers */
  MPC.PWPR.BYTE = 0x80;
  
  /* Protection off */
  SYSTEM.PRCR.WORD = 0xA503;
  
  /* Cancel the MTU3 module clock stop mode */
  MSTP_MTU3 = 0x0;
    
  /* Protection on */
  SYSTEM.PRCR.WORD = 0xA500;
              
  /* Set MTU3 TGIA3 interrupt priority level to 7 */  
  IPR(MTU3,TGIA3) = 0x7;
  /* Enable MTU3 TGIA3 interrupts */
  IEN(MTU3,TGIA3) = 0x1;
  /* Clear MTU3 TGIA3 interrupt flag */
  IR(MTU3,TGIA3) = 0x0;
  
  /* Set MTU3 TGIB3 interrupt priority level to 7 */  
  IPR(MTU3,TGIB3) = 0x7;
  /* Enable MTU3 TGIB3 interrupts */
  IEN(MTU3,TGIB3) = 0x1;
  /* Clear MTU3 TGIB3 interrupt flag */
  IR(MTU3,TGIB3) = 0x0;
  
  /* Clock PCLK/1=48MHz, count at falling edge, 
     TCNT cleared by TGRA compare match */
  MTU3.TCR.BYTE = 0x28;
  
  /* Operate timers in pwm mode */
  MTU3.TMDR.BYTE = 0x02;
  
  /* Initial MTIOC3A output is 1 and toggled at 
     TGRA compare match. TRGB output is disabled. */
  MTU3.TIORH.BYTE = 0x37;
  
  /* Enable TGIEA interrupts */
  MTU3.TIER.BYTE = 0x01;
  
  /* Enable access to protected MTU registers */
  MTU.TRWER.BIT.RWE = 0x1;
  
  /* Use TGRA to set the pwm cycle to 1KHz*/
  MTU3.TGRA = 48000;
  
  /* TGRB used to set the pwm duty cycle to 90% */
  MTU3.TGRB = 4800;
  
  /* Set a switch press callback function */
  SetSwitchReleaseCallback(CB_Switch_Press);

  /* Start the TCNT counter */  
  MTU.TSTR.BIT.CST3 = 0x1;
}
/*******************************************************************************
* End of function Init_PWM
*******************************************************************************/

/*******************************************************************************
* Outline      : uint16_ToBCDString
* Description  : This function converts an 16bit integer into a BCD string,
*         which is inserted into the character array passed as an 
*         argument.
* Argument     : uint8_t * string  - Pointer to array to store BCD string 
*         uint8_t pos    - Array element to start BCD string from.
*         uint16_t number  - 16bit integer to convert.
* Return value : none
* Note       : No input validation is used, so output data can overflow the
*         array passed.
*******************************************************************************/
static void uint16_ToBCDString(uint8_t * output_string, uint8_t pos, 
      uint16_t number)
{
  /* Local digit position variable */
  uint8_t digit = 5u;
  
  /* Digit is a leading zero indicator variable */
  bool leading_zero = true;
  
  /* Local string buffer variable */
  uint8_t buffer_string[5] = {0,0,0,0,0};
  
  /* Decrement digit each iteration, until digit is zero */
  while(digit--)
  {
    /* Write BCD value to string element */
    buffer_string[digit] =(uint8_t) (0x30 + (number % 10));
    
    /* Move to the next decimal digit */
    number /= 10u;
  }
  
  /* Set digit position to zero */
  digit = 0u;
  
  /* Loop through each digit, and replace leading zeros with a space
     character */
  while(digit < 5u)
  {
    /* Check if digit is not a zero character, or if leading_zero indicator is
       false */
    if((buffer_string[digit] != 0x30) || (!leading_zero))
    {
      /* Copy digit from buffer string to output string */
      output_string[digit + pos] = buffer_string[digit];
      
      /* Set the leading zero indicator variable to false */
      leading_zero = false;  
    }
    
    /* Increment the loop counter */
    digit++;
  }
}
/*******************************************************************************
* End of function uint16_ToBCDString
*******************************************************************************/

/*******************************************************************************
* Outline      : CB_Switch_Press
* Description  : Switch Press callback function. This function checks which 
*         switch was pressed. The second line of the debud LCD is cleared 
*         if switch SW1 is pressed and if the gClearLCD flag is set to 
*         true, allowing the duty cycle to be varied. If the flag isn't 
*         set but SW1 has been pressed, duty cycle variation is stopped 
*         and the duty is displayed as a 
*         percentage.   
* Argument     : none
* Return value : none
*******************************************************************************/
void CB_Switch_Press(void)
{  
  /* Local LCD buffer string */
  uint8_t lcd_buffer[] = "D.C:   %";
  uint32_t calc_duty_percent = 0;
  
  /* Has switch SW1 been pressed and LCD clearance allowed? */
  if(!(gSwitchFlag & SWITCHPRESS_1) && (gClearLCD == true))
  {  
    /* Allow duty cycle to be updated */
    gVaryDutyCycle  = true;

    /* Clear the second line of the debug LCD */
    //Display_LCD(LCD_LINE2, "        ");
  }

  /* Set the gVaryDutyCycle to false and display the duty 
     cycle as a percentage if SW1 has been pressed */
  if((gSwitchFlag & SWITCHPRESS_1))
  {
    /* Set flag to indicate stoppage of duty cycle updates. */
    gVaryDutyCycle  = false;
    
    /* Set to true to allow clearance of LCD's line 2 */
    gClearLCD = true;
        
    /* Calculate the percentage */
    calc_duty_percent = (uint32_t)((gDutyPercentage * 100) / 48000); 
            
    /* Convert the calculated percentage into a BCD character string */
    uint16_ToBCDString(lcd_buffer, 2u, (uint16_t)calc_duty_percent);

    /* Display contents of lcd_buffer onto the debug LCD */
    //Display_LCD(LCD_LINE2, (uint8_t*)lcd_buffer);
  }
  
  /* Clear the switch press flags */
  gSwitchFlag &= 0x0F;
}
/*******************************************************************************
* End of function CB_Switch_Press
*******************************************************************************/
  
/*******************************************************************************
* Outline      : Excep_MTU3_TGIA3
* Description  : MTU3 TGIA3 interrupt handler. This function updates the duty 
*         cycle by 1%, resets the duty to 10% if it reaches 90%, and
*         automatically toggles the MTIOC3A pin on every TGRA output 
*         compare match. 
* Argument     : none
* Return value : none
*******************************************************************************/

#pragma vector=VECT_MTU3_TGIA3
__interrupt void Excep_MTU3_TGIA3(void)
{  
  static uint16_t counter=0;
  /* Stop the TCNT counter */  
  MTU.TSTR.BIT.CST3 = 0x0;
#if 1
  /* Update the duty cycle if SW1 hasn't been pressed. */
  if(gVaryDutyCycle == true)
  {  
    /* Reset the duty cyccle to 10% */
    MTU3.TGRB += 12;
    
    /* Read duty cycle */
    gDutyPercentage = MTU3.TGRB;
        
    /* Check if the duty cycle equals 90% */
    if(gDutyPercentage > 43200)
    {
#endif
      /* Reset the duty cyccle to 10% */
      MTU3.TGRB = 4800;
#if 1
    }
  }
#endif
  //unsigned char *p = (unsigned char *)0x07000000;
  switch(counter) {
  case 0:
      LED0 ^=1;
      /* Configure buzzer for peripheral function */
      PORT1.PMR.BIT.B7 = 1;

      counter++;
      break;
  case 2:
      /* Configure buzzer for NOT peripheral function */
      PORT1.PMR.BIT.B7 = 0;
//      for(int i=0;i<10;i++) *p++  = i;      
      counter++;
      break;
  case 999:
      counter=0;
      break;
  default:
      counter++;
  }
  if(counter >= 999)
     counter=0;
#if 0
  if(counter%2)  
    OLED_DATA_PORT=0x55;
  else
    OLED_DATA_PORT=0xAA;
#endif  
  
//  Init_OLED();
  /* Start the TCNT counter */  
  MTU.TSTR.BIT.CST3 = 0x1;    
}
