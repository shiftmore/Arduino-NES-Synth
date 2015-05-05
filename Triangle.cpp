#include <Triangle.h>
 
/*
 *	We don't need to do all the fancy moves on the tri wave. just set the freq.
 */
void Triangle::_setWavelength(uint16_t newWavelength){
	//if(_notesPressed == 0 || _wavelength != newWavelength){	// i don't think we need to make this check.. what situation would the same pitch come therough twice?
		if(_notesPressed == 0){  					// if there are no notes currently being played
			_sendAddrData(0x08, B11111111);			// turn square 1 wave to max volume, set duty cycle, turn off sweeps
			
		}

		//uint16_t detunedWL = (uint16_t)((int)newWavelength - (int)_getDetune());

		_sendAddrData(0x0B, highByte(newWavelength));	// and set high byte directly
		_sendAddrData(0x0A, lowByte(newWavelength));	// set low byte
		_sendAddrData(0x1F, 0x00); 					// end write
		_wavelength = newWavelength;						// update the current note
	//}	
}

void Triangle::_stop(){
	_sendAddrData(0x08, 0x00);  
	_sendAddrData(0x1F, 0x00);           
}

uint8_t Triangle::_getWaveDataMessage(){  
	return B11111111;
}

void Triangle::_sendWaveDataMessage(){
	_sendAddrData(0x08, B11111111);
	_sendAddrData(0x1F, 0x00);
}