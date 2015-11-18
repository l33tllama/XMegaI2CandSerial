/*
 * TWI.h
 *
 *  Created on: 21 Oct 2015
 *      Author: leo
 */

#ifndef TWI_H_
#define TWI_H_

#include <avr/io.h>

/*! Transaction result enumeration. */
typedef enum TWIM_RESULT_enum {
	TWIM_RESULT_UNKNOWN          = (0x00<<0),
	TWIM_RESULT_OK               = (0x01<<0),
	TWIM_RESULT_BUFFER_OVERFLOW  = (0x02<<0),
	TWIM_RESULT_ARBITRATION_LOST = (0x03<<0),
	TWIM_RESULT_BUS_ERROR        = (0x04<<0),
	TWIM_RESULT_NACK_RECEIVED    = (0x05<<0),
	TWIM_RESULT_FAIL             = (0x06<<0),
} TWIM_RESULT_t;

typedef enum TWIM_STATUS_enum {
	TWIM_STATUS_BUSY = (0x00<<0),
	TWIM_STATUS_READY = (0x01<<0)
} TWIM_STATUS_t;

typedef struct TWI_Data_struct{
	TWI_t * twi_port;
	PORT_t * port;
	uint8_t baud_khz;
	uint8_t maxDataLength;
	register8_t master_addr;
	register8_t result;
	register8_t status;
} TWI_Data;

class TWI {
private:
	inline register8_t getBaudVal(int baud);
	TWI_Data * twi_data;
public:
	TWI(TWI_Data * twi_data);
	void writeData(register8_t address, const char * data);
	char * readData(register8_t address);
	void setDataLength(uint8_t length);
	int * pollBus();

	virtual ~TWI();
};

#endif /* TWI_H_ */
