/*
 * syn.c
 *
 * Created: 15/05/2019 10:46:45
 *  Author: Margreet
 */ 

#include "syn.h"

void init_sink(void){
	Read_RTC();
	time_old = bcd_to_decimal(tbuffer[0]);
	sink = true;
	while(sink){
		Read_RTC();
		print_dbg("Test while");
		if(time_old != bcd_to_decimal(tbuffer[0])){
			tc_tick = 0;
			timer = bcd_to_decimal(tbuffer[0]);
			print_dbg("\nsink ready\n");;
			sink = false;
		}
	}
}