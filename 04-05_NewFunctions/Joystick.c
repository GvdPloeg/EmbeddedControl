/*
 * Joystick.c
 *
 * Created: 04/05/2019 13:41:29
 *  Author: Margreet
 */ 

#include "Joystick.h"

void Init_Joystick(void)
{
	//Configure_joystick_IT();
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_UP , GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_DOWN , GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_RIGHT , GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_PUSH , GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_LEFT , GPIO_RISING_EDGE);

	Disable_global_interrupt();
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_UP/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_DOWN/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_RIGHT/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_LEFT/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_PUSH/8), AVR32_INTC_INT1);
	Enable_global_interrupt();
	
	joy_button = 0;
	x = 1;
	y = 1;
}

void Joy_int_handler(){
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_UP)){
		var_delay_ms(1);
		y = y - 1;
		if (y < 1){
			y = 4;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_UP); // allow new interrupt : clear the IFR flag
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_DOWN)){
		y = y + 1;
		if (y > 4){
			y = 1;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_DOWN); // allow new interrupt : clear the IFR flag
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_LEFT)){
		x = x - 1;
		if (x < 1){
			x = 20;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_LEFT); // allow new interrupt : clear the IFR flag
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_RIGHT)){
		x = x + 1;
		if (x > 20){
			x = 1;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_RIGHT); // allow new interrupt : clear the IFR flag
	}
	
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_PUSH)){ // change time when pushed
		joy_button = joy_button + 1;
		if (joy_button > 1){
			joy_button = 0;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_PUSH);  // allow new interrupt : clear the IFR flag
	}
	var_delay_ms(1);
}
