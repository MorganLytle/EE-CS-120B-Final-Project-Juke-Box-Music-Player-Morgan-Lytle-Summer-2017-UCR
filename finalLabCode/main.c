/* Name & E-mail: Morgan Lytle : mlytl001@ucr.edu
* Lab Section: B21
* Assignment: Lab 9 Exercise 2
* Exercise Description:
* I acknowledge all content contained herein, excluding template or example
* code, is my own original work.
*/
// 0.954 hz is lowest frequency possible with this function,
// based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound

//NOTE: To add songs add note value array and note sequence array, and increment songNumber 
//add if statement in songData which is in songSM, and displayString function
#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"

unsigned char A0 = 0x00; //increment
unsigned char A1 = 0x00; //decrement
unsigned char A2 = 0x00; //play
unsigned char A3 = 0x00;//stop
unsigned int i = 0x00; // counter for navigating the arrays
unsigned char scaleTone = 0x00; //array counter
unsigned int arraySize = 65; //generic var
unsigned int song0ArraySize = 33;
unsigned int song1ArraySize = 33;
unsigned int song2ArraySize = 65;
unsigned char displayFlag = 0x00; //equal to 1 when it has already been displayed to avoid unessesary refreshing
unsigned int light = 0x00; // 0 = green 1 = red 2 = yellow 3 = green
unsigned char songChoice = 0x00; //current song
unsigned char play = 0x00;//1=play
unsigned char songNumber = 0x02; //The number of songs included in jukebox
								 //This has 3 songs=> song 0, song 1, song 2
unsigned int timeVal = 0; //variable for time counter in state machines that need to hold a value for a certain amount of time, corresponds to multiples of 100
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

enum songStates{songInit, songWaitForPlay, songData, songPlay, songHoldNote, songWaitForRelease} songState;
// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
unsigned char buzzStatus = 0x00; //0=off 1=on

//const double cScale [9] = {0.00, 261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};//Mary Had A Little Lamb

const unsigned int song0NoteVal[33] =      {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
											2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 4, 2}; //Mary Had A Little Lamb

const unsigned int song0NoteSequence[33] = {17, 15, 13, 15, 17, 17, 17, 0, 15, 15, 15, 0, 17,
											20, 20, 0, 17, 15, 13, 15, 17, 17, 17, 0, 15, 0,
											15, 17, 15, 13, 0, 13, 0};/*{17, 15, 13, 15, 17, 17, 17, 0, 15, 15, 15, 0, 15,
											17, 17, 0, 17, 15, 13, 15, 17, 17, 17, 0, 15, 0,
											15, 17, 15, 13, 0, 13, 0};*/

const unsigned int song1NoteVal[33] =         {2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
											   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};//Giant Steps

const unsigned int song1NoteSequence[33] =    {19, 15, 12, 8, 11, 0, 12, 10, 15, 11, 8, 4,
											   7, 0, 8, 6, 11, 0, 12, 10, 15, 0, 16, 16, 19,
											   0, 20, 20, 23, 0, 19, 0, 19};

const unsigned int song2NoteVal[65] =    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
										  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8};//Sweet Child O' Mine

const unsigned int song2NoteSequence[65] =    {8, 20, 15, 13, 25, 15, 24, 15, 8, 20, 15, 13, 25, 15, 24, 15, 10, 20,
											  15, 13, 25, 15, 24, 15, 10, 20, 15, 13, 25, 15, 24, 15, 13, 20, 15, 13,
											  25, 15, 24, 15, 10, 20, 15, 13, 25, 15, 24, 15,
											  22, 15, 20, 15, 22, 15, 24, 15, 25, 15, 24, 15, 22, 15, 20, 15, 20};
unsigned int noteVal[65] =    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
							   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
							   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8};//generic Var

unsigned int noteSequence[65]=   {8, 20, 15, 13, 25, 15, 24, 15, 8, 20, 15, 13, 25, 15, 24, 15, 10, 20,
										15, 13, 25, 15, 24, 15, 10, 20, 15, 13, 25, 15, 24, 15, 13, 20, 15, 13,
										25, 15, 24, 15, 10, 20, 15, 13, 25, 15, 24, 15,
										22, 15, 20, 15, 22, 15, 24, 15, 25, 15, 24, 15, 22, 15, 20, 15, 20}; //generic var

const double cScale [26] = {0.00, 130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.00, 196.00, 207.65, //chromatic scale octave 3 and octave 4
						   220.00, 233.08, 246.94, 261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99,
						   392.00, 415.30, 440.00, 466.16, 493.88, 523.25};
                           //0 rest, 1 C, 2 Db, 3  D, 4 Eb, 5 E, 6 F, 7 F#, 8 G, 9 G#, 10 A, 11 Bb, 12 B, 13 MIDDLE C,
                           // 14 C#, 15 D, 16 Eb, 17 E, 18 F, 19 F#, 20 G, 21 G#, 22 A, 23 Bb, 24 B, 25 C


unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}
unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}
//pwm code start
void set_PWM(double frequency) {
	
	
	// Keeps track of the currently set frequency
	// Will only update the registers when the frequency
	// changes, plays music uninterrupted.
	static double current_frequency;
	if (frequency != current_frequency) {

		if (!frequency) TCCR3B &= 0x08; //stops timer/counter
		else TCCR3B |= 0x03; // resumes/continues timer/counter
		
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) OCR3A = 0xFFFF;
		
		// prevents OCR3A from underflowing, using prescaler 64	
		// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) OCR3A = 0x0000;
		
		// set OCR3A based on desired frequency
		else OCR3A = (short)(8000000 / (128 * frequency)) - 1;

		TCNT3 = 0; // resets counter
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT3) matches OCR3A, reset counter
	// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}
//pwm code end

void readButton() //reads the three buttons
{
	A0 = !GetBit(PINA, 0); //up
	A1 = !GetBit(PINA, 1); //down
	A2 = !GetBit(PINA, 2); //toggle on
	A3 = !GetBit(PINA, 3);//toggle stop
}

//Timer code starts

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

//timer code stop

void displayString()
{
		if (play == 0x00){
		if(songChoice == 0x00)
		{
			LCD_DisplayString(1, "Song 0: Mary Had A Little Lamb");
		}
		else if(songChoice == 0x01)
		{
			LCD_DisplayString(1, "Song 1: Giant   Steps");
		}
		else if(songChoice == 0x02)
		{
			LCD_DisplayString(1, "Song 2: Sweet   Child O' Mine");
		}
		}
		else if (play == 0x01)
		{
			if(songChoice == 0x00)
			{
				LCD_DisplayString(1, "Playing:Mary Had A Little Lamb");
			}
			else if(songChoice == 0x01)
			{
				LCD_DisplayString(1, "Playing: Giant   Steps");
			}
			else if(songChoice == 0x02)
			{
				LCD_DisplayString(1, "Playing: Sweet   Child O' Mine");
			}	
		}
}

void displayWelcome()
{
	LCD_DisplayString(1, "Welcome to Juke Box Deluxe!! :-)"); //welcome message
}

void lightShow()
{
	
	if(light == 4)
	{
		light = 0;
	}
	unsigned char D = 0x01;
	PORTD = D << light;
	++light;

}
	
void songSMTick()
{
	switch(songState) //transitions
	{
		case songInit:
		PORTD = 0XFF;
		songState = songWaitForPlay;
		break;
		
		case songWaitForPlay: //wait for play flag to be set by A2
		PORTD = 0xFF;
		if(play == 0x00)
		{
			songState = songWaitForPlay;
		}
		else if(play == 0x01)
		{
			PWM_on();
			set_PWM(0.00);
			songState = songData;
		}
		break;
		
		case songData://populate generic array with the arrays of the chosen song
		songState = songPlay;
		break;
		
		case songPlay: //go to hold the current note unless A3 is pressed
		if(!A3)
		{
			songState = songHoldNote;
		}
		else if(A3)
		{
			play = 0x00;
			songState = songWaitForPlay;
		}
		break;
		
		case songHoldNote: //Hold the current note for its full duration unless stop is pressed
		if(timeVal < noteVal[i])//holds note for whole duration specified in
		{
			if(!A3)
			{
				songState = songHoldNote;
			}
			else if(A3)
			{
				play = 0x00;
				songState = songWaitForPlay;
			}
		}
		else if((timeVal == noteVal[i]) && (i == (arraySize - 1)))
		{
			songState = songWaitForRelease;
			set_PWM(0);
		}
		else if((timeVal == noteVal[i]) && (i < (arraySize - 1)))
		{
			
			if(!A3)
			{	
			++i; // increment array counter
			lightShow(); //Change lights in sequence concurrently with each new note, including rests which are considered a note
			songState = songPlay; 
			}
			else if(A3)
			{
				play = 0x00;
				songState = songWaitForPlay;
			}
		}
		break;
		
		case songWaitForRelease: //wait to release button so song only plays once per press
		if(A2)
		{
			songState = songWaitForRelease;
		}
		else if(!A2)
		{
			play = 0x00;
			songState = songWaitForPlay;
		}
		break;
		
		default : songState = songInit;
		break;
	}
	
	switch(songState) //output
	{
		case songInit:
		i = 0x00;
		timeVal = 0x00;
		break;
		
		case songWaitForPlay:
		PWM_off();
		i = 0;
		PORTD = 0xFF;
		break;
		
		case songData: //transfer desired song data to current song variables
		if(songChoice == 0)
		{
			arraySize = song0ArraySize;
			for(int j = 0; j < (song0ArraySize ); ++j)
			{
				noteSequence[j] = song0NoteSequence[j];
			}
			for(int j = 0; j < (song0ArraySize ); ++j)
			{
				noteVal[j] = song0NoteVal[j];
			}
		}
		else if(songChoice == 1)
		{
			arraySize = song1ArraySize;
			for(int j = 0; j < (song1ArraySize ); ++j)
			{
				noteSequence[j] = song1NoteSequence[j];
			}
			for(int j = 0; j < (song1ArraySize ); ++j)
			{
				noteVal[j] = song1NoteVal[j];
			}
		}
		else if(songChoice == 2)
		{
			arraySize = song2ArraySize;
			for(int j = 0; j < (song2ArraySize ); ++j)
			{
				noteSequence[j] = song2NoteSequence[j];
			}
			for(int j = 0; j < (song2ArraySize ); ++j)
			{
				noteVal[j] = song2NoteVal[j];
			}
		}
		break;
		break;
		
		
		case songPlay:
		timeVal = 0x00;
		set_PWM(cScale[noteSequence[i]]); //set the pwm to the frequency of the desired note
		break;
		
		case songHoldNote:
		++timeVal; //hold note for entire duration
		break;
		
		case songWaitForRelease:
		set_PWM(0);
		break;
	}
}

enum ioStates{ioInit, ioWelcome, ioSongList,  ioInstructions, ioWaitForInput, ioSongIncrement, ioSongDecrement, ioWaitForRelease, ioPlay} ioState;

void ioSMTick() //state machine for lcd and input
{

	
	switch(ioState)
	{
		case ioInit:
		ioState = ioWelcome;///////////////FIXME CHANGE BACK TO IOINSTRUCTION
		displayWelcome(); //display welcome message
		break;
		
		case ioWelcome:
		if(timeVal < 20)
		{
			ioState = ioWelcome;
		}
		else if(timeVal >= 20) //display the song list for 2 seconds each song
		{
			ioState = ioSongList;
			timeVal = 0;
			displayString();
		}
		break;
		
		case ioSongList:
		if(timeVal < 20)
		{
			songChoice = 0;
			if(displayFlag == 0x00) //cycle through song list
			{
				displayString();
				displayFlag = 0x01;
			}
			if(timeVal == 19)
			{
				displayFlag = 0x00;
			}
			ioState = ioSongList;
		}
		else if((timeVal < 40) && (timeVal >= 20))
		{
			songChoice = 1;
			if(displayFlag == 0x00)
			{
				displayString();
				displayFlag = 0x01;
			}
			if(timeVal == 39)
			{
				displayFlag = 0x00;
			}
			ioState = ioSongList;
		}
		if((timeVal < 60) && (timeVal >= 40))
		{
			songChoice = 2;
			if(displayFlag == 0x00)
			{
			displayString();
			displayFlag = 0x01;
			}
			if(timeVal == 59)
			{
				displayFlag = 0x00;
			}
			ioState = ioSongList;
		}		
		else if(timeVal >= 60)
		{
			timeVal = 0;
			songChoice = 0;
			ioState = ioInstructions;
			LCD_DisplayString(1, "A0=up A1=down   A2=Play A3=Stop."); //instructions
		}
		break;
		
		case ioInstructions:
		if(timeVal < 50) //display instructions for 5 seconds
		{
			ioState = ioInstructions;
		}
		else if(timeVal >= 50)
		{
			ioState = ioWaitForInput;
			displayString();
			timeVal = 0;
		}
		break;

		case ioWaitForInput:
		if(!A0 && !A1 && !A2) //no input
		{
			ioState = ioWaitForInput;
		}
		else if((A0 && !A1 && !A2) && (songChoice < songNumber)) //increment transition
		{
			ioState = ioSongIncrement;
		}
		else if((!A0 && A1 && !A2) && (songChoice > 0)) //decrement
		{
			ioState = ioSongDecrement;
		}	
		else if(!A0 && !A1 && A2) //play
		{
			play = 0x01;
			ioState = ioPlay;
			displayString();////////////////////////////////////////////////////////////////////uncomment
		}		
		else //some other combination of inputs
		{
			ioState = ioWaitForInput;
		} 
		break;
		
		case ioSongIncrement:
		ioState = ioWaitForRelease;
		displayString();
		break;
		
		case ioSongDecrement:
		ioState = ioWaitForRelease;
		displayString();
		break; 
		
		case ioWaitForRelease: //wait for release of pressed button
		if(A0 || A1)
		{
			ioState = ioWaitForRelease;
		}
		else if(!A0 && !A1)
		{
			ioState = ioWaitForInput;
			displayString();
		}
		else
		{
			ioState = ioWaitForRelease;
		}
		break;
		
		case ioPlay:
		if(play == 0x00) //play is set by song sm when song is over or if stop is pressed
		{
			ioState = ioWaitForInput;
			displayString();
		}
		else if(play == 0x01)
		{
			ioState = ioPlay;
		}
		break;
		
		default: 
		ioState = ioInit;
		break;
	}
	
	switch(ioState)
	{
		case ioInit:
		
		songChoice = 0x00;
		play = 0x00;
		timeVal = 0x00;
		break;
		
		case ioWelcome:
		++timeVal;
		break;
		
		case ioSongList:
		++timeVal;
		break;
		
		case ioInstructions:
		++timeVal;
		break;
		
		case ioWaitForInput:
		break;
		
		case ioSongIncrement:
		++songChoice;
		break;
		
		case ioSongDecrement:
		--songChoice;
		break;
		
		case ioWaitForRelease:
		break;
		
		case ioPlay:
		break;
		
	}	

	
}

int main()
{
	PWM_on();
	TimerSet(100); //quarter note = 100ms
	TimerOn();
	
	DDRA = 0x00; PORTA = 0xFF;//button input
	DDRB = 0xFF; PORTB = 0x00;//buzzer
	DDRC = 0xFF; PORTC = 0x00; //lcd control
	DDRD = 0xFF; PORTD = 0x00;//lcd control AND LIGHTS

	ioState = ioInit;//state initializer
	songState = songInit;
	
	LCD_init(); //setup lcd screen
	LCD_ClearScreen();

	while(1)
	{
		readButton(); //check for buttons every 100ms
		ioSMTick();
		songSMTick();
		
		while(!TimerFlag);
		TimerFlag = 0;
	}
	
	return 0;
}



