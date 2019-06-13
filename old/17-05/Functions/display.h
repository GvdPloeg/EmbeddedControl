/*
 * display.h
 *
 * Created: 07/05/2019 20:32:09
 *  Author: Margreet
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <asf.h>
#include "RTC.h"
#include "interrupts.h"

static const spi_options_t spi_options = {.reg = DIP204_SPI_NPCS,.baudrate = 120000,.bits = 8,.spck_delay = 0,.trans_delay = 0,.stay_act = 1,.spi_mode = 0,.modfdis = 1};
static const gpio_map_t DIP204_SPI_GPIO_MAP = {
	{DIP204_SPI_SCK_PIN,  DIP204_SPI_SCK_FUNCTION },  // SPI Clock.
	{DIP204_SPI_MISO_PIN, DIP204_SPI_MISO_FUNCTION},  // MISO.
	{DIP204_SPI_MOSI_PIN, DIP204_SPI_MOSI_FUNCTION},  // MOSI.
	{DIP204_SPI_NPCS_PIN, DIP204_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
};

void Init_display(void);
void display_cases(void);
void display_update(void);

void set_display(int, int, char*);
void display_time(void);
void set_time(void);

void Number_to_Month(int Number);
void Number_to_DAY(int Number);

char bcd_to_decimal(char);
char decimal_to_bcd(char);
char* itoa(int);

#endif /* DISPLAY_H_ */