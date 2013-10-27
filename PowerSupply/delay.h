
/* Defines RX630 port registers */
#include "intrinsics.h"

static inline void
uDelay(unsigned char l) {
    while (l--)
        __no_operation();

}

static inline void udelay(unsigned int usec) {
    // On 48 Mhz we do 48 000 000 cycles per second
    // or 48 cycles per microsecond
    __delay_cycles(48UL*(unsigned long)usec);
}