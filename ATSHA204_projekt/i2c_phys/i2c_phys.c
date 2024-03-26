/*
 * i2c_phys.c
 *
 * Created: 3/26/2024 14:33:31
 *  Author: Student
 */ 

// ************* PHYSICAL, HARDWARE DEPENDENT LAYER *************

// INCLUDES

#include "i2c_phys.h"
#include <stdio.h>
#include <avr/io.h>
#include <util/twi.h>

// FUCTIONS

void i2c_enable(void)
{
	/* na cvikach jsme delali takhle
	TWSR = 0x00;
	TWBR = 0x02;
	TWCR = (1<<TWEN);
	*/
	
	// tohle by melo mit stejny efekt
	
	TWBR = ((uint8_t) (((double) F_CPU / I2C_CLOCK - 16.0) / 2.0 + 0.5)); // Set the baud rate
}

void i2c_disable(void)
{
	TWCR = 0; // disable TWI
}

uint8_t i2c_send_start(void)
{
	uint8_t timeout_counter = I2C_START_TIMEOUT;
	uint8_t i2c_status;

	TWCR = (1 << TWEN) | (1 << TWSTA) | (1 << TWINT);
	do {
		if (timeout_counter-- == 0) {
			return I2C_FUNCTION_RETCODE_TIMEOUT;
		}
	} while ((TWCR & (TWCR & (1 << TWINT))) == 0);

	i2c_status = TW_STATUS;
	if ((i2c_status != TW_START) && (i2c_status != TW_REP_START)) {
		return I2C_FUNCTION_RETCODE_COMM_FAIL;
	}
	
	return I2C_FUNCTION_RETCODE_SUCCESS;
}

uint8_t i2c_send_stop(void)
{
	uint8_t timeout_counter = I2C_STOP_TIMEOUT;

	TWCR = (1 << TWEN) | (1 << TWSTO) | (1 << TWINT);
	
	do {
		if (timeout_counter-- == 0) {
			return I2C_FUNCTION_RETCODE_TIMEOUT;
		}
	} while ((TWCR & (1 << TWSTO)) > 0);

	if (TW_STATUS == TW_BUS_ERROR) {
		return I2C_FUNCTION_RETCODE_COMM_FAIL;
	}

	return I2C_FUNCTION_RETCODE_SUCCESS;
}

uint8_t i2c_send_bytes(uint8_t count, uint8_t *data)
{
	uint8_t timeout_counter;
	uint8_t twi_status;

	for (uint8_t i = 0; i < count; i++) {
		TWDR = *data++;
		TWCR = (1 << TWEN) | (1 << TWINT);

		timeout_counter = I2C_BYTE_TIMEOUT;
		do {
			if (timeout_counter-- == 0)
			return I2C_FUNCTION_RETCODE_TIMEOUT;
		} while ((TWCR & (1 << TWINT)) == 0);

		twi_status = TW_STATUS;
		if ((twi_status != TW_MT_SLA_ACK)
		&& (twi_status != TW_MT_DATA_ACK)
		&& (twi_status != TW_MR_SLA_ACK))
		// return error if byte got nacked.
		return I2C_FUNCTION_RETCODE_NACK;
	}

	return I2C_FUNCTION_RETCODE_SUCCESS;
}

uint8_t i2c_receive_byte(uint8_t *data)
{
	uint8_t timeout_counter = I2C_BYTE_TIMEOUT;

	// Enable acknowledging data.
	TWCR = ((1 << TWEN) | (1 << TWINT) | (1 << TWEA));
	do {
		if (timeout_counter-- == 0)
			return I2C_FUNCTION_RETCODE_TIMEOUT;
	} while ((TWCR & (1 << TWINT)) == 0);

	if (TW_STATUS != TW_MR_DATA_ACK) {
		// Do not override original error.
		(void) i2c_send_stop();
		return I2C_FUNCTION_RETCODE_COMM_FAIL;
	}
	*data = TWDR;

	return I2C_FUNCTION_RETCODE_SUCCESS;
}

uint8_t i2c_receive_bytes(uint8_t count, uint8_t *data)
{
	uint8_t timeout_counter;
	
	// acknowledge all bytes except the last one
	
	for (uint8_t i = 0; i < count - 1; i++) {
		// Enable acknowledging data.
		TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA);
		timeout_counter = I2C_BYTE_TIMEOUT;
		do {
			if (timeout_counter-- == 0)
				return I2C_FUNCTION_RETCODE_TIMEOUT;
		} while ((TWCR & ((1 << TWINT))) == 0);

		if (TW_STATUS != TW_MR_DATA_ACK) {
			// Do not override original error.
			(void) i2c_send_stop();
			return I2C_FUNCTION_RETCODE_COMM_FAIL;
		}
		*data++ = TWDR;
	}

	// disable acknowledging data for the last byte
	TWCR = ((1 << TWEN) | (1 << TWINT));
	timeout_counter = I2C_BYTE_TIMEOUT;
	do {
		if (timeout_counter-- == 0)
			return I2C_FUNCTION_RETCODE_TIMEOUT;
	} while ((TWCR & (1 << TWINT)) == 0);

	if (TW_STATUS != TW_MR_DATA_NACK) {
		// do not override original error.
		(void) i2c_send_stop();
		return I2C_FUNCTION_RETCODE_COMM_FAIL;
	}
	*data = TWDR;

	return i2c_send_stop();
}
