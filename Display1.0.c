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
void Led_array(int);
int Push_buttons(void);
void Potmeter_control(void);
int  Potmeter_value(void);
void Light_sensor(void);
void Temperature_sensor(void);
char* itoa(int);
static void Joy_int_handler(void);
void Configure_joystick_IT(void);
		
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
		//Led_array(1000);
		//Potmeter_control();
		//Light_sensor();
		//Temperature_sensor();
		//Potmeter_value();
		//var_delay_ms(1000);
		
		Joy_int_handler();
		Configure_joystick_IT();
				
		switch(Push_buttons()){
			case 1:
				dip204_clear_display();
				display_1(4, "Schakelklok 1");
				display_2(1, "De tijd is 22:47:22");
				display_3(6, "Helloo..");
				dip204_set_cursor_position(y,x);
				break;
			case 2:
				dip204_clear_display();
				display_1(4,"Schakelklok 2");
				dip204_set_cursor_position(y,x);
				break;
			case 3:
				dip204_clear_display();
				display_1(4,"Schakelklok 3");
				display_4(9,"yay");
				dip204_set_cursor_position(y,x);
				break;			
		}
		
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
void Led_array(int speed_LEDS)
{
	for(int i=0; i<6; i++){LED_On(LEDS_GREEN[i]); delay_ms(speed_LEDS); LED_Off(LEDS_GREEN[i]);}
	for(int i=4; i>0; i--){LED_On(LEDS_GREEN[i]); delay_ms(speed_LEDS); LED_Off(LEDS_GREEN[i]);}
}
int Push_buttons(void)
{
	gpio_enable_gpio_pin(PB0);
	gpio_enable_gpio_pin(PB1);
	gpio_enable_gpio_pin(PB2);
	
	if (gpio_get_pin_value(PB0)==0){LED_On(0x01); return 1;}
	else{LED_Off(0x01);}
	
	if (gpio_get_pin_value(PB1)==0){LED_On(0x02); return 2;}
	else{LED_Off(0x02);}
	
	if (gpio_get_pin_value(PB2)==0){LED_On(0x04); return 3;}
	else{LED_Off(0x04);}
}
/*void Potmeter_control(void)
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
void Light_sensor(void)
{
	adc_enable(&AVR32_ADC, ADC_LIGHT_CHANNEL);
	adc_start(&AVR32_ADC);
	adc_value_light = adc_get_value(&AVR32_ADC, ADC_LIGHT_CHANNEL);
	
	if (adc_value_light > 512){LED_On(0x02);}
	else{LED_Off(0x02);}
	adc_disable(&AVR32_ADC, ADC_LIGHT_CHANNEL);
}
void Temperature_sensor(void)
{
	adc_enable(&AVR32_ADC, ADC_TEMPERATURE_CHANNEL);
	adc_start(&AVR32_ADC);
	adc_value_temp = adc_get_value(&AVR32_ADC, ADC_TEMPERATURE_CHANNEL);
	if (adc_value_temp > 512){LED_On(0x04);}
	else{LED_Off(0x04);}
	adc_disable(&AVR32_ADC, ADC_TEMPERATURE_CHANNEL);
}*/

static void Joy_int_handler(){
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_UP)){
		x = x - 1;
		dip204_set_cursor_position(y,x);
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_UP); // allow new interrupt : clear the IFR flag
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_DOWN)){
		x = x + 1;
		dip204_set_cursor_position(y,x);
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_DOWN); // allow new interrupt : clear the IFR flag
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_LEFT)){
		y = y - 1;
		dip204_set_cursor_position(y,x);
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_LEFT); // allow new interrupt : clear the IFR flag
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_RIGHT)){
		y = y + 1;
		dip204_set_cursor_position(y,x);
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_RIGHT); // allow new interrupt : clear the IFR flag
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_PUSH)){ // change time when pushed
		dip204_set_cursor_position(y,x);
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_PUSH);  // allow new interrupt : clear the IFR flag
	}
}

void Configure_joystick_IT(){
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_UP , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_DOWN , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_RIGHT , GPIO_FALLING_EDGE);
	//gpio_enable_pin_interrupt(GPIO_JOYSTICK_PUSH , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_LEFT , GPIO_FALLING_EDGE);

	Disable_global_interrupt();  //Disable all interrupts
	/* register PB0 handler on level 1 */
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_UP/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_DOWN/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_RIGHT/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_LEFT/8), AVR32_INTC_INT1);
	//INTC_register_interrupt( &Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_PUSH/8), AVR32_INTC_INT1);
	Enable_global_interrupt();  //Enable all interrupts
}