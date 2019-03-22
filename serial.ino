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

void setSpeedDial(){
	char digit = Serial.read() - '1';
	
	// read expected newline
	Serial.read();

	if(digit >= 0 && digit <= 8){
		Serial.readBytesUntil('\n',getSpeedDial(digit),29);
	}else{
		Serial.println("e");
	}
}

bool updateSerial(){
	static void* lastState = NULL;

	if(connectionActive()){

		//if the state change, write it to serial
		if(lastState != state){
			Serial.println(stateString(state));
			lastState = state;
		}

		if(Serial.available()){
			// see serial.md for details on the serial communication
			char function = Serial.read();

			// read expected newline
			Serial.read();

			switch(function){
			case 's':
				setSpeedDial();
				break;
			default:
				Serial.println("e");
				break;
			}
		}
	}
}

