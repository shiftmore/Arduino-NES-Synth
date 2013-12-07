#ifndef LIB_NES2A03_H_
#define LIB_NES2A03_H_

#include <Arduino.h>
#include <Rectangle1.h>
#include <Rectangle2.h>
#include <Triangle.h>

#define PIN_LATCH 10							//SPI Latch Pin  
#define PIN_PWR 3								//Pin to control power to 2A03 
#define PIN_RST 4								//Pin to reset 2A03
 

//#define QUEUE_SIZE 10

class NES2A03{
	public:
		void init(); 
		void run();
		Rectangle1 rectangle1;
		Rectangle2 rectangle2;
		Triangle triangle;
		
	private:
		
	};

#endif
