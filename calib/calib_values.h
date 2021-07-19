/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief
 *      Configuration and macros
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
 * $RCSfile: calib_values.h,v $
 * $Date: 2006/02/17 12:49:26 $  \n
 ******************************************************************************/


#ifndef __CALIB_VALUES_H__
#define __CALIB_VALUES_H__

#include "Config/clock_config.h"
#include <util/delay.h>

/*! Calibration methods, Binary search WITH Neighborsearch is default method
 * Uncomment to use ONE of the two following methods instead:
 */
//#define CALIBRATION_METHOD_BINARY_WITHOUT_NEIGHBOR
//#define CALIBRATION_METHOD_SIMPLE

/*! Modify CALIBRATION_FREQUENCY to desired calibration frequency
 */

#include "Config/clock_config.h"
#define CALIBRATION_FREQUENCY F_CPU

/*! Frequency of the external oscillator. A 32kHz crystal is recommended
 */
#define XTAL_FREQUENCY 32768
#define EXTERNAL_TICKS 100               // ticks on XTAL. Modify to increase/decrease accuracy

/*! \brief Fixed calibration values and macros
 *
 * These values are fixed and used by all calibration methods. Not to be modified.
 *
 */
#define FALSE 0
#define TRUE 1
#define RUNNING 0
#define FINISHED 1
#define DEFAULT_OSCCAL_MASK        0xC0  // Lower half and

#define OSCCALR							CLKCTRL.OSC20MCALIBA
#define STATUS_TIMER_REGISTER			RTC.STATUS
#define TIMER_COUNT                     RTC.CNT
#define OSCCAL_RESOLUTION                  6
#define LOOP_CYCLES                       12

#define INITIAL_STEP         (1 << (OSCCAL_RESOLUTION - 2))
#define DEFAULT_OSCCAL      ((1 << (OSCCAL_RESOLUTION - 1)) | DEFAULT_OSCCAL_MASK)

#define NOP() _delay(CALIBRATION_FREQUENCY/10000000)
//#define NOP() _NOP()

/*
#define NOP() _NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP();\
_NOP()
*/

// Absolute value macro.
#define ABS(var) (((var) < 0) ? -(var) : (var));

unsigned char CalibInternalRc(void);

#endif

