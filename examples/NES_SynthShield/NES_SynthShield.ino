#include "NES2A03.h"
#include <MIDI.h>
#include <SPI.h>

#define CHANNEL_REC1 1
#define CHANNEL_REC2 2
#define CHANNEL_TRI  3

#define MIDI_NOTE_OFFSET 21

NES2A03 nes;

void handleNoteOn(byte c,byte n,byte v){
  if(n < MIDI_NOTE_OFFSET) return;
  switch(c){
    case CHANNEL_REC1:
        nes.rectangle1.handleNoteOn(c,n-MIDI_NOTE_OFFSET,v);
        break;
    case CHANNEL_REC2:
        nes.rectangle2.handleNoteOn(c,n-MIDI_NOTE_OFFSET,v);
        break;
    case CHANNEL_TRI:
        nes.triangle.handleNoteOn(c,n-MIDI_NOTE_OFFSET,v);
        break;
  } 
}

void handleNoteOff(byte c,byte n,byte v){
  if(n < MIDI_NOTE_OFFSET) return;
  switch(c){
    case CHANNEL_REC1:
        nes.rectangle1.handleNoteOff(c,n-MIDI_NOTE_OFFSET,v);
        break;
    case CHANNEL_REC2:
        nes.rectangle2.handleNoteOff(c,n-MIDI_NOTE_OFFSET,v);
        break;
    case CHANNEL_TRI:
        nes.triangle.handleNoteOff(c,n-MIDI_NOTE_OFFSET,v);
        break;
  }  
}

void handlePitchBend(byte c,int b){
  switch(c){
    case CHANNEL_REC1:
        nes.rectangle1.handlePitchBend(c,b);
        break;
    case CHANNEL_REC2:
        nes.rectangle2.handlePitchBend(c,b);
        break;
    case CHANNEL_TRI:
        nes.triangle.handlePitchBend(c,b);
        break;
  }  
}

void handleCC(byte channel, byte number, byte value){
  switch(number){
  	case 0x4B:
      nes.rectangle1._setDutyCycle(value);
      break;  
    case 0x4C:
      nes.rectangle2._setDutyCycle(value);
      break;  
    case 0x5C:
      nes.rectangle1._setVolume(value);
      break;  
    case 0x5F:
      nes.rectangle2._setVolume(value);
      break;  
    case 0x0C:
      nes.rectangle1._setAttack(value);
      break;  
    case 0x0D:
      nes.rectangle2._setAttack(value);
      break;  
  	/*
    case 0x0C:
      setArpRate(value);
      break;
    case 0x5B:
      setDutyCycle(0,value);
      break;  
    case 0x5D:
      setDutyCycle(1,value);
      break;  
    case 0x5F:
      setRect2Detune(value);
      break;
    case 0x49:
      setAttackValue(value);
      break;
    case 0x48:
      setReleaseValue(value);
      break;  
    case 0x0B:
      setVelocitySensitivityValue(value);
      break;
    case 0x05:
      setRect1Volume(value); 
      break;   
    case 0x54:
      setRect2Volume(value);
     */
    default:
     break; 
  }
}


//
//	Initiate MIDI communications 
//
void MIDISetup(){
	MIDI.begin(MIDI_CHANNEL_OMNI);					// listen to any MIDI channel
	MIDI.turnThruOff();								// turn this off or else!
	MIDI.setHandleNoteOn(handleNoteOn);				// attach handler for key presses
	MIDI.setHandleNoteOff(handleNoteOff);			// attach handler for key releases
	MIDI.setHandlePitchBend(handlePitchBend);	
	MIDI.setHandleControlChange(handleCC);
}

void setup() {  
	nes.init(); 
	MIDISetup();  
}
void loop() {
	MIDI.read();									// Call MIDI.read the fastest you can for real-time performance.
  nes.run();
}



