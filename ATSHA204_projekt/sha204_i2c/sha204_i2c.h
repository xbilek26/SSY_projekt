/*
 * sha204_i2c.h
 *
 * Created: 3/26/2024 14:21:00
 *  Author: Student
 */ 

// ************* PHYSICAL, HARDWARE INDEPENDENT LAYER *************


#ifndef I2C_H_
#define I2C_H_

//INCLUDES

#include <stdio.h>
#include "phys_i2c/phys_i2c.h"
#include "sha204/sha204.h"

// PROTOTYPES

void sha204p_set_device_id(uint8_t id);
void sha204p_init(void);
uint8_t sha204p_wakeup(void);
static uint8_t sha204p_send_slave_address(uint8_t read);
static uint8_t sha204p_i2c_send(uint8_t word_address, uint8_t count, uint8_t *buffer);
uint8_t sha204p_send_command(uint8_t count, uint8_t *command);
uint8_t sha204p_idle(void);
uint8_t sha204p_sleep(void);
uint8_t sha204p_reset_io(void);
uint8_t sha204p_receive_response(uint8_t size, uint8_t *response);
uint8_t sha204p_resync(uint8_t size, uint8_t *response);

#endif /* I2C_H_ */