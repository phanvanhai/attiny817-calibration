/*
 * calibRC.h
 *
 * Created: 7/20/2021 11:30:08 AM
 *  Author: PhanHai
 */ 


#ifndef CALIBRC_H_
#define CALIBRC_H_

#include "Config/clock_config.h"

/*! Calibration methods, Binary search WITH Neighborsearch is default method
 * Uncomment to use ONE of the two following methods instead:
 */
//#define CALIBRATION_METHOD_BINARY_WITHOUT_NEIGHBOR
//#define CALIBRATION_METHOD_SIMPLE

#define CALIBRATION_FREQUENCY F_CPU
#define XTAL_FREQUENCY 32768				// Frequency of the external oscillator. A 32kHz crystal is recommended
#define EXTERNAL_TICKS 100					// ticks on XTAL. Modify to increase/decrease accuracy

#define FALSE 0
#define TRUE 1
#define RUNNING 0
#define FINISHED 1

/*
Depends on device type, see the datasheet for suitable selection
*/
#define DEFAULT_OSCCAL_MASK				0xC0
#define OSCCALR							CLKCTRL.OSC20MCALIBA
#define STATUS_TIMER_REGISTER			RTC.STATUS
#define TIMER_COUNT                     RTC.CNT
#define OSCCAL_RESOLUTION                  6
#define LOOP_CYCLES                       12

#define INITIAL_STEP         (1 << (OSCCAL_RESOLUTION - 2))
#define DEFAULT_OSCCAL      ((1 << (OSCCAL_RESOLUTION - 1)) | DEFAULT_OSCCAL_MASK)

#define NOP() _delay_5us()				// Time for the oscillation to stabilize every time it changes

// Absolute value macro.
#define ABS(var) (((var) < 0) ? -(var) : (var));

unsigned char CalibInternalRc(void);


#endif /* CALIBRC_H_ */