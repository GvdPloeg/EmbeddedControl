/*
 * interrupts.h
 *
 * Created: 07/05/2019 19:54:16
 *  Author: Margreet
 */ 


#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include <asf.h>

#define PB0         GPIO_PUSH_BUTTON_0
#define PB1			GPIO_PUSH_BUTTON_1
#define PB2			GPIO_PUSH_BUTTON_2

#define TC_CHANNEL 0

int update_display;
int display;
int joy_button;
int x;
int y;

volatile static bool update_timer = true;
volatile static uint32_t tc_tick;

void configure_push_buttons_IT(void);
void configure_joystick_IT(void);

void tc_init(volatile avr32_tc_t *tc);


#endif /* INTERRUPTS_H_ */