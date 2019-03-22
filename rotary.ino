#include <avr/sleep.h>
#include <avr/power.h>
#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>
#include "config.h"

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

void (*state)();

void RINGING();
void PHONING();
void STANBY();
void DIALING();
void SPEED_DIAL();

void setup() {
	//prepare the pins
	pinMode(HOOK_PIN, INPUT_PULLUP);
	pinMode(DIAL_PIN, INPUT_PULLUP);
	pinMode(TICK_PIN, INPUT_PULLUP);
	pinMode(RING_PIN, INPUT_PULLUP);
	pinMode(RING_OUT_PIN, OUTPUT);
	pinMode(SLEEP_PIN, OUTPUT);

	//setup fona
	fonaSerial->begin(4800);
	fona.begin(*fonaSerial);

	fona.setAudio(FONA_EXTAUDIO);
	fona.setVolume(VOLUME);
	fona.setMicVolume(FONA_EXTAUDIO, MIC_VOLUME);

	digitalWrite(RING_OUT_PIN, LOW);
	digitalWrite(SLEEP_PIN, !SLEEP_EN_STATE);

	//set initial state
	state=&STANDBY;
}

void loop() {
	updateSerial();
	(*state)();
}

char readDigit(){
	const char digitMap[]={0,'1','2','3','4','5','6','7','8','9','0'};

	//wait a few msecs for debouncing
	delay(50);

	unsigned char ticks=0;
	char tickState=(digitalRead(TICK_PIN)==TICK_EN_STATE);
	char lastTickState=tickState;

	while(digitalRead(DIAL_PIN)==DIAL_EN_STATE){
		lastTickState=tickState;
		
		//check for rising edge
		tickState=(digitalRead(TICK_PIN)==TICK_EN_STATE);
		if(lastTickState!=tickState && tickState){
			ticks++;
		}
		//wait a few msecs for debouncing
		delay(30);
	}

	if(ticks < 11){
		return digitMap[ticks];
	}else{
		return 0;
	}
}

/*void sleepPinInterrupt(void){
	detachInterrupt(digitalPinToInterrupt(HOOK_PIN));
	detachInterrupt(digitalPinToInterrupt(DIAL_PIN));
}*/

void waitForPinChange(){/*
	delay(100);

	set_sleep_mode(SLEEP_MODE_IDLE);  
	cli();
	//attachInterrupt(digitalPinToInterrupt(HOOK_PIN), sleepPinInterrupt, CHANGE);
	//attachInterrupt(digitalPinToInterrupt(DIAL_PIN), sleepPinInterrupt, CHANGE);
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();*/
}

void enterSleepMode(){
	digitalWrite(SLEEP_PIN, SLEEP_EN_STATE);
	waitForPinChange();
}

void STANDBY(){
	/*
	-go to sleep mode, turn everything off
	-hook up:
		->DIALING
	-incoming call:
		->RINGING
	-rotary dial up:
		->perform speed dial
	*/
	
	//debounce
	delay(100);
	enterSleepMode();
	
	digitalWrite(SLEEP_PIN, !SLEEP_EN_STATE);
  
	if(digitalRead(RING_PIN)==RINGING_STATE){
		//if its ringing, switch to RINGING 
		state=&RINGING;
	}else if(digitalRead(HOOK_PIN)==HOOK_UP_STATE){
		//if hook was taken off, switch to DIALING
		state=&DIALING;
	}else if(digitalRead(DIAL_PIN)==DIAL_EN_STATE){
		state=&SPEED_DIAL;
	}
}

void SPEED_DIAL(){

	char digit = readDigit();

	if(digit){
		char* number = getSpeedDial(digit - '1');
		if(*number){
			//if the number is non-empty, call it
			fona.callPhone(number);
			state = &PHONING;
			return;
		}
	}

	//return to standby if anything goes wrong
	state = &STANDBY;
}

void DIALING(){
	/*
	-hook down:
		->STANDBY
	-3s passed and hook is up
		-start call
		->PHONING
	-incoming call:
		->?
	-dial enable:
		-read digit
		->DIALING
  	*/

	static char number[50];
	static char numberLength=0;
	unsigned long startMillis=millis();

	//do busy wait, as arduino timer libraries don't work properly
	//TODO: implement this properly, if needed with hardware registers...
	char hookState=digitalRead(HOOK_PIN);
	char dialState=digitalRead(DIAL_PIN);

	while(hookState==HOOK_UP_STATE
		&& dialState!=DIAL_EN_STATE
		&& millis()-startMillis < 3000){
			hookState=digitalRead(HOOK_PIN);
			dialState=digitalRead(DIAL_PIN);
	}

	//react to input state
	if(hookState!=HOOK_UP_STATE){
		//hook has been laid down, return to STANDBY
		numberLength=0;
		state=&STANDBY;
	}else if(dialState==DIAL_EN_STATE){
		//the rotary dial has been turned, read the digit
		char digit = readDigit();
		if(digit){
			number[numberLength] = readDigit();
			numberLength++;
		}

		state=&DIALING;
	//TODO: implement ringing while dialing behaviour
	//-> just ignore it/pretend to be occupied?! would match a rotary phones behaviour...
	}else if(numberLength>0){
		//timer expired, start the call
		number[numberLength]=0;
		numberLength=0;
		fona.callPhone(number);
		state=&PHONING;
	}
}

void PHONING(){
	/*
	-hook down:
		-close call
		->STANDBY
	*/
	
	//wait a few msecs for debouncing
	delay(100);

	waitForPinChange();

	//debounce
	delay(100);
	if(digitalRead(HOOK_PIN)!=HOOK_UP_STATE){
		//laid down hook, hang up and goto STANDBY
		fona.hangUp();
		state=&STANDBY;
		//if for some reason the rotary dial was turned while hanging up, wait for it to rotate back
		while(digitalRead(DIAL_PIN) == DIAL_EN_STATE){
			delay(10);
		}
	}
}

void RINGING(){
	/*
	-hook up:
		PHONING
	-incoming call ended:
		->STANDBY
	*/

	//TODO: further enhance this to use a timer
	char hookState=digitalRead(HOOK_PIN);
	char ringState=digitalRead(RING_PIN);
	unsigned long startMillis=millis();
	char ringOut=HIGH;

	digitalWrite(RING_OUT_PIN,HIGH);

	while(ringState==RINGING_STATE && hookState!=HOOK_UP_STATE){
		if(millis()-startMillis>2500){
			ringOut=((ringOut==HIGH)? LOW : HIGH);
			digitalWrite(RING_OUT_PIN,ringOut);
			startMillis=millis();
		}
		hookState=digitalRead(HOOK_PIN);
		ringState=digitalRead(RING_PIN);
	}

	digitalWrite(RING_OUT_PIN,LOW);

	if(hookState==HOOK_UP_STATE){
		//hook has been picked up
		if(fona.pickUp()){
			//if call is still active, go to phoning
			state=&PHONING;
		}else{
			//if call is not ative anymore, go to dialing 
			state=&DIALING;
		}
	}else if(ringState!=RINGING_STATE){
		//missed the call, goto STANDBY
		state=&STANDBY;
	}
}
