#include "LEDManager.h"

CRGB* LEDManager::leds = NULL;
CFastLED* LEDManager::FastLedConfig = NULL;
uint64 LEDManager::oldMillis = 0;
uint16 LEDManager::refreshIntervall = 10 ;

void LEDManager::init(uint16 numLEDs)
{
    if(leds != NULL)
	{
		free(leds);
	}
	leds = new CRGB[numLEDs];
	if(leds == NULL)
	{
		DisplayManager::PrintStatus("Not enought memory", 1);
	}
	else
	{
		FastLedConfig = new CFastLED();
		FastLedConfig->addLeds<LED_TYPE, LED_DATA_PIN, LED_COLOR_MODE>(leds, numLEDs);
		FastLED.setBrightness(SettingsManager::Brightness);
	}
}

void LEDManager::Print(uint16 frameNum)
{
	for (uint16 i = 0; i < SettingsManager::NumLeds; i++)
	{
		leds[i] = FrameBuffer::getLED(frameNum, i);
	}
	FastLED.show();
}

void LEDManager::setFramerate(uint16 Framerate)
{
	if(Framerate == 0)
	{
		SettingsManager::Framerate = 1;
		SettingsManager::animationActive = false;
	}
	else
	{
		SettingsManager::Framerate = Framerate;
	}
	refreshIntervall = 1000 / Framerate;
}

void LEDManager::update()
{
	Print(SettingsManager::frameCounter);
	if(oldMillis + refreshIntervall < millis())
	{
		if(SettingsManager::animationActive)
		{
			SettingsManager::frameCounter++;
			if(SettingsManager::frameCounter >= SettingsManager::NumFrames)
			{
				SettingsManager::frameCounter = 0;
			}
		}
		oldMillis = millis();
	}
}

uint32 LEDManager::getColorCode(CRGB led)
{
	uint32 colorCode = 0x0;
	colorCode |= led.red;
	colorCode |= (led.green << 8);
	colorCode |= (led.blue << 16);
	return colorCode;
}

void LEDManager::setBrightness(uint8 brightness)
{
	SettingsManager::Brightness = brightness;
	FastLED.setBrightness(brightness);
}