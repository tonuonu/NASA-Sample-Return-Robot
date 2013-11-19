/* Copyright (C) 2012 Renesas Electronics Corporation. All rights reserved.   */
/*******************************************************************************
* File Name    : ADC12repeat.c
* Version     : 1.00
* Device     : R5F5630E
* Tool Chain   : IAR Embedded Workbench
* H/W Platform  : RSKRX630
* Description   : Defines ADC repeat functions used for 12bit ADC unit.
*******************************************************************************/
/* Defines RX630 port registers */
#include "iorx630.h"
/* Defines macros relating to the RX630 user LEDs and switches */
#include "rskrx630def.h"
/* LCD controlling function prototypes & macro definitions */
#include "oled.h"
/* Defines switch functions and variables used in this file */
#include "switch.h"
/* LCD controlling function prototypes & macro definitions */
#include "oled.h"
/* Provides declarations of functions defined in this file */
#include "adc12repeat.h"

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* The variable gADC_Result is used to contain the value of the 12 bit ADC */
volatile uint16_t gADC_Result;

/* Declare a variable used to specify if the delay 
function should only generate the specific delay or not */
uint8_t gTimer_Mode_Flag = 0;
/* Dclare a variable to hold the periodic delay specified */
volatile uint32_t gPeriodic_Delay;
/* Declare a variable to store the global delay count value */
volatile uint32_t gDelay_Counter = 0;
/* Init_Timer function prototype */
static void Init_Timer(void);
/* 16 bit integer to character string function prototype declaration */
//static void uint16_ToString(uint8_t *, uint8_t, uint16_t);
/* Timer_Delay function prototype */
static void Timer_Delay(uint32_t, uint8_t, uint8_t);

/*******************************************************************************
* Outline     : Init_ADC12Repeat
* Description   : ADC initialisation function. This function configures the ADC
*          unit for continuous scan operation, then configures a timer  
*          for a periodic interrupt every 300 milliseconds.
* Argument    : none
* Return value  : none
*******************************************************************************/
void Init_ADC12Repeat(void)
{
  /* Initialise CMT2 */
  Init_Timer();
  
  /* Protection off */
  SYSTEM.PRCR.WORD = 0xA503;
  
  /* Cancel the S12AD module clock stop mode */
  MSTP_S12AD = 0;
  
  /* Protection on */
  SYSTEM.PRCR.WORD = 0xA500;
  
  /* Use the AN000 (Potentiometer) pin as an I/O for peripheral functions */
  PORT4.PMR.BYTE = 0x01;
  
  /* ADC clock = PCLK/8, continuous scan mode */
  S12AD.ADCSR.BYTE |= 0x40;
  
  /* Selects AN000..AN007 */
  S12AD.ADANS0.WORD = 0x00FF;  

  /* Set the sampling cycle to 255 states (approximately 5 us) */
  S12AD.ADSSTR01.WORD = 0x14FF;

  /* Start ADC */
  S12AD.ADCSR.BIT.ADST = 1;

  /* Configure a 10 ms periodic delay used 
     to update the ADC result on to the LCD */
  Timer_Delay(10, 'm', PERIODIC_MODE);
}
/*******************************************************************************
* End of function Init_ADC12Repeat
*******************************************************************************/

/*******************************************************************************
* Outline      : Init_Timer
* Description  : This function initialises the CMT channel 2.
* Argument     : none
* Return value : none
*******************************************************************************/
static void Init_Timer(void)
{
  /* Protection off */
  SYSTEM.PRCR.WORD = 0xA503;
  
  /* Cancel the CMT2 module clock stop mode */
  MSTP_CMT2 = 0;
  
  /* Protection on */
  SYSTEM.PRCR.WORD = 0xA500;

  /* Set CMT2 interrupt priority level to 5 */  
  IPR(CMT2,CMI2) = 0x8;
  /* Enable CMT2 interrupts */
  IEN(CMT2,CMI2) = 0x1;
  /* Clear CMT2 interrupt flag */
  IR(CMT2,CMI2) = 0x0;
  
}
/*******************************************************************************
* End of function Init_Timer
*******************************************************************************/

/*******************************************************************************
* Outline      : uint16_ToString
* Description  : Function converts a 16 bit integer into a character string, 
*         inserts it into the array via the pointer passed at execution.
* Argument     : * output_string : Pointer to uint8_t array that will hold 
*                   character string.
*               pos : uint8_t number, element number to begin 
*                   inserting the character string from (offset).
*          input_number : 16 bit integer to convert into a string.
* Return value : none
* Note       : No input validation is used, so output data can overflow the
*         array passed.
*******************************************************************************/
void uint16_ToString(uint8_t *output_string, uint8_t pos, 
      uint16_t input_number)
{
  /* Declare temporary character storage variable, and bit_shift variable */  
  uint8_t a = 0x00, bit_shift = 12u;
  
  /* Declare 16bit mask variable */
  uint16_t mask = 0xF000;
  
  /* Loop through until each hex digit is converted to an ASCII character */
  while(bit_shift < 30u)
  {
    /* Mask and shift the hex digit, and store in temporary variable, a */ 
    a = (uint8_t)((input_number & mask) >> bit_shift);
    
    /* Convert the hex digit into an ASCII character, and store in output
       string */
    output_string[pos] = (uint8_t)((a < 0x0A) ? (a + 0x30) : (a + 0x37));
    
    /* Shift the bit mask 4 bits to the right, to convert the next digit */
    mask = (uint16_t) (mask >> 4u);
    
    /* Decrement the bit_shift counter by 4 (bits in a each digit) */
    bit_shift -= 4u;
    
    /* Increment the output string location */
    pos++;
  }
}
/*******************************************************************************
* End of function uint16_ToString
*******************************************************************************/

/******************************************************************************
* Outline    : Timer_Delay
* Description  : Function used to create delays in milliseconds or 
*          microseconds depending on the user selection for controlling 
*          the debug LCD function's calls. 
* Argument      : uint32_t   -   _Delay_Period
*          uint8   -   Unit  
* Return value  : none
******************************************************************************/
static void Timer_Delay(uint32_t user_delay, uint8_t unit, uint8_t timer_mode)
{
  /* Clear the timer's count */
  gDelay_Counter = 0;
    
  /* Check if microseconds delay is required */
  if(unit == 'u')
  {  
    /* Select the PCLK clock division as PCLK/8 = 6MHz */ 
    CMT2.CMCR.BIT.CKS = 0x0;
    
    /* Store a copy of the user delay value to gPeriodic_Delay */
    gPeriodic_Delay = user_delay * 6;
  
    /* Specify the timer period */
    CMT2.CMCOR = gPeriodic_Delay;
  }
  
  /* Check if milliseconds delay is required */
  if(unit == 'm')
  {
    /* Select the PCLK clock division as PCLK/128 = 375KHz */ 
    CMT2.CMCR.BIT.CKS = 0x2;
    
    /* Stor a copy of the user delay value to gPeriodic_Delay */
    gPeriodic_Delay = user_delay * 375;
  
    /* Specify the timer period */
    CMT2.CMCOR = gPeriodic_Delay;
  }
  
  /* Enable the compare match interrupt */
  CMT2.CMCR.BIT.CMIE = 1;

  /* Start CMT2 count */
  CMT.CMSTR1.BIT.STR2 = 1;

  /* Skip the following instructions if a periodic timer is required */
  if((timer_mode != PERIODIC_MODE))
  {
    /* Wait for the timer to timeout */
    while((gDelay_Counter != gPeriodic_Delay))
    {
      /* Wait */
    }  

    /* Stop CMT2 count */
    CMT.CMSTR1.BIT.STR2 = 0;
  }      
}
/******************************************************************************
* End of function Timer_Delay
******************************************************************************/

/******************************************************************************
* Regular ADC interrupt. 
******************************************************************************/
volatile float adc[8] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

#pragma vector=VECT_CMT2_CMI2
__interrupt void Excep_CMTU1_CMT2(void)
{
    /* Declare temporary character string */
    // uint8_t lcd_buffer[9] = "=H\'WXYZ\0";
  
    /* Fetch 12 bit ADC value */
    // Resistor divider made from 33k and 100k 
    // Actual voltage is 133/33=4.03 times higher than ADC reading.
    // So when reference is 3.3V, maximum ADC reading is 133k/33k*3.3V=13.3V
    // Because we have 12bit ADC, formula for voltage is 13.3/(2^12-1)*reading
    // 13.3/(2^12)*4095=13.3V etc
#define VCOEFF ((133.0/33.0*3.3)/(4095.0))
  
  
    // ACS712 5A version outputs 185mV for each A
    // Center is at VCC/2, so 2.5V
    // Maximum current we can measure is (3.3-2.5)/0.185=4.32A
    // ACS711 can do 3.3V. ACS711EEXLT-15AB-T3 90 mv/A
    // Zero current point is 2.5/3.3/(2^12-1)*reading
    // 2.5/3.3*4095 = 3102 is middle point
    // For every volt 4095/3.3=1240.90909091 units
    // For every amper change is 4095/3.3*0.185=229.568181818
#define ICOEFF (4095.0/3.3*   0.185)
#define IZBASE (   2.5/3.3*4095.0  )

    adc[0] =  (float)S12AD.ADDR0*VCOEFF; // VBAT0_AD 
    adc[1] =  (float)S12AD.ADDR1*VCOEFF; // VBAT1_AD
    adc[2] =  (float)S12AD.ADDR2*VCOEFF; // VBAT2_AD
    adc[3] =  (float)S12AD.ADDR3*VCOEFF; // VBAT3_AD

    adc[4] = ((float)S12AD.ADDR4-IZBASE)/ICOEFF; // IBAT0_AD
    adc[5] = ((float)S12AD.ADDR5-IZBASE)/ICOEFF; // IBAT1_AD
    adc[6] = ((float)S12AD.ADDR6-IZBASE)/ICOEFF; // IBAT2_AD
    adc[7] = ((float)S12AD.ADDR7-IZBASE)/ICOEFF; // IBAT3_AD
    
    /* Convert ADC result into a character string, and store in the local
     string lcd_buffer */  
    
    /* Display the contents of the local string lcd_buffer */
    LED1 ^=1;
}
/******************************************************************************
* End of function Excep_CMTU1_CMT2
******************************************************************************/
