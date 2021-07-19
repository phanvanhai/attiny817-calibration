#include <atmel_start.h>
#include <avr/cpufunc.h>
#include "calib_values.h"
#include <util/delay.h>

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	CalibInternalRc();
	NOP();
	NOP();
	NOP();
	NOP();
	/* Replace with your application code */
	while (1) {
	}
}
