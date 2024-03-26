/*
 * i2c_phys.h
 *
 * Created: 3/26/2024 14:33:15
 *  Author: Student
 */ 

// ************* PHYSICAL, HARDWARE DEPENDENT LAYER *************

#ifndef PHYS_I2C_H_
#define PHYS_I2C_H_

// DEFINES

#define I2C_CLOCK  (400000.0)
#define I2C_BYTE_TIMEOUT  ((uint8_t) 200)
#define I2C_STOP_TIMEOUT  ((uint8_t) 250)
#define I2C_START_TIMEOUT  ((uint8_t) 250)

// PROTOTYPES

void i2c_enable(void);
void i2c_disable(void);
uint8_t i2c_send_start(void);
uint8_t i2c_send_stop(void);
uint8_t i2c_send_bytes(uint8_t count, uint8_t *data);
uint8_t i2c_receive_byte(uint8_t *data);
uint8_t i2c_receive_bytes(uint8_t count, uint8_t *data);

#endif /* PHYS_I2C_H_ */
