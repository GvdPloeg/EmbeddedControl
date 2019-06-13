/*
 * RTC.c
 *
 * Created: 03/05/2019 16:11:59
 *  Author: Margreet
 */ 

#include "RTC.h"

void Init_RTC()
{
	gpio_enable_module(TWI_GPIO_MAP, 2);
	
	//TWI init
	opt.pba_hz = FOSC0;
	opt.speed = TWI_SPEED;
	
	output_nmb = 0;
}

void TEST_TWI_MASTER_EEPROM(void){
	#define EEPROM_ADDRESS        0x57        // EEPROM's TWI address
	#define EEPROM_ADDR_LGT       3           // Address length of the EEPROM memory
	#define VIRTUALMEM_ADDR_START 0x123456   // Address of the virtual mem in the EEPROM

	#define  PATTERN_TEST_LENGTH (sizeof(test_pattern)/sizeof(U8))
	const U8 test_pattern[] =  { 0xAA, 0x55, 0xA5, 0x5A, 0x77, 0x99 };
	int status, j;

	U8 data_received[PATTERN_TEST_LENGTH] = {0};

	print_dbg("\x0C\r\nTWI EEPROM Test\n");
	gpio_enable_module(TWI_GPIO_MAP, sizeof(TWI_GPIO_MAP) / sizeof(TWI_GPIO_MAP[0]));

	opt.chip = EEPROM_ADDRESS;
	status = twi_master_init(&AVR32_TWI, &opt);

	if (status == TWI_SUCCESS) { print_dbg("Probe test:\tPASS\r\n"); }
	else { print_dbg("Probe test:\tFAIL\r\n"); }

	packet_EEPROM_send.chip = EEPROM_ADDRESS;
	packet_EEPROM_send.addr[0] = VIRTUALMEM_ADDR_START >> 16;
	packet_EEPROM_send.addr[1] = VIRTUALMEM_ADDR_START >> 8;
	packet_EEPROM_send.addr[2] = VIRTUALMEM_ADDR_START;
	packet_EEPROM_send.addr_length = EEPROM_ADDR_LGT;
	packet_EEPROM_send.buffer = (void*) test_pattern;
	packet_EEPROM_send.length = PATTERN_TEST_LENGTH;

	status = twi_master_write(&AVR32_TWI, &packet_EEPROM_send);

	if (status == TWI_SUCCESS) { print_dbg("Write test:\tPASS\r\n"); }
	else { print_dbg("Write test:\tFAIL\r\n"); }

	packet_EEPROM_received.chip = EEPROM_ADDRESS ;
	packet_EEPROM_received.addr_length = EEPROM_ADDR_LGT;
	packet_EEPROM_received.length = PATTERN_TEST_LENGTH;
	packet_EEPROM_received.addr[0] = VIRTUALMEM_ADDR_START >> 16;
	packet_EEPROM_received.addr[1] = VIRTUALMEM_ADDR_START >> 8;
	packet_EEPROM_received.addr[2] = VIRTUALMEM_ADDR_START;
	packet_EEPROM_received.buffer = data_received;

	status = twi_master_read(&AVR32_TWI, &packet_EEPROM_received);

	if (status == TWI_SUCCESS) { print_dbg("Read test:\tPASS\r\n"); }
	else{ print_dbg("Read test:\tFAIL\r\n"); }

	for (j = 0 ; j < PATTERN_TEST_LENGTH; j++) {
		if (data_received[j] != test_pattern[j]) {
			print_dbg("Check Read:\tFAIL\r\n");
			while(1);
		}
	}
	print_dbg("Check Read:\tPASS\r\n\n");
}
void Read_EEPROM(void){
	
	opt.chip = 0x57;
	twi_master_init(&AVR32_TWI, &opt);
	
	U8 data_received[ 5 ] = {0};
	
	for(int output = 0 ; output < 4 ; output++){
		
		packet_EEPROM_received.chip = 0x57 ;
		packet_EEPROM_received.addr_length = 3;
		packet_EEPROM_received.length = 5;
		packet_EEPROM_received.addr[0] = decimal_to_bcd(output);			
		packet_EEPROM_received.addr[1] = 0x00;
		packet_EEPROM_received.addr[2] = 0x00;
		packet_EEPROM_received.buffer = data_received;
		int status = twi_master_read(&AVR32_TWI, &packet_EEPROM_received);
		
		time_uitgang[output] = data_received[0] * 60 * 60 * 1000 + data_received[1] * 60 * 1000 + data_received[2] * 1000 + data_received[3] * 100 + data_received[4];
		
		if (status != TWI_SUCCESS) { print_dbg("Read test:\tFAIL\r\n"); }
		else {print_dbg("Read test:\tSUCCES\r\n");}
	}
}
void Write_EEPROM(void){
	opt.chip = 0x57;
	twi_master_init(&AVR32_TWI, &opt);
	
	U8 write_pattern[] =  { ((time_uitgang[output_nmb] / 3600000) % 24), ((time_uitgang[output_nmb] / 60000) % 60), ((time_uitgang[output_nmb] / 1000) % 60), (time_uitgang[output_nmb] % 1000 / 100), (time_uitgang[output_nmb] % 1000 % 100) };
	
	packet_EEPROM_send.chip = 0x57;
	packet_EEPROM_send.addr_length = 3;
	packet_EEPROM_send.addr[0] = decimal_to_bcd(output_nmb);	
	packet_EEPROM_send.addr[1] = 0x00;
	packet_EEPROM_send.addr[2] = 0x00;
	packet_EEPROM_send.buffer = (void*) write_pattern;
	packet_EEPROM_send.length = 5;
	
	twi_master_write(&AVR32_TWI, &packet_EEPROM_send);
}
void Read_RTC(void){
	opt.chip = RTC_ADDRESS;
	twi_master_init(&AVR32_TWI, &opt);
	
	packet_received.chip = 0x68;
	packet_received.addr_length = sizeof (uint8_t);
	packet_received.length = 16;
	packet_received.addr[0] = 0x00;
	packet_received.buffer = &tbuffer;
	twi_master_read(&AVR32_TWI, &packet_received);
}
void Write_RTC(void){
	packet_received.chip = 0b01101000;
	packet_received.addr_length = sizeof (uint8_t);
	packet_received.length = 7;
	packet_received.addr[0] = 0x00;
	packet_received.buffer = &tbuffer;
	twi_master_read(&AVR32_TWI, &packet_received);
	
	int time_h = (bcd_to_decimal(tbuffer[2])) / 10 % 10;
	int time_hh = (bcd_to_decimal(tbuffer[2])) % 10;
	int time_m = (bcd_to_decimal(tbuffer[1])) / 10 % 10;
	int time_mm = (bcd_to_decimal(tbuffer[1])) % 10;
	int time_s = (bcd_to_decimal(tbuffer[0])) / 10 % 10;
	int time_ss = (bcd_to_decimal(tbuffer[0])) % 10;
	
	int time_day = bcd_to_decimal(tbuffer[3]);
	int time_d = (bcd_to_decimal(tbuffer[4])) / 10 % 10;
	int time_dd = (bcd_to_decimal(tbuffer[4])) % 10;
	int time_month = (bcd_to_decimal(tbuffer[5])) ;
	int time_y = (bcd_to_decimal(tbuffer[6])) / 10 % 10;
	int time_yy = (bcd_to_decimal(tbuffer[6])) % 10;
	
	if(joy_button == 1 && y == 2){
		switch(x){
			case 7:
			if(Potmeter_value() > 2){
				time_h = 2;
				if(time_hh > 3){
					time_hh = 3;
				}
			}
			else{
				time_h = Potmeter_value();
			}
			break;
			
			case 8:
			if(Potmeter_value() > 3 && time_h == 2){ time_hh = 3; }
			else{ time_hh = Potmeter_value(); }
			break;
			
			case 10:
			if(Potmeter_value() > 5){ time_m = 5; }
			else{ time_m = Potmeter_value(); }
			break;
			
			case 11:
			time_mm = Potmeter_value();
			break;
			
			case 13:
			if(Potmeter_value() > 5){ time_s = 5; }
			else{ time_s = Potmeter_value(); }
			break;
			
			case 14:
			time_ss = Potmeter_value();
			break;
		}
		tbuffer[2] = decimal_to_bcd(time_h * 10 + time_hh);
		tbuffer[1] = decimal_to_bcd(time_m * 10 + time_mm);
		tbuffer[0] = decimal_to_bcd(time_s * 10 + time_ss);
	}
	
	if(joy_button == 1 && y == 3){
		switch(x){
			case 7:
			if(Potmeter_value() > 7){ time_day = 7; }
			else if(Potmeter_value() == 0){ time_day = 1; }
			else{ time_day = Potmeter_value(); }
			break;
			
			case 9:
			if(Potmeter_value() == 0 && time_dd == 0){ time_dd = 1;}
			if(Potmeter_value() > 3){ time_d = 3; if(time_dd > 1 ){time_dd = 1;} }
			else{ time_d = Potmeter_value(); }
			break;
			
			case 10:
			if (Potmeter_value() == 0 && time_d == 0){time_dd = 1;}
			else if(time_d == 3 && Potmeter_value() > 1){ time_dd = 1; }
			else if(time_d == 3 && Potmeter_value() == 0){ time_dd = 0; }
			else{ time_dd = Potmeter_value(); }
			break;
			
			case 18:
			time_y = Potmeter_value();
			break;
			
			case 19:
			time_yy = Potmeter_value();
			break;
		}
		
		if(x > 11 && x < 15){
			time_month = Potmeter_value_Month();
		}
		
		tbuffer[3] = decimal_to_bcd(time_day);
		tbuffer[4] = decimal_to_bcd(time_d * 10 + time_dd);
		tbuffer[5] = decimal_to_bcd(time_month);
		tbuffer[6] = decimal_to_bcd(time_y * 10 + time_yy);
		
	}
	
	packet_send.chip = 0b01101000;
	packet_send.addr_length = sizeof (uint8_t);
	packet_send.length = 16;
	packet_send.addr[0] = 0x00;
	packet_send.buffer = &tbuffer;
	twi_master_write(&AVR32_TWI, &packet_send);
}

