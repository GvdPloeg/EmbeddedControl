/*
 * Potmeter.h
 *
 * Created: 04/05/2019 08:13:19
 *  Author: Margreet
 */ 


#ifndef POTMETER_H_
#define POTMETER_H_

#include <asf.h>

static const gpio_map_t POT_GPIO_MAP = {{AVR32_ADC_AD_1_PIN, AVR32_ADC_AD_1_FUNCTION}};
signed short adc_value_pot;//   = -1;

void Init_Potmeter(void);
int Potmeter_value(void);
int Potmeter_value_Month(void);

#endif /* POTMETER_H_ */