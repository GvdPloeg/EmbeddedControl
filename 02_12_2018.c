#include "evk1100.h"
#include "board.h"
#include "compiler.h"
#include "dip204.h"
#include "intc.h"
#include "gpio.h"
#include "pm.h"
#include "delay.h"
#include "spi.h"
#include "conf_clock.h"
#include "string.h"

#include <avr32/io.h>
#include <math.h>
//#include "print_funcs.h"
//#include "twi.h"

#define PB0		GPIO_PUSH_BUTTON_0		  // Push button 0 definition
#define PB1		GPIO_PUSH_BUTTON_1		  // Push button 1 definition
#define PB2		GPIO_PUSH_BUTTON_2		  // Push button 2 definition

void var_delay_ms(int);
void Potmeter_control(void);
int  Potmeter_value(void);
char* itoa(int);
static void Joy_int_handler(void);
void Configure_joystick_IT(void);
static void PB_int_handler(void);
void configure_push_buttons_IT(void);
void change_time_poort(void);
void set_time(void);
	
void display_1(int, char*);
void display_2(int, char*);
void display_3(int, char*);
void display_4(int, char*);

int LEDS_GREEN[6] = {0x01, 0x02, 0x04, 0x08, 0x20, 0x80};
int LEDS_RED[2]	=	{0x10, 0x40};
int adc_value_pot = 0;
int adc_value_light = 0;
int adc_value_temp  = 0;
int testvalue = 100;
int x = 1;
int y = 1;
int Button_cases = 1;
int time_uitgang1 = 5183999;
int joy_button = 0;

static const gpio_map_t ADC_GPIO_MAP = {{ADC_POTENTIOMETER_PIN, ADC_POTENTIOMETER_FUNCTION}, {ADC_LIGHT_PIN, ADC_LIGHT_FUNCTION}, {ADC_TEMPERATURE_PIN, ADC_TEMPERATURE_FUNCTION}};
static const gpio_map_t USART_GPIO_MAP = {{AVR32_USART1_RXD_0_0_PIN, AVR32_USART1_RXD_0_0_FUNCTION}, {AVR32_USART1_TXD_0_0_PIN, AVR32_USART1_TXD_0_0_FUNCTION}};
static const gpio_map_t DIP204_SPI_GPIO_MAP = {{DIP204_SPI_SCK_PIN,  DIP204_SPI_SCK_FUNCTION }, {DIP204_SPI_MISO_PIN, DIP204_SPI_MISO_FUNCTION}, {DIP204_SPI_MOSI_PIN, DIP204_SPI_MOSI_FUNCTION}, {DIP204_SPI_NPCS_PIN, DIP204_SPI_NPCS_FUNCTION}};
static const gpio_map_t TWI_GPIO_MAP = {{AVR32_TWI_SDA_0_0_PIN, AVR32_TWI_SDA_0_0_FUNCTION},{AVR32_TWI_SCL_0_0_PIN, AVR32_TWI_SCL_0_0_FUNCTION}};

void var_delay_ms(int mills)
{
	delay_ms(mills * (FOSC0/AVR32_PM_RCOSC_FREQUENCY));
}

int main(void)
{
	static const gpio_map_t DIP204_SPI_GPIO_MAP =
	{
		{DIP204_SPI_SCK_PIN,  DIP204_SPI_SCK_FUNCTION },  // SPI Clock.
		{DIP204_SPI_MISO_PIN, DIP204_SPI_MISO_FUNCTION},  // MISO.
		{DIP204_SPI_MOSI_PIN, DIP204_SPI_MOSI_FUNCTION},  // MOSI.
		{DIP204_SPI_NPCS_PIN, DIP204_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
	};

	// Switch the CPU main clock to oscillator 0
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);

	// add the spi options driver structure for the LCD DIP204
	spi_options_t spiOptions =
	{
		.reg          = DIP204_SPI_NPCS,
		.baudrate     = 1000000,
		.bits         = 8,
		.spck_delay   = 0,
		.trans_delay  = 0,
		.stay_act     = 1,
		.spi_mode     = 0,
		.modfdis      = 1
	};

	gpio_enable_module(DIP204_SPI_GPIO_MAP,
	sizeof(DIP204_SPI_GPIO_MAP) / sizeof(DIP204_SPI_GPIO_MAP[0]));	// Assign I/Os to SPI
	spi_initMaster(DIP204_SPI, &spiOptions);			// Initialize as master
	spi_selectionMode(DIP204_SPI, 0, 0, 0);				// Set selection mode: variable_ps, pcs_decode, delay
	spi_enable(DIP204_SPI);								// Enable SPI
	spi_setupChipReg(DIP204_SPI, &spiOptions, FOSC0);	// setup chip registers
	dip204_init(backlight_PWM, true);					// initialize LCD


	while (1)
	{
		//Potmeter_control();
		//Potmeter_value();

		Configure_joystick_IT();		
		Joy_int_handler();
		configure_push_buttons_IT();
		PB_int_handler();
		
		LED_Off(0x02);
		LED_Off(0x04);
		
		dip204_clear_display();
		switch(Button_cases){
			case 1:
				display_1(4, "Schakelklok 1");
				break;
			case 2:
				display_1(4,"Schakelklok 2");
				break;
			case 3:
				display_1(4,"Schakelklok 3");
				break;			
		}	
		set_time();
		dip204_set_cursor_position(y,x);
		
		//var_delay_ms(1);
		
	}
}

void display_1(int pos_1, char* string_1){
	dip204_set_cursor_position(pos_1,1);
	dip204_write_string(string_1);
}
void display_2(int pos_2, char* string_2){
	dip204_set_cursor_position(pos_2,2);
	dip204_write_string(string_2);
}
void display_3(int pos_3, char* string_3){
	dip204_set_cursor_position(pos_3,3);
	dip204_write_string(string_3);
}
void display_4(int pos_4, char* string_4){
	dip204_set_cursor_position(pos_4,4);
	dip204_write_string(string_4);
}

char* itoa(int i){
	char const digit[] = "0123456789";
	char* p = 10;
	if(i<0){
		*p++ = '-';
		i *= -1;
	}
	int shifter = i;
	do{ //Move to where representation ends
		++p;
		shifter = shifter/10;
	}while(shifter);
	*p = '\0';
	do{ //Move back, inserting digits as u go
		*--p = digit[i%10];
		i = i/10;
	}while(i);
	return 10;
}

/*
void Potmeter_control(void)
{
	adc_enable(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	adc_start(&AVR32_ADC);
	adc_value_pot = adc_get_value(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	
	if (adc_value_pot > 512){LED_On(0x01);}
	else{LED_Off(0x01);}
	adc_disable(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
}
int Potmeter_value(void)
{
	adc_enable(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	adc_start(&AVR32_ADC);
	adc_value_pot = adc_get_value(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	adc_disable(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	return adc_value_pot;
}
*/

static void Joy_int_handler(){
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_UP)){
		var_delay_ms(1);
		x = x - 1;
		if (x < 1){
			x = 4;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_UP); // allow new interrupt : clear the IFR flag
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_DOWN)){
		x = x + 1;
		if (x > 4){
			x = 1;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_DOWN); // allow new interrupt : clear the IFR flag
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_LEFT)){
		y = y - 1;
		if (y < 1){
			y = 20;
		}
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_LEFT); // allow new interrupt : clear the IFR flag
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_RIGHT)){
		y = y + 1;
		if (y > 20){
			y = 1;
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

void Configure_joystick_IT(){	
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_UP , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_DOWN , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_RIGHT , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_PUSH , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_LEFT , GPIO_FALLING_EDGE);

	Disable_global_interrupt();  //Disable all interrupts
	/* register PB0 handler on level 1 */
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_UP/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_DOWN/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_RIGHT/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_LEFT/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_PUSH/8), AVR32_INTC_INT1);
	Enable_global_interrupt();  //Enable all interrupts
}


static void PB_int_handler(){
	if (gpio_get_pin_interrupt_flag(PB0)){
		Button_cases = Button_cases + 1;
		if (Button_cases == 4){
			Button_cases = 1;
		}
		gpio_clear_pin_interrupt_flag(PB0);
	}

	if (gpio_get_pin_interrupt_flag(PB1)){
		LED_On(0x02);
		gpio_clear_pin_interrupt_flag(PB1);
	}

	if (gpio_get_pin_interrupt_flag(PB2)){
		LED_On(0x04);
		gpio_clear_pin_interrupt_flag(PB2);
	}
	var_delay_ms(1);
}


void configure_push_buttons_IT(){
	gpio_enable_pin_interrupt(PB0 , GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(PB1 , GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(PB2 , GPIO_RISING_EDGE);

	/* Disable all interrupts */
	Disable_global_interrupt();
	/* register PB0 handler on level 1 */
	INTC_register_interrupt( &PB_int_handler, AVR32_GPIO_IRQ_0 + (PB2/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &PB_int_handler, AVR32_GPIO_IRQ_0 + (PB1/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &PB_int_handler, AVR32_GPIO_IRQ_0 + (PB0/8), AVR32_INTC_INT1);
	/* Enable all interrupts */
	Enable_global_interrupt();
}

void set_time(){
	//test_time is in ms
	int timeport_h1 = (time_uitgang1 / 216000) % 24 % 10;
	int timeport_h10 = (time_uitgang1 / 216000) % 24 / 10 % 10;
	int timeport_m1 = (time_uitgang1 / 3600) % 60 % 10;
	int timeport_m10 = (time_uitgang1 / 3600) % 60 / 10 % 10;
	int timeport_s1 = (time_uitgang1 / 60) % 60 % 10;
	int timeport_s10 = (time_uitgang1 / 60) % 60 / 10 % 10;
	int timeport_ms1 = time_uitgang1 % 60 % 10;
	int timeport_ms10 = time_uitgang1 % 60 / 10 % 10;
	
	display_2(1,"B:");
	display_2(3,itoa(joy_button));
	display_3(1,"X:");
	display_3(3,itoa(x));
	display_4(1,"Y:");
	display_4(3,itoa(y));
	
	display_3(5,"Port ");
	display_3(10,itoa(timeport_h10));
	display_3(11,itoa(timeport_h1));
	display_3(12,":");
	display_3(13,itoa(timeport_m10));	
	display_3(14,itoa(timeport_m1));
	display_3(15,":");	
	display_3(16,itoa(timeport_s10));	
	display_3(17,itoa(timeport_s1));	
	display_3(18,":");	
	display_3(19,itoa(timeport_ms10));	
	display_3(20,itoa(timeport_ms1));	
	
	if(joy_button == 1 && x == 3){
		//change_time_poort();
		if(y == 20){
			time_uitgang1 = time_uitgang1 + 1;
		}
		if(y == 19){
			time_uitgang1 = time_uitgang1 + 10;
		}
		if(y == 17){
			time_uitgang1 = time_uitgang1 + 1 * 60;
		}
		if(y == 16){
			time_uitgang1 = time_uitgang1 + 10 * 60;
		}
		if(y == 14){
			time_uitgang1 = time_uitgang1 + 1 * 60 * 60;
		}
		if(y == 13){
			time_uitgang1 = time_uitgang1 + 10 * 60 * 60;
		}
		if(y == 11){
			time_uitgang1 = time_uitgang1 + 1 * 60 * 60 * 60;
		}
		if(y == 10){
			time_uitgang1 = time_uitgang1 + 10 * 60 * 60 * 60;
		}		
	}
}

void change_time_poort(){
	//something with x and y but shorter...
	int x;
	x = 0;
}