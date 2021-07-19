/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief
 *      Device specific definitions
 *
 * \par Application note:
 *      AVR055: Using a 32 kHz crystal to calibrate the internal RC oscillator
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Name: RELEASE_1_1 $
 * $Revision: 1.2 $
 * $RCSfile: device_specific.h,v $
 * $Date: 2006/02/17 12:49:26 $  \n
 ******************************************************************************/


#ifndef __DEVICE_SPECIFIC_H__
#define __DEVICE_SPECIFIC_H__

#define __ATtiny814__
/*! \brief Device specific register names
 *
 * This file conatins all device specific defifinitions for
 * all supported parts used by the compiler
 *
 */

#if defined(__ATtiny814__) | defined(__ATtiny814__)
#define OSCCALR							OSC20MCALIBA
#define ASYNC_TIMER                        AS0
#define NO_PRESCALING                      CS20
#define ASYNC_TIMER_CONTROL_REGISTER       TCCR2
#define ASYNC_TIMER_CONTROL_UPDATE_BUSY    TCR2UB
#define OUTPUT_COMPARE_UPDATE_BUSY         OCR2UB
#define TIMER_UPDATE_BUSY                  TCN2UB
#define TIMER                              TCNT2
#define OSCCAL_RESOLUTION                  5
#define LOOP_CYCLES                        6
#endif


#if defined(__ATmega64__) | defined(__ATmega128__)
#define ASYNC_TIMER                        AS0
#define NO_PRESCALING                      CS00
#define ASYNC_TIMER_CONTROL_REGISTER       TCCR0
#define ASYNC_TIMER_CONTROL_UPDATE_BUSY    TCR0UB
#define OUTPUT_COMPARE_UPDATE_BUSY         OCR0UB
#define TIMER_UPDATE_BUSY                  TCN0UB
#define TIMER                              TCNT0
#define OSCCAL_RESOLUTION                  8
#define LOOP_CYCLES                        6
#endif

#endif

