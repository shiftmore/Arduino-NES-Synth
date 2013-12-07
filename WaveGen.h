#ifndef LIB_WAVEGEN_H_
#define LIB_WAVEGEN_H_
 
#include <Arduino.h>  

#define PIN_LATCH 10							//SPI Latch Pin 
#define PIN_INTERRUPT 2 						//Interrupt pin

#define QUEUE_SIZE 10

#define PITCHBENDRANGE 1 

#define NOTESTATE_OFF 0
#define NOTESTATE_ATTACK 1
#define NOTESTATE_SUSTAIN 2
#define NOTESTATE_RELEASE 3

class WaveGen {
	public:    
		void handleNoteOn(byte,byte,byte);
		void handleNoteOff(byte,byte,byte);
		void handlePitchBend(byte,int);
	//protected: 
		uint16_t _wavelength;
		uint8_t _currentNote; 
		int _currentBend;
		int _notesPressed;
		uint8_t _noteQueue[QUEUE_SIZE];  

		uint8_t _dutyCycleValue;
		uint8_t _volume;
		uint8_t _currentVolume;
		int _fineDetune;

		uint8_t _noteState;

 		unsigned long _timer_applyAttack;
		unsigned long _cycle_applyAttack;

		unsigned long _timer_applyRelease;
		unsigned long _cycle_applyRelease;

 		void init();

		void _sendAddrData(uint8_t,uint8_t);
		void _pushNoteOnQueue(uint8_t);
		void _removeNoteFromQueue(uint8_t);
		uint8_t _getLastNoteInQueue(); 

		uint8_t _getNoteLowByte(uint8_t);
		uint8_t _getNoteHighByte(uint8_t);

		void _playNote(uint8_t);

		void _setDutyCycle(uint8_t);
		uint8_t _getDutyCycle();
		void _setVolume(uint8_t);
		uint8_t _getVolume();

		void _setAttack(uint8_t);

		//void _setFineDetune(uint8_t);
		//uint8_t _getFineDetune();
		//uint16_t _getFineDetuneAmount(uint16_t);

		virtual void _setWavelength(uint16_t);
		virtual void _stop(); 
		virtual uint8_t _getWaveDataMessage();
		virtual void _sendWaveDataMessage(); 

		void _handleNoteStates();
		void _applyAttack();
		void _applyRelease();
		//unsigned long _cycleCheck(unsigned long, unsigned long);
		bool _cycleCheck(unsigned long *, unsigned long);

		 
	};

#endif
