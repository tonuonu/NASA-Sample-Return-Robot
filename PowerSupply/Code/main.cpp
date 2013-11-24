
#include "intrinsics.h"
#include "rtc.h"
#include "pwm.h"
#include "oled.h"
#include "hwsetup.h"
//#include "async.h"
#include "usb_hal.h"
#include "usb_cdc.h"
#include "usb_cdc_app.h"
#include "adc12repeat.h"
#include "low_voltage_detection.h"


int main() {
    HardwareSetup();
    __enable_interrupt();
    Init_OLED(); // Make sure noone else calls OLED calls in interrupts before init
    Init_PWM();
    // Init_VoltageDetect();
    Init_ADC12Repeat();
    Init_RTC();
    // Init_UART();  
#if 1
    USBCDC_Init();  
#endif

    __delay_cycles(96UL*2000UL); // 2000us delay    
    OLED_Fill_RAM(0x00);				   // Clear Screen
    OLED_Show_String(  1, "Battery statuses", 0, 0*8);

    while(1) {
        __wait_for_interrupt();
    }

}

