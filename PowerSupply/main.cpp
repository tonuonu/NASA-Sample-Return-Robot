
#include "intrinsics.h"
#include "rtc.h"
#include "pwm.h"
#include "oled.h"
#include "hwsetup.h"
//#include "async.h"
#include "usb_cdc_app.h"
#include "adc12repeat.h"

int main() {
    HardwareSetup();
    //__enable_interrupt();
//    Init_OLED();
    Init_RTC();
    Init_PWM();
    Init_ADC12Repeat();
    //Init_UART();  
    USB_CDC_APP_Main();    
    return 0;
}

