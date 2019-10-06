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
void MENU();
void ERROR();
void BATTERY();
void NETWORK();

void setup() {
	//prepare the pins
	pinMode(HOOK_PIN, INPUT_PULLUP);
	pinMode(DIAL_PIN, INPUT_PULLUP);
	pinMode(TICK_PIN, INPUT_PULLUP);
	pinMode(RING_PIN, INPUT_PULLUP);
	pinMode(RING_OUT_PIN, OUTPUT);
	pinMode(SLEEP_PIN, OUTPUT);

	//setup led ring
	initLed();

	//setup fona
	fonaSerial->begin(4800);
	fona.begin(*fonaSerial);

	//wait for fona to be ready
	delay(800);

	//indicate reboot
	indicateReboot();

	fona.setAudio(FONA_EXTAUDIO);
	fona.setVolume(VOLUME);

	//set mic boost
	fona.sendCheckReply("AT+CMIC=1,5", "OK");

	//configure phonebook for speed dial
	fona.sendCheckReply("AT+CPBS?","OK");
	fona.sendCheckReply("AT+CPBS=\"ME\"", "OK");

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

void wake(){}

void sleep(){
	sleep_enable();
	attachInterrupt(RING_PIN, wake, CHANGE);
	attachInterrupt(HOOK_PIN, wake, CHANGE);
	attachInterrupt(DIAL_PIN, wake, CHANGE);

	digitalWrite(SLEEP_PIN, !SLEEP_EN_STATE);
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_cpu();
	digitalWrite(SLEEP_PIN, SLEEP_EN_STATE);
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
	delay(50);
	
	sleep();

	//debounce
	delay(50);
  
	if(digitalRead(RING_PIN)==RINGING_STATE){
		state=&RINGING;
	}else if(digitalRead(HOOK_PIN)==HOOK_UP_STATE){
		state=&DIALING;
	}else if(digitalRead(DIAL_PIN)==DIAL_EN_STATE){
		state=&MENU;
	}
}

void waitChange(uint16_t timeout){

	unsigned long startMillis = millis();

	char hookState = digitalRead(HOOK_PIN);
	char dialState = digitalRead(DIAL_PIN);
	char ringState = digitalRead(RING_PIN);

	while(hookState != HOOK_UP_STATE
	  && dialState != DIAL_EN_STATE
	  && ringState != RINGING_STATE
	  && millis() - startMillis < timeout){
		hookState = digitalRead(HOOK_PIN);
		dialState = digitalRead(DIAL_PIN);
		ringState = digitalRead(RING_PIN);
		delay(100);
	}

	if(digitalRead(RING_PIN)==RINGING_STATE){
		state=&RINGING;
	}else if(digitalRead(HOOK_PIN)==HOOK_UP_STATE){
		state=&DIALING;
	}else if(digitalRead(DIAL_PIN)==DIAL_EN_STATE){
		state=&MENU;
	}else{
		state=&STANDBY;
	}
}

void BATTERY(){
	uint16_t percent;
	fona.getBattPercent(&percent);
	fancyPie(percent/100., 0x00ff00, 0xff0000);

	waitChange(2000);

	clearLed();
}

void NETWORK(){
	state = &STANDBY;
	uint8_t rssi = fona.getRSSI();
	fancyPie(rssi/22., 0x0000ff, 0xff0000);

	waitChange(2000);

	clearLed();
}

void MENU(){
	char digit = readDigit();

	if(!digit){
		state = &STANDBY;
		return;
	}

	if(digit == '0'){
		//TODO: mute
		state = &STANDBY;
		return;
	}

	if(digit == '9'){
		state = &BATTERY;
		return;
	}

	if(digit == '8'){
		state = &NETWORK;
		return;
	}

	unsigned long startMillis = millis();

	char hookState = digitalRead(HOOK_PIN);
	char dialState = digitalRead(DIAL_PIN);
	char ringState = digitalRead(RING_PIN);

	while(hookState != HOOK_UP_STATE
	  && dialState != DIAL_EN_STATE
	  && ringState != RINGING_STATE
	  && millis() - startMillis < 3000){
		hookState = digitalRead(HOOK_PIN);
		dialState = digitalRead(DIAL_PIN);
		ringState = digitalRead(RING_PIN);
		if(millis() - startMillis >= 200){
			if(millis() - startMillis < 1600){
				fancyPie((millis() - startMillis - 200) / 1400., 0x00ff00, 0);
			}else{
				fancyPie((millis() - startMillis - 1600) / 1400., 0, 0x00ff00);
			}
		}
	}

	clearLed();

	if(hookState == HOOK_UP_STATE){
		//TODO: call speed dial number
		char* cmd = "ATD>X;";
		cmd[4] = digit;
		fona.sendCheckReply(cmd,"OK");
		state = PHONING;
	}else if(dialState == DIAL_EN_STATE){
		//overwrite the dialed number
		state = MENU;
	}else if(ringState == RINGING_STATE){
		//incoming call overrides speed dial
		state = RINGING;
	}else{
		//speed dial timed out, return to standby
		state = STANDBY;
	}
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
			number[numberLength] = digit;
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
	
	//debounce
	delay(50);

	sleep();

	delay(50);

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
		//TODO: remove this internal loop
		if(millis()-startMillis>2500){
			ringOut=((ringOut==HIGH)? LOW : HIGH);
			digitalWrite(RING_OUT_PIN,ringOut);
			startMillis=millis();
		}
		hookState=digitalRead(HOOK_PIN);
		ringState=digitalRead(RING_PIN);

		showRing();
		delay(30);
	}

	clearLed();

	digitalWrite(RING_OUT_PIN,LOW);

	if(hookState==HOOK_UP_STATE){
		//hook has been picked up
		if(fona.pickUp()){
			//if call is still active, go to phoning
			state=&PHONING;
		}else{
			if(connectionActive()){
				Serial.println("d: pickUp() failed");
			}
			state=&ERROR;
		}
	}else if(ringState!=RINGING_STATE){
		//missed the call, goto STANDBY
		state=&STANDBY;
	}
}

void ERROR(){
	delay(1000);
	
	if(digitalRead(HOOK_PIN) != HOOK_UP_STATE
		&& digitalRead(DIAL_PIN) != DIAL_EN_STATE
		&& digitalRead(TICK_PIN) != TICK_EN_STATE
		&& digitalRead(RING_PIN) != RINGING_STATE
	){
		state = &STANDBY;
	}
}
