
#include "intrinsics.h"
#include "rtc.h"
#include "pwm.h"
#include "oled.h"
#include "hwsetup.h"
//#include "async.h"
#include "usb_cdc_app.h"
#include "adc12repeat.h"
#include "low_voltage_detection.h"

int main() {
    HardwareSetup();
    //Init_VoltageDetect();
//__enable_interrupt();
//    Init_OLED();
//    Init_RTC();
    Init_OLED();
    Init_PWM();
//    Init_ADC12Repeat();
    //Init_UART();  
    USB_CDC_APP_Main();    
    return 0;
}

