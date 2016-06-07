#include <avr/sleep.h>
#include <avr/power.h>
#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>
#include <TimerOne.h>

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

#define HOOK_PIN 0
#define DIAL_PIN 1
#define TICK_PIN 2
#define RING_PIN 7
#define RING_OUT_PIN 

#define HOOK_UP_STATE LOW
#define DIAL_EN_STATE LOW
#define TICK_EN_STATE HIGH
#define RINGING_STATE LOW

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

void (*state)();

void RINGING();
void PHONING();
void STANBY();
void DIALING();
void DIALING_ACTIVE();

void setup() {
	//prepare the pins
	pinMode(HOOK_PIN,INPUT_PULLUP);
	pinMode(DIAL_PIN,INPUT_PULLUP);
	pinMode(TICK_PIN,INPUT_PULLUP);
	pinMode(RING_PIN,INPUT_PULLUP);

	//setup fona
	fonaSerial->begin(4800);
	fona.begin(*fonaSerial);
	//wait for serial
	while(!Serial);

	//open serial with 115200 baud
	Serial.begin(115200);
	Serial.println("ROTARY V0.1");
	Serial.println("programmed by AberDerBart");
	Serial.println("based on Adafruit Feather FONA, thanks, guys!");

	//setup timer (for calling after entering a digit)
	Timer1.initialize(3000 * 1000);

	//set initial state
	state=&STANDBY;
	pinMode(13, OUTPUT);
}


void loop() {
	(*state)();
}

void printPinStates(){
	Serial.println("Pin states:");
	Serial.print("HOOK: ");
	Serial.println(digitalRead(HOOK_PIN)==LOW ? "LOW" : "HIGH");
	Serial.print("DIAL: ");
	Serial.println(digitalRead(DIAL_PIN)==LOW ? "LOW" : "HIGH");
	Serial.print("TICK: ");
	Serial.println(digitalRead(TICK_PIN)==LOW ? "LOW" : "HIGH");
	Serial.print("RING: ");
	Serial.println(digitalRead(RING_PIN)==LOW ? "LOW" : "HIGH");
}

void sleepPinInterrupt(void){
	detachInterrupt(digitalPinToInterrupt(HOOK_PIN));
	detachInterrupt(digitalPinToInterrupt(DIAL_PIN));
}

void enterSleepMode(void){    
	Serial.println(F("SLEEP"));
	delay(100);
	sleep_enable();
	cli();
	attachInterrupt(digitalPinToInterrupt(HOOK_PIN), sleepPinInterrupt, CHANGE);
	attachInterrupt(digitalPinToInterrupt(DIAL_PIN), sleepPinInterrupt, CHANGE);
	set_sleep_mode(SLEEP_MODE_IDLE);  
	power_all_disable();
	sei();
	sleep_cpu();
	sleep_disable();
	power_all_enable();
	Serial.println(F("WACH"));
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

	enterSleepMode();
	
	printPinStates();

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

char digit=0;

char dialTimeout=0;
void dialing_timer_interrupt(){
	dialTimeout=1;
}

void DIALING(){
	/*
	-rotary dial up:
		->DIALING_ACTIVE
	-hook down:
		->STANDBY
	-3s passed and hook is up
		-start call
		->PHONING
	-incoming call:
		->?
  	*/
	if(Serial) Serial.println("State: DIALING");

	static char number[30];
	static char numberLength=0;


	dialTimeout=0;

	//add digit to number to be called
	if(digit){
		number[numberLength]=digit;
		digit=0;
		numberLength++;
	}

	//TODO: implement timer interrupt
	if(numberLength){
		Timer1.start();
		Timer1.attachInterrupt(&dialing_timer_interrupt);
	}

	number[numberLength]=0;
	Serial.println(number);
	
	enterSleepMode();
	Timer1.detachInterrupt();

	printPinStates();

	if(digitalRead(HOOK_PIN)!=HOOK_UP_STATE){
		//hook has been laid down, return to STANDBY
		numberLength=0;
		state=&STANDBY;
	}else if(digitalRead(DIAL_PIN)==DIAL_EN_STATE){
		state=&DIALING_ACTIVE;
	//TODO: implement ringing while dialing behaviour
	}else if(dialTimeout){
		//TODO: implement proper timeout for calling
		number[numberLength]=0;
		//fona.callPhone(number);
		//state=&PHONING;
	}
}

const char digitMap[]={0,'1','2','3','4','5','6','7','8','9','0'};

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

	//wait a few msecs for debouncing
	delay(50);

	unsigned char ticks=0;
	char tickState=(digitalRead(TICK_PIN)==TICK_EN_STATE);
	char lastTickState=tickState;

	while(digitalRead(DIAL_PIN)==DIAL_EN_STATE && digitalRead(HOOK_PIN)==HOOK_UP_STATE){
		lastTickState=tickState;
		
		//check for rising edge
		tickState=(digitalRead(TICK_PIN)==TICK_EN_STATE);
		if(lastTickState!=tickState && tickState){
			ticks++;
		}
		//wait a few msecs for debouncing
		delay(30);
	}

	if(digitalRead(HOOK_PIN)!=HOOK_UP_STATE){
		digit=0;
		state=&STANDBY;
	}else if(digitalRead(DIAL_PIN)!=DIAL_EN_STATE){
		if(ticks < 11){
			digit=digitMap[ticks];
		}else{
			digit=0;
		}
		state=&DIALING;
	}
}

void PHONING(){
	/*
	-hook down:
		-close call
		->STANDBY
	*/
	if(Serial) Serial.println("State: PHONING");
	
	//wait a few msecs for debouncing
	delay(100);

	enterSleepMode();

	printPinStates();
	
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
	//TODO: implement ringing

	if(Serial) Serial.println("State: RINGING");
	enterSleepMode();

	printPinStates();

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
