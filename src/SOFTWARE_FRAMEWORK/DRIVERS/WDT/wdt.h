/* This header file is part of the ATMEL AVR-UC3-SoftwareFramework-1.7.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief WDT driver for AVR32 UC3.
 *
 * This file contains definitions and services for the AVR32 WatchDog Timer.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with a WDT module can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ******************************************************************************/

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 */

#ifndef _WDT_H_
#define _WDT_H_

/*! \brief Gets the time-out period of the WatchDog Timer in microseconds.
 *
 * \retval <0   The WatchDog Timer is disabled.
 * \retval >=0  Active time-out period of the WatchDog Timer in microseconds.
 */
extern long long wdt_get_us_timeout_period(void);

/*! \brief Disables the WatchDog Timer.
 */
extern void wdt_disable(void);

/*! \brief Enables the WatchDog Timer with the \a us_timeout_period time-out
 *         period saturated to the supported range and rounded up to the nearest
 *         supported greater time-out period.
 *
 * \param us_timeout_period Time-out period to configure in microseconds.
 *
 * \return Actually configured time-out period in microseconds.
 */
extern unsigned long long wdt_enable(unsigned long long us_timeout_period);

/*! \brief Re-enables the WatchDog Timer with the last time-out period
 *         configured.
 */
extern void wdt_reenable(void);

/*! \brief Clears the WatchDog Timer.
 */
extern void wdt_clear(void);

/*! \brief Resets the MCU with the WatchDog Timer as fast as possible.
 */
extern void wdt_reset_mcu(void);


#endif  // _WDT_H_
