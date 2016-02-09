#define F_CPU 3200000

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "USART_Debug.h"
#include "USART.h"
#include "TWI.h"


#define HT16K33_BLINK_CMD 0x80
#define HT16K33_BLINK_DISPLAYON 0x01
#define HT16K33_BLINK_OFF 0
#define HT16K33_BLINK_2HZ  1
#define HT16K33_BLINK_1HZ  2
#define HT16K33_BLINK_HALFHZ  3

#define HT16K33_CMD_BRIGHTNESS 0xE0

void initClocks(){	
	// Use internal oscillator (22MHz)
	OSC.CTRL |= OSC_RC32MEN_bm;

	// Wait for internal oscillator to stabilise
	while ((OSC.STATUS & OSC_RC32MRDY_bm) == 0);

	// disable register for clock control
	CCP = CCP_IOREG_gc;
	
	// set clock to 32MHz
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
	
	// disable default 2MHz internal osc.
	OSC.CTRL &= (~OSC_RC2MEN_bm);
	
	//enable 32khz calibrated internal osc
	OSC.CTRL |= OSC_RC32KEN_bm;
	while (!(OSC.STATUS & OSC_RC32KRDY_bm)); 
	
	// set bit to 0 to indicate we use the internal 32kHz
	// callibrated osc as auto-calibration source for
	// our 32MHz osc
	OSC.DFLLCTRL &= ~OSC_RC32MCREF0_bm;
	
	//enable auto-calibration for the 32MHz osc
	DFLLRC32M.CTRL |= DFLL_ENABLE_bm;
}

void initClocks2(){
	OSC.CTRL |= OSC_RC32MEN_bm | OSC_RC32KEN_bm;  /* Enable the internal 32MHz & 32KHz oscillators */
	while(!(OSC.STATUS & OSC_RC32KRDY_bm));       /* Wait for 32Khz oscillator to stabilize */
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));       /* Wait for 32MHz oscillator to stabilize */
	DFLLRC32M.CTRL = DFLL_ENABLE_bm ;             /* Enable DFLL - defaults to calibrate against internal 32Khz clock */
	CCP = CCP_IOREG_gc;                           /* Disable register security for clock update */
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;              /* Switch to 32MHz clock */
	OSC.CTRL &= ~OSC_RC2MEN_bm;                   /* Disable 2Mhz oscillator */
}

void restartInterrupts(){
	cli();
	PMIC.CTRL |= PMIC_HILVLEN_bm | PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();
}

int main(){

	initClocks2();
	restartInterrupts();
	_delay_ms(100);
	
	PORTE.DIR = 0xff;
	
	PORTE.OUT = 0x01;
	_delay_ms(100);
	PORTE.OUT = 0x00;
	_delay_ms(100);
	PORTE.OUT = 0x01;
	_delay_ms(100);
	PORTE.OUT = 0x00;
	_delay_ms(100);
	
	for(int i = 0; i < 10; i++){
		PORTE.OUTTGL = 0x01;
		_delay_ms(10);
	}
	for(int i = 0; i < 10; i++){
		PORTE.OUTTGL = 0x02;
		_delay_ms(10);
	}
	
	USART_Data uc0 = {};
	USART_Data uc1 = {};

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

	TWI_Data rtcData = {};
	rtcData.baud_hz = 400000L;
	rtcData.master_addr = 0x00;
	rtcData.maxDataLength = 64;
	rtcData.port = 0x00;
	rtcData.twi_port = &TWIE;

	TWI leds = TWI(&rtcData);
	char leds_addr = 0x70;

	_delay_ms(10);

	//char tmp[80];
	printf("Hello world\n");

	printf("Polling I2C bus...");
	register8_t * i2cAddresses;
	i2cAddresses = leds.pollBus();
	
	char i2c_data[64];//
	// = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	for(int i = 0; i < 64; i++){
		i2c_data[i] = (char)i;
	}
	//rtc.writeData(0x70, i2c_data, 64);
	
	printf("Done.\n");
	printf("LEDs might work, who knows..\n");
	
	for(int i = 0; i < 127; i++){
		if (i == 127){
			printf("%x\n", i2cAddresses[i]);
		}
		if (i % 25 == 0){
			printf("\n");
		}
		printf("%x, ", i2cAddresses[i]);
		
	} 
	printf("Turning on led matrix osc.\n");	
	// turn on oscillator
	
	leds.beginWrite(leds_addr);
	printf("Write begun..");
	leds.putChar(0x21);
	printf("Wrote a char");
	leds.endTransmission();
	
	printf("Setting led matrix blink rate\n");
	// set blink rate
	leds.beginWrite(leds_addr);
	leds.putChar(HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (HT16K33_BLINK_OFF << 1));
	leds.endTransmission();
	
	// set brightness (15 - max)
	leds.beginWrite(leds_addr);
	leds.putChar(HT16K33_CMD_BRIGHTNESS | 15);
	leds.endTransmission();
	
	uint8_t counter = 0;
	uint16_t displaybuffer[8]; 
	
	printf("LED init finished?\n");

	while(true){
		for (uint8_t i=0; i<8; i++) {
			// draw a diagonal row of pixels
			displaybuffer[i] = 1 << ((counter+i) % 16) | 1 << ((counter+i+8) % 16) ;
			//printf("%d\n", displaybuffer[i]);
		}
		//test write
		leds.beginWrite(leds_addr);
		leds.putChar((uint8_t) 0x00);
		
		for(uint8_t i=0; i < 8; i++){
			leds.putChar(displaybuffer[i] & 0xFF);
			leds.putChar(displaybuffer[i] >> 8);
		}
		leds.endTransmission();
		counter++;
		if (counter >= 16) counter = 0;
		_delay_ms(1000);
	}
	leds.end();
	return 0;
}
