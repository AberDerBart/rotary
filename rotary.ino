void (*state)();

void setup() {
	Serial.begin(11520);
	Serial.println("Hello World!");
	state=&STANDBY;
}

void loop() {
	(*state)();
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
}

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
}

void PHONING(){
	/*
	-hook down:
		-close call
		->STANDBY
	*/
	state=&STANDBY;
}

void RINGING(){
	/*
	-hook up:
		PHONING
	-incoming call ended:
		->STANDBY
	*/
}
