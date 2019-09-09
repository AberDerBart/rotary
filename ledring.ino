
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

void showRing(){
  for(long firstPixelHue = 0; firstPixelHue < 65536; firstPixelHue += 256) {
    for(int i=0; i<16; i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / 16);
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
    }
    pixels.show(); // Update strip with new contents
    delay(10);  // Pause for a moment
  }
  pixels.clear();
  pixels.show();
  delay(500);
  
}

void showRing2(){
	for(int i = 16; i > 0; i--){
		pixels.setPixelColor((i + LED_OFFSET + 1) % 16, pixels.Color(10,0,0));
		pixels.setPixelColor((i + LED_OFFSET + 2) % 16, pixels.Color(20,0,0));
		pixels.setPixelColor((i + LED_OFFSET + 3) % 16, pixels.Color(40,0,0));
		pixels.setPixelColor((i + LED_OFFSET + 4) % 16, pixels.Color(80,0,0));
		pixels.setPixelColor((i + LED_OFFSET + 5) % 16, pixels.Color(40,0,0));
		pixels.setPixelColor((i + LED_OFFSET + 6) % 16, pixels.Color(20,0,0));
		pixels.setPixelColor((i + LED_OFFSET + 7) % 16, pixels.Color(10,0,0));
		pixels.setPixelColor((i + LED_OFFSET + 0) % 16, pixels.Color(0,0,0));

		pixels.show();
		delay(100);
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

void clear(){
	pixels.clear();
	pixels.show();
}

void showRing3(){
	for(int rot = 0; rot < 64; rot++){
		for(int i = 0; i < 16; i++){
			int pixelBrightness = brightnessCurve[(i * 4 + rot) % 64];
			int pixelBrightness2 = brightnessCurve[(i * 4 + rot + 8) % 64];
			int pixelBrightness3 = brightnessCurve[(i * 4 + rot + 16) % 64];
			pixels.setPixelColor(i, pixels.Color(pixelBrightness, pixelBrightness2, pixelBrightness3));
		}
		delay(30);
		pixels.show();
	}
}

void showNetwork(){
  for(int i = 0; i <= 16; i++){
    showPie(i, pixels.Color(0,0,10), pixels.Color(10,0,0));  
    delay(300);
  }
}
