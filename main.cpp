#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "USART_Debug.h"
#include "USART.h"
#include "TWI.h"

void initClocks(){
	// disable register for osc. control
	CCP = CCP_IOREG_gc;
	// Use internal oscillator (2MHz)
	OSC.CTRL |= OSC_RC32MEN_bm;

	// Wait for internal oscillator to stabilise
	while ((OSC.STATUS & OSC_RC32MRDY_bm) == 0);

	// disable register for clock control
	CCP = CCP_IOREG_gc;
	// set clock to 32MHz
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
}

void restartInterrupts(){
	cli();
	PMIC.CTRL |= PMIC_HILVLEN_bm | PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();
}

int main(){

	initClocks();
	restartInterrupts();

	USART_Data uc0 = {};
	USART_Data uc1 = {};

	TWI_Data rtcData = {};
	rtcData.baud_khz = 400;
	rtcData.master_addr = 0x00;
	rtcData.maxDataLength = 64;
	rtcData.port = 0x00;
	rtcData.twi_port = &TWIC;

	TWI rtc = TWI(&rtcData);

	uc0.port = &PORTC;
	uc0.usart_port = &USARTC0;
	uc0.txPin = PIN3_bm;
	uc0.rxPin = PIN2_bm;
	uc0.baudRate = 9600;

	uc1.port = &PORTC;
	uc1.usart_port = &USARTC1;
	uc1.txPin = PIN7_bm;
	uc1.rxPin = PIN6_bm;
	uc1.baudRate = 9600;

	USART C0 = USART(&uc0, false);
	USART C1 = USART(&uc1, false);

	setDebugOutputPort(&USARTC0);

	//char tmp[80];
	printf("Hello world\n");
	//scanf("%s", tmp);
	//printf("Hi %s!\n", tmp);

	printf("Polling I2C bus...");
	int * i2cAddresses = rtc.pollBus();
	printf("Done.\n");
	/*for(int i = 0; i < 127; i++){
		printf("%d\n", i2cAddresses[i]);
	}*/

	while(true){
		//C0.PutChar(C1.GetChar());
		//__asm__ volatile("nop");
	}

	return 0;
}
