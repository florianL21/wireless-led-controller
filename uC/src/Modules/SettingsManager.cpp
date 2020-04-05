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
	return initOK;
}

bool SettingsManager::loadConfigFromMemory()
{
	if(SPIFFS.exists(CONFIG_FILE_NAME))
	{

	}
	return true;
}

bool SettingsManager::saveConfigToMemory()
{
	return true;
}

void SettingsManager::closeMemory()
{
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

bool SettingsManager::deserializeConfiguration(String json)
{
	//try  to initialize a json document big enought to fit all the data
	uint32 DocumentSize = JSON_BUFFER_CHUNK_SIZE;
	DynamicJsonDocument* doc;
	bool successful = false;

	bool debugMode = DEFAULT_DEBUG_MODE;
	uint16 NumFrames;
	uint16 NumLeds;
	uint8 Brightness;
	uint16 Framerate;
	uint16 frameCounter;
	bool animationActive;
	String fileVersion;

	do
	{
		doc = new DynamicJsonDocument(DocumentSize);
		DeserializationError error = deserializeJson(*doc, json);
		if (error)
		{
			delete doc;
			if (error.code() == DeserializationError::NoMemory)
			{
				DocumentSize += JSON_BUFFER_CHUNK_SIZE;
			}
			else
			{
				DisplayManager::PrintStatus("E: json:" + String(error.c_str()), 4);
				return false;
			}
		}
		else
		{
			successful = true;
			break; //no error detected -> memory size is okay
		}
	} while (DocumentSize < MAX_JSON_BUFFER);
	
	if(successful == true)
	{
		JsonObject Settings;
		if(parseValue<JsonObject>(doc, "Settings", &Settings) == false)
		{
			DisplayManager::PrintStatus("E: Settings is mandatory", 4);
			delete doc;
			return false;
		}
		if(parseValue<uint16>(&Settings, "NumFrames", &NumFrames) == false)
		{
			DisplayManager::PrintStatus("E: NumFrames is mandatory", 4);
			delete doc;
			return false;
		}
		if(parseValue<uint16>(&Settings, "NumLeds", &NumLeds) == false)
		{
			DisplayManager::PrintStatus("E: NumLeds is mandatory", 4);
			delete doc;
			return false;
		}

		parseValue<uint8>(&Settings, "Brightness", &Brightness);
		parseValue<uint16>(&Settings, "Framerate", &Framerate);
		parseValue<uint16>(&Settings, "ActiveFrame", &frameCounter);
		parseValue<bool>(&Settings, "AnimationActive", &animationActive);
		parseValue<String>(&Settings, "ConfigFileVersion", &fileVersion);
		parseValue<bool>(&Settings, "DisplayDebugInfo", &debugMode);

		JsonObject Frames;
		if(parseValue<JsonObject>(doc, "Frames", &Frames) == false)
		{
			DisplayManager::PrintStatus("E: No Frame data", 4);
			delete doc;
			return false;
		}

		// initializing Framebuffer
		if(FrameBuffer::init(NumLeds, NumFrames) == false)
		{
			delete doc;
			return false;
		}

		//Setting all values after all sanity checks are passed to not create an invalid config if something along the line fails
		SettingsManager::NumFrames = NumFrames;
		SettingsManager::NumLeds = NumLeds;
		SettingsManager::Brightness = Brightness;
		SettingsManager::Framerate = Framerate;
		SettingsManager::frameCounter = frameCounter;
		SettingsManager::animationActive = animationActive;
		SettingsManager::fileVersion = fileVersion;

		if(debugMode == true)
		{
			SettingsManager::DisplayDebugInfo = DISPLAY_DEBUG_ENABLED;
		}
		else
		{
			SettingsManager::DisplayDebugInfo = DISPLAY_DEBUG_DISABLED;
		}

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
		delete doc;
		return true;
	}
	return false;
}