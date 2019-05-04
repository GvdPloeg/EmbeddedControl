/*
 * Joystick.h
 *
 * Created: 04/05/2019 13:41:36
 *  Author: Margreet
 */ 


#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include <asf.h>
#include "delay.h"

int joy_button;
int x;
int y;

void Init_Joystick(void);
void Joy_int_handler(void);

#endif /* JOYSTICK_H_ */