#include <atmel_start.h>
#include <avr/cpufunc.h>
#include "calibRC.h"
#include <util/delay.h>

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	//_delay_ms(10);
	//_NOP();
	//CalibInternalRc();
	//_NOP();
	/* Replace with your application code */
	while (1) {
		_delay_ms(10000);
		CalibInternalRc();
	}
}
