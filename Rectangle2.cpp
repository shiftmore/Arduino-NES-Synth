#include <Rectangle2.h>

 

/*
 *	send the note data to the 2A03
 *	a quirk of the 2A03 is that the rect wave gen counter gets reset whenever a value is written to the high byte address register
 *	when this happens while a note is currently playing, there is a noticable click
 */
void Rectangle2::_setWavelength(uint16_t newWavelength, bool force){
		 
	if(force || _notesPressed == 0){  					// if there are no notes currently being played 
		_sendAddrData(0x04, _getWaveDataMessage());
		if(newWavelength != _wavelength) _sendAddrData(0x07, highByte(newWavelength));	// and set high byte directly
	}else{										// otherwise, the wave gen should already be running (we are ignoring velocity for now) and we need to use the frequency sweep to set the high byte to avoid the click..
		if(newWavelength > _wavelength){				// if new note is lower, we need to increase the note period
			for(int i=0; i<(highByte(newWavelength)-highByte(_wavelength)); i++){	
				_incrementHighByte();
			}
		}else{ 									// otherwise decrease the period
			for(int i=0; i<(highByte(_wavelength)-highByte(newWavelength)); i++){
				_decrementHighByte();
			}
		}
	}      
	
	_sendAddrData(0x06, lowByte(newWavelength));	// set low byte
	_sendAddrData(0x1F, 0x00); 					// end write

	_wavelength = newWavelength;						// update the current note
}

void Rectangle2::_sendWaveDataMessage(){
	_sendAddrData(0x04, _getWaveDataMessage());
	_sendAddrData(0x1F, 0x00);
}

/*
 *  http://forums.nesdev.com/viewtopic.php?f=2&t=231&p=99658#p99658
 *  Writing to the high byte address of either of the square wave generators will reset that wave's phase
 *  causing an audible click if the current note being played is cut off a the right (wrong) moment.
 *  writing the low byte does not have this effect, nor does applying a frequency shift instruction.
 *  This routine will effectively notch the high byte of the rectangle wave genarator period up one without resetting the phase
 *  by setting the low byte as close to the edge as possible (0x00 or 0xFF depending) and performing a freq shift for a single clock cycle.
 */
void Rectangle2::_incrementHighByte(){
	_sendAddrData(0x17,0x40); 						//reset frame counter in case it was about to clock
	_sendAddrData(0x06,0xFF);						//be sure low 8 bits of timer period are $FF
	_sendAddrData(0x05,0x87);						//sweep enabled, shift = 7 (1/128)
	_sendAddrData(0x17,0xC0);						//clock sweep immediately
	_sendAddrData(0x05,0x0F);						//disable sweep
}
 
/*
 *  The same as above, in the opposite direction.
 */ 
void Rectangle2::_decrementHighByte(){
	_sendAddrData(0x17,0x40); 						//reset frame counter in case it was about to clock           
	_sendAddrData(0x06,0x00); 						//be sure low 8 bits of timer period are $00
	_sendAddrData(0x05,0x8F); 						//sweep enabled, shift = 7 (1/128)
	_sendAddrData(0x17,0xC0); 						//clock sweep immediately
	_sendAddrData(0x05,0x0F);						//disable sweep
}

void Rectangle2::_stop(){
	//_sendAddrData(0x04, B00110000);  
	//_sendAddrData(0x1F, 0x00);           
}

uint8_t Rectangle2::_getWaveDataMessage(){ 
  	
	uint8_t data = B00110000 + _getVolume();
	//uint8_t data = B00100000 + _getVolume();

    if( _getDutyCycle() == 3 ) return data | B11000000;
    if( _getDutyCycle() == 2 ) return data | B10000000;
    if( _getDutyCycle() == 1 ) return data | B01000000;
    return data + B00000000;
    
}

void Rectangle2::_applyAttack(){ 
	if(_currentVolume < _volume){ 
		if(_cycleCheck(&_timer_applyAttack, _cycle_applyAttack)){
			_currentVolume++; 
			_sendWaveDataMessage(); 
		}
	}else{ 
		_noteState = NOTESTATE_SUSTAIN; 
		_timer_applyAttack=0;
	} 
}

void Rectangle2::_applyRelease(){ 
	if(_currentVolume > 0){ 
		if(_cycleCheck(&_timer_applyRelease, _cycle_applyRelease)){
			_currentVolume--; 
			_sendWaveDataMessage(); 
		}
	}else{ 
		_noteState = NOTESTATE_OFF; 
		_timer_applyRelease=0;
	} 
}
 