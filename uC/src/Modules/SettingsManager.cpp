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
	bool initOK = true;
	SPIFFSConfig cfg;
	cfg.setAutoFormat(false);
	initOK &= SPIFFS.setConfig(cfg);
	initOK &= SPIFFS.begin();
	SPIFFS.end();
	return initOK;
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

template <class valueType, class jsonObjectType>
bool SettingsManager::parseValue(jsonObjectType* root, String key, valueType* value) 
{
	bool sucessful = false;
	JsonVariant jvalue = (*root)[key];
	if (!jvalue.isNull())
	{
		if(!jvalue.is<valueType>())
		{
			DisplayManager::PrintStatus("E: " + key + " out of bouds", 4);
		}
		else
		{
			*value = jvalue.as<valueType>();
			sucessful = true;
		}
	}
	return sucessful;
}

void SettingsManager::deserializeConfiguration(String json, uint16 numFrames, uint16 numLeds)
{
	//							settings+headroom		the 2 nested arrays			all led arrays					frames array length			general headroom for copy operations
	const size_t FrameCapacity = JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(2) + numFrames*JSON_ARRAY_SIZE(numLeds)  + JSON_OBJECT_SIZE(numFrames) + 10 * numFrames;
	DynamicJsonDocument doc(FrameCapacity);
	DeserializationError error = deserializeJson(doc, json);

	if (error)
    {
		DisplayManager::PrintStatus("E: json:" + String(error.c_str()), 4);
	}
	else
	{
		JsonObject Settings;
		if(parseValue<JsonObject>(&doc, "Settings", &Settings) == false)
		{
			DisplayManager::PrintStatus("E: Settings is mandatory", 4);
			return;
		}
		parseValue<uint8>(&Settings, "Brightness", &SettingsManager::Brightness);
		parseValue<uint16>(&Settings, "Framerate", &SettingsManager::Framerate);
		if(parseValue<uint16>(&Settings, "NumFrames", &SettingsManager::NumFrames) == false)
		{
			DisplayManager::PrintStatus("E: NumFrames is mandatory", 4);
			return;
		}
		
		if(parseValue<uint16>(&Settings, "NumLeds", &SettingsManager::NumLeds) == false)
		{
			DisplayManager::PrintStatus("E: NumLeds is mandatory", 4);
			return;
		}
		parseValue<uint16>(&Settings, "ActiveFrame", &SettingsManager::frameCounter);
		parseValue<bool>(&Settings, "AnimationActive", &SettingsManager::animationActive);
		parseValue<String>(&Settings, "ConfigFileVersion", &SettingsManager::fileVersion);
		bool debugMode = DEFAULT_DEBUG_MODE;
		parseValue<bool>(&Settings, "DisplayDebugInfo", &debugMode);

		if(debugMode == true)
		{
			SettingsManager::DisplayDebugInfo = DISPLAY_DEBUG_ENABLED;
		}
		else
		{
			SettingsManager::DisplayDebugInfo = DISPLAY_DEBUG_DISABLED;
		}

		JsonObject Frames;
		if(parseValue<JsonObject>(&doc, "Frames", &Frames) == false)
		{
			DisplayManager::PrintStatus("E: No Frame data", 4);
			
			return;
		}
		FrameBuffer::init(SettingsManager::NumLeds, SettingsManager::NumFrames);

		JsonArray Leds;
		uint16 i = 0;
		for(uint16 f = 0; f < SettingsManager::NumFrames; f++)
		{
			if(parseValue<JsonArray>(&Frames, String(f), &Leds) == true)
			{
				i = 0;
				for(JsonVariant v : Leds)
				{
					if(i < SettingsManager::NumLeds)
					{
						if(!v.is<uint32>())
						{
							DisplayManager::PrintStatus("E: Color led "  + String(i) + " not uint32", 4);
						}
						else
						{
							FrameBuffer::setLED(f, i, v.as<uint32>());
						}
					}
					else
					{
						DisplayManager::PrintStatus("Led id " + String(i) + " ignored. Too high", 4);
					}
					i++;
				}
			}
		}
	}
}