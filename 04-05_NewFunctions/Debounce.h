/*
 * Debounce.h
 *
 * Created: 03/05/2019 15:12:14
 *  Author: Margreet
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