/*
 * Debounce.h
 *
 * Created: 7-01-2019 22:54:01
 *  Author: doede
 */ 

#ifndef DEBOUNCE_H_
#define DEBOUNCE_H_

#include "delay.h"
#include "conf_clock.h"

//variablen
t_cpu_time timer;

//functie om de timer aan te zetten met de ingevoerde 'time'
void SetDebounceTimer(int time);
//functie die checkt of de timer nog loopt of afgelopen is en dit dan returned.
Bool DebounceTimer();

#endif /* DEBOUNCE_H_ */