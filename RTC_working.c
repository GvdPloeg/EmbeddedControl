#include <avr32/io.h>
#include "asf.h"
#include "evk1100.h"
#include "print_funcs.h"
#include "gpio.h"
#include "pm.h"
#include "twi.h"
#include "string.h"
#include "math.h"

//#define EEPROM_ADDRESS        0x57        // EEPROM's TWI address
#define RTC_ADDRESS			  0x68        // RTC's TWI address
#define EEPROM_ADDR_LGT       1    // Address length of the EEPROM memory
#define TWI_SPEED             FOSC32       // Speed of TWI

#define DATA_LENGTH			  3
#define Register_Seconds	  0x00
#define Register_Minutes	  0x01
#define Register_Hours		  0x02
#define Register_Days	      0x03

int DEC_to_HEX(int);

int main(void)
{
  static const gpio_map_t TWI_GPIO_MAP = {{AVR32_TWI_SDA_0_0_PIN, AVR32_TWI_SDA_0_0_FUNCTION},{AVR32_TWI_SCL_0_0_PIN, AVR32_TWI_SCL_0_0_FUNCTION}};
  twi_options_t opt;
  twi_package_t packet_received;
  int status;
   
  U8 data_received[DATA_LENGTH] = {0, 0, 0, 0};
	  
  pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);
  init_dbg_rs232(FOSC0);

  // Initialize and enable interrupt
  irq_initialize_vectors();
  cpu_irq_enable();

  //print_dbg("TWI Example\r\nSenpai!\r\n");

  // TWI gpio pins configuration
  gpio_enable_module(TWI_GPIO_MAP, 2);

  // options settings
  opt.pba_hz = FOSC0;
  opt.speed = TWI_SPEED;
  opt.chip = RTC_ADDRESS;

  // initialize TWI driver with options
  status = twi_master_init(&AVR32_TWI, &opt);
  // check init result
  if (status == TWI_SUCCESS)
  {
    // display test result to user
    //print_dbg("Probe test:\tPASS\r\n");
  }
  else
  {
    // display test result to user
    print_dbg("Probe test:\tFAIL\r\n");
  }

  packet_received.chip = RTC_ADDRESS;
  // Length of the TWI data address segment (1-3 bytes)
  packet_received.addr_length = EEPROM_ADDR_LGT;
  // How many bytes do we want to write
  packet_received.length = DATA_LENGTH;
  // TWI address/commands to issue to the other chip (node)
  packet_received.addr[0] = Register_Seconds;
  packet_received.addr[1] = Register_Minutes;
  packet_received.addr[2] = Register_Hours;
  //packet_received.addr[3] = Register_Days;
  packet_received.buffer = data_received;

  // perform a read access
  status = twi_master_read(&AVR32_TWI, &packet_received);	
	
  // check read result
  if (status == TWI_SUCCESS)
  {
	print_dbg("Time (hh:mm:ss): ");
	print_dbg_ulong(DEC_to_HEX(data_received[2]));
	print_dbg("\t");
	print_dbg_ulong(DEC_to_HEX(data_received[1]));
	print_dbg("\t");
	print_dbg_ulong(DEC_to_HEX(data_received[0]));
	print_dbg("\r\n");
	print_dbg("Day of week: ");
	//print_dbg_ulong(data_received[3]);
	print_dbg("\r\n");
  }
  else
  {
    print_dbg("Read test:\tFAIL\r\n");
  }
}

int DEC_to_HEX(int input)
{
	int hexadecimal_number, remainder, count = 0;
	for(count = 0; input > 0; count++)
	{
		remainder = input % 16;
		hexadecimal_number = hexadecimal_number + remainder * pow(10, count);
		input = input / 16;
	}
	return hexadecimal_number;
}