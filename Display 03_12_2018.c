#include "evk1100.h"
#include <asf.h>
//#include "print_funcs.h"
#include "pm.h"
#include "string.h"
#include "stdlib.h"
#include <math.h>

#define PB0						GPIO_PUSH_BUTTON_0			// Push button 0 definition
#define PB1						GPIO_PUSH_BUTTON_1			// Push button 1 definition
#define PB2						GPIO_PUSH_BUTTON_2			// Push button 2 definition
#define EEPROM_ADDRESS			0x57						// EEPROM's TWI address
#define RTC_ADDRESS				0x68						// RTC's TWI address
#define RTC_ADDR_LGT			1							// Package length for data transmission
#define TWI_SPEED				FOSC32						// Speed of TWI communication
#define DATA_LENGTH				3							// Length of data in a package (max 3 bytes)
#define Register_Seconds		0x00						// Address for the register that holds the seconds data
#define Register_Minutes		0x01						// Address for the register that holds the minutes data
#define Register_Hours			0x02						// Address for the register that holds the hours data
#define Register_Days			0x03						// Address for the register that holds the days data
#define usart0					&AVR32_USART0				// Definition for USART0 communication
#define usart1					&AVR32_USART1				// Definition for USART1 communication


//static const gpio_map_t ADC_GPIO_MAP = {{ADC_POTENTIOMETER_PIN, ADC_POTENTIOMETER_FUNCTION}, {ADC_LIGHT_PIN, ADC_LIGHT_FUNCTION}, {ADC_TEMPERATURE_PIN, ADC_TEMPERATURE_FUNCTION}};
static const gpio_map_t POT_GPIO_MAP = {{AVR32_ADC_AD_1_PIN, AVR32_ADC_AD_1_FUNCTION}};
static const gpio_map_t USART1_GPIO_MAP = {{AVR32_USART1_RXD_0_0_PIN, AVR32_USART1_RXD_0_0_FUNCTION}, {AVR32_USART1_TXD_0_0_PIN, AVR32_USART1_TXD_0_0_FUNCTION}};
static const gpio_map_t DIP204_SPI_GPIO_MAP = {{DIP204_SPI_SCK_PIN,  DIP204_SPI_SCK_FUNCTION }, {DIP204_SPI_MISO_PIN, DIP204_SPI_MISO_FUNCTION}, {DIP204_SPI_MOSI_PIN, DIP204_SPI_MOSI_FUNCTION}, {DIP204_SPI_NPCS_PIN, DIP204_SPI_NPCS_FUNCTION}};
static const gpio_map_t TWI_GPIO_MAP = {{AVR32_TWI_SDA_0_0_PIN, AVR32_TWI_SDA_0_0_FUNCTION},{AVR32_TWI_SCL_0_0_PIN, AVR32_TWI_SCL_0_0_FUNCTION}};

static const usart_options_t USART_OPTIONS = {.baudrate = 115200,.charlength = 8,.paritytype = USART_NO_PARITY,.stopbits = USART_1_STOPBIT,.channelmode = USART_NORMAL_CHMODE};
static const spi_options_t SPI_OPTIONS = {.reg = DIP204_SPI_NPCS,.baudrate = 120000,.bits = 8,.spck_delay = 0,.trans_delay = 0,.stay_act = 1,.spi_mode = 0,.modfdis = 1};
//static const twi_options_t TWI_OPTIONS = {.pba_hz = FOSC0,.speed = TWI_SPEED,.chip = RTC_ADDRESS};

twi_options_t opt;
twi_package_t packet_received;
int status;

void var_delay_ms(int);
void Potmeter_control(void);
int  Potmeter_value(void);
char* itoa(int);
static void Joy_int_handler(void);
void Configure_joystick_IT(void);
static void PB_int_handler(void);
void configure_push_buttons_IT(void);
char* set_time(void);
int Pot_meter(void);

signed short adc_value_pot   = -1;
int LEDS_GREEN[6] = {0x01, 0x02, 0x04, 0x08, 0x20, 0x80};
int LEDS_RED[2]	=	{0x10, 0x40};

void display_1(int, char*);
void display_2(int, char*);
void display_3(int, char*);
void display_4(int, char*);

//int testvalue = 100;
int x = 1;
int y = 1;
int Button_cases = 1;
int time_uitgang[ 4 ];
int joy_button = 0;
int pot_value = 0; 

int poort_number = 1;

//__attribute__((__interrupt__)) 
void var_delay_ms(int mills) {
	delay_ms(mills * (FOSC0/AVR32_PM_RCOSC_FREQUENCY));
}


int main(void) {
	//ADC init
	AVR32_ADC.mr |= 0x1 << AVR32_ADC_MR_PRESCAL_OFFSET;
	adc_configure(&AVR32_ADC);
	adc_enable(&AVR32_ADC, 1);
		
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
		
		Configure_joystick_IT();		
		Joy_int_handler();
		configure_push_buttons_IT();
		PB_int_handler(); 
		Potmeter_control();
		
		dip204_clear_display();
					
		switch(Button_cases){
			case 1:
				display_1(1,"B:");
				display_1(3,itoa(joy_button));
				display_1(6, "Schakelklok 1");
				display_3(1,set_time());
			//	display_4(10,"hh:mm:ss:ms");
				break;
			case 2:
				display_1(1,"B:");
				display_1(3,itoa(joy_button));
				display_1(6,"Schakelklok 2");
				
				display_2(10,itoa(time_uitgang[poort_number]));
				break;
			case 3:
				display_1(1,"B:");
				display_1(3,itoa(joy_button));
				display_1(6,"Schakelklok 3");
				break;			
		}	
		
		set_time();
		dip204_set_cursor_position(y,x);
		
		var_delay_ms(1);
		
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

void Potmeter_control(void)
{
	adc_enable(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	adc_start(&AVR32_ADC);
	adc_value_pot = adc_get_value(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	adc_disable(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
}
int Potmeter_value(void)
{
	adc_enable(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	adc_start(&AVR32_ADC);
	adc_value_pot = adc_get_value(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	adc_disable(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	
	return (adc_value_pot / 103 % 10);
}


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
		//LED_On(0x02);
		gpio_clear_pin_interrupt_flag(PB1);
	}

	if (gpio_get_pin_interrupt_flag(PB2)){
		//LED_On(0x04);
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

int time_20 = 0;
int time_19 = 0;
int time_17 = 0;
int time_16 = 0;
int time_14 = 0;
int time_13 = 0;
int time_11 = 0;
int time_10 = 0;
	
char* set_time(){
	char *result = malloc(20);
		
	if(joy_button == 1 && x == 3 && y > 9){
		if(y == 20){ time_20 = Potmeter_value(); }
		if(y == 19){
			if(Potmeter_value() > 5){ time_19 = 5; }
			else{ time_19 = Potmeter_value(); }
		}
		if(y == 17){ time_17 = Potmeter_value(); }
		if(y == 16){
			if(Potmeter_value() > 5){ time_16 = 5; }
			else{ time_16 = Potmeter_value(); }
		}
		if(y == 14){ time_14 = Potmeter_value(); }
		if(y == 13){
			if(Potmeter_value() > 5){ time_13 = 5; }
			else{ time_13 = Potmeter_value(); }	
		}
		if(y == 11){
			if(Potmeter_value() > 3){ time_11 = 3; }
			else{ time_11 = Potmeter_value(); }	
		}
		if(y == 10){
			if(Potmeter_value() > 2){ time_10 = 2; }
			else{ time_10 = Potmeter_value(); }				
		}		
			
		time_uitgang[poort_number] = (time_10 * 10 + time_11) * 60 * 60 * 60 + (time_13 * 10 + time_14) * 60 * 60 + (time_16 * 10 + time_17) * 60 + (time_19 * 10 + time_20);
	}
	
	if(joy_button == 1 && y == 7 && x == 3){
		poort_number = (Potmeter_value()/3%4 + 1);
		time_20 = 0;	//reset all.... so something without time_20
		time_19 = 0;
		time_17 = 0;
		time_16 = 0;
		time_14 = 0;
		time_13 = 0;
		time_11 = 0;
		time_10 = 0;
	}
	
	strcpy(result, "Poort ");
	strcat(result, itoa(poort_number));
	strcat(result, ": ");
	strcat(result, itoa((time_uitgang[poort_number] / 216000) % 24 / 10 % 10));
	strcat(result, itoa((time_uitgang[poort_number] / 216000) % 24 % 10));
	strcat(result, ":");
	strcat(result, itoa((time_uitgang[poort_number] / 3600) % 60 / 10 % 10));
	strcat(result, itoa((time_uitgang[poort_number] / 3600) % 60 % 10));
	strcat(result, ":");
	strcat(result, itoa((time_uitgang[poort_number] / 60) % 60 / 10 % 10));
	strcat(result, itoa((time_uitgang[poort_number] / 60) % 60 % 10));
	strcat(result, ":");
	strcat(result, itoa(time_uitgang[poort_number] % 60 / 10 % 10));
	strcat(result, itoa(time_uitgang[poort_number] % 60 % 10));
	
	return result;
}

