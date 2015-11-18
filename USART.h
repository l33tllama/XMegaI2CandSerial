/*
 * USART.h
 *
 *  Created on: 20 Oct 2015
 *      Author: leo
 */

#ifndef USART_H_
#define USART_H_
#include <stdio.h>
#include <avr/io.h>

typedef struct USART_Data_Struct{
	PORT_t * port;
	USART_t * usart_port;
	int txPin;
	int rxPin;
	int baudRate;
} USART_Data;

class USART {
private:
	PORT_t * port;
	USART_t * usart_port;
	USART();
public:
	USART(USART_Data * usart_data, bool interrupt_en);
	int PutChar(int c);
	int GetChar();
	virtual ~USART();
};

#endif /* USART_H_ */
