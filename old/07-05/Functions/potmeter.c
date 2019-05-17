/*
 * Potmeter.c
 *
 * Created: 04/05/2019 08:12:56
 *  Author: Margreet
 */ 

#include "potmeter.h"

void Init_Potmeter()
{
	gpio_enable_module(POT_GPIO_MAP, 2);
	
	//ADC init
	AVR32_ADC.mr |= 0x1 << AVR32_ADC_MR_PRESCAL_OFFSET;
	adc_configure(&AVR32_ADC);
	adc_enable(&AVR32_ADC, 1);
}

int Potmeter_value(void)
{
	adc_enable(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	adc_start(&AVR32_ADC);
	adc_value_pot = adc_get_value(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	adc_disable(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	
	return (adc_value_pot / 103 % 10);
}

int Potmeter_value_Month(void)
{
	adc_enable(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	adc_start(&AVR32_ADC);
	adc_value_pot = adc_get_value(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	adc_disable(&AVR32_ADC, ADC_POTENTIOMETER_CHANNEL);
	
	return (adc_value_pot / 93 % 12 + 1);
}
