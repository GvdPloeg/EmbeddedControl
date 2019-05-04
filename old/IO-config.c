#include "evk1100.h"
#include "gpio.h"
#include "pm.h"
#include "delay.h"
#include <asf.h>

#define PB0		GPIO_PUSH_BUTTON_0
#define PB1		GPIO_PUSH_BUTTON_1
#define PB2		GPIO_PUSH_BUTTON_2
const gpio_map_t ADC_GPIO_MAP = {{ADC_POTENTIOMETER_PIN, ADC_POTENTIOMETER_FUNCTION}, {ADC_LIGHT_PIN, ADC_LIGHT_FUNCTION}, {ADC_TEMPERATURE_PIN, ADC_TEMPERATURE_FUNCTION}};

void Led_array(int);
void Push_buttons(void);
void Potmeter_control(void);
int Potmeter_value(void);
void Light_sensor(void);
void Temperature_sensor(void);

int LEDS_GREEN[6] = {0x01, 0x02, 0x04, 0x08, 0x20, 0x80};
int LEDS_RED[2]	=	{0x10, 0x40};
int adc_value_pot = 0;
int adc_value_light = 0;
int adc_value_temp  = 0;

int main(void)
{
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);
	gpio_enable_module(ADC_GPIO_MAP, 3);
	
	while (1) 
	{
		// Uncomment ONE Function

		Led_array(Potmeter_value());
		//Push_buttons();
		//Potmeter_control();
		//Light_sensor();
		//Temperature_sensor();
	}
	return 0;
}

void Led_array(int speed_LEDS)
{
	for(int i=0; i<6; i++){LED_On(LEDS_GREEN[i]); delay_ms(speed_LEDS); LED_Off(LEDS_GREEN[i]);}
	for(int i=4; i>0; i--){LED_On(LEDS_GREEN[i]); delay_ms(speed_LEDS); LED_Off(LEDS_GREEN[i]);}
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
	return adc_value_pot;
	adc_disable(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
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
