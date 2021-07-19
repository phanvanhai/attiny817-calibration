/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief
 *      Main routine and all functions
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
 * $RCSfile: calib_32kHz.c,v $
 * $Date: 2006/02/17 12:49:26 $  \n
 ******************************************************************************/


#include "calib_values.h"
#include <avr/cpufunc.h> 
#include <atmel_start.h>

//! Holds the number of neighbors searched
unsigned char neighborsSearched;
//! The binary search step size
unsigned char calStep;
//! The lowest difference between desired and measured counter value
unsigned char bestCountDiff = 0xFF;
//! Stores the lowest difference between desired and measured counter value for the first search
unsigned char bestCountDiff_first;
//! The OSCCALR value corresponding to the bestCountDiff
unsigned char bestOSCCAL;
//! Stores the OSCCALR value corresponding to the bestCountDiff for the first search
unsigned char bestOSCCAL_first;
//! The desired counter value
unsigned int countVal;
//! Calibration status
unsigned int calibration;
//! Stores the direction of the binary step (-1 or 1)
signed char sign;

//Functions used
void CalibrationInit(void);
void CalibrateInternalRc(void);
unsigned int Counter(void);
void BinarySearch(unsigned int ct);
void NeighborSearch(void);

/*! \brief Program entry point.
*
* Main initializes all subsystems, prepares and starts calibration
* according to the calibration method chosen, and the device specific
* oscillator characteristics. Ends in an eternal loop.
*
*/

unsigned char CalibInternalRc(void){
	CalibrationInit();                                            // Initiates calibration
	// Sets initial stepsize and sets calibration state to "running"
	calStep = INITIAL_STEP;
	calibration = RUNNING;
	CalibrateInternalRc();                                        // Calibrates to selected frequency
  
	return OSCCALR;  
}


/*! \brief Initializes the calibration.
*
* Computes the count value needed to compare the desired internal oscillator
* speed with the external watch crystal, and sets up the asynchronous timer.
*
*/
void CalibrationInit(void){
  // Computes countVal for use in the calibration
  countVal = ((EXTERNAL_TICKS*CALIBRATION_FREQUENCY)/(XTAL_FREQUENCY*LOOP_CYCLES));
  ccp_write_io((void*)&(OSCCALR), DEFAULT_OSCCAL);
  NOP();
}

/*! \brief Calibration function
*
* Performs the calibration according to calibration method chosen.
* Compares different calibration results in order to achieve optimal results.
*
*/
void CalibrateInternalRc(void){
  unsigned int count;

#ifdef CALIBRATION_METHOD_SIMPLE                                // Simple search method
  unsigned char cycles = 0x20;

  do{
    count = Counter();
    if (count > countVal)
      ccp_write_io((void*)&(OSCCALR),OSCCALR - 1);                                               // If count is more than count value corresponding to the given frequency
    NOP();                                                      // - decrease speed
    if (count < countVal)
      ccp_write_io((void*)&(OSCCALR),OSCCALR + 1);
    NOP();                                                      // If count is less: - increase speed
    if (count == countVal)
      cycles=1;			
  } while(--cycles);                                            // Calibrate using 32(0x20) calibration cycles

#else                                                           // Binary search with or without neighbor search
  unsigned char countDiff;
  unsigned char neighborSearchStatus = FINISHED;

  while(calibration == RUNNING){
    count = Counter();                                          // Counter returns the count value after external ticks on XTAL
    if (calStep != 0)
    {
      BinarySearch(count);                                      // Do binary search until stepsize is zero
    }
    else
    {
      if(neighborSearchStatus == RUNNING)
      {
        countDiff = ABS((signed int)count-(signed int)countVal);
        if (countDiff < bestCountDiff)                          // Store OSCCALR if higher accuracy is achieved
        {
          bestCountDiff = countDiff;
          bestOSCCAL = OSCCALR;
        }
        NeighborSearch();                                       // Do neighbor search
      }
      else                                                      // Prepare and start neighbor search
      {
#ifdef CALIBRATION_METHOD_BINARY_WITHOUT_NEIGHBOR               // No neighbor search if deselected
        calibration = FINISHED;
        countDiff = ABS((signed int)count-(signed int)countVal);
        bestCountDiff = countDiff;
        bestOSCCAL = OSCCALR;
#else
        neighborSearchStatus = RUNNING;                         // Do neighbor search by default
        neighborsSearched = 0;
        countDiff = ABS((signed int)count-(signed int)countVal);
        bestCountDiff = countDiff;
        bestOSCCAL = OSCCALR;
#endif
      }
    }
  }
#endif
}

/*! \brief The Counter function
*
* This function increments a counter for a given ammount of ticks on
* on the external watch crystal.
*
*/
unsigned int Counter(void){
  unsigned int cnt;

  cnt = 0;                                                      // Reset counter
  TIMER_COUNT = 0x00;                                                 // Reset async timer/counter
  while (STATUS_TIMER_REGISTER > 0); // Wait until async timer is updated  (Async Status reg. busy flags).
  do{                                                           // cnt++: Increment counter - the add immediate to word (ADIW) takes 2 cycles of code.
    cnt++;                                                      // Devices with async TCNT in I/0 space use 1 cycle reading, 2 for devices with async TCNT in extended I/O space
  } while (TIMER_COUNT < EXTERNAL_TICKS);                             // CPI takes 1 cycle, BRCS takes 2 cycles, resulting in: 2+1(or 2)+1+2=6(or 7) CPU cycles
  return cnt;                                                   // NB! Different compilers may give different CPU cycles!
}                                                               // Until 32.7KHz (XTAL FREQUENCY) * EXTERNAL TICKS

/*! \brief The binary search method
*
* This function uses the binary search method to find the
* correct OSSCAL value.
*
*/
void BinarySearch(unsigned int ct){

  if (ct > countVal)                                            // Check if count is larger than desired value
  {
    sign = -1;                                                  // Saves the direction
    ccp_write_io((void*)&(OSCCALR),OSCCALR - calStep);                                      // Decrease OSCCALR if count is too high
    NOP();
  }
  else if (ct < countVal)                                       // Opposite procedure for lower value
  {
    sign = 1;
    ccp_write_io((void*)&(OSCCALR),OSCCALR + calStep);
    NOP();
  }
  else                                                          // Perfect match, OSCCALR stays unchanged
  {
    calibration = FINISHED;
  }
  calStep >>= 1;
}

/*! \brief The neighbor search method
*
* This function uses the neighbor search method to improve
* binary search result. Will always be called with a binary search
* prior to it.
*
*/
void NeighborSearch(void){

  neighborsSearched++;
  if (neighborsSearched == 4)                                   // Finish if 3 neighbors searched
  {
	 ccp_write_io((void*)&(OSCCALR), bestOSCCAL);
    calibration = FINISHED;
  }
  else
  {
	ccp_write_io((void*)&(OSCCALR),OSCCALR + sign);
    NOP();
  }
}


/*! \mainpage
 * \section Intro Introduction
 * This documents data structures, functions, variables, defines, enums, and
 * typedefs in the software for application note AVR055.
 *
 * \section CI Compilation Info
 * This software was written for the IAR Embedded Workbench 4.11A.
 *
 * To make project:
 * <ol>
 * <li> Add the file calib_32kHz.c to project.
 * <li> Under processor configuration, select any device listed in the device_specific.h file.
 * <li> Memory size: Small
 * <li> Enable bit definitions in I/O include files
 * <li> High optimization on size is recommended on release target for lower code size.
 * <li> Output format: ubrof 8 (forced) for debug and intel-extended for release target.
 * </ol>
 *
 * \section DI Device Info
 * The included source code is written for all devices with with a tunable
 * internal oscillator and a timer with asynchronous operation mode. These
 * devices are listed in the device_specific.h file. \n
 * Note that when calibrating the ATCAN90 device using the STK501 top module,
 * a 32kHz crystal other than the one on the STK501 should be used in order to
 * achieve maximum accuracy. Also note that the ATmega169 and ATmega169P has
 * different oscillators, and when calibrating  the latter, modify the
 * ATmega169 device specific defines in the device_specific.h file.
 *
 * \section TDL ToDo List
 * \todo Change \ref CALIBRATION_FREQUENCY to desired frequency.
 * \todo Define either CALIBRATION_METHOD_BINARY_WITHOUT_NEIGHBOR or
 * CALIBRATION_METHOD_SIMPLE in the calib_values.h file if a other search methods than
 * the default search method with neighbor search is desired.
 * \todo If a crystal running at a different frequency than 32kHz is used,
 * modify \ref XTAL_FREQUENCY accordingly. This is not recommended.
 */

