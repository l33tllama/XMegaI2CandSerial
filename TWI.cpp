/*
 * TWI.cpp
 *
 *  Created on: 21 Oct 2015
 *      Author: leo
 */
#define DEFAULT_DATA_LENGTH 128
#include "TWI.h"
#include <string.h>
#include <stdio.h>

//temp
#include <util/delay.h>

TWI::TWI(TWI_Data * twi_data) {
	this->twi_data = twi_data;

	TWI_t * twi_port = twi_data->twi_port;
	twi_port->CTRL |= TWI_SDAHOLD_400NS_gc;
	twi_port->MASTER.BAUD = getBaudVal(twi_data->baud_khz);

	// Just enable the TWI for now
	// TODO: Interrupts
	twi_port->MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
}

inline register8_t TWI::getBaudVal(int baud){
	return (F_CPU / (2 * baud)) - 5;
}

void TWI::setDataLength(uint8_t length){
	twi_data->master_addr = length;
}

void TWI::writeData(register8_t address, const char * data){
	unsigned int dataSent = 0;
	unsigned int dataLength = 0;

	TWI_t * port = twi_data->twi_port;

	// set address with start bit
	port->MASTER.ADDR = address & ~0x01;

	twi_data->status = TWIM_STATUS_BUSY;
	twi_data->result = TWIM_RESULT_UNKNOWN;

	dataLength = strlen(data);

	// if too much data, end
	if(dataLength > twi_data->maxDataLength){
		port->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		twi_data->result = TWIM_RESULT_BUFFER_OVERFLOW;
	} else {
	// otherwise, send ze datas!
		while(*(data) && dataSent < twi_data->maxDataLength){
			if(port->MASTER.STATUS & TWI_MASTER_RXACK_bm){
				port->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
				twi_data->result = TWIM_RESULT_NACK_RECEIVED;
				twi_data->status = TWIM_STATUS_READY;
			} else {
				port->MASTER.DATA = *(data++);
				//*(data++);
				dataSent++;
			}
		}
		// Finish and set statuses
		port->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		twi_data->result = TWIM_RESULT_OK;
		twi_data->status = TWIM_STATUS_READY;
	}
}

char * TWI::readData(register8_t address){
	unsigned int dataReceived = 0;
	TWI_t * port = twi_data->twi_port;
	char * data;

	// set address with start bit
	port->MASTER.ADDR = address | 0x01;

	while(*(data) && dataReceived < twi_data->maxDataLength){
		*(data++) = port->MASTER.DATA;
	}

	return data;
}

int * TWI::pollBus(){

	int * returnAddresses;
	//*(returnAddresses) = -1;

	int8_t address;
	TWI_t * port = twi_data->twi_port;
	printf("Beginning I2C polling..\n");
	for(address = 0; address < 127; address++){
		uint8_t addr = (address | 0x01);
		printf("Address: %d ", address);
		port->MASTER.ADDR = address;
		while(!(port->MASTER.STATUS & TWI_MASTER_WIF_bm));
		/*port->MASTER.DATA = 0x00;
		printf("status: %x, data: %d\n", port->MASTER.STATUS, port->MASTER.DATA); */

		port->MASTER.DATA = 0x01;       // write word addrpointer first
		printf("Sending read command\n");
		port->MASTER.ADDR = addr;    // send read command
		for(int i=0;i<7;i++){                  // read date and time
			while(!(port->MASTER.STATUS & TWI_MASTER_RIF_bm)){

				_delay_ms(20);
				break;
			}
			int data = port->MASTER.DATA;
			if(data >  0){
				printf("%d",data );
			}

		}
		printf("\n");

		port->MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;


	   /*
		int8_t addr = address & 0x01;

		port->MASTER.ADDR = addr;

		while(!(port->MASTER.STATUS & TWI_MASTER_WIF_bm));

		port->MASTER.DATA = 0x00;

		while (port->MASTER.DATA){
			printf("\d", port->MASTER.DATA);
		}

		printf("address: %d port status: %x: ", address, port->MASTER.STATUS);
		// If no ACK, skip to next address..
		if(port->MASTER.STATUS != TWI_MASTER_RXACK_bm){
			printf("No device at %d\n", (int)address);
		} else {
			// Woo! found something... Add to array
			printf("Found a device!!! %d\n", (int)address);
			*(returnAddresses++) = address;
		}
		*/
	}
	return returnAddresses;
}

TWI::~TWI() {
	// TODO Auto-generated destructor stub
}

