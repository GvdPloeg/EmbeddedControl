void Sqr_wav(){
	int status;
	twi_package_t packet_w;
	const U8 pattern[] =  {0x18};
	gpio_enable_module(TWI_GPIO_MAP, sizeof(TWI_GPIO_MAP) / sizeof(TWI_GPIO_MAP[0]));
	opt.chip = RTC_ADDRESS;
	packet_w.chip = RTC_ADDRESS;
	packet_w.addr[0] = 0x0E;
	packet_w.addr_length = 1;
	packet_w.buffer = (void*) pattern;
	packet_w.length = 1;
	status = twi_master_write(&AVR32_TWI, &packet_w);
	if (status == TWI_SUCCESS) { print_dbg("Sqr_wav actief\r\n"); }
	else { print_dbg("Sqr_wav inactief\r\n"); }
}

void Light_sensor(void)
{
	adc_enable(&AVR32_ADC, ADC_LIGHT_CHANNEL);
	adc_start(&AVR32_ADC);
	adc_value_light = adc_get_value(&AVR32_ADC, ADC_LIGHT_CHANNEL);
	
	if (adc_value_light > 512){LED_On(0
	02);}
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