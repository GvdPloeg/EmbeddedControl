/*
 * Alarm.c
 *
 * Created: 04/05/2019 14:09:43
 *  Author: Margreet
 */ 

#include "Alarm.h"

void Schakelmoment(void){
	if(time_RTC == time_uitgang[1]){ print_dbg("Alarm 1 \n"); }
	if(time_RTC == time_uitgang[2]){ print_dbg("Alarm 2 \n"); }
	if(time_RTC == time_uitgang[3]){ print_dbg("Alarm 3 \n"); }
	if( time_uitgang[4] == time_RTC){ print_dbg("Alarm 4 \n"); }
	
	// PC00 - 64
	// PC01 - 65
	// PC02 - 66
	// PC03 - 67
	// PC04 - 68
	// PC05 - 69
}
