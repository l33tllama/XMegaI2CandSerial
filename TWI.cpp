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
#include <avr/io.h>


 // life saving code - https://github.com/alexforencich/xgrid/blob/master/firmware/xmega/i2c.cpp
 
TWI::TWI(TWI_Data * twi_data) {
	
	printf("Starting setting up an i2c port..\n");
	
	this->twi_data = twi_data;
	port = twi_data->port;
	
	twim_status = TWIM_STATUS_BUSY;
	
	// check if port not set - default to TWIC
	if(twi_data->twi_port != NULL){
		twi_port = twi_data->twi_port;
	} else {
		twi_port = &TWIC;
	}
	
	// Set up TWI port (address, status, CTRL, baud)
	twi_port->MASTER.ADDR = twi_data->master_addr;
	twi_port->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
	twi_port->CTRL |= TWI_SDAHOLD_400NS_gc;
	twi_port->MASTER.BAUD = getBaudVal((int)twi_data->baud_hz);

	// Enable TWI on whatever port
	// TODO: Interrupts
	twi_port->MASTER.CTRLA = TWI_MASTER_INTLVL_OFF_gc | TWI_MASTER_ENABLE_bm; // | TWI_MASTER_RIEN_bm | TWI_MASTER_WIEN_bm;
	twim_status = TWIM_STATUS_READY;
	
	//twi_port->MASTER.CTRLB = 
	//printf("Finished setting up an i2c port\n");
}
inline register8_t TWI::getBaudVal(int baud){
	register8_t baudval = (F_CPU / (2 * baud)) - 5;
	printf("Baud val: %d\n", baudval);
	return baudval;
}

void TWI::setDataLength(uint8_t length){
	twi_data->master_addr = length;
}

void TWI::begin(register8_t address){
	// Just enable the TWI for now
	//this->twi_data->twi_port->MASTER.CTRLA = TWI_MASTER_ENABLE_bm | TWI_MASTER_CMD_;
}

void TWI::end(){
	this->twi_data->twi_port->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
}

void TWI::checkTWIStatus(){
	if(twi_port->MASTER.STATUS > 0){
		
		register8_t master_status = twi_port->MASTER.STATUS;
		if (master_status & TWI_MASTER_BUSERR_bm){
			printf(" bus error");
		}
		if(master_status & TWI_MASTER_ARBLOST_bm){
			printf(" arbitration lost");
		}
		if(master_status & TWI_MASTER_TIMEOUT0_bm){
			printf(" timeout 0");
		}
		if(master_status & TWI_MASTER_TIMEOUT1_bm){
			printf(" timeout 1");
		}
		if(master_status & TWI_MASTER_RIF_bm){
			printf(" read IF ..?");
		}
		if(master_status & TWI_MASTER_WIF_bm){
			printf(" write IF..?");
		}
		if(master_status & TWI_MASTER_CLKHOLD_bm){
			printf(" clock hold..?");
		}
		if(master_status & TWI_MASTER_RXACK_bm){
			printf(" rx ack!");
		}
		if(master_status & TWI_MASTER_BUSSTATE0_bm){
			printf(" bus state 0..?");
		}
		if(master_status & TWI_MASTER_BUSSTATE1_bm){
			printf(" bus state 1..?");
		}
		printf("\n");
	}
}

void TWI::beginWrite(register8_t address){
	if(twim_status == TWIM_STATUS_READY){
		// Test if address exists..
		
		// start the TWI transaction (no action required..?)
		twi_port->MASTER.CTRLC = TWI_MASTER_CMD_NOACT_gc;
		
		// ADDR is slave address!
		twi_port->MASTER.ADDR = (register8_t) (address << 1) | 0x00;
		
		// Asking for a response
		twi_port->MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
		
		// Wait until this transaction is complete
		while(!(twi_port->MASTER.STATUS & TWI_MASTER_WIF_bm)){	}
		//printf("a:%3d - s:%2d - d:%2d\n", address << 0, twi_port->MASTER.STATUS, twi_port->MASTER.DATA);
		
		// See if we got a RXACK!
		if((twi_port->MASTER.STATUS & TWI_MASTER_RXACK_bm)){
			printf("Error! Device not found at address %x\n", address);
		}
		// End transaction
		//twi_port->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		
		twim_status = TWIM_STATUS_BEGIN_WRITE;
	} else {
		printf("Error beginning write - TWI bus not ready.");
	}
	
}

void TWI::beginRead(register8_t address){
	if(twim_status == TWIM_STATUS_READY){
		// Test if address exists..
		
		// start the TWI transaction (no action required..?)
		twi_port->MASTER.CTRLC = TWI_MASTER_CMD_NOACT_gc;
		
		// ADDR is slave address!
		twi_port->MASTER.ADDR = (register8_t) (address << 1) | 0x01;
		
		// Asking for a response
		twi_port->MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
		
		// Wait until this transaction is complete
		while(!(twi_port->MASTER.STATUS & TWI_MASTER_RIF_bm)){	}
		//printf("a:%3d - s:%2d - d:%2d\n", address << 0, twi_port->MASTER.STATUS, twi_port->MASTER.DATA);
		
		// See if we got a RXACK!
		if((twi_port->MASTER.STATUS & TWI_MASTER_RXACK_bm)){
			printf("Error! Device not found at address %x\n", address);
		}
		
		// End transaction
		//twi_port->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		twim_status = TWIM_STATUS_BEGIN_READ;
	} else {
		printf("Error beginning read - TWI bus not ready.");
	}
	
}

void TWI::putChar(char c){
	if(twim_status == TWIM_STATUS_BEGIN_WRITE){
		
		twi_port->MASTER.DATA = (uint8_t)c;
		twi_port->MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
		while(!(twi_port->MASTER.STATUS & TWI_MASTER_WIF_bm)){	}
			
	} else {
		printf("Error - Not ready to write.");
	}
}

char TWI::getChar(){
	char c = 0;
	if(twim_status == TWIM_STATUS_BEGIN_READ){
		
		twi_port->MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
		while(!(twi_port->MASTER.STATUS & TWI_MASTER_RIF_bm)){	}
		c = twi_port->MASTER.DATA;
	}
	return c;
}

void TWI::endTransmission(){
	if(twim_status == TWIM_STATUS_BEGIN_READ || twim_status == TWIM_STATUS_BEGIN_WRITE){
		twi_port->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		twim_status = TWIM_STATUS_READY;
	} else {
		printf("Error - trying to end i2c transmission while bus is busy.");
	}
	
}

void TWI::writeData(register8_t address, const char * data, int length){
	twi_port->MASTER.ADDR = (address << 1) | 0x00;
	
	twi_port->MASTER.CTRLC = TWI_MASTER_CMD_REPSTART_gc;
	for(int i = 0; i < length; i++){
		twi_port->MASTER.DATA = *(data++);
		twi_port->MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
		
		while(!(twi_port->MASTER.STATUS & TWI_MASTER_WIF_bm)){	}
			checkTWIStatus();
	}
	
	twi_port->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
	
	
	/*
	unsigned int dataSent = 0;
	unsigned int dataLength = 0;

	TWI_t * port = twi_data->twi_port;

	// set address with start bit
	//port->MASTER.ADDR = address & ~0x01;

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
	} */
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

register8_t * TWI::pollBus(){
	
	for(int i = 0;i < 127; i++){
		returnAddresses[i] = 0;
	}

	int8_t address;
	printf("Beginning I2C polling..\n");

	for(address = 0x00; address < 127; address++){
		
		// start the TWI transaction (no action required..?)
		twi_port->MASTER.CTRLC = TWI_MASTER_CMD_NOACT_gc;
		
		// ADDR is slave address!
		twi_port->MASTER.ADDR = (register8_t) (address << 1) | 0x00;
		
		// Asking for a response
		twi_port->MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
		
		// Wait until this transaction is complete
		while(!(twi_port->MASTER.STATUS & TWI_MASTER_WIF_bm)){	}
		//printf("a:%3d - s:%2d - d:%2d\n", address << 0, twi_port->MASTER.STATUS, twi_port->MASTER.DATA);
		
		// See if we got a RXACK!
		if(!(twi_port->MASTER.STATUS & TWI_MASTER_RXACK_bm)){
			//printf("Found a thing?? %x", address);
			// id so, add to array
			returnAddresses[address] = address;
		}
		
		// End transaction
		twi_port->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
	}

	return returnAddresses;
}

TWI::~TWI() {
	// TODO Auto-generated destructor stub
}

