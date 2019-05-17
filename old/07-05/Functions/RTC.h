/*
 * RTC.h
 *
 * Created: 03/05/2019 16:11:48
 *  Author: Margreet
 */ 


#ifndef RTC_H_
#define RTC_H_

#include <asf.h>
#include "print_funcs.h"
#include "display.h"
#include "potmeter.h"
#include "interrupts.h"

#define RTC_ADDRESS				0x68						// RTC's TWI address
#define TWI_SPEED				FOSC32						// Speed of TWI communication

static const gpio_map_t TWI_GPIO_MAP = {{AVR32_TWI_SDA_0_0_PIN, AVR32_TWI_SDA_0_0_FUNCTION},{AVR32_TWI_SCL_0_0_PIN, AVR32_TWI_SCL_0_0_FUNCTION}};
		
twi_options_t opt;
twi_package_t packet_received, packet_send, packet_EEPROM_send, packet_EEPROM_received;

uint8_t tbuffer[9];

int time_uitgang[ 4 ];
int output_nmb;

void Init_RTC(void);
void TEST_TWI_MASTER_EEPROM(void);
void Read_EEPROM(void);
void Write_EEPROM(void);
void Read_RTC(void);
void Write_RTC(void);

#endif /* RTC_H_ */