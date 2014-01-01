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

/* Defines RX630 port registers */
#include "intrinsics.h"

static inline void
uDelay(unsigned char l) {
    while (l--)
        __no_operation();

}
/*
static inline void udelay(const unsigned long usec) {
    // On 48 Mhz we do 48 000 000 cycles per second
    // or 48 cycles per microsecond
    __delay_cycles(48UL*(const unsigned long)usec);
}
*/