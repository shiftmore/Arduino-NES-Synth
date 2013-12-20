#include "NES2A03.h"
#include <MIDI.h>
#include <SPI.h>

// The MIDI channel that will control each wave gen.
// Default: all set to channel 1, all wave gens play in unison
#define CHANNEL_REC1 1
#define CHANNEL_REC2 1
#define CHANNEL_TRI  1

// Here is where you can configure which knob controls which parameter. 
#define CC_RECT1_DUTYCYCLE  0x4B //75
#define CC_RECT2_DUTYCYCLE  0x4C //76
#define CC_RECT1_VOLUME     0x5C //92
#define CC_RECT2_VOLUME     0x5F //95
#define CC_RECT1_ATTACK     0x0C //12
#define CC_RECT2_ATTACK     0x0D //13

#define MIDI_NOTE_OFFSET 21

NES2A03 nes;

void handleNoteOn(byte c,byte n,byte v){
  if(n < MIDI_NOTE_OFFSET) return;

  if(v == 0){
    handleNoteOff(c,n,v); //direct midi keyboard sends 0 velocity rather than note off command
    return;
  } 

  //Use a series of ifs rather than a switch to support controlling multiple wave gens with the same midi channel
  if(c == CHANNEL_REC1) nes.rectangle1.handleNoteOn(c,n-MIDI_NOTE_OFFSET,v);
  if(c == CHANNEL_REC2) nes.rectangle2.handleNoteOn(c,n-MIDI_NOTE_OFFSET,v);
  if(c == CHANNEL_TRI) nes.triangle.handleNoteOn(c,n-MIDI_NOTE_OFFSET,v);

}

void handleNoteOff(byte c,byte n,byte v){
  if(n < MIDI_NOTE_OFFSET) return;

  if(c == CHANNEL_REC1) nes.rectangle1.handleNoteOff(c,n-MIDI_NOTE_OFFSET,v);
  if(c == CHANNEL_REC2) nes.rectangle2.handleNoteOff(c,n-MIDI_NOTE_OFFSET,v);
  if(c == CHANNEL_TRI) nes.triangle.handleNoteOff(c,n-MIDI_NOTE_OFFSET,v);

}

void handlePitchBend(byte c,int b){

 if(c == CHANNEL_REC1) nes.rectangle1.handlePitchBend(c,b);
 if(c == CHANNEL_REC2) nes.rectangle2.handlePitchBend(c,b);
 if(c == CHANNEL_TRI) nes.triangle.handlePitchBend(c,b);
  
}

void handleCC(byte channel, byte number, byte value){
  switch(number){
  	case CC_RECT1_DUTYCYCLE:
      nes.rectangle1._setDutyCycle(value);
      break;  
    case CC_RECT2_DUTYCYCLE:
      nes.rectangle2._setDutyCycle(value);
      break;  
    case CC_RECT1_VOLUME:
      nes.rectangle1._setVolume(value);
      break;  
    case CC_RECT2_VOLUME:
      nes.rectangle2._setVolume(value);
      break;  
    case CC_RECT1_ATTACK:
      nes.rectangle1._setAttack(value);
      break;  
    case CC_RECT2_ATTACK:
      nes.rectangle2._setAttack(value);
      break;  
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

  //set some default values so we can hear something..
  nes.rectangle1._setVolume(127);
  nes.rectangle2._setVolume(127);
  nes.triangle._setVolume(127);
}
void loop() {
	MIDI.read();									// Call MIDI.read the fastest you can for real-time performance.
  nes.run();
}



