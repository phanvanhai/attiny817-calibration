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

void _delay(uint8_t t)
{
	while(t--);
}

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
    if (count > countVal)										// If count is more than count value corresponding to the given frequency 
	{
		ccp_write_io((void*)&(OSCCALR), OSCCALR - 1);           // - decrease speed
		NOP();
	}                                                         
    if (count < countVal)
	{
		 ccp_write_io((void*)&(OSCCALR), OSCCALR + 1);			// If count is less: - increase speed
		 NOP();
	}
     
    if (count == countVal) cycles = 1;			
  } while(--cycles);                                            // Calibrate using 32 (0x20) calibration cycles

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
  TIMER_COUNT = 0x00;                                           // Reset async timer/counter
  while (STATUS_TIMER_REGISTER > 0); // Wait until async timer is updated  (Async Status reg. busy flags).
  do{                                                           // cnt++: Increment counter - the add immediate to word (ADIW) takes 2 cycles of code.
    cnt++;                                                      // Devices with async TCNT in I/0 space use 1 cycle reading, 2 for devices with async TCNT in extended I/O space
  } while (TIMER_COUNT < EXTERNAL_TICKS);                             // CPI takes 1 cycle, BRCS takes 2 cycles, resulting in: 2+1(or 2)+1+2=6(or 7) CPU cycles
  return cnt;                                                   // NB! Different compilers may give different CPU cycles!
}                                                               // Until 32.7KHz (XTAL FREQUENCY) * EXTERNAL TICKS

/*
cnt++;                                             
0000004E  ADIW R24,0x01		Add immediate to word				// 2
} while (TIMER_COUNT < EXTERNAL_TICKS);
0000004F  LDS R18,0x0148		Load direct from data space		// 3
00000051  LDS R19,0x0149		Load direct from data space		// 3
00000053  CPI R18,0x64		Compare with immediate				// 1
00000054  CPC R19,R1		Compare with carry					// 1
00000055  BRCS PC-0x07		Branch if carry set					// 1 or 2
*/

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
	//NOP();
	calibration = FINISHED;
  }
  else
  {
	ccp_write_io((void*)&(OSCCALR),OSCCALR + sign);
    NOP();
  }
}
