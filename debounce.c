/*
 * Debounce.c
 *
 * Created: 17-01-2019 22:54:01
 *  Author: doede
 */ 

#include "Debounce.h"

void SetDebounceTimer(int time) //
{
	//zet de timer
	cpu_set_timeout(cpu_ms_2_cy(time, sysclk_get_cpu_hz()), &timer);
}

Bool DebounceTimer() 
{	
	//check of de timer is afgelopen
	if (cpu_is_timeout(&timer)){
		return true;
	}
	else { 
		return false;
	}
}