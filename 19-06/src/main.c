#include "evk1100.h"
#include <asf.h>

#include "pm.h"
#include "string.h"
#include "print_funcs.h"

#include "display.h"
#include "RTC.h"
#include "Potmeter.h"
#include "interrupts.h"

#define usart1					&AVR32_USART1				// Definition for USART1 communication

int time_RTC = 0;
volatile static bool uitgang_1 = false;
volatile static bool uitgang_2 = false;
volatile static bool uitgang_3 = false;
volatile static bool uitgang_4 = false;

static const gpio_map_t USART1_GPIO_MAP = {{AVR32_USART1_RXD_0_0_PIN, AVR32_USART1_RXD_0_0_FUNCTION}, {AVR32_USART1_TXD_0_0_PIN, AVR32_USART1_TXD_0_0_FUNCTION}};
static const usart_options_t USART_OPTIONS = {.baudrate = 115200,.charlength = 8,.paritytype = USART_NO_PARITY,.stopbits = USART_1_STOPBIT,.channelmode = USART_NORMAL_CHMODE};

volatile static bool update_date = true;

__attribute__((__interrupt__)) static void tc_irq(void){
	// Clear the interrupt flag. This is a side effect of reading the TC SR.
	tc_tick++;
	if(joy_button==0){
		if (tc_tick == 1000){sec_timer++; tc_tick = 0;}
		if(sec_timer==60){min_timer++;sec_timer=0;}
		if(min_timer==60){hr_timer++;min_timer=0;}
		if(hr_timer==24){hr_timer=0; update_date = true;}				
	}
	else{if (tc_tick == 1000){tc_tick = 0;}}
		
	LED_Toggle(0x01);
	
	//time_RTC = tc_tick + bcd_to_decimal(tbuffer[0]) * 1000 + bcd_to_decimal(tbuffer[1]) * 60 * 1000 + bcd_to_decimal(tbuffer[2]) * 60 * 60 * 1000;
	//time_RTC = tc_tick + sec_timer * 1000 + min_timer * 60 * 1000 + hr_timer * 60 * 60 * 1000;
	
	print_dbg_ulong(tc_tick);
	print_dbg("\n");
	
	time_RTC =  sec_timer * 1000 + min_timer * 60 * 1000 + hr_timer * 60 * 60 * 1000;
	if(time_RTC == time_uitgang[0] && tc_tick == time_uitgang_ms[0]){ print_dbg("Alarm 1 \n"); }
	if(time_RTC == time_uitgang[1] && tc_tick == time_uitgang_ms[1]){ print_dbg("Alarm 2 \n"); }
	if(time_RTC == time_uitgang[2] && tc_tick == time_uitgang_ms[2]){ print_dbg("Alarm 3 \n"); }
	if(time_RTC == time_uitgang[3] && tc_tick == time_uitgang_ms[3]){ print_dbg("Alarm 4 \n"); }
						
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
	display_number = 1;

	volatile avr32_tc_t *tc = &AVR32_TC;
	
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
		
	//TEST_TWI_MASTER_EEPROM();
	print_dbg("\x0C\r\nTWI EEPROM Test\n");
	Read_EEPROM();
	print_dbg("\n");
	
	update_display = 1;
	
	volatile static bool synch = true;
	int time_old;
		
	Read_RTC();
	time_old = bcd_to_decimal(tbuffer[0]);
	while(synch){
		Read_RTC();
		if(time_old != bcd_to_decimal(tbuffer[0])){
			tc_tick = 0;
			sec_timer = bcd_to_decimal(tbuffer[0]);
			min_timer = bcd_to_decimal(tbuffer[1]);
			hr_timer = bcd_to_decimal(tbuffer[2]);
			synch = false;
		}
	}
			
	while(1){
		if (update_timer && (!(tc_tick%200))){			
			if (joy_button == 0 && display_number == 1){
				set_display(16,2,itoa(tc_tick/100));
			}
			
			LED_Toggle(LED6);								
			//Read_RTC();
			display_cases();
			dip204_set_cursor_position(x,y);
			if(update_date && sec_timer == 1){ // tc_tick == 200);
				Read_RTC();
				update_date = false;
			}
			update_timer = false;
		}
		else if(update_display){
			display_init();
			update_display = 0;
			dip204_set_cursor_position(x,y);
		}
	}
}