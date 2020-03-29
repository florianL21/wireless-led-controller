#include "SettingsManager.h"

String SettingsManager::fileVersion = "0.0.0";

uint16 SettingsManager::Framerate				= DEFAULT_FRAMERATE;
uint8 SettingsManager::Brightness				= DEFAULT_BRIGHTNESS;
uint16 SettingsManager::NumLeds 				= DEFAULT_NUM_LED;
uint16 SettingsManager::NumFrames				= DEFAULT_NUM_FRAMES;
uint16 SettingsManager::frameCounter			= 0;
bool SettingsManager::animationActive			= DEFAULT_ANIM_ENABLE;
debugOutput SettingsManager::DisplayDebugInfo	= DEFAULT_DEBUG_MODE;


bool SettingsManager::init()
{
	SPIFFSConfig cfg;
	cfg.setAutoFormat(false);
	SPIFFS.setConfig(cfg);
	SPIFFS.begin();
	SPIFFS.end();
}

String SettingsManager::serializeConfiguration()
{
	//							settings+headroom		the 2 nested arrays						all led arrays												single led array length					frames array length							general headroom for copy operations
	const size_t FrameCapacity = JSON_OBJECT_SIZE(10) + JSON_OBJECT_SIZE(2) + SettingsManager::NumFrames*JSON_ARRAY_SIZE(SettingsManager::NumLeds) + JSON_OBJECT_SIZE(SettingsManager::NumLeds) + JSON_OBJECT_SIZE(SettingsManager::NumFrames) + 130 + 40 * SettingsManager::NumFrames;
	DynamicJsonDocument doc(FrameCapacity);
	JsonObject root = doc.to<JsonObject>();
	JsonObject Settings = root.createNestedObject("Settings");
	Settings["Brightness"] = SettingsManager::Brightness;
	Settings["Framerate"] = SettingsManager::Framerate;
	Settings["NumFrames"] = SettingsManager::NumFrames;
	Settings["NumLeds"] = SettingsManager::NumLeds;
	Settings["ActiveFrame"] = SettingsManager::frameCounter;
	Settings["AnimationActive"] = SettingsManager::animationActive;
	Settings["ConfigFileVersion"] = SettingsManager::fileVersion;
	if(SettingsManager::DisplayDebugInfo == DISPLAY_DEBUG_ENABLED)
	{
		Settings["DisplayDebugInfo"] = true;
	}
	else
	{
		Settings["DisplayDebugInfo"] = false;
	}

	JsonObject Frames = root.createNestedObject("Frames");
	for(uint16 f = 0; f < SettingsManager::NumFrames; f++)
	{
		JsonArray FrameArray= Frames.createNestedArray(String(f));
		for(uint16 l = 0; l < SettingsManager::NumLeds; l++)
		{
			uint32 color = LEDManager::getColorCode(FrameBuffer::getLED(f, l));
			FrameArray.add(color);
		}
	}
	uint32 length = measureJson(doc) + 1;
	char *JSONmessageBuffer = new char[length];
	serializeJson(doc, JSONmessageBuffer, length);
	return String(JSONmessageBuffer);
}

