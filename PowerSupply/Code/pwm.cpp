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
#include <stdio.h>
/* Following header file provides string type definitions. */
#include <string.h>

/*******************************************************************************
User Includes (Project Level Includes)
*******************************************************************************/
/* Defines RX630 LEDs and switches */
#include "rskrx630def.h"
/* Defines RX630 port registers */
#include "iorx630.h"
#include "switch.h"
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
  PORT1.PDR.BIT.B5 = 1;
  PORT1.PDR.BIT.B7 = 1;
  
  /* Configure pin 7 of port 1 for peripheral function */
  PORT1.PMR.BIT.B5 = 1;
  PORT1.PMR.BIT.B7 = 1;
  
  /* Enable write to PFSWE bit */
  MPC.PWPR.BYTE = 0x00;
  
  /* Disable write protection to PFS registers */
  MPC.PWPR.BYTE = 0x40;
  
  /* Configure pin 7 of port 1 for MTIOC3A outputs */
  MPC.P15PFS.BIT.PSEL = 0x1;
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

/***************/
  
  /*
   * Battery charger PWMs init
   */
  SYSTEM.PRCR.WORD = 0xA502;
  SYSTEM.MSTPCRA.BIT.MSTPA13=0; // Get module out of stop mode

  /* Configure pins of port E for peripheral function */
  PORTA.PMR.BIT.B0 = 1;
  PORTA.PMR.BIT.B4 = 1;
  PORTA.PMR.BIT.B6 = 1;
  PORTB.PMR.BIT.B0 = 1;

  /* Configure pins of port E as an output */
  PORTA.PDR.BIT.B0 = 1;
  PORTA.PDR.BIT.B4 = 1;
  PORTA.PDR.BIT.B6 = 1;
  PORTB.PDR.BIT.B0 = 1;

  MPC.PWPR.BIT.B0WI=0; // Enable writing to PFSWE
  MPC.PWPR.BIT.PFSWE=1;// Enable writing to PFS registers

  /* Configure pins of ports for MTIOCxA outputs */
  MPC.PA0PFS.BIT.PSEL = 0x3;
  MPC.PA4PFS.BIT.PSEL = 0x3;
  MPC.PA6PFS.BIT.PSEL = 0x3;
  MPC.PB0PFS.BIT.PSEL = 0x3;

  MPC.PWPR.BIT.PFSWE=0;// Enable writing to PFS registers
  MPC.PWPR.BIT.B0WI=1; // Enable writing to PFSWE
  
  /* Protection off */
  SYSTEM.PRCR.WORD = 0xA503;

  TPUA.TSTR.BIT.CST0=0;// stop clock
  TPUA.TSTR.BIT.CST1=0;// stop clock
  TPUA.TSTR.BIT.CST2=0;// stop clock
  TPUA.TSTR.BIT.CST3=0;// stop clock
  
  /* Protection on */
  SYSTEM.PRCR.WORD = 0xA500;

  TPUA.TSYR.BIT.SYNC0=1; // TCNT performs synchronous operation
  TPUA.TSYR.BIT.SYNC1=1; // --""--
  TPUA.TSYR.BIT.SYNC2=1; // --""--
  TPUA.TSYR.BIT.SYNC3=1; // --""--
  
  TPU0.TCR.BIT.TPSC=1; // input PCLK/4
  TPU1.TCR.BIT.TPSC=1; // --""--
  TPU2.TCR.BIT.TPSC=1; // --""--
  TPU3.TCR.BIT.TPSC=1; // --""--

  TPU0.TCR.BIT.CKEG=0; // Count only falling edges
  TPU1.TCR.BIT.CKEG=0; // --""--
  TPU2.TCR.BIT.CKEG=0; // --""--
  TPU3.TCR.BIT.CKEG=0; // --""--

  // page 757
  TPU0.TCR.BIT.CCLR=2; // TCNT counter cleared by TGRB compare match/input capture
  TPU1.TCR.BIT.CCLR=3; // TCNT counter cleared by counter clearing for another channel performing synchronous clearing/synchronous operation
  TPU2.TCR.BIT.CCLR=3; // --""--
  TPU3.TCR.BIT.CCLR=3; // --""--

  /* Read hardware manual chapters "24.3.5 PWM Modes" and 
   * "Table 24.24 PWM Output Registers and Output Pins" for poor explanation
   */
  TPU0.TMDR.BIT.MD = 2; // PWM mode 1  
  TPU1.TMDR.BIT.MD = 2; // PWM mode 1  
  TPU2.TMDR.BIT.MD = 2; // PWM mode 1
  TPU3.TMDR.BIT.MD = 2; // PWM mode 1
  
  /* 
   * In PWM mode 1 we use both TPUx.TGRA and TPUx.TGRB to trigger waveform 
   * of TIACAx pin, not TIACBx as you may think after reading hardware manual
   * about TGRB register function.
  
   * TPUx.TIORH.BIT.IOA=2; 
   * TIOCAx function, output compare of TPUx.TGRA: 
   * Initial output is low output; high output at compare match. 
   * TPUx.TIORH.BIT.IOB=5; 
   * TIOCAx function, output compare of TPUx.TGRB: 
   * Initial output is high output; low output at compare match
   */
  TPU0.TIORH.BIT.IOA=2; 
  TPU0.TIORH.BIT.IOB=5;
  
  TPU1.TIOR.BIT.IOA=2;
  TPU1.TIOR.BIT.IOB=5;
  
  TPU2.TIOR.BIT.IOA=2;
  TPU2.TIOR.BIT.IOB=5;
  
  TPU3.TIORH.BIT.IOA=2;
  TPU3.TIORH.BIT.IOB=5;

  // TPU0.TIER.BIT.TCIEU; // Cool interrupt stuff
  
  // 48Mhz/4/400 == 30000Hz
  /* Set a period */
  
#define length0 1
#define length1 0
#define length2 1
#define length3 0
  
#if 1 // we just play around now
  TPU3.TGRA = 100-length3;
  TPU3.TGRB = 100;

  TPU2.TGRA = 200-length2;
  TPU2.TGRB = 200;

  TPU1.TGRA = 300-length1;
  TPU1.TGRB = 300;

  TPU0.TGRA = 400-length0;
  TPU0.TGRB = 400;
#else // this will be in final code for safety
  TPU3.TGRA = 0;
  TPU3.TGRB = 0;

  TPU2.TGRA = 0;
  TPU2.TGRB = 0;

  TPU1.TGRA = 0;
  TPU1.TGRB = 0;

  TPU0.TGRA = 400;
  TPU0.TGRB = 400;
#endif
  
  SYSTEM.PRCR.WORD = 0xA503;
  TPUA.TSTR.BIT.CST0=1; // start counter
  TPUA.TSTR.BIT.CST1=1; // start counter
  TPUA.TSTR.BIT.CST2=1; // start counter
  TPUA.TSTR.BIT.CST3=1; // start counter
  /* Protection on */
  SYSTEM.PRCR.WORD = 0xA500;

  
  
  
  
  
  
  
  
  
  /*
   * Servo PWMs init
   */
  /* Configure pins of port E for peripheral function */

#if 0
  PORTA.PMR.BIT.B0 = 1;
  PORTA.PMR.BIT.B4 = 1;
  PORTA.PMR.BIT.B6 = 1;
  PORTB.PMR.BIT.B0 = 1;

  /* Configure pins of port E as an output */
  PORTA.PDR.BIT.B0 = 1;
  PORTA.PDR.BIT.B4 = 1;
  PORTA.PDR.BIT.B6 = 1;
  PORTB.PDR.BIT.B0 = 1;

  MPC.PWPR.BIT.B0WI=0; // Enable writing to PFSWE
  MPC.PWPR.BIT.PFSWE=1;// Enable writing to PFS registers

  /* Configure pins of ports for MTIOCxA outputs */
  MPC.PA0PFS.BIT.PSEL = 0x3;
  MPC.PA4PFS.BIT.PSEL = 0x3;
  MPC.PA6PFS.BIT.PSEL = 0x3;
  MPC.PB0PFS.BIT.PSEL = 0x3;
#endif

  MPC.PWPR.BIT.PFSWE=0;// Enable writing to PFS registers
  MPC.PWPR.BIT.B0WI=1; // Enable writing to PFSWE
  /* Protection off */
  SYSTEM.PRCR.WORD = 0xA503;

  TPUA.TSTR.BIT.CST0=0;// stop clock
  TPUA.TSTR.BIT.CST1=0;// stop clock
  TPUA.TSTR.BIT.CST2=0;// stop clock
  TPUA.TSTR.BIT.CST3=0;// stop clock
  
  /* Protection on */
  SYSTEM.PRCR.WORD = 0xA500;
  
  TPU4.TCR.BIT.TPSC=3; // input PCLK/64
  TPU5.TCR.BIT.TPSC=3; // --""--

  TPU4.TCR.BIT.CKEG=0; // Count only falling edges
  TPU5.TCR.BIT.CKEG=0; // --""--

  // page 757
  TPU4.TCR.BIT.CCLR=2; // TCNT counter cleared by TGRB compare match/input capture
  TPU5.TCR.BIT.CCLR=2; // TCNT counter cleared by TGRB compare match/input capture

  /* Read hardware manual chapters "24.3.5 PWM Modes" and 
   * "Table 24.24 PWM Output Registers and Output Pins" for poor explanation
   */
  TPU4.TMDR.BIT.MD = 2; // PWM mode 1  
  TPU5.TMDR.BIT.MD = 2; // PWM mode 1  
  
  /* 
   * In PWM mode 1 we use both TPUx.TGRA and TPUx.TGRB to trigger waveform 
   * of TIACAx pin, not TIACBx as you may think after reading hardware manual
   * about TGRB register function.
  
   * TPUx.TIORH.BIT.IOA=2; 
   * TIOCAx function, output compare of TPUx.TGRA: 
   * Initial output is low output; high output at compare match. 
   * TPUx.TIORH.BIT.IOB=5; 
   * TIOCAx function, output compare of TPUx.TGRB: 
   * Initial output is high output; low output at compare match
   */
  TPU4.TIORH.BIT.IOA=2; 
  TPU4.TIORH.BIT.IOB=5;
  
  TPU5.TIOR.BIT.IOA=2;
  TPU5.TIOR.BIT.IOB=5;
  
  
  /* 
   * http://en.wikipedia.org/wiki/Servo_control 
   * recommends using 20ms (50Hz) cycle.
   * 48Mhz/64/15000 == 50Hz
   * Same page says 1.5ms high pulse keeps servo at middle
   * 15000/20*1.5=1125
   */

  // 48Mhz/64/15000 == 50Hz
  /* Set a period */

  TPU4.TGRA = 15000-1125;
  TPU4.TGRB = 15000;

  TPU5.TGRA = 15000-1125;
  TPU5.TGRB = 15000;
  
  SYSTEM.PRCR.WORD = 0xA503;
  TPUA.TSTR.BIT.CST0=1; // start counter
  TPUA.TSTR.BIT.CST1=1; // start counter
  TPUA.TSTR.BIT.CST2=1; // start counter
  TPUA.TSTR.BIT.CST3=1; // start counter
  /* Protection on */
  SYSTEM.PRCR.WORD = 0xA500;
  
}

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
  /* Reset the duty cyccle to 10% */
  MTU3.TGRB = 4800;
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

  /* Start the TCNT counter */  
  MTU.TSTR.BIT.CST3 = 0x1;    
  /* Clear the interrupt flag */
  ICU.IR[IR_MTU3_TGIA3].BIT.IR = 0;

}

#pragma vector=VECT_MTU3_TGIB3
__interrupt void Excep_MTU3_TGIB3(void) {
  
  /* Clear the interrupt flag */
  ICU.IR[IR_MTU3_TGIB3].BIT.IR = 0;

}