/*
 * sink.h
 *
 * Created: 15/05/2019 10:46:39
 *  Author: Margreet
 */ 


#ifndef SYN_H_
#define SYN_H_

#include "interrupts.h"
#include "RTC.h"

volatile static bool sink;
volatile static bool show_timer;
int time_old;
uint32_t timer;

void init_sink(void);

#endif /* SYN_H_ */