/*
 * calibRC.c
 *
 * Created: 7/20/2021 11:30:24 AM
 *  Author: PhanHai
 */ 

#include "calibRC.h"
#include <avr/cpufunc.h>
#include <atmel_start.h>

//! Holds the number of neighbors searched
unsigned char neighborsSearched;
//! The binary search step size
unsigned char calStep;
//! The lowest difference between desired and measured counter value
unsigned int bestCountDiff = 0xFFFF;
//! The OSCCALR value corresponding to the bestCountDiff
unsigned char bestOSCCAL;
//! The desired counter value
unsigned int countVal;
//! Calibration status
unsigned int calibration;
//! Stores the direction of the binary step (-1 or 1)
signed char sign;

//Functions used
void CalibrateInternalRc(void);
unsigned int Counter(void);
void BinarySearch(unsigned int ct);
void NeighborSearch(void);
void _delay_5us(void);

/*! \brief Program entry point.
*
* Main initializes all subsystems, prepares and starts calibration
* according to the calibration method chosen, and the device specific
* oscillator characteristics. Ends in an eternal loop.
*
*/

unsigned char CalibInternalRc(void){
	// Sets initial stepsize and sets calibration state to "running"
	calStep = INITIAL_STEP;
	calibration = RUNNING;
	countVal = ((EXTERNAL_TICKS * CALIBRATION_FREQUENCY) / (XTAL_FREQUENCY * LOOP_CYCLES));
	
	while (STATUS_TIMER_REGISTER > 0);					// Wait until async timer is updated  (Async Status reg. busy flags).
	
	ccp_write_io((void*)&(OSCCALR), DEFAULT_OSCCAL);	// Initiates calibration
	NOP();                                           
	
	CalibrateInternalRc();								// Calibrates to selected frequency
	
	return OSCCALR;
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
	unsigned char cycles = (1 << OSCCAL_RESOLUTION);			// = 1/2 calib value range

	do{
		count = Counter();
		if (count > countVal)										// If count is more than count value corresponding to the given frequency
		{
			ccp_write_io((void*)&(OSCCALR), OSCCALR - 1);           // decrease speed
			NOP();
		}
		if (count < countVal)
		{
			ccp_write_io((void*)&(OSCCALR), OSCCALR + 1);			// If count is less: increase speed
			NOP();
		}
		
		if (count == countVal) cycles = 1;
	} while(--cycles);												// Calibrate using 32 (0x20) calibration cycles
	
#else																// Binary search with or without neighbor search
	unsigned int countDiff;
	unsigned char neighborSearchStatus = FINISHED;

	while(calibration == RUNNING){
		count = Counter();                                          // Counter returns the count value after external ticks on XTAL
		if (calStep != 0)
		{
			BinarySearch(count);									// Do binary search until stepsize is zero
		}
		else
		{
			if(neighborSearchStatus == RUNNING)
			{
				countDiff = ABS((signed int)count-(signed int)countVal);
				if (countDiff < bestCountDiff)						// Store OSCCALR if higher accuracy is achieved
				{
					bestCountDiff = countDiff;
					bestOSCCAL = OSCCALR;
				}
				NeighborSearch();									// Do neighbor search
			}
			else													// Prepare and start neighbor search
			{
				#ifdef CALIBRATION_METHOD_BINARY_WITHOUT_NEIGHBOR	// No neighbor search if deselected
					calibration = FINISHED;
					countDiff = ABS((signed int)count-(signed int)countVal);
					bestCountDiff = countDiff;
					bestOSCCAL = OSCCALR;
				#else
					neighborSearchStatus = RUNNING;					// Do neighbor search by default
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

	cnt = 0;													// Reset counter
	TIMER_COUNT = 0x00;											// Reset async timer/counter
	while (STATUS_TIMER_REGISTER > 0);							// Wait until async timer is updated  (Async Status reg. busy flags).
	do{
		cnt++;													
	} while (TIMER_COUNT < EXTERNAL_TICKS);						// Until 32.7KHz (XTAL FREQUENCY) * EXTERNAL TICKS
	
	/* caculate number of CPU clocks:
	cnt++;
		0000004E  ADIW R24,0x01		Add immediate to word				// 2 clocks
	
	while (TIMER_COUNT < EXTERNAL_TICKS);
		0000004F  LDS R18,0x0148		Load direct from data space		// 3 clocks
		00000051  LDS R19,0x0149		Load direct from data space		// 3 clocks
		00000053  CPI R18,0x64		Compare with immediate				// 1 clock
		00000054  CPC R19,R1		Compare with carry					// 1 clock
		00000055  BRCS PC-0x07		Branch if carry set					// 1 or 2
	=================================================================	= 2 + 3 + 3 + 1 + 1 + 2 = 12 clocks
	*/
	
	return cnt;
}                                                               

/*! \brief The binary search method
*
* This function uses the binary search method to find the
* correct OSSCAL value.
*
*/
void BinarySearch(unsigned int ct){

	if (ct > countVal)											// Check if count is larger than desired value
	{
		sign = -1;												// Saves the direction
		ccp_write_io((void*)&(OSCCALR),OSCCALR - calStep);		// Decrease OSCCALR if count is too high
		NOP();
	}
	else if (ct < countVal)										// Opposite procedure for lower value
	{
		sign = 1;
		ccp_write_io((void*)&(OSCCALR),OSCCALR + calStep);
		NOP();
	}
	else														// Perfect match, OSCCALR stays unchanged
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
	if (neighborsSearched == 4)										// Finish if 3 neighbors searched
	{
		ccp_write_io((void*)&(OSCCALR), bestOSCCAL);
		NOP();
		calibration = FINISHED;
	}
	else
	{
		ccp_write_io((void*)&(OSCCALR),OSCCALR + sign);
		NOP();
	}
}

void _delay_5us(void)
{
	unsigned char t = 5 * (CALIBRATION_FREQUENCY / 1000000);
	while(t--);
}