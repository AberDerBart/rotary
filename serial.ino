// defines debug strings for the states
char* stateString(void* state){
	if(state == &STANDBY) return "STANDBY";
	if(state == &DIALING) return "DIALING";
	if(state == &PHONING) return "PHONING";
	if(state == &RINGING) return "RINGING";
	if(state == &MENU) return "MENU";
	if(state == &BATTERY) return "BATTERY";
	if(state == &NETWORK) return "NETWORK";
	if(state == &ERROR) return "ERROR";
	return "INVALID";
}

// for handling serial connects and disconnects
bool connectionActive(){
	static bool active = false;

	if(Serial && !active){
		Serial.begin(115200);
		active = true;
		Serial.println("rotary v0.1");
	}

	if(!Serial && active){
		Serial.end();
		active = false;
	}

	return active;
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

void handleAtCommand(){
	fona.write('A');
	fona.write('T');

	Serial.print("AT");

	for(char tmp = Serial.read(); tmp != '\r'; tmp = Serial.read()){
		fona.write(tmp);
		Serial.write(tmp);
	}

	fona.write('\r');
	
	while(fona.available()){
		Serial.write(fona.read());
		delay(1);
	}

	Serial.println("");
}

void printInfo(){
	Serial.print("State:   ");
	Serial.println(stateString(state));
}


bool updateSerial(){
	static void* lastState = NULL;

	if(connectionActive()){

		//if the state change, write it to serial
		if(lastState != state){
			printInfo();
			lastState = state;
		}

		//pass through serial communication
		while(Serial.available()){
			fona.write(Serial.read());
		}

		while(fona.available()){
			Serial.write(fona.read());
		}
	}
}

