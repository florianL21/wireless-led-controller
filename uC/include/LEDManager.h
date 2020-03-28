#pragma once

#include <Arduino.h>
#define FASTLED_INTERNAL
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include "SettingsManager.h"
#include "DisplayManager.h"
#include "Configuration.h"
#include "FrameBuffer.h"

class LEDManager
{
private:
	static CRGB *leds;
	static CFastLED* FastLedConfig;
	static uint64 oldMillis;
	static uint16 refreshIntervall;

	static void Print(uint16 frameNum);

	LEDManager();
public:
	static void init(uint16 numLEDs);
	
	static void update();
	static void setFramerate(uint16 Framerate);
	static uint32 getColorCode(CRGB led);
	static void setBrightness(uint8 brightness);
};