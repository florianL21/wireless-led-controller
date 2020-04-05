#pragma once

#include <Arduino.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "DisplayManager.h"
#include "Configuration.h"
#include "LEDManager.h"
#include "FrameBuffer.h"

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
	static debugOutput DisplayDebugInfo;
	static String fileVersion;

	static bool init();
	template <class valueType, class jsonObjectType>
	static bool parseValue(jsonObjectType* root, String key, valueType* value);
	static String serializeConfiguration();
	static bool deserializeConfiguration(String json);
	static bool loadConfigFromMemory();
	static bool saveConfigToMemory();
	static void closeMemory();
};

