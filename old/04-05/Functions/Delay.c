/*
 * Delay.c
 *
 * Created: 04/05/2019 12:10:42
 *  Author: Margreet
 */ 

#include "Delay.h"

void var_delay_ms(int mills) {
	delay_ms(mills * (FOSC0/AVR32_PM_RCOSC_FREQUENCY));
}