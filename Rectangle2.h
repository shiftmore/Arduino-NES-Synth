#ifndef LIB_RECTANGLE2_H_
#define LIB_RECTANGLE2_H_
 
#include <Arduino.h>  
#include <WaveGen.h>
 
class Rectangle2 : public WaveGen{
	public:     
		
	private: 
 		virtual void _setWavelength(uint16_t);
 		virtual void _stop(); 
 		virtual uint8_t _getWaveDataMessage();
 		virtual void _sendWaveDataMessage(); 

		void _incrementHighByte();
		void _decrementHighByte(); 
	};

#endif
