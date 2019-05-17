/*
 * Pushbutton.h
 *
 * Created: 04/05/2019 12:17:31
 *  Author: Margreet
 */ 


#ifndef PUSHBUTTON_H_
#define PUSHBUTTON_H_

#include <asf.h>
#include "delay.h"

//#define Push Buttons
#define PB0						GPIO_PUSH_BUTTON_0			// Push button 0 definition
#define PB1						GPIO_PUSH_BUTTON_1			// Push button 1 definition
#define PB2						GPIO_PUSH_BUTTON_2			// Push button 2 definition

int Button_cases;

void Init_Pushbutton(void);
void PB_int_handler(void);

#endif /* PUSHBUTTON_H_ */