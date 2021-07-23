#include <atmel_start.h>
#include <avr/cpufunc.h>
#include "calibRC.h"
#include <util/delay.h>

signed char result = 0;

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	InitCalibRc();
	_delay_ms(10);
	//_NOP();
	//CalibInternalRc();
	//_NOP();
	/* Replace with your application code */
	while (1) {		
		result = CalibInternalRc();
		_NOP();
		_delay_ms(5000);		
	}
}
