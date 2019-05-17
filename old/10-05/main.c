#include "evk1100.h"
#include <asf.h>

#include "pm.h"
#include "string.h"
#include "print_funcs.h"

#include "display.h"
#include "RTC.h"
#include "Potmeter.h"
#include "Alarm.h"
#include "interrupts.h"

#define usart1					&AVR32_USART1				// Definition for USART1 communication

static const gpio_map_t USART1_GPIO_MAP = {{AVR32_USART1_RXD_0_0_PIN, AVR32_USART1_RXD_0_0_FUNCTION}, {AVR32_USART1_TXD_0_0_PIN, AVR32_USART1_TXD_0_0_FUNCTION}};
static const usart_options_t USART_OPTIONS = {.baudrate = 115200,.charlength = 8,.paritytype = USART_NO_PARITY,.stopbits = USART_1_STOPBIT,.channelmode = USART_NORMAL_CHMODE};

__attribute__((__interrupt__)) static void tc_irq(void){
	// Clear the interrupt flag. This is a side effect of reading the TC SR.
	tc_tick++;
	//if (tc_tick == 1000){ tc_tick = 0;}
				
	tc_read_sr(&AVR32_TC,0);
	update_timer = true;
}
void configure_tc_IT(void){
	gpio_enable_module_pin(AVR32_TC_A0_0_0_PIN, AVR32_TC_A0_0_0_FUNCTION);
	
	Disable_global_interrupt();
	INTC_register_interrupt(&tc_irq, AVR32_TC_IRQ0, AVR32_INTC_INT0);
	Enable_global_interrupt();
}

int main(void){
	volatile avr32_tc_t *tc = &AVR32_TC;
	uint32_t timer = 0;
		
	sysclk_init();
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);
	
	//pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);
	//irq_initialize_vectors();
	//cpu_irq_enable();
		
	gpio_enable_module(USART1_GPIO_MAP, 2);				//enable module
	usart_init_rs232(usart1, &USART_OPTIONS, FOSC0);	//USART init
	
	Init_display();
	Init_RTC();
	Init_Potmeter();
	
	configure_push_buttons_IT();
	configure_joystick_IT();
	configure_tc_IT();
	tc_init(tc);
		
	TEST_TWI_MASTER_EEPROM();
	Read_EEPROM();
	
	update_display = 1;
	display = 1;
	
	/*while(0){
		if (update_timer && (!(tc_tick%200))) { 	
			//timer++;
			//print_dbg("Timer: ");
			print_dbg_ulong(tc_tick);
			print_dbg(" ms\n");		
			
			set_display(15,2,":");
			set_display(16,2,itoa(tc_tick/100));
				
			dip204_set_cursor_position(x,y);
			
			update_timer = false;
			
		}
	}*/
	
	volatile static bool synch = true;
	volatile static bool show_timer = true;
	int time_old;
		
	Read_RTC();
	time_old = bcd_to_decimal(tbuffer[0]);
	while(synch){
		Read_RTC();
		if(time_old != bcd_to_decimal(tbuffer[0])){
			tc_tick = 0;
			timer = bcd_to_decimal(tbuffer[0]);
			synch = false;
		}
	}
	
	int syn = 0;
	
	while(1){
		if (update_timer && (!(tc_tick%200))){
			if (tc_tick == 1000){
				timer++;
				if (display == 1){ set_display(18,2,itoa(timer));
				set_display(20,2," ");}
				if (timer == 60){
					if (bcd_to_decimal(tbuffer[0]) != (timer-1)){
						synch = true;	
						Read_RTC();
						time_old = bcd_to_decimal(tbuffer[0]);
						while(synch){
							Read_RTC();
							if(time_old != bcd_to_decimal(tbuffer[0])){
								timer = bcd_to_decimal(tbuffer[0]);
								synch = false;
							}
						}
					}
					else{ timer = 0; }
				}
				tc_tick = 0;
			}
			if (display == 1){set_display(16,2,itoa(tc_tick/100));}
											
			Read_RTC();			
			time_RTC = tc_tick + bcd_to_decimal(tbuffer[0]) * 1000 + bcd_to_decimal(tbuffer[1]) * 60 * 1000 + bcd_to_decimal(tbuffer[2]) * 60 * 60 * 1000;
			//Schakelmoment();
			
			display_cases();
			dip204_set_cursor_position(x,y);
			
			update_timer = false;
		}
		else if(update_display==1){
			display_update();
			update_display = 0;
			dip204_set_cursor_position(x,y);
		}
		
		print_dbg("\nTime1: ");
		print_dbg_ulong(time_RTC);
		print_dbg("\nTime2: ");
		print_dbg_ulong(time_uitgang[3]);
		Schakelmoment();
	}
}