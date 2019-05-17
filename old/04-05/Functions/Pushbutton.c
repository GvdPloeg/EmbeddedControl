/*
 * Pushbutton.c
 *
 * Created: 04/05/2019 12:17:38
 *  Author: Margreet
 */ 

#include "Pushbutton.h"


void Init_Pushbutton(){
		//configure_push_buttons_IT();
		gpio_enable_pin_interrupt(PB0 , GPIO_RISING_EDGE);
		gpio_enable_pin_interrupt(PB1 , GPIO_RISING_EDGE);
		gpio_enable_pin_interrupt(PB2 , GPIO_RISING_EDGE);

		Disable_global_interrupt();
		INTC_register_interrupt( &PB_int_handler, AVR32_GPIO_IRQ_0 + (PB2/8), AVR32_INTC_INT1);
		INTC_register_interrupt( &PB_int_handler, AVR32_GPIO_IRQ_0 + (PB1/8), AVR32_INTC_INT1);
		INTC_register_interrupt( &PB_int_handler, AVR32_GPIO_IRQ_0 + (PB0/8), AVR32_INTC_INT1);
		Enable_global_interrupt();
		
		Button_cases = 1;
}

void PB_int_handler(){
	if (gpio_get_pin_interrupt_flag(PB0)){
		Button_cases = Button_cases + 1;
		if (Button_cases == 4){
			Button_cases = 1;
		}
		gpio_clear_pin_interrupt_flag(PB0);
	}

	if (gpio_get_pin_interrupt_flag(PB1)){
		Button_cases = 2;
		gpio_clear_pin_interrupt_flag(PB1);
	}

	if (gpio_get_pin_interrupt_flag(PB2)){
		Button_cases = 3;
		gpio_clear_pin_interrupt_flag(PB2);
	}
	
	var_delay_ms(1);
}