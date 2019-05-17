/*
 * interrupts.c
 *
 * Created: 07/05/2019 19:54:09
 *  Author: Margreet
 */ 

#include "interrupts.h"

__attribute__((__interrupt__)) static void PB_int_handler(void){
	if (gpio_get_pin_interrupt_flag(PB0)){
		display = 1;
		update_display = 1;
		gpio_clear_pin_interrupt_flag(PB0);
	}
	if (gpio_get_pin_interrupt_flag(PB1)){
		display = 2;
		update_display = 1;
		dip204_set_backlight(backlight_power_increase);
		gpio_clear_pin_interrupt_flag(PB1);
	}

	if (gpio_get_pin_interrupt_flag(PB2)){
		display = 3;
		update_display = 1;
		dip204_set_backlight(backlight_power_decrease);
		gpio_clear_pin_interrupt_flag(PB2);
	}
}
__attribute__((__interrupt__)) static void Joy_int_handler(void){
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_UP)){
		y = y - 1;
		if (y < 1){ y = 4; }
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_UP);
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_DOWN)){
		y = y + 1;
		if (y > 4){ y = 1; }
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_DOWN);
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_LEFT)){
		x = x - 1;
		if (x < 1){ x = 20; }
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_LEFT);
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_RIGHT)){
		x = x + 1;
		if (x > 20){ x = 1; }
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_RIGHT);
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_PUSH)){
		joy_button = joy_button + 1;
		if (joy_button > 1){ joy_button = 0; }
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_PUSH);
	}
}

void configure_push_buttons_IT(void){
	gpio_enable_pin_interrupt(PB0 , GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(PB2 , GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(PB1 , GPIO_RISING_EDGE);

	Disable_global_interrupt();
	INTC_register_interrupt( &PB_int_handler, AVR32_GPIO_IRQ_0 + (PB2/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &PB_int_handler, AVR32_GPIO_IRQ_0 + (PB1/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &PB_int_handler, AVR32_GPIO_IRQ_0 + (PB0/8), AVR32_INTC_INT1);
	Enable_global_interrupt();
}
void configure_joystick_IT(void){
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_UP , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_DOWN , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_RIGHT , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_PUSH , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_LEFT , GPIO_FALLING_EDGE);

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
void tc_init(volatile avr32_tc_t *tc){
	// Options for waveform generation.
	static const tc_waveform_opt_t waveform_opt = {
		.channel  = TC_CHANNEL,
		.bswtrg   = TC_EVT_EFFECT_NOOP,
		.beevt    = TC_EVT_EFFECT_NOOP,
		.bcpc     = TC_EVT_EFFECT_NOOP,
		.bcpb     = TC_EVT_EFFECT_NOOP,
		.aswtrg   = TC_EVT_EFFECT_NOOP,
		.aeevt    = TC_EVT_EFFECT_NOOP,
		.acpc     = TC_EVT_EFFECT_NOOP,
		.acpa     = TC_EVT_EFFECT_NOOP,
		//.wavsel   = TC_WAVEFORM_SEL_UP_MODE,	
		.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,
		.enetrg   = false,
		.eevt     = 0,
		.eevtedg  = TC_SEL_NO_EDGE,
		.cpcdis   = false,
		.cpcstop  = false,
		//.burst    = TC_BURST_NOT_GATED,
		.burst    = false,
		//.clki     = TC_CLOCK_RISING_EDGE,
		.clki     = false,
		//.tcclks   = TC_CLOCK_SOURCE_TC2			
		.tcclks   = TC_CLOCK_SOURCE_TC3
	};

	// Options for enabling TC interrupts
	static const tc_interrupt_t tc_interrupt = {
		.etrgs = 0,
		.ldrbs = 0,
		.ldras = 0,
		.cpcs  = 1, // Enable interrupt on RC compare alone
		.cpbs  = 0,
		.cpas  = 0,
		.lovrs = 0,
		.covfs = 0
	};
	// Initialize the timer/counter.
	tc_init_waveform(tc, &waveform_opt);
	tc_write_rc(tc, TC_CHANNEL, (sysclk_get_pba_hz() / 8 / 1000));
	tc_configure_interrupts(tc, 0, &tc_interrupt);
	
	//tc_write_ra(tc, TC_CHANNEL, 0x0600); // Set RA value.
	//tc_write_rc(tc, TC_CHANNEL, 0x2000); // Set RC value.
		
	// Start the timer/counter.
	tc_start(tc, 0);
	
	tc_tick = 0;
}