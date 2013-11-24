#include <stdint.h>
#include <stdio.h>

/* Defines RX630 port registers */
#include "iorx630.h"
/* Defines macros relating to the RX630 user LEDs and switches */
#include "rskrx630def.h"
/* OLED controlling function prototypes & macro definitions */
#include "oled.h"
/* Provides declarations of functions defined in this file */
#include "rtc.h"

/* Variable used to store the string to be displayed on the OLED */
uint8_t oled_buffer[9];
/* Variable used to hold current time */
uint32_t time_data = 0x0;
/* Variable used to hold current time */
uint32_t date_data = 0x0;

/* Integer to string function prototype declaration */
//static void uint32_ToBCDString(uint8_t *, uint8_t, uint32_t);

/*******************************************************************************
* Outline     : Init_RTC
* Description   : The RTC initialisation function. This function presets the 
*          time & date of the RTC. It configures the RTC to generate 
*          an interrupt every second. The alarm is also configured to 
*          trigger at a specified time.  
*******************************************************************************/
void Init_RTC(void)
{  
  /* Protection off */
  SYSTEM.PRCR.WORD = 0xA503;    
    
  /* Check if the MCU has come from a cold start (power on reset) */
  if(0 == SYSTEM.RSTSR1.BIT.CWSF)
  {  
    /* Set the warm start flag */
    SYSTEM.RSTSR1.BIT.CWSF = 1;
    
    /* Indicate waiting for subclock to stablise on OLED */
    //Display_OLED(OLED_LINE1, "Init Clk");    
      
    /* Disable the sub-clock oscillator */
    SYSTEM.SOSCCR.BIT.SOSTP = 1;
    
    /* Wait for register modification to complete */
    while(1 != SYSTEM.SOSCCR.BIT.SOSTP);
    
    /* Disable the input from the sub-clock */
    RTC.RCR3.BYTE = 0x0C;  
    
    /* Wait for the register modification to complete */
    while(0 != RTC.RCR3.BIT.RTCEN);    
    
    /* Wait for at least 5 cycles of sub-clock */
    volatile uint32_t wait = 0x6000;
    while(wait--);
    
    /* Start sub-clock */
    SYSTEM.SOSCCR.BIT.SOSTP = 0;
    
    /* Declare progress bar string */
    //uint8_t progress[] = "        ";    
    
    /* Perform 8 delay iterations */
    for(uint8_t i = 0; i < 8; i++)
    {
      /* Wait in while loop for ~0.5 seconds */
      wait = 0x2FFFFE;
      while(wait--);
      
      /* Add block character to progress bar string */
      //progress[i] = 0xFF;
          
      /* Update progress bar string */
      //Display_OLED(OLED_LINE2, progress);
    }
  }
  else
  {
    /* Start sub-clock */
    SYSTEM.SOSCCR.BIT.SOSTP = 0;  
    
    /* Wait for the register modification to complete */
    while(0 != SYSTEM.SOSCCR.BIT.SOSTP);
  }  
    
  /* Set RTC clock input from sub-clock, and supply to RTC module */
  RTC.RCR4.BIT.RCKSEL = 0;  
  RTC.RCR3.BIT.RTCEN = 1;  
  
  /* Wait for at least 5 cycles of sub-clock */
  volatile uint32_t wait = 0x6000;
  while(wait--);
  
  /* It is now safe to set the RTC registers */
  
  /* Write 0 to RTC start bit */
  RTC.RCR2.BIT.START = 0x0;
    
  /* Wait for start bit to clear */
  while(0 != RTC.RCR2.BIT.START);
        
  /* Reset the RTC unit */
  RTC.RCR2.BIT.RESET = 1;
  
  /* Wait until reset is complete */
  while(RTC.RCR2.BIT.RESET);

  /* Write 0 to RTC start bit */
  RTC.RCR2.BIT.START = 0x0;
    
  /* Wait for start bit to clear */
  while(0 != RTC.RCR2.BIT.START);
  
  /* Operate RTC in 24-hr mode */
  RTC.RCR2.BIT.HR24 = 0x1;
  
  RTC.RADJ.BIT.PMADJ=1; // Addition
  RTC.RADJ.BIT.ADJ=1; // This is added once per minute to clock. 5...0 bits.
  RTC.RCR2.BIT.AADJP=1;
  RTC.RCR2.BIT.AADJE=1;
#if 0
  /* Configure the clock as follows - 
    Initial time - 11:59:30   */
  RTC.RSECCNT.BYTE = 0x00;
  RTC.RMINCNT.BYTE = 0x31;
  RTC.RHRCNT.BYTE = 0x18;
  
  /* Configure the date as follows -
    Initial date - 21/11/2011  */
  RTC.RDAYCNT.BYTE = 0x21;
  RTC.RMONCNT.BYTE = 0x11;
  RTC.RYRCNT.WORD = 0x0013;
  RTC.RWKCNT.BYTE = 0x04; // Thursday
#endif  
  /* Configure the alarm as follows -
    Alarm time - 12:00:00
    Enable the hour, minutes and seconds alarm */
  RTC.RSECAR.BYTE = 0x80;
  RTC.RMINAR.BYTE = 0x80;
  RTC.RHRAR.BYTE = 0xD2;
  
  /* Alarm enable bits are undefined after a reset,
     disable non-required alarm features */
  RTC.RWKAR.BIT.ENB = 0;
  RTC.RDAYAR.BIT.ENB = 0;
  RTC.RMONAR.BIT.ENB = 0;
  RTC.RYRAREN.BIT.ENB = 0;
  
  /* Enable alarm and periodic interrupts, 
  generate periodic interrupts every 1 second */  
  RTC.RCR1.BYTE = 0xE5;
  
  /* Verify RCR1 register write */
  while(0xE5 != RTC.RCR1.BYTE);
#if 1 
  /* Enable RTC Alarm interrupts */  
  ICU.IPR[IPR_RTC_ALM].BYTE = 0x0A;
  ICU.IER[IER_RTC_ALM].BIT.IEN4 = 1;
  ICU.IR[IR_RTC_ALM].BIT.IR = 0;
    
  /* Enable RTC Periodic interrupts */  
  ICU.IPR[IPR_RTC_PRD].BYTE =  0x08 + 0x02  ; // 16 times in second
  /*
    b7 to b4 PES[3:0] Periodic Interrupt
    0 1 1 0: A periodic interrupt is generated every 1/256 second.
    (However, when the main clock is selected (RCR4.RCKSEL = 1) while
    PES[3:0] = 0110b, a periodic interrupt is generated every 1/128
    second.)
    0 1 1 1: A periodic interrupt is generated every 1/128 second.
    1 0 0 0: A periodic interrupt is generated every 1/64 second.
    1 0 0 1: A periodic interrupt is generated every 1/32 second.
    1 0 1 0: A periodic interrupt is generated every 1/16 second.
    1 0 1 1: A periodic interrupt is generated every 1/8 second.
    1 1 0 0: A periodic interrupt is generated every 1/4 second.
    1 1 0 1: A periodic interrupt is generated every 1/2 second.
    1 1 1 0: A periodic interrupt is generated every 1 second.
    1 1 1 1: A periodic interrupt is generated every 2 seconds.
    Other than above, no periodic interrupts are generated.
  */
  ICU.IER[IER_RTC_PRD].BIT.IEN5 = 1;
  ICU.IR[IR_RTC_PRD].BIT.IR = 0;
#endif 
  /* Start the clock */
  RTC.RCR2.BIT.START = 0x1;
  
  /* Wait until the start bit is set to 1 */
  while(1 != RTC.RCR2.BIT.START);
  
  /* Set LED1 port pin to output */
  LED1_PORT_DIR = 1;    
}

/*******************************************************************************
* Outline     : Excep_RTC_ALARM
* Description   : RTC alarm interrupt function generated when the time set in 
*          the alarm registers matches with the current RTC time. 
*******************************************************************************/
#pragma vector=VECT_RTC_ALM
__interrupt void Excep_RTC_ALM(void)
{
  /* Turn on LED1 to indicate that the alarm is triggered  */
  LED1 = LED_ON;

  /* Clear the interrupt flag */
  ICU.IR[IR_RTC_ALM].BIT.IR = 0;
}
extern volatile float adc[8];

/*******************************************************************************
* Outline     : CB_1HZ_RTC
* Description   : RTC periodic interrupt handler generated every 1 sec. It is 
*          used to update the time on the debug OLED. It read the RTC 
*          register values and converts them into a string before 
*          displaying on the OLED.
* Argument    : none
* Return value  : none
*******************************************************************************/
#pragma vector=VECT_RTC_PRD
__interrupt void Excep_RTC_SLEEP(void) {
    /* Read the time and status flags */
    /* Read the seconds count register */
    time_data  = (uint32_t)(RTC.RSECCNT.BYTE & 0x0000007F);
    /* Read the minutes count register */
    time_data |= (RTC.RMINCNT.BYTE & 0x0000007F) << 8;
    /* Read the hours count register */
    time_data |= (RTC.RHRCNT.BYTE  & 0x0000003F) << 16;
  
    /* Convert the data to string & display on the OLED */
    // uint32_ToBCDString(oled_buffer, 0, time_data);
    /* Update time on the debug OLED */
    // OLED_Show_String(  1, (char*)oled_buffer, 0, 6*8);

    char buf[65];
    char *dow;
    switch(RTC.RWKCNT.BYTE & 0x07) {
    case 0:
        dow="Sun";
         break;
    case 1:
        dow="Mon";
         break;
    case 2:
        dow="Tue";
         break;
    case 3:
        dow="Wed";
         break;
    case 4:
        dow="Thu";
         break;
    case 5:
        dow="Fri";
         break;
    case 6:
        dow="Sat";
         break;
    }
    
    snprintf(buf,sizeof(buf),"%3s %2x/%02x/20%02x %2x:%02x:%02x",
                 dow,
                 RTC.RDAYCNT.BYTE,
                 RTC.RMONCNT.BYTE,
                 RTC.RYRCNT.WORD,
                 RTC.RHRCNT.BYTE & 0x3F ,
                 RTC.RMINCNT.BYTE & 0x7F,
                 RTC.RSECCNT.BYTE & 0x7F
    );
    OLED_Show_String(  1,buf, 0, 7*8);
#define BAT_MISSING_THRESHOLD (2.5f*3.f) 
#define BAT_CRIT_THRESHOLD (3.3f*3.f) 
#define BAT_LOW_THRESHOLD  (3.5f*3.f)
#define BAT_FULL_THRESHOLD (4.2f*3.f)
    
    for(int i=0;i<4;i++) {
        char *statustext;
        float percent;
        percent=(adc[i]-BAT_CRIT_THRESHOLD)/(BAT_FULL_THRESHOLD-BAT_CRIT_THRESHOLD)*100.0f;
        if(adc[i]<=BAT_MISSING_THRESHOLD) {
            statustext="Missing";
            percent=0.0f;
        } else if(adc[i]<=BAT_CRIT_THRESHOLD) {
            statustext="Critical";
            percent=0.0f;
        } else if(adc[i]<=BAT_LOW_THRESHOLD) {
            statustext="Low";
        } else if(adc[i]>BAT_FULL_THRESHOLD) {
            statustext="Overvoltage";
            percent=0.0f;
        } else {
            statustext="Normal";
        }
        snprintf(buf,sizeof(buf),"%d: %4.1fV %6.2fA %11s %3.0f",i,adc[i],adc[i+4],statustext,percent);
        OLED_Show_String(  1, buf, 0, (i+1)*8);

    }
}

/*******************************************************************************
* Outline      : uint32_ToBCDString
* Description  : This function converts an 16bit integer into a BCD string,
*         which is inserted into the character array passed as an 
*         argument.
* Argument     : uint8_t * string  - Pointer to array to store BCD string 
*         uint8_t pos    - Array element to start BCD string from.
*         uint32_t number  - 32bit integer to convert.
* Return value : none
* Note       : No input validation is used, so output data can overflow the
*         array passed.
*******************************************************************************/
#if 0
static void uint32_ToBCDString(uint8_t * output_string, uint8_t pos, 
                 uint32_t number)
{
  /* Local digit position variable */
  uint8_t disp_ptr = 0;
  uint8_t digit = 6u;
  
  /* Local string buffer variable */
  uint8_t buffer_string[6] = {0,0,0,0,0,0};
  
  /* Decrement digit each iteration, until digit is zero */
  while(digit--)
  {
    /* Write BCD value to string element */
    buffer_string[digit] = (uint8_t) (0x30 + (number % 16));
    
    /* Move to the next decimal digit */
    number >>= 4;
  }
  
  /* Set digit position to zero */
  digit = 0u;
  
  /* Loop through each digit, and replace leading zeros with a space
     character */
  while(disp_ptr < 6u)
  {
    /* Copy digit from buffer string to output string */
    output_string[digit + pos] = buffer_string[disp_ptr];
      
    /* Increment the loop counter */
    digit++;
    disp_ptr++;
    
    /* Check the digit position. */
    if((digit == 5)||(digit == 2))
    {
      /* Add colon between hours and minutes 
         and between minutes & seconds. (hh:mm:ss) */
      output_string[digit + pos] = ':';
      
      /* Increment the loop counter */
      digit++;
    }
  }
}
#endif