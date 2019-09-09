
#include <Adafruit_NeoPixel.h>
#include "config.h"

Adafruit_NeoPixel pixels(16, LED_PIN, NEO_GRB + NEO_KHZ800);

void initLed(){
  pixels.begin();
}

void showPie(int part, uint32_t c1, uint32_t c2){
  pixels.clear();
  for(int i = 0; i < 16; i++){
    uint32_t c = i < part ? c1:c2;
    int index = (i + LED_OFFSET) % 16;
    pixels.setPixelColor(index, c);
  }
  pixels.show();
}

void showBattery(){
  for(int i = 0; i <= 16; i++){
    showPie(4, pixels.Color(0,10,0), pixels.Color(10,0,0));  
    delay(300);
  }
  
}

int brightnessCurve[] = {
	10,20,40,80,
	120,160,180,190,
	200,200,190,180,
	160,120,80,40,
	20,10,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0
};

void clearLed(){
	pixels.clear();
	pixels.show();
}

void showRing(){
	static int rot = 0;

	// rotate further each time this is called
	rot++;

	for(int i = 0; i < 16; i++){
		int pixelBrightness = brightnessCurve[(i * 4 + rot) % 64];
		int pixelBrightness2 = brightnessCurve[(i * 4 + rot + 8) % 64];
		int pixelBrightness3 = brightnessCurve[(i * 4 + rot + 16) % 64];
		pixels.setPixelColor(i, pixels.Color(pixelBrightness, pixelBrightness2, pixelBrightness3));
	}
	pixels.show();
}

void showNetwork(){
  for(int i = 0; i <= 16; i++){
    showPie(i, pixels.Color(0,0,10), pixels.Color(10,0,0));  
    delay(300);
  }
}
