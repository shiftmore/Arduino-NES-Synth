#include <Rectangle1.h>


/*
 *	send the note data to the 2A03
 *	a quirk of the 2A03 is that the rect wave gen counter gets reset whenever a value is written to the high byte address register
 *	when this happens while a note is currently playing, there is a noticable click
 */
void Rectangle1::_setWavelength(uint16_t newWavelength){
	
	uint16_t detunedWavelength = newWavelength;// - _getFineDetuneAmount(newWavelength);
	
	//uint16_t wordVal = word(tableHi[noteTableIndex],tableLo[noteTableIndex]);
    //uint16_t currentInterval =  wordVal - word(tableHi[noteTableIndex+12],tableLo[noteTableIndex+12]);
    //wordVal -= (uint16_t)((((float)(rect2Detune))/(float)127.0) * (float)currentInterval);     
    //sendAddrData(0x06, lowByte(wordVal));                 
    //sendAddrData(0x07, highByte(wordVal)); 


	if(_notesPressed == 0){  					// if there are no notes currently being played 
		_sendAddrData(0x00, _getWaveDataMessage());
		if(detunedWavelength != _wavelength) _sendAddrData(0x03, highByte(detunedWavelength));	// and set high byte directly only if the note has changed.. if we are in a long release, we don;t want to click..
	}else{										// otherwise, the wave gen should already be running (we are ignoring velocity for now) and we need to use the frequency sweep to set the high byte to avoid the click..
		if(detunedWavelength > _wavelength){				// if new note is lower, we need to increase the note period
			for(int i=0; i<(highByte(detunedWavelength)-highByte(_wavelength)); i++){	
				_incrementHighByte();
			}
		}else{ 									// otherwise decrease the period
			for(int i=0; i<(highByte(_wavelength)-highByte(detunedWavelength)); i++){
				_decrementHighByte();
			}
		}
	}      
	
	_sendAddrData(0x02, lowByte(detunedWavelength));	// set low byte
	_sendAddrData(0x1F, 0x00); 					// end write

	_wavelength = newWavelength;						// update the current note
}

void Rectangle1::_sendWaveDataMessage(){
	_sendAddrData(0x00, _getWaveDataMessage());
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
void Rectangle1::_incrementHighByte(){
	_sendAddrData(0x17,0x40);						//reset frame counter in case it was about to clock
	_sendAddrData(0x02,0xFF);						//be sure low 8 bits of timer period are $FF
	_sendAddrData(0x01,0x87);						//sweep enabled, shift = 7 (1/128)
	_sendAddrData(0x17,0xC0);						//clock sweep immediately
	_sendAddrData(0x01,0x0F);						//disable sweep
}

/*
 *  The same as above, in the opposite direction.
 */ 
void Rectangle1::_decrementHighByte(){
	_sendAddrData(0x17,0x40);						//reset frame counter in case it was about to clock           
	_sendAddrData(0x02,0x00);						//be sure low 8 bits of timer period are $00
	_sendAddrData(0x01,0x8F);						//sweep enabled, shift = 7 (1/128)
	_sendAddrData(0x17,0xC0);						//clock sweep immediately
	_sendAddrData(0x01,0x0F);						//disable sweep
}

void Rectangle1::_stop(){
	//_sendAddrData(0x00, B00110000);  
	//_sendAddrData(0x1F, 0x00);           
}



uint8_t Rectangle1::_getWaveDataMessage(){ 

    uint8_t data = B00110000 + _getVolume();
	//uint8_t data = B00100000 + _getVolume();

    if( _getDutyCycle() == 3 ) return data | B11000000;
    if( _getDutyCycle() == 2 ) return data | B10000000;
    if( _getDutyCycle() == 1 ) return data | B01000000;
    return data + B00000000;
  
 }


