#include <asf.h>
#include "evk1100.h"

#include "pm.h"
#include "string.h"
#include "print_funcs.h"

#include "display.h"
#include "interrupts.h"
#include "RTC.h"
#include "potmeter.h"
#include "alarm.h"

#define usart1					&AVR32_USART1				// Definition for USART1 communication

static const gpio_map_t USART1_GPIO_MAP = {{AVR32_USART1_RXD_0_0_PIN, AVR32_USART1_RXD_0_0_FUNCTION}, {AVR32_USART1_TXD_0_0_PIN, AVR32_USART1_TXD_0_0_FUNCTION}};
static const usart_options_t USART_OPTIONS = {.baudrate = 115200,.charlength = 8,.paritytype = USART_NO_PARITY,.stopbits = USART_1_STOPBIT,.channelmode = USART_NORMAL_CHMODE};

__attribute__((__interrupt__)) static void tc_irq(void){
	// Clear the interrupt flag. This is a side effect of reading the TC SR.
	tc_tick++;
	tc_read_sr(&AVR32_TC,0);
	update_timer = true;
}
void configure_tc_IT(void){
	gpio_enable_module_pin(AVR32_TC_A0_0_0_PIN, AVR32_TC_A0_0_0_FUNCTION);
	
	Disable_global_interrupt();
	INTC_register_interrupt(&tc_irq, AVR32_TC_IRQ0, AVR32_INTC_INT0);
	Enable_global_interrupt();
}

int main (void)
{
	volatile avr32_tc_t *tc = &AVR32_TC;
	uint32_t timer = 0;
	
	sysclk_init();
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);
	//pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);
	
	//irq_initialize_vectors();
	//cpu_irq_enable();
	
	configure_push_buttons_IT();
	configure_joystick_IT();
	configure_tc_IT();
	tc_init(tc);
	
	gpio_enable_module(USART1_GPIO_MAP, 2);				//enable module
	usart_init_rs232(usart1, &USART_OPTIONS, FOSC0);	//USART init
	
	Init_display();
	Init_RTC();
	Init_Potmeter();
	
	TEST_TWI_MASTER_EEPROM();
	Read_EEPROM();
		
	update_display = 1;
	display = 1;
	
	while(1){
		if ((update_timer) && (tc_tick==20)) { //update every 200ms (tc_tick = 100 -> 1sec)
			Read_RTC();
			time_RTC = bcd_to_decimal(tbuffer[0]) * 60 + bcd_to_decimal(tbuffer[1]) * 60 * 60 + bcd_to_decimal(tbuffer[2]) * 60 * 60 * 60;
			Schakelmoment();
			
			display_cases();
									
			tc_tick = 0;
			update_timer = false;
			dip204_set_cursor_position(x,y);
		}
		
		if (update_display==1){
			joy_button = 0;
			switch(display){
				case 1:			
					dip204_clear_display();
					set_display(1,1,"-------DS3231-    --");
					set_display(1,2,"Time: --:--:--");
					set_display(1,3,"Date: - -- --- 20--");
					set_display(2,4,"Today is ");
					update_display = 0;
				break;
				case 2:
					dip204_clear_display();
					set_display(1,3,"Poort  : --:--:--:--");
					update_display = 0;
				break;
				case 3:
					dip204_clear_display();
					for(int pos = 1 ; pos < 5 ; pos++){
						dip204_set_cursor_position(1,pos);
						dip204_write_string("Poort  : --:--:--:--");
						dip204_set_cursor_position(7,pos);
						dip204_write_string(itoa(pos));
					
						dip204_set_cursor_position(10,pos);
						dip204_write_string(itoa((time_uitgang[pos] / 216000) % 24 / 10 % 10));
						dip204_set_cursor_position(11,pos);
						dip204_write_string(itoa((time_uitgang[pos] / 216000) % 24 % 10));
						dip204_set_cursor_position(13,pos);
						dip204_write_string(itoa((time_uitgang[pos] / 3600) % 60 / 10 % 10));
						dip204_set_cursor_position(14,pos);
						dip204_write_string(itoa((time_uitgang[pos] / 3600) % 60 % 10));
						dip204_set_cursor_position(16,pos);
						dip204_write_string(itoa((time_uitgang[pos] / 60) % 60 / 10 % 10));
						dip204_set_cursor_position(17,pos);
						dip204_write_string(itoa((time_uitgang[pos] / 60) % 60 % 10));
						dip204_set_cursor_position(19,pos);
						dip204_write_string(itoa(time_uitgang[pos] % 60 / 10 % 10));
						dip204_set_cursor_position(20,pos);
						dip204_write_string(itoa(time_uitgang[pos] % 60 % 10));
					}
					update_display = 0;
				break;
			}
			dip204_set_cursor_position(x,y);
		}
		//dip204_set_cursor_position(x,y);
	}
}
