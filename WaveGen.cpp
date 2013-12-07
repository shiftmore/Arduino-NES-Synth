f#include <WaveGen.h>
#include <SPI.h> 

//These tables were taken from http://www.freewebs.com/the_bott/NotesTableNTSC.txt
const uint8_t _noteTableHi[] = 
{0x07,0x07,0x07,0x06,0x06,0x05,0x05,0x05,0x05,0x04,0x04,0x04,0x03,0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//{0x07,0x07,0x07,0x06,0x06,0x05,0x05,0x05,0x05,0x04,0x04,0x04,0x03,0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const uint8_t _noteTableLo[] = 
{0xF1,0x80,0x13,0xAD,0x4D,0xF3,0x9D,0x4D,0x00,0xB8,0x75,0x35,0xF8,0xBF,0x89,0x56,0x26,0xF9,0xCE,0xA6,0x80,0x5C,0x3A,0x1A,0xFC,0xDF,0xC4,0xAB,0x93,0x7C,0x67,0x52,0x3F,0x2D,0x1C,0x0C,0xFD,0xEF,0xE2,0xD5,0xC9,0xBD,0xB3,0xA9,0x9F,0x96,0x8E,0x86,0x7E,0x77,0x70,0x6A,0x64,0x5E,0x59,0x54,0x4F,0x4B,0x46,0x42,0x3F,0x3B,0x38,0x34,0x31,0x2F,0x2C,0x29,0x27,0x25,0x23,0x21,0x1F,0x1D,0x1B,0x1A,0x18,0x17,0x15,0x14,0x13,0x12,0x11,0x10,0x0F,0x0E,0x0D};
//{0xF1,0x80,0x13,0xAD,0x4D,0xF3,0x9D,0x4D,0x00,0xB8,0x75,0x35,0xF8,0xBF,0x89,0x56,0x26,0xF9,0xCE,0xA6,0x7F,0x5C,0x3A,0x1A,0xFB,0xDF,0xC4,0xAB,0x93,0x7C,0x67,0x52,0x3F,0x2D,0x1C,0x0C,0xFD,0xEF,0xE2,0xD2,0xC9,0xBD,0xB3,0xA9,0x9F,0x96,0x8E,0x86,0x7E,0x77,0x70,0x6A,0x64,0x5E,0x59,0x54,0x4F,0x4B,0x46,0x42};





  

void WaveGen::init(){ 
	_noteState = NOTESTATE_OFF; 
	_timer_applyAttack = 0;
	_cycle_applyAttack = 1;

	_timer_applyRelease = 0;
	_cycle_applyRelease = 1;

	_currentNote = 43; //midway
	_wavelength = word(_getNoteHighByte(_currentNote),_getNoteLowByte(_currentNote));
 	
 	_currentBend = 0;
 	_notesPressed = 0; 
 	_fineDetune = 0;

 	_volume = 15;


}

//
//	write data byte to the specific 2A03 address
//
void WaveGen::_sendAddrData(uint8_t address, uint8_t data){   
	while(digitalRead(PIN_INTERRUPT) == HIGH);  
	while(digitalRead(PIN_INTERRUPT) == LOW);		// the interrupt worked well in a sketch, but got messy with a class. this works fine. 
													// we are just waiting for the rising edge on this pin which is our next opportunity to write.
   
	digitalWrite(PIN_LATCH, LOW);					// set latch LOW to enable write
	SPI.transfer(address);							// send address first
	SPI.transfer(data);								// then data..
	digitalWrite(PIN_LATCH, HIGH);					// set latch HIGH to end the write
}

/*
 *	handle MIDI key press
 */ 
void WaveGen::handleNoteOn(uint8_t channel, uint8_t pitch, uint8_t velocity){
	if(_notesPressed<QUEUE_SIZE){					// only accept additional notes if there's room in the queue
		_playNote(pitch);			// play it
		_pushNoteOnQueue(pitch); 					// keep track of it 
	}
}

/*
 *	handle MIDI key release
 */
void WaveGen::handleNoteOff(uint8_t channel, uint8_t note, uint8_t velocity){
	_removeNoteFromQueue(note); 					// pop the note off the queue
	if(_notesPressed==0){							// if there are no other notes to play, silence!
		_noteState = NOTESTATE_RELEASE;
		//_currentVolume = 0;
		_stop();            
	}else{											//	otherwise, play last note in queue
		_playNote(_getLastNoteInQueue()); 
	}
}

void WaveGen::handlePitchBend(uint8_t channel, int bend){
	if(_currentBend == bend) return; 
	if(bend == 0){
		if(_notesPressed > 0 ) {
			_setWavelength(word(_getNoteHighByte(_currentNote),_getNoteLowByte(_currentNote))); 
		} 
	}
	int currentBendDifference = bend - _currentBend; 
	//uint16_t currentInterval =  word(_getNoteHighByte(_currentNote-PITCHBENDRANGE),_getNoteLowByte(_currentNote-PITCHBENDRANGE)) - word(_getNoteHighByte(_currentNote+PITCHBENDRANGE),_getNoteLowByte(_currentNote+PITCHBENDRANGE));
	int bendDetune = (int)((((float)(currentBendDifference))/(float)8193.0) * ( (float)_wavelength*0.06*6 ));
	if(_notesPressed > 0 ) {
		_setWavelength(_wavelength - bendDetune); 
	}  
	_currentBend = bend;	
}





uint8_t WaveGen::_getNoteLowByte(uint8_t noteTableIdx){
	return _noteTableLo[noteTableIdx];
}
uint8_t WaveGen::_getNoteHighByte(uint8_t noteTableIdx){
	return _noteTableHi[noteTableIdx];
}


/*
 * Functions to maintain the note queue
 */
void WaveGen::_pushNoteOnQueue(uint8_t note){
	_noteQueue[_notesPressed] = note;
	_notesPressed++;
}
void WaveGen::_removeNoteFromQueue(uint8_t pitch){
	boolean found=false;
	for(int i=0; i<_notesPressed-1; i++){
		if(_noteQueue[i] == pitch){
			found=true;
		}
		if(found){
			_noteQueue[i] = _noteQueue[i+1]; 
		}
	}
	_noteQueue[_notesPressed-1] = 0;
	_notesPressed--;
}
uint8_t WaveGen::_getLastNoteInQueue(){
	return _noteQueue[_notesPressed-1]; 
}

void WaveGen::_playNote(uint8_t note){
	if(_noteState == NOTESTATE_OFF || _noteState == NOTESTATE_RELEASE){  
      _noteState = NOTESTATE_ATTACK; 
    } 
	_setWavelength(word(_getNoteHighByte(note),_getNoteLowByte(note)));
	_currentNote = note;
}

void WaveGen::_setDutyCycle(uint8_t dc){
	//_dutyCycleValue = dc;
	uint8_t duty = map(dc,0,127,0,3);
	if(duty != _dutyCycleValue){ 
		_dutyCycleValue = duty; 
		if(_notesPressed > 0) _sendWaveDataMessage();
	}
}

uint8_t WaveGen::_getDutyCycle(){
	return _dutyCycleValue;
}

void WaveGen::_setVolume(uint8_t v){ 
	uint8_t vol = map(v,0,127,0,15);
	if(vol != _volume){ 
		_volume = vol; 
		if(_notesPressed > 0) _sendWaveDataMessage();
	}
}

uint8_t WaveGen::_getVolume(){
	//return _volume;
	return _currentVolume;
}

void WaveGen::_setAttack(uint8_t a){ 
	unsigned long atk = 0;
	if(a > 0) atk = map(a,0,127,0,100000);
	//if(vol != _volume){ 
		_cycle_applyAttack = atk; 
	//	if(_notesPressed > 0) _sendWaveDataMessage();
	//}
}

/*
void WaveGen::_setFineDetune(uint8_t fd){ 
	int mapped_fd = fd;//map(fd,0,127,0,100);
	if(mapped_fd != _fineDetune){ 
		_fineDetune = mapped_fd; 
		if(_notesPressed > 0) _setWavelength(_wavelength);
	}
}

uint8_t WaveGen::_getFineDetune(){
	return _fineDetune;
}

uint16_t WaveGen::_getFineDetuneAmount(uint16_t wl){
	if(_getFineDetune() == 0) return 0;
	//uint16_t currentInterval =  word(_getNoteHighByte(_currentNote),_getNoteLowByte(_currentNote)) - word(_getNoteHighByte(_currentNote+1),_getNoteLowByte(_currentNote+1));
	//int d = (int)((((float)(_getFineDetune()))/(float)100.0) * (float)currentInterval);
	uint16_t d = (uint16_t)((((float)(_getFineDetune()))/(float)127.0) * ( (float)_wavelength*0.06*12 ));
	return d;
}
*/


// Only applies to rect gens, until we implement glide/arp/vibrato
void WaveGen::_handleNoteStates(){ 
  switch (_noteState) {
    //case NOTESTATE_TRANSITION_DECAY:
      	//applyTransDecay();
    //  	break;
    //case NOTESTATE_TRANSITION_ATTACK:
      	//applyTransAttack();
    //  	break;
    case NOTESTATE_RELEASE:
      	_applyRelease();  
      	break;
    case NOTESTATE_ATTACK:
      	_applyAttack(); 
      	break; 
  }
  
}
 
void WaveGen::_applyAttack(){ 
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

void WaveGen::_applyRelease(){ 
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

bool WaveGen::_cycleCheck(unsigned long *lastMicros, unsigned long cycle) {
	unsigned long currentMicros = micros();
	if(*lastMicros == 0 || currentMicros - *lastMicros >= cycle) {
		*lastMicros = currentMicros;
		return true;
	}
	else
		return false;
}
 
