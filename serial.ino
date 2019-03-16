bool connectionActive(){
	// for handling serial connects and disconnects
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

bool updateSerial(){
	//if the state change, write it to serial
	static void* lastState = NULL;

	if(connectionActive()){
		if(lastState != state){
			Serial.println(stateString(state));
			lastState = state;
		}

		if(Serial.available()){

		}
	}
}

