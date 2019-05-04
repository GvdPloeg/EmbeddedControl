#include "evk1100.h"
#include <asf.h>

//#include <avr32/io.h>
#include "print_funcs.h"
#include "pm.h"
//#include "twi.h"
#include "string.h"
//#include "gpio.h"
#include <math.h>
//#include "compiler.h"
//#include "power_clocks_lib.h"
//#include "usart.h"

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
void hex_to_ascii(unsigned char);
void Led_array(int);
void Push_buttons(void);
int  Pot_meter(void);
void Light_sensor(void);
void Temperature_sensor(void);
void Set_display(int);
int Read_RTC_time();
int  DEC_to_HEX(int);
void USART_read_example(void);
void POT_METER(void);

int LEDS_GREEN[6] = {0x01, 0x02, 0x04, 0x08, 0x20, 0x80};
int LEDS_RED[2]	=	{0x10, 0x40};
int adc_value_light = 0;
int adc_value_temp  = 0;
signed short adc_value_pot   = -1;
U8 data_received[DATA_LENGTH] = {0, 0, 0};

__attribute__((__interrupt__)) static void uart_interrupt(void)
{
	int input;
	input = usart_getchar(usart1);
	switch (input)
	{
		case 'x':
		LED_On(0x01);
		break;
		case 'y' :
		LED_On(0x02);
		break;
		case 'z' :
		LED_On(0x04);
		break;
	}
}

void var_delay_ms(int mills)
{
	delay_ms(mills * (FOSC0/AVR32_PM_RCOSC_FREQUENCY));
}

void hex_to_ascii(unsigned char display_data)
{
	unsigned char temp;
	temp = ((display_data & 0xF0)>>4);
	if (temp <= 0x09)
	usart_putchar(usart1, temp+'0');
	else
	usart_putchar(usart1, temp+'0'+0x07);

	temp = display_data & 0x0F;
	if (temp <= 0x09)
	usart_putchar(usart1, temp+'0');
	else
	usart_putchar(usart1, temp+'0'+0x07);

}

void INIT()
{	
	board_init();
	
	//CLOCK init
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);
	pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);
	sysclk_init();
		
	//INTERUPT init
	//Disable_global_interrupt();
	//INTC_init_interrupts();
	//INTC_register_interrupt(&uart_interrupt, AVR32_USART1_IRQ, AVR32_INTC_INT1);
	//Enable_global_interrupt();
	irq_initialize_vectors();
	cpu_irq_enable();
	
	//enable modules
	gpio_enable_module(DIP204_SPI_GPIO_MAP, 4);
	gpio_enable_module(POT_GPIO_MAP, 2);
	gpio_enable_module(USART1_GPIO_MAP, 2);	
	gpio_enable_module(TWI_GPIO_MAP, 2);
	
	//SPI Display init
	spi_initMaster(DIP204_SPI, &SPI_OPTIONS);
	spi_selectionMode(DIP204_SPI, 0, 0, 0);
	spi_setupChipReg(DIP204_SPI, &SPI_OPTIONS, FOSC0);
	spi_enable(DIP204_SPI);
	dip204_init(backlight_PWM, true);
	
	//ADC init
	AVR32_ADC.mr |= 0x1 << AVR32_ADC_MR_PRESCAL_OFFSET;
	adc_configure(&AVR32_ADC);
	adc_enable(&AVR32_ADC, 1);
	
	//USART init
	usart_init_rs232(usart1, &USART_OPTIONS, FOSC0);
	
	//TWI init
	//twi_master_init(&AVR32_TWI, &TWI_OPTIONS);
	opt.pba_hz = FOSC0;
	opt.speed = TWI_SPEED;
	opt.chip = RTC_ADDRESS;
	twi_master_init(&AVR32_TWI, &opt);
}

int main(void)
{
	INIT();
	
	if (!Read_RTC_time() == 0){
		print_dbg("Mission failed, we'll get 'em next time.");
		while(1);
	}
	else{
		print_dbg("Starting program...\n");
	}
	
	print_dbg("Time (hh:mm:ss) == ");
	print_dbg_ulong(DEC_to_HEX(data_received[2]));
	print_dbg(":");
	print_dbg_ulong(DEC_to_HEX(data_received[1]));
	print_dbg(":");
	print_dbg_ulong(DEC_to_HEX(data_received[0]));
	print_dbg("\n");
	
	while (1)
	{
		// ---Uncomment ONE Function---
		//print_dbg_ulong(Pot_meter());
		//print_dbg("\n");
		//USART_read_example();
		
		//Led_array(100);
		//Push_buttons();
		//Potmeter_control();
		//Light_sensor();
		//Temperature_sensor();
		var_delay_ms(1000);
	}
	return 0;
}

void Led_array(int speed_LEDS)
{
	for(int i=0; i<6; i++){LED_On(LEDS_GREEN[i]); var_delay_ms(speed_LEDS); LED_Off(LEDS_GREEN[i]);}
	for(int i=4; i>=0; i--){LED_On(LEDS_GREEN[i]); var_delay_ms(speed_LEDS); LED_Off(LEDS_GREEN[i]);}
}
void Push_buttons(void)
{
	gpio_enable_gpio_pin(PB0);
	gpio_enable_gpio_pin(PB1);
	gpio_enable_gpio_pin(PB2);
	
	if (gpio_get_pin_value(PB0)==0){LED_On(0x01);}
	else{LED_Off(0x01);}
	
	if (gpio_get_pin_value(PB1)==0){LED_On(0x02);}
	else{LED_Off(0x02);}
	
	if (gpio_get_pin_value(PB2)==0){LED_On(0x04);}
	else{LED_Off(0x04);}
}
int Pot_meter(void)
{
	adc_start(&AVR32_ADC);
	adc_value_pot = adc_get_value(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
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
}
void Set_display(int value)
{
	dip204_set_cursor_position(8,1);
	dip204_write_string("HI!!1");
	dip204_set_cursor_position(5,2);
	dip204_write_string("Tweede regel2");
	dip204_set_cursor_position(5,3);
	dip204_write_string("Derde regel3");
	dip204_set_cursor_position(5,4);
	dip204_write_string(value);
	dip204_hide_cursor();
}
int Read_RTC_time()
{
	packet_received.chip = RTC_ADDRESS;
	
	// Length of the TWI data address segment (1-3 bytes)
	packet_received.addr_length = RTC_ADDR_LGT;
	
	// How many bytes do we want to write
	packet_received.length = DATA_LENGTH;
	
	// TWI address/commands to issue to the other chip (node)
	packet_received.addr[0] = Register_Seconds;
	packet_received.addr[1] = Register_Minutes;
	packet_received.addr[2] = Register_Hours;

	packet_received.buffer = data_received;
	
	// perform a read access
	status = twi_master_read(&AVR32_TWI, &packet_received);		
	
	// check read result
	if (!status == TWI_SUCCESS)
	{
		print_dbg("Read test:\tFAIL\r\n");
		return -1;
	}
	return 0;
}
int DEC_to_HEX(int input)
{
	int hexadecimal_number = 0, remainder, count = 0;
	for(count = 0; input > 0; count++)
	{
		remainder = input % 16;
		hexadecimal_number = hexadecimal_number + remainder * (int)pow(10, count);
		input = input / 16;
	}
	return hexadecimal_number;
}
void USART_read_example(void)
{
	usart_write_line(usart1, "Begin session:\r\n");

	int rx_char;
	int retval = USART_SUCCESS;

	while (1)
	{
		rx_char = usart_getchar(usart1);
		if (rx_char == USART_FAILURE)
		{
			usart_write_line(usart1, "Error!!!\r\n");
			retval = USART_FAILURE;
			break;
		}
		if (rx_char == '\x03')
		{
			retval = USART_FAILURE;
			break;
		}
		usart_putchar(usart1, rx_char);
		if (rx_char == '\r')
		{
			usart_putchar(usart1, '\n');
			break;
		}
		if (rx_char == 'A')
		{
			LED_On(0x03);
			var_delay_ms(50);
		}
		else
		{
			LED_Off(0x02);
		}
	}
}
void TimeFunction(void)
{
	
}