#include <avr/sleep.h>
#include <avr/power.h>
#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

#define HOOK_PIN 0
#define DIAL_PIN 1
#define TICK_PIN 2
#define RING_PIN 3

#define HOOK_UP_STATE HIGH
#define DIAL_EN_STATE HIGH
#define TICK_EN_STATE HIGH
#define RINGING_STATE HIGH

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

void (*state)();

void setup() {
	//save power
	power_adc_disable();
	power_spi_disable();
//	power_timer0_disable();
	power_timer1_disable();
	power_timer2_disable();
	power_twi_disable();

	//prepare the pins
	pinMode(HOOK_PIN,INPUT);
	pinMode(DIAL_PIN,INPUT);
	pinMode(TICK_PIN,INPUT);
	pinMode(RING_PIN,INPUT);

	//setup fona
	fonaSerial->begin(4800);
	fona.begin(*fonaSerial);

	//Wait for serial connection to be ready
	while(!Serial);

	//open serial with 115200 baud
	Serial.begin(115200);
	Serial.println("ROTARY V0.1");
	Serial.println("programmed by AberDerBart");
	Serial.println("based on Adafruit Feather FONA, thanks, guys!");

	//set initial state
	state=&STANDBY;
}

void loop() {
	(*state)();
}

void enableSleepInterrupt(){
	attachInterrupt(digitalPinToInterrupt(HOOK_PIN),&interrupt,CHANGE);
	attachInterrupt(digitalPinToInterrupt(DIAL_PIN),&interrupt,CHANGE);
	attachInterrupt(digitalPinToInterrupt(RING_PIN),&interrupt,CHANGE);
}

void goToSleep(){
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sleep_mode();
	sleep_disable();
}

void interrupt(){
	detachInterrupt(digitalPinToInterrupt(HOOK_PIN));
	detachInterrupt(digitalPinToInterrupt(DIAL_PIN));
	detachInterrupt(digitalPinToInterrupt(RING_PIN));
}

void STANDBY(){
	/*
	-go to sleep mode, turn everything off
	-hook up:
		->DIALING
	-incoming call:
		->RINGING
	-rotary dial up:
		->DIALING_ACTIVE
	*/
	if(Serial) Serial.println("State: STANDBY");
	enableSleepInterrupt();
	goToSleep();

	if(digitalRead(RING_PIN)==RINGING_STATE){
		//if its ringing, switch to RINGING 
		state=&RINGING;
	}else if(digitalRead(HOOK_PIN)==HOOK_UP_STATE){
		//if hook was taken off, switch to DIALING
		state=&DIALING;
	/*TODO: implement speed dial
	}else if(digitalRead(DIAL_PIN)==DIAL_EN_STATE){
		state=&DIALING;*/
	}
}

char digit;

void DIALING(){
	/*
	-rotary dial up:
		->DIALING_ACTIVE
	-hook down:
		->STANDBY
	-hook up:
		-replace number with phonebook number
		->PHONING
	-3s passed and hook is up
		-start call
		->PHONING
	-incoming call:
		->?
  	*/
	if(Serial) Serial.println("State: DIALING");
	//TODO:properly implement DIALING

	static char number[30];
	static char numberIndex;

	//add digit to number to be called
	if(digit){
		number[numberIndex]=digit;
		digit=0;
		numberIndex++;
		number[numberIndex]=0;
	}

	enableSleepInterrupt();
	goToSleep();
}

void DIALING_ACTIVE(){
	/*
	-count clicks
	-rotary dial down:
		-add cipher to number
		->DIALING
	-hook down:
		->STANDBY
	-incoming call:
		->?
  	*/
	if(Serial) Serial.println("State: DIALING_ACTIVE");
	//TODO: implement DIALING_ACTIVE
}

void PHONING(){
	/*
	-hook down:
		-close call
		->STANDBY
	*/
	if(Serial) Serial.println("State: PHONING");
	
	//TODO: sleep for ~100ms before reading pins (for debouncing)

	enableSleepInterrupt();
	goToSleep();
	
	if(digitalRead(HOOK_PIN)!=HOOK_UP_STATE){
		//laid down hook, hang up and goto STANDBY
		fona.hangUp();
		state=&STANDBY;
	}
}

void RINGING(){
	/*
	-hook up:
		PHONING
	-incoming call ended:
		->STANDBY
	*/
	if(Serial) Serial.println("State: RINGING");
	enableSleepInterrupt();
	goToSleep();

	if(digitalRead(HOOK_PIN)==HOOK_UP_STATE){
		//hook has been picked up
		if(fona.pickUp()){
			//if call is still active, go to phoning
			state=&PHONING;
		}else{
			//if call is not ative anymore, go to dialing 
			state=&DIALING;
		}
	}else if(digitalRead(RING_PIN)!=RINGING_STATE){
		//missed the call, goto STANDBY
		state=&STANDBY;
	}
}
