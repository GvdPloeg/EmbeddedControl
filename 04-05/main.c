#include "evk1100.h"
#include <asf.h>

#include "pm.h"
#include "string.h"

#include "print_funcs.h"
#include "Debounce.h"
#include "display.h"
#include "RTC.h"
#include "Potmeter.h"
#include "Delay.h"
#include "Pushbutton.h"
#include "Joystick.h"
#include "Alarm.h"

#define usart1					&AVR32_USART1				// Definition for USART1 communication

static const gpio_map_t USART1_GPIO_MAP = {{AVR32_USART1_RXD_0_0_PIN, AVR32_USART1_RXD_0_0_FUNCTION}, {AVR32_USART1_TXD_0_0_PIN, AVR32_USART1_TXD_0_0_FUNCTION}};
static const usart_options_t USART_OPTIONS = {.baudrate = 115200,.charlength = 8,.paritytype = USART_NO_PARITY,.stopbits = USART_1_STOPBIT,.channelmode = USART_NORMAL_CHMODE};

void Init()
{
	//CLOCK init
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);
	pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);
	sysclk_init();

	irq_initialize_vectors();
	cpu_irq_enable();
	
	//enable module
	gpio_enable_module(USART1_GPIO_MAP, 2);
	
	//USART init
	usart_init_rs232(usart1, &USART_OPTIONS, FOSC0);
	
}

int main(void)
{
	Init();
	Init_display();
	Init_RTC();
	Init_Joystick();
	Init_Pushbutton();
	Init_Potmeter();
	
	//TEST_TWI_MASTER_EEPROM();
	
	Read_EEPROM();
	//Write_EEPROM();
	
	while (1){
		Read_RTC();
		time_RTC = bcd_to_decimal(tbuffer[0]) * 60 + bcd_to_decimal(tbuffer[1]) * 60 * 60 + bcd_to_decimal(tbuffer[2]) * 60 * 60 * 60;
		Schakelmoment();
		
		Joy_int_handler();
		PB_int_handler();
		dip204_clear_display();
				
		switch(Button_cases){
			case 1:
				display(1,1,"-------DS3231-    --");
				display(1,2,"Time: --:--:--");
				if(joy_button == 1){ display(15,1,"(ON)"); }
				else{ display(15,1,"(OFF)"); }
				display(1,3,"Date: - -- --- 20--");
				dip204_set_cursor_position(x,y);
				int place = 13;
				for(int i = 0; i < 3 ; i ++){
					display(place,2,itoa((bcd_to_decimal(tbuffer[i])) / 10 % 10));
					display((place+1),2,itoa((bcd_to_decimal(tbuffer[i])) % 10));
					place = place - 3;
				}
				display(7,3,itoa(bcd_to_decimal(tbuffer[3])));
				
				display(9,3,itoa((bcd_to_decimal(tbuffer[4])) / 10 % 10));
				display(10,3,itoa((bcd_to_decimal(tbuffer[4])) % 10));
				
				Number_to_Month(bcd_to_decimal(tbuffer[5]));
				
				display(18,3,itoa((bcd_to_decimal(tbuffer[6])) / 10 % 10));
				display(19,3,itoa((bcd_to_decimal(tbuffer[6])) % 10));
				
				display(2,4,"Today is ");
				Number_to_DAY(bcd_to_decimal(tbuffer[3]));
				
				if(joy_button == 1){ Write_RTC(); }
			break;
			case 2:
				display(1,3,"Poort  : --:--:--:--");
				if(joy_button == 1){ display(2,1,"Change poort (ON)"); }
				else{ display(2,1,"Change poort (OFF)"); }
				set_time();
				Write_EEPROM();
			break;
			case 3:
				display_time();
			break;
		}
		
		dip204_set_cursor_position(x,y);
		var_delay_ms(100);
	}
	return 0;
}
