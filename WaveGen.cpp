#include <WaveGen.h>
//#include <SPI.h> 

//These tables were taken from http://www.freewebs.com/the_bott/NotesTableNTSC.txt
const uint8_t _noteTableHi[] = 
{0x07,0x07,0x07,0x06,0x06,0x05,0x05,0x05,0x05,0x04,0x04,0x04,0x03,0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//{0x07,0x07,0x07,0x06,0x06,0x05,0x05,0x05,0x05,0x04,0x04,0x04,0x03,0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const uint8_t _noteTableLo[] = 
{0xF1,0x80,0x13,0xAD,0x4D,0xF3,0x9D,0x4D,0x00,0xB8,0x75,0x35,0xF8,0xBF,0x89,0x56,0x26,0xF9,0xCE,0xA6,0x80,0x5C,0x3A,0x1A,0xFC,0xDF,0xC4,0xAB,0x93,0x7C,0x67,0x52,0x3F,0x2D,0x1C,0x0C,0xFD,0xEF,0xE2,0xD5,0xC9,0xBD,0xB3,0xA9,0x9F,0x96,0x8E,0x86,0x7E,0x77,0x70,0x6A,0x64,0x5E,0x59,0x54,0x4F,0x4B,0x46,0x42,0x3F,0x3B,0x38,0x34,0x31,0x2F,0x2C,0x29,0x27,0x25,0x23,0x21,0x1F,0x1D,0x1B,0x1A,0x18,0x17,0x15,0x14,0x13,0x12,0x11,0x10,0x0F,0x0E,0x0D};
//{0xF1,0x80,0x13,0xAD,0x4D,0xF3,0x9D,0x4D,0x00,0xB8,0x75,0x35,0xF8,0xBF,0x89,0x56,0x26,0xF9,0xCE,0xA6,0x7F,0x5C,0x3A,0x1A,0xFB,0xDF,0xC4,0xAB,0x93,0x7C,0x67,0x52,0x3F,0x2D,0x1C,0x0C,0xFD,0xEF,0xE2,0xD2,0xC9,0xBD,0xB3,0xA9,0x9F,0x96,0x8E,0x86,0x7E,0x77,0x70,0x6A,0x64,0x5E,0x59,0x54,0x4F,0x4B,0x46,0x42};


  

void WaveGen::init(){ 
	pinMode(PIN_INTERRUPT,INPUT);

	_noteState = NOTESTATE_OFF; 
	_timer_applyAttack = 0;
	_cycle_applyAttack = 1;

	_timer_applyRelease = 0;
	_cycle_applyRelease = 1;

	_timer_applyTremalo = 0;
	_cycle_applyTremalo = 1000;

	_tremaloDepth = 5;
	_tremaloWaveForm = TREMALOWAVEFORM_SINE;

	_noteOffset = 0;
	_currentNote = 43; //midway
	_wavelength = word(_getNoteHighByte(_currentNote),_getNoteLowByte(_currentNote));
 	
 	_currentBend = 0;
 	_notesPressed = 0; 
 	_fineDetune = 0;

 	_LFOMode = LFOMODE_DISABLE;
 	_LFOMillis = 1000;
 	
	_arpNoteQueuePosition = 0;
	_arpDirectionAscend = true; 
	_arpStyle = ARPSTYLE_ASPLAYED;

 	_volume = 15;


}

//
//	write data byte to the specific 2A03 address
//
void WaveGen::_sendAddrData(uint8_t address, uint8_t data){   
	while(digitalRead(PIN_INTERRUPT) == HIGH);  
	while(digitalRead(PIN_INTERRUPT) == LOW);		// the interrupt worked well in a sketch, but got messy with a class. this works fine. 
													// we are just waiting for the rising edge on this pin which is our next opportunity to write.
	//delay(10);
	
	PORTE = address;
	PORTA = data;

	// digitalWrite(PIN_LATCH, LOW);					// set latch LOW to enable write
	// SPI.transfer(address);							// send address first
	// SPI.transfer(data);								// then data..
	// digitalWrite(PIN_LATCH, HIGH);					// set latch HIGH to end the write

}

/*
 *	handle MIDI key press
 */ 
void WaveGen::handleNoteOn(uint8_t channel, uint8_t pitch, uint8_t velocity){
	if(_notesPressed<QUEUE_SIZE){					// only accept additional notes if there's room in the queue
		//_setVolume(velocity);
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
	// if(_currentBend == bend) return; 
	// if(bend == 0){
	// 	if(_notesPressed > 0 ) {
	// 		_setWavelength(word(_getNoteHighByte(_currentNote),_getNoteLowByte(_currentNote))); 
	// 	} 
	// }
	// int currentBendDifference = bend - _currentBend; 
	// //uint16_t currentInterval =  word(_getNoteHighByte(_currentNote-PITCHBENDRANGE),_getNoteLowByte(_currentNote-PITCHBENDRANGE)) - word(_getNoteHighByte(_currentNote+PITCHBENDRANGE),_getNoteLowByte(_currentNote+PITCHBENDRANGE));
	// int bendDetune = (int)((((float)(currentBendDifference))/(float)8193.0) * ( (float)_wavelength*0.06*6 ));
	// if(_notesPressed > 0 ) {
	// 	_setWavelength(_wavelength - bendDetune); 
	// }  
	// _currentBend = bend;	
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
void bubble_sort(uint8_t list[], uint8_t n)
{
  uint8_t c, d, t;
 
  for (c = 0 ; c < ( n - 1 ); c++)
  {
    for (d = 0 ; d < n - c - 1; d++)
    {
      if (list[d] > list[d+1])
      {
        /* Swapping */
 
        t         = list[d];
        list[d]   = list[d+1];
        list[d+1] = t;
      }
    }
  }
} 
void WaveGen::createSortedQueues(){
	 for(int i=0;i<_notesPressed;i++){
	 	_noteQueue_UP[i] = _noteQueue[i]; 
	 }
	 bubble_sort(_noteQueue_UP,_notesPressed); 
}

void WaveGen::_pushNoteOnQueue(uint8_t note){
	_noteQueue[_notesPressed] = note;

	_notesPressed++;

	createSortedQueues();
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

	createSortedQueues();
}
uint8_t WaveGen::_getLastNoteInQueue(){
	return _noteQueue[_notesPressed-1]; 
}

void WaveGen::_playNote(uint8_t note){
	if(_noteState == NOTESTATE_OFF || _noteState == NOTESTATE_RELEASE){  
      _noteState = NOTESTATE_ATTACK; 
    } 
	_setWavelength(word(_getNoteHighByte(note+_getNoteOffset()),_getNoteLowByte(note+_getNoteOffset())),false);
	_currentNote = note;
}

void WaveGen::_setNoteOffset(int offset){
	if(_notesPressed > 0){
		_setWavelength(word(_getNoteHighByte(_currentNote+_getNoteOffset()),_getNoteLowByte(_currentNote+_getNoteOffset())),false);
	}
	_noteOffset = offset;
}
int WaveGen::_getNoteOffset(){
	return _noteOffset;
}

// 0	 2/14
// 1	 4/12
// 2	 8/ 8
// 3	12/ 4
void WaveGen::_setDutyCycle(uint8_t dc){ 
	if(dc > 3) dc = 3;
	if(dc != _dutyCycleValue){ 
		_dutyCycleValue = dc; 
		if(_notesPressed > 0) _sendWaveDataMessage();
	}
}

uint8_t WaveGen::_getDutyCycle(){
	return _dutyCycleValue;
}

//Set Volume 0000-1111 (0-15)
void WaveGen::_setVolume(uint8_t v){ 
	//uint8_t vol = map(v,0,127,0,15);
	//if(vol != _volume){ 
	//	_volume = vol; 
	if(v > 15) v=15;
	_volume = v;
		if(_notesPressed > 0) _sendWaveDataMessage();
	//}
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

  if(_LFOMode == LFOMODE_TREMALO){
  	_applyTremalo();
  }
  
}
 



void WaveGen::_runLFO(){

	if(_LFOMode == LFOMODE_ARP){
		if(_notesPressed > 1){  
			if(_notesPressed == 2 || _arpStyle == ARPSTYLE_ASPLAYED){
				// if there are only 2 notes, we just oscillate between them..
				_arpNoteQueuePosition = (_arpNoteQueuePosition+1)%(_notesPressed); 
				_playNote(_noteQueue[_arpNoteQueuePosition]);
			}else{
				

				if(_arpStyle == ARPSTYLE_UP || _arpStyle == ARPSTYLE_CONVERGE){
					_arpNoteQueuePosition = (_arpNoteQueuePosition+1)%(_notesPressed); 
					if(_arpStyle == ARPSTYLE_UP) _playNote(_noteQueue_UP[_arpNoteQueuePosition]);
					//else _playNote(_noteQueue_CONVERGE[_arpNoteQueuePosition]);
				}else if(_arpStyle == ARPSTYLE_DOWN || _arpStyle == ARPSTYLE_DIVERGE){
					_arpNoteQueuePosition = (_arpNoteQueuePosition+1)%(_notesPressed); 
					if(_arpStyle == ARPSTYLE_DOWN) _playNote(_noteQueue_UP[_notesPressed-_arpNoteQueuePosition-1]);
					//else _playNote(_noteQueue_CONVERGE[_arpNoteQueuePosition]);
				}else if(_arpStyle == ARPSTYLE_UPDOWN || _arpStyle == ARPSTYLE_CONVERGEDIVERGE){
					if(_arpNoteQueuePosition >= _notesPressed-1){
						_arpDirectionAscend = false;
					}else if(_arpNoteQueuePosition <= 0){
						_arpDirectionAscend = true; 
					}

					if(_arpDirectionAscend) 
						_arpNoteQueuePosition++;
					else 
						_arpNoteQueuePosition--;

					if(_arpStyle == ARPSTYLE_UPDOWN) _playNote(_noteQueue_UP[_arpNoteQueuePosition]);
					//else _playNote(_noteQueue_CONVERGE[_arpNoteQueuePosition]);
				} 
			} 
		} 
	}else if(_LFOMode == LFOMODE_TREMALO){
		if(_arpDirectionAscend){
			//_sendAddrData(0x01,B11110001);						//0x87 sweep enabled, shift = 7 (1/128)
			//_sendAddrData(0x17,0xC0);						//clock sweep immediately
			_arpDirectionAscend = false; //flip 
			_risingEdgeMicros = micros();
		}else{
			//_sendAddrData(0x01,B11111001);						//sweep enabled, shift = 7 (1/128)
			//_sendAddrData(0x17,0xC0);
			_arpDirectionAscend = true; //flip 
		} 
	}
	
}

void WaveGen::setLFOMillis(unsigned long lfoMillis){
	_LFOMillis = lfoMillis;
}

unsigned long WaveGen::getTremaloCycle(){
	// uint16_t currentWavelength = word(_getNoteHighByte(_currentNote),_getNoteLowByte(_currentNote));
	// uint16_t nextWavelength = word(_getNoteHighByte(_currentNote+1),_getNoteLowByte(_currentNote+1));
	// uint16_t prevWavelength = word(_getNoteHighByte(_currentNote-1),_getNoteLowByte(_currentNote-1));

	// uint16_t long diff = currentWavelength - nextWavelength;
	// if(!_arpDirectionAscend){
	// 	diff = prevWavelength - currentWavelength;
	// } 
	// return (unsigned long)(((double)_LFOMillis*(double)1000)/((double)1*(((double)diff)/(double)_tremaloDepth)));

	return 1;
}

void WaveGen::_applyTremalo(){  
	unsigned long cycle = getTremaloCycle();
	if(_cycleCheck(&_timer_applyTremalo, cycle)){
		if(_notesPressed > 0 && _tremaloDepth > 0){   

			uint16_t currentWavelength = word(_getNoteHighByte(_currentNote),_getNoteLowByte(_currentNote));
	 		//uint16_t nextWavelength = word(_getNoteHighByte(_currentNote+2),_getNoteLowByte(_currentNote+2));
	 		uint16_t prevWavelength = word(_getNoteHighByte(_currentNote-2),_getNoteLowByte(_currentNote-2));

	 		uint16_t w = currentWavelength;

			

			if(_tremaloWaveForm == TREMALOWAVEFORM_SINE){
				float delta;// = (float)(currentWavelength - nextWavelength);
				//if(!_arpDirectionAscend){
					delta = (float)(prevWavelength - currentWavelength);
				//} 
				delta *= (float)_tremaloDepth/(float)10;

				unsigned long x = micros() - _risingEdgeMicros;
				unsigned long period_micros = (unsigned long)2*(_LFOMillis*(unsigned long)1000);
				double multiplier = sin(((double)x/(double)period_micros)*(double)6.28318);
				double offset = multiplier*(double)delta;
				w = (uint16_t)((double)currentWavelength+offset);
			} 
			else if(_tremaloWaveForm == TREMALOWAVEFORM_SQUARE){
				

				if(!_arpDirectionAscend){ 
					float delta = (float)(prevWavelength - currentWavelength);
					delta *= (float)_tremaloDepth/(float)10;
					w = (uint16_t)((float)currentWavelength+delta); 
				} 
				//otherwise wavelength stays at root note
			}
			

			if(w!=_wavelength) _setWavelength(w,false);
		} 
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



 