/*****************************************************************************
 *
 * \file
 *
 * \brief TWI driver for AVR32 UC3.
 *
 * This file defines a useful set of functions for TWI on AVR32 devices.
 *
 * Copyright (c) 2009-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 ******************************************************************************/
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */


#ifndef _TWI_H_
#define _TWI_H_

/**
 * \defgroup group_avr32_drivers_twi TWI - Two-Wire Interface
 *
 * Driver for the TWI (Two-Wire Interface). This driver provides access to the main features of the TWI controller.
 * The TWI interconnects components on a unique two-wire bus.
 * The TWI is programmable as a master or a slave with sequential or single-byte access.
 * Multiple master capability is supported.
 *
 * \{
 */

#include <avr32/io.h>

#include "compiler.h"
#include "conf_twi.h"

/*! \name Error Codes for the Module
 */
//! @{
#define TWI_SUCCESS              0
#define TWI_INVALID_ARGUMENT    -1
#define TWI_ARBITRATION_LOST    -2
#define TWI_NO_CHIP_FOUND       -3
#define TWI_RECEIVE_OVERRUN     -4
#define TWI_RECEIVE_NACK        -5
#define TWI_SEND_OVERRUN        -6
#define TWI_SEND_NACK           -7
#define TWI_BUSY                -8
//! @}


/*!
 * \brief Input parameters when initializing the twi module mode
 */
typedef struct
{
  //! The PBA clock frequency.
  unsigned long pba_hz;
  //! The baudrate of the TWI bus.
  unsigned long speed;
  //! The desired address.
  char chip;
} twi_options_t;

/*!
 * \brief Information concerning the data transmission
 */
typedef struct
{
  //! TWI chip address to communicate with.
  char chip;
  //! TWI address/commands to issue to the other chip (node).
  uint8_t addr[3];
  //! Length of the TWI data address segment (1-3 bytes).
  int addr_length;
  //! Where to find the data to be written.
  void *buffer;
  //! How many bytes do we want to write.
  unsigned int length;
} twi_package_t;

#ifndef AVR32_TWI_180_H_INCLUDED

/*!
 * \brief Pointer on TWI slave application routines
 */
typedef struct
{
  //! Routine to receive data from TWI master
  void (*rx)(U8);
  //! Routine to transmit data to TWI master
  U8 (*tx)(void);
  //! Routine to signal a TWI STOP
  void (*stop)(void);
} twi_slave_fct_t;

#endif


/*! \brief Enable Master Mode of the TWI.
 *
 * \param twi         Base address of the TWI instance.
 */
static inline void twi_master_enable(volatile avr32_twi_t *twi)
{
	twi->cr = AVR32_TWI_CR_MSEN_MASK;;
}

/*! \brief Disable Master Mode of the TWI.
 *
 * \param twi         Base address of the TWI instance.
 */
static inline void twi_master_disable(volatile avr32_twi_t *twi)
{
	twi->cr = AVR32_TWI_CR_MSDIS_MASK;;
}

/*!
 * \brief Initialize the twi master module
 *
 * \param twi   Base address of the TWI (i.e. &AVR32_TWI).
 * \param *opt  Options for initializing the twi module
 *              (see \ref twi_options_t)
 */
extern int twi_master_init(volatile avr32_twi_t *twi, const twi_options_t *opt);

#ifndef AVR32_TWI_180_H_INCLUDED

/*!
 * \brief Initialize the twi slave module
 *
 * \param twi   Base address of the TWI (i.e. &AVR32_TWI).
 * \param *opt  Options for initializing the twi module
 *              (see \ref twi_options_t)
 * \param *slave_fct  Pointer on application functions
 */
extern int twi_slave_init(volatile avr32_twi_t *twi, const twi_options_t *opt, const twi_slave_fct_t *slave_fct);

#endif

/*!
 * \brief Test if a chip answers for a given twi address
 *
 * \param twi        Base address of the TWI (i.e. &AVR32_TWI).
 * \param chip_addr  Address of the chip which is searched for
 * \return TWI_SUCCESS if a chip was found, error code otherwise
 */
extern int twi_probe(volatile avr32_twi_t *twi, char chip_addr);

/*!
 * \brief Disable all TWI interrupts
 *
 * \param twi        Base address of the TWI (i.e. &AVR32_TWI).
 *
 */
extern void twi_disable_interrupt(volatile avr32_twi_t *twi);

/*!
 * \brief Read multiple bytes from a TWI compatible slave device
 *
 * \param twi     Base address of the TWI (i.e. &AVR32_TWI).
 * \param package Package information and data
 *                (see \ref twi_package_t)
 * \return TWI_SUCCESS if all bytes were read, error code otherwise
 */
extern int twi_master_read(volatile avr32_twi_t *twi, const twi_package_t *package);

/*!
 * \brief Write multiple bytes to a TWI compatible slave device
 *
 * \param twi       Base address of the TWI (i.e. &AVR32_TWI).
 * \param *package  Package information and data
 *                  (see \ref twi_package_t)
 * \return TWI_SUCCESS if all bytes were written, error code otherwise
 */
extern int twi_master_write(volatile avr32_twi_t *twi, const twi_package_t *package);

/*!
 * \brief Write multiple bytes to a TWI compatible slave device. This function is not blocking.
 *
 * The function does not wait that all the bytes are written.
 *
 * \param twi       Base address of the TWI (i.e. &AVR32_TWI).
 * \param *package  Package information and data
 *                  (see \ref twi_package_t)
 * \return TWI_SUCCESS if all bytes were written, error code otherwise
 */
extern int twi_master_write_ex(volatile avr32_twi_t *twi, const twi_package_t *package);

/*!
 * \brief Test if a TWI read/write is pending.
 *
 * \return true if a write/read access is pending, false otherwise
 */
extern bool twi_is_busy(void);

/**
 * \}
 */

#endif  // _TWI_H_
