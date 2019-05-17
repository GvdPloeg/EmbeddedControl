/*
 * display.h
 *
 * Created: 03/05/2019 15:19:45
 *  Author: Margreet
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "RTC.h"

static const gpio_map_t DIP204_SPI_GPIO_MAP = {{DIP204_SPI_SCK_PIN,  DIP204_SPI_SCK_FUNCTION }, {DIP204_SPI_MISO_PIN, DIP204_SPI_MISO_FUNCTION}, {DIP204_SPI_MOSI_PIN, DIP204_SPI_MOSI_FUNCTION}, {DIP204_SPI_NPCS_PIN, DIP204_SPI_NPCS_FUNCTION}};
static const spi_options_t SPI_OPTIONS = {.reg = DIP204_SPI_NPCS,.baudrate = 120000,.bits = 8,.spck_delay = 0,.trans_delay = 0,.stay_act = 1,.spi_mode = 0,.modfdis = 1};

void Init_display(void);

void display(int, int, char*);
void display_time(void);
void set_time(void);

void Number_to_Month(int Number);
void Number_to_DAY(int Number);

char* itoa(int);
char bcd_to_decimal(char);
char decimal_to_bcd(char);

#endif /* DISPLAY_H_ */