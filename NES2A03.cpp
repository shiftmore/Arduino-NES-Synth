#include <NES2A03.h>
#include <SPI.h> 
 
//
//	get everything set up to talk to the 2A03
//
void NES2A03::init(){
	SPI.begin();				// enable SPI communication
	
	pinMode(PIN_PWR, OUTPUT);		// set up pins
	pinMode(PIN_RST, OUTPUT);
	pinMode(PIN_LATCH, OUTPUT);
	pinMode(PIN_INTERRUPT, INPUT);
	
	digitalWrite(PIN_LATCH, LOW);		// populate the address and data busses with some harmless inital values
	SPI.transfer(0x15);
	SPI.transfer(0x0F); 
	digitalWrite(PIN_LATCH, HIGH);
	
	digitalWrite(PIN_PWR, LOW);		// Turn power to NES CPU off
	digitalWrite(PIN_RST, LOW);		// Bring /RST low
	delay(500);				// wait .5 seconds
	digitalWrite(PIN_PWR,HIGH);		// turn on NES CPU
	delay(500);				// wait .5 seconds
	digitalWrite(PIN_RST,HIGH);		// bring /RST pin high
  	
 	rectangle1.init();
	rectangle2.init();
	triangle.init();
 
}

void NES2A03::run(){
	rectangle1._handleNoteStates();
	rectangle2._handleNoteStates();
}

