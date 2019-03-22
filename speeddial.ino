
//TODO: change this to use EEPROM
char speeddial[9][30];

char* getSpeedDial(int digit){
	return speeddial[digit];
}

void setSpeedDial(int digit, char* number){
	int i=0; 
	for(;number[0]; i++){
		speeddial[digit][i] = number[i];
	}
	speeddial[digit][i] = 0;
}

