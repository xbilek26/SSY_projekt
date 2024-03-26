/*
 * sha204_i2c.c
 *
 * Created: 3/26/2024 14:20:49
 *  Author: Student
 */ 

// ************* PHYSICAL, HARDWARE INDEPENDENT LAYER *************

// INCLUDES

#include <avr/io.h>
#include "sha204_i2c.h"

// DEFINES

#define SHA204_I2C_DEFAULT_ADDRESS   ((uint8_t) 0xC8)

// ENUMS

enum i2c_word_address {
	SHA204_I2C_PACKET_FUNCTION_RESET,  // reset device (hodnota 0)
	SHA204_I2C_PACKET_FUNCTION_SLEEP,  // put device into Sleep mode (hodnota 1)
	SHA204_I2C_PACKET_FUNCTION_IDLE,   // put device into Idle mode (hodnota 2)
	SHA204_I2C_PACKET_FUNCTION_NORMAL  // write / evaluate data that follow this word address byte (hodnota 3)
};

enum i2c_read_write_flag {
	I2C_WRITE = (uint8_t) 0x00,  // write command flag
	I2C_READ  = (uint8_t) 0x01   // read command flag
};

enum i2c_read_write_flag {
	I2C_WRITE = (uint8_t) 0x00,  //!< write command flag
	I2C_READ  = (uint8_t) 0x01   //!< read command flag
};

static uint8_t device_address;

// FUNCTIONS

void sha204p_set_device_id(uint8_t id)
{
	device_address = id;
}

void sha204p_init(void)
{
	i2c_enable();
	device_address = SHA204_I2C_DEFAULT_ADDRESS;
}

uint8_t sha204p_wakeup(void)
{
	// Generate wakeup pulse by writing a 0 on the I2C bus.
	uint8_t dummy_byte = 0;
	uint8_t i2c_status = i2c_send_start();
	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS)
		return SHA204_COMM_FAIL;

	// To send eight zero bits it takes 10E6 / I2C clock * 8 us.
	delay_10us(SHA204_WAKEUP_PULSE_WIDTH - (uint8_t) (1000000.0 / 10.0 / I2C_CLOCK * 8.0)); // DOD?LAT DELAY (https://github.com/thiseldo/cryptoauth-arduino/blob/master/src/common-atmel/timer_utilities.c#L112)

	// We have to send at least one byte between an I2C Start and an I2C Stop.
	(void) i2c_send_bytes(1, &dummy_byte);
	i2c_status = i2c_send_stop();
	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS) {
		return SHA204_COMM_FAIL;
	}

	delay_ms(SHA204_WAKEUP_DELAY); // DOD?LAT DELAY

	return SHA204_SUCCESS;
}

static uint8_t sha204p_send_slave_address(uint8_t read)
{
	uint8_t sla = device_address | read;
	uint8_t ret_code = i2c_send_start();
	if (ret_code != I2C_FUNCTION_RETCODE_SUCCESS)
		return ret_code;

	ret_code = i2c_send_bytes(1, &sla);

	if (ret_code != I2C_FUNCTION_RETCODE_SUCCESS)
		(void) i2c_send_stop();

	return ret_code;
}

static uint8_t sha204p_i2c_send(uint8_t word_address, uint8_t count, uint8_t *buffer)
{
	uint8_t i2c_status = sha204p_send_slave_address(I2C_WRITE);
	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS)
		return SHA204_COMM_FAIL;

	i2c_status = i2c_send_bytes(1, &word_address);
	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS)
		return SHA204_COMM_FAIL;

	if (count == 0) {
		// We are done for packets that are not commands (Sleep, Idle, Reset).
		(void) i2c_send_stop();
		return SHA204_SUCCESS;
	}

	i2c_status = i2c_send_bytes(count, buffer);

	(void) i2c_send_stop();

	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS)
		return SHA204_COMM_FAIL;
	else
		return SHA204_SUCCESS;
}

uint8_t sha204p_send_command(uint8_t count, uint8_t *command)
{
	return sha204p_i2c_send(SHA204_I2C_PACKET_FUNCTION_NORMAL, count, command);
}

uint8_t sha204p_idle(void)
{
	return sha204p_i2c_send(SHA204_I2C_PACKET_FUNCTION_IDLE, 0, NULL);
}

uint8_t sha204p_sleep(void)
{
	return sha204p_i2c_send(SHA204_I2C_PACKET_FUNCTION_SLEEP, 0, NULL);
}

uint8_t sha204p_reset_io(void)
{
	return sha204p_i2c_send(SHA204_I2C_PACKET_FUNCTION_RESET, 0, NULL);
}

uint8_t sha204p_receive_response(uint8_t size, uint8_t *response)
{
	uint8_t count;

	// Address the device and indicate that bytes are to be read.
	uint8_t i2c_status = sha204p_send_slave_address(I2C_READ);
	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS) {
		// Translate error so that the Communication layer
		// can distinguish between a real error or the
		// device being busy executing a command.
		if (i2c_status == I2C_FUNCTION_RETCODE_NACK)
			i2c_status = SHA204_RX_NO_RESPONSE;

		return i2c_status;
	}

	// Receive count byte.
	i2c_status = i2c_receive_byte(response);
	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS)
		return SHA204_COMM_FAIL;

	count = response[SHA204_BUFFER_POS_COUNT];
	if ((count < SHA204_RSP_SIZE_MIN) || (count > size)) {
		(void) i2c_send_stop();
		return SHA204_INVALID_SIZE;
	}

	i2c_status = i2c_receive_bytes(count - 1, &response[SHA204_BUFFER_POS_DATA]);

	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS)
		return SHA204_COMM_FAIL;
	else
		return SHA204_SUCCESS;
}

uint8_t sha204p_resync(uint8_t size, uint8_t *response)
{
	uint8_t nine_clocks = 0xFF;
	uint8_t ret_code = i2c_send_start();

	// Do not evaluate the return code that most likely indicates error,
	// since nine_clocks is unlikely to be acknowledged.
	(void) i2c_send_bytes(1, &nine_clocks);

	// Send another Start. The function sends also one byte,
	// the I2C address of the device, because I2C specification
	// does not allow sending a Stop right after a Start condition.
	ret_code = sha204p_send_slave_address(I2C_READ);

	// Send only a Stop if the above call succeeded.
	// Otherwise the above function has sent it already.
	if (ret_code == I2C_FUNCTION_RETCODE_SUCCESS)
		ret_code = i2c_send_stop();

	// Return error status if we failed to re-sync.
	if (ret_code != I2C_FUNCTION_RETCODE_SUCCESS)
		return SHA204_COMM_FAIL;

	// Try to send a Reset IO command if re-sync succeeded.
	return sha204p_reset_io();
}
