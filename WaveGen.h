#ifndef LIB_WAVEGEN_H_
#define LIB_WAVEGEN_H_
 
#include <Arduino.h>  
 

#define PIN_INTERRUPT 18 //pin 46

//#define PIN_INTERRUPT 2 						//Interrupt pin
//#define PIN_LATCH 10							//SPI Latch Pin 


#define QUEUE_SIZE 10

#define PITCHBENDRANGE 1 

#define NOTESTATE_OFF 0
#define NOTESTATE_ATTACK 1
#define NOTESTATE_SUSTAIN 2
#define NOTESTATE_RELEASE 3

#define ARPSTYLE_ASPLAYED 0
#define ARPSTYLE_UP 1
#define ARPSTYLE_DOWN 2
#define ARPSTYLE_UPDOWN 3
#define ARPSTYLE_CONVERGE 4
#define ARPSTYLE_DIVERGE 5
#define ARPSTYLE_CONVERGEDIVERGE 6
#define ARPSTYLE_RANDOM 7

#define LFOMODE_DISABLE 0
#define LFOMODE_ARP 1
#define LFOMODE_MOD 2

#define MODWAVEFORM_SINE 1
#define MODWAVEFORM_SQUARE 2
#define MODWAVEFORM_SAW 3
#define MODWAVEFORM_NOISE 4

#define MOD_TREMALO 0
#define MOD_VIBRATO 1

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

		uint8_t _noteQueue_UP[QUEUE_SIZE];  


		int _noteOffset;

		uint8_t _dutyCycleValue;		//0-3
		uint8_t _volume; 				//0-15
		uint8_t _currentVolume;			//0-15
		int _fineDetune;

		uint8_t _noteState;

 		unsigned long _timer_applyAttack;
		unsigned long _cycle_applyAttack;

		unsigned long _timer_applyRelease;
		unsigned long _cycle_applyRelease;

		unsigned long _timer_applyMod;
		unsigned long _cycle_applyMod;
		
		uint8_t _LFOMode;
		unsigned long _LFOMillis;
		unsigned long _risingEdgeMicros;

		uint8_t _modDepth;
		uint8_t _modWaveForm;

		uint8_t _arpNoteQueuePosition;
		boolean _arpDirectionAscend; 
		uint8_t _arpStyle;
		boolean _risingEdgeModFlag;
		boolean _fallingEdgeModFlag;

		uint8_t _modMode;

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
		void _setRelease(uint8_t);

		void _setNoteOffset(int);
		int _getNoteOffset();
		//void _setFineDetune(uint8_t);
		//uint8_t _getFineDetune();
		//uint16_t _getFineDetuneAmount(uint16_t);

		virtual void _setWavelength(uint16_t,bool);
		virtual void _stop(); 
		virtual uint8_t _getWaveDataMessage();
		virtual void _sendWaveDataMessage(); 
		virtual void _applyAttack();
		virtual void _applyRelease();

		void _handleNoteStates();
		

		void _applyMod();

		//unsigned long _cycleCheck(unsigned long, unsigned long);
		bool _cycleCheck(unsigned long *, unsigned long);
		bool _cycleCheck_millis(unsigned long *, unsigned long);

		void _runLFO();
		void setLFOMillis(unsigned long);
		unsigned long getModCycle();
		void createSortedQueues();
	};

#endif