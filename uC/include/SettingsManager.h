#pragma once

#include <Arduino.h>
#include "Configuration.h"

class SettingsManager
{
private:
	SettingsManager();
public:
	static uint16 Framerate;
	static uint8 Brightness;
	static uint16 NumLeds;
	static uint16 NumFrames;
	static uint16 frameCounter;
	static bool animationActive;
};

