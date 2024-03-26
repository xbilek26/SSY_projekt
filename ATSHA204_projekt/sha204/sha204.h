/*
 * sha204.h
 *
 * Created: 3/26/2024 14:46:18
 *  Author: Student
 */ 


#ifndef SHA204_H_
#define SHA204_H_

// DEFINES

#define SHA204_PACKET_FUNC_RESET  ((uint8_t) 0x00)
#define SHA204_PACKET_FUNC_SLEEP  ((uint8_t) 0x01)
#define SHA204_PACKET_FUNC_IDLE  ((uint8_t) 0x02)
#define SHA204_PACKET_FUNC_CMD  ((uint8_t) 0x03)

#define I2C_FUNCTION_RETCODE_SUCCESS  ((uint8_t) 0x00) // communication with device succeeded
#define I2C_FUNCTION_RETCODE_COMM_FAIL  ((uint8_t) 0xF0) // communication with device failed
#define I2C_FUNCTION_RETCODE_TIMEOUT  ((uint8_t) 0xF1) // communication timed out
#define I2C_FUNCTION_RETCODE_NACK  ((uint8_t) 0xF8) // TWI NACK


#endif /* SHA204_H_ */
