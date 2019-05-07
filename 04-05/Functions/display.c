/*
 * display.c
 *
 * Created: 03/05/2019 15:19:37
 *  Author: Margreet
 */ 

#include "display.h"
#include <asf.h>

void Init_display()
{
	gpio_enable_module(DIP204_SPI_GPIO_MAP, 4);
	
	//SPI Display init
	spi_initMaster(DIP204_SPI, &SPI_OPTIONS);
	spi_selectionMode(DIP204_SPI, 0, 0, 0);
	spi_setupChipReg(DIP204_SPI, &SPI_OPTIONS, FOSC0);
	spi_enable(DIP204_SPI);
	dip204_init(backlight_PWM, true);
}

void display(int pos_x, int pos_y, char* string){
	dip204_set_cursor_position(pos_x,pos_y);
	dip204_write_string(string);
}
void display_time(void){
	for(int pos = 1 ; pos < 5 ; pos++){		//pos is het poort_nummer
		dip204_set_cursor_position(1,pos);
		dip204_write_string("Poort  : --:--:--:--");
		dip204_set_cursor_position(7,pos);
		dip204_write_string(itoa(pos));
		
		dip204_set_cursor_position(10,pos);
		dip204_write_string(itoa((time_uitgang[pos] / 216000) % 24 / 10 % 10));
		dip204_set_cursor_position(11,pos);
		dip204_write_string(itoa((time_uitgang[pos] / 216000) % 24 % 10));
		dip204_set_cursor_position(13,pos);
		dip204_write_string(itoa((time_uitgang[pos] / 3600) % 60 / 10 % 10));
		dip204_set_cursor_position(14,pos);
		dip204_write_string(itoa((time_uitgang[pos] / 3600) % 60 % 10));
		dip204_set_cursor_position(16,pos);
		dip204_write_string(itoa((time_uitgang[pos] / 60) % 60 / 10 % 10));
		dip204_set_cursor_position(17,pos);
		dip204_write_string(itoa((time_uitgang[pos] / 60) % 60 % 10));
		dip204_set_cursor_position(19,pos);
		dip204_write_string(itoa(time_uitgang[pos] % 60 / 10 % 10));
		dip204_set_cursor_position(20,pos);
		dip204_write_string(itoa(time_uitgang[pos] % 60 % 10));
	}
}
void set_time(void){
	int time_20 = time_uitgang[output_nmb] % 60 % 10;
	int time_19 = time_uitgang[output_nmb] % 60 / 10 % 10;
	int time_17 = (time_uitgang[output_nmb] / 60) % 60 % 10;
	int time_16 = (time_uitgang[output_nmb] / 60) % 60 / 10 % 10;
	int time_14 = (time_uitgang[output_nmb] / 3600) % 60 % 10;
	int time_13 = (time_uitgang[output_nmb] / 3600) % 60 / 10 % 10;
	int time_11 = (time_uitgang[output_nmb] / 216000) % 24 % 10;
	int time_10 = (time_uitgang[output_nmb] / 216000) % 24 / 10 % 10;
	
	display(7,3, itoa(output_nmb));
	display(10,3, itoa((time_uitgang[output_nmb] / 216000) % 24 / 10 % 10));
	display(11,3, itoa((time_uitgang[output_nmb] / 216000) % 24 % 10));
	display(13,3, itoa((time_uitgang[output_nmb] / 3600) % 60 / 10 % 10));
	display(14,3, itoa((time_uitgang[output_nmb] / 3600) % 60 % 10));
	display(16,3, itoa((time_uitgang[output_nmb] / 60) % 60 / 10 % 10));
	display(17,3, itoa((time_uitgang[output_nmb] / 60) % 60 % 10));
	display(19,3, itoa(time_uitgang[output_nmb] % 60 / 10 % 10));
	display(20,3, itoa(time_uitgang[output_nmb] % 60 % 10));
	
	if(joy_button == 1 && y == 3 && x == 7){ output_nmb = (Potmeter_value()/3%4 + 1); }
	else if(joy_button == 1 && y == 3 && x > 9){
		switch(x){
			case 10:
			if(Potmeter_value() > 2){ time_10 = 2; time_11 = 3;}
			else{ time_10 = Potmeter_value();}
			break;
			case 11:
			if(Potmeter_value() > 3 && time_10 == 2){ time_11 = 3; }
			else{ time_11 = Potmeter_value();}
			break;
			case 13:
			if(Potmeter_value() > 5){ time_13 = 5; }
			else{ time_13 = Potmeter_value();}
			break;
			case 14:
			time_14 = Potmeter_value();
			break;
			case 16:
			if(Potmeter_value() > 5){ time_16 = 5; }
			else{ time_16 = Potmeter_value(); }
			break;
			case 17:
			time_17 = Potmeter_value();
			break;
			case 19:
			if(Potmeter_value() > 5){ time_19 = 5; }
			else{ time_19 = Potmeter_value(); }
			break;
			case 20:
			time_20 = Potmeter_value();
			break;
		}
		time_uitgang[output_nmb] = (time_10 * 10 + time_11) * 60 * 60 * 60 + (time_13 * 10 + time_14) * 60 * 60 + (time_16 * 10 + time_17) * 60 + (time_19 * 10 + time_20);
	}
}

void Number_to_Month(int Number){
	switch(Number){
		case 1:
		display(12,3,"JAN");
		break;
		case 2:
		display(12,3,"FEB");
		break;
		case 3:
		display(12,3,"MAR");
		break;
		case 4:
		display(12,3,"APR");
		break;
		case 5:
		display(12,3,"MAY");
		break;
		case 6:
		display(12,3,"JUN");
		break;
		case 7:
		display(12,3,"JUL");
		break;
		case 8:
		display(12,3,"AUG");
		break;
		case 9:
		display(12,3,"SEP");
		break;
		case 10:
		display(12,3,"OCT");
		break;
		case 11:
		display(12,3,"NOV");
		break;
		case 12:
		display(12,3,"DEC");
		break;
	}
}
void Number_to_DAY(int Number){
	switch(Number){
		case 1:
		display(11,4,"Monday :(");
		break;
		case 2:
		display(11,4,"Tuesday");
		break;
		case 3:
		display(11,4,"Wednesday");
		break;
		case 4:
		display(11,4,"Thursday");
		break;
		case 5:
		display(11,4,"Friday");
		break;
		case 6:
		display(11,4,"Saturday");
		break;
		case 7:
		display(11,4,"Sunday :)");
		break;
	}
}

char bcd_to_decimal(char d){
	return ((d & 0x0F) + (((d & 0xF0) >> 4) * 10));
}
char decimal_to_bcd(char d){
	return (((d / 10) << 4) & 0xF0) | ((d % 10) & 0x0F);
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