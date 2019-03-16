
bool updateSerial(){
	// for handling serial connects and disconnects
	static bool connectionActive = false;

	if(Serial && !connectionActive){
		Serial.begin(115200);
		connectionActive = true;
		Serial.println("rotary v0.1");
	}

	if(!Serial && connectionActive){
		Serial.end();
		connectionActive = false;
	}

	//if the state change, write it to serial
	static void* lastState = NULL;
	if(connectionActive){
		if(lastState != state){
			Serial.println(stateString(state));
			lastState = state;
		}
	}

	return connectionActive;
}

