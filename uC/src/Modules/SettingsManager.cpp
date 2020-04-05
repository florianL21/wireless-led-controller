#include "SettingsManager.h"

const String SettingsManager::fileVersion = "0.0.0";

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
	cfg.setAutoFormat(true); // automatically format if the filesystem is not usable
	initOK &= SPIFFS.setConfig(cfg);
	initOK &= SPIFFS.begin();
	return initOK;
}

bool SettingsManager::loadConfigFromMemory()
{
	bool loadOK = false;
	if(SPIFFS.exists(CONFIG_FILE_NAME))
	{
		File f = SPIFFS.open(CONFIG_FILE_NAME, "r");
		if (!f) 
		{
			DisplayManager::PrintStatus("E: Open config  file failed", 4);
		}
		else
		{
			loadOK = deserializeConfiguration(f.readString());
			f.close();
		}
	}
	else
	{
		//if the file does not exist yet init everything to the defaults values and write the file so that it can be used next time
		initWithDefaults();
		if(saveConfigToMemory() == false) // only display a warning here since saving the config file is not mandatory for correct system operation.
		{
			DisplayManager::PrintStatus("W: Config save error", 4);
		} 
		loadOK = true;
	}
	return loadOK;
}

bool SettingsManager::saveConfigToMemory()
{
	bool writeOK = false;
	String fileContent = serializeConfiguration();
	File f = SPIFFS.open(CONFIG_FILE_NAME, "w+");
	if (!f) 
	{
		DisplayManager::PrintStatus("E: Open config file failed", 4);
	}
	else
	{
		size_t writtenLen = f.print(fileContent);
		f.close();
		if(writtenLen == fileContent.length())
		{
			writeOK = true;
		}
		else
		{
			DisplayManager::PrintStatus("E: config write suspect", 4);
		}
	}
	return writeOK;
}

void SettingsManager::closeMemory()
{
	SPIFFS.end();
}

bool SettingsManager::initWithDefaults()
{
	SettingsManager::Framerate			= DEFAULT_FRAMERATE;
	SettingsManager::Brightness			= DEFAULT_BRIGHTNESS;
	SettingsManager::NumLeds 			= DEFAULT_NUM_LED;
	SettingsManager::NumFrames			= DEFAULT_NUM_FRAMES;
	SettingsManager::frameCounter		= 0;
	SettingsManager::animationActive	= DEFAULT_ANIM_ENABLE;
	SettingsManager::DisplayDebugInfo	= DEFAULT_DEBUG_MODE;

	DisplayManager::setDebugOutput(SettingsManager::DisplayDebugInfo);
	if(LEDManager::init(SettingsManager::NumLeds) == false)
	{
		DisplayManager::PrintStatus("LED init", 3);
		return false;
	}
	LEDManager::setFramerate(SettingsManager::Framerate);
	if(FrameBuffer::init(SettingsManager::NumLeds, SettingsManager::NumFrames) == false)
	{
		DisplayManager::PrintStatus("Framebuffer init", 3);
		return false;
	}
	return true;
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
	uint32 color = 0; //TODO: Maybe replace with some default color define
	for(uint16 f = 0; f < SettingsManager::NumFrames; f++)
	{
		JsonArray FrameArray= Frames.createNestedArray(String(f));
		for(uint16 l = 0; l < SettingsManager::NumLeds; l++)
		{
			if(FrameBuffer::initCheck() == true) //check if frame buffer is ready to be used
			{
				color = LEDManager::getColorCode(FrameBuffer::getLED(f, l));
			}
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

bool SettingsManager::deserializeConfiguration(String json, uint64 initialDocSize)
{
	#define ERROR_HANDLING(x, msg) if(x == false) { DisplayManager::PrintStatus(msg, 4); delete doc; return false;}
	//try  to initialize a json document big enought to fit all the data
	uint64 DocumentSize = initialDocSize;
	DynamicJsonDocument* doc;
	bool successful = false;

	//settings that have to be parsed. Must be initialized with their current values
	bool debugMode = DEFAULT_DEBUG_MODE;
	uint16 NumFrames = SettingsManager::NumFrames;
	uint16 NumLeds = SettingsManager::NumLeds;
	uint8 Brightness = SettingsManager::Brightness;
	uint16 Framerate = SettingsManager::Framerate;
	uint16 frameCounter = SettingsManager::frameCounter;
	bool animationActive = SettingsManager::animationActive;
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
		ERROR_HANDLING(parseValue<JsonObject>(doc, "Settings", &Settings), "E: Settings is mandatory");
		ERROR_HANDLING(parseValue<JsonObject>(doc, "Settings", &Settings), "E: Settings is mandatory");

		if(parseValue<String>(&Settings, "ConfigFileVersion", &fileVersion) == true) //check file version if present otherwise skip this step
		{
			if(fileVersion.equals(SettingsManager::fileVersion) == false)
			{
				DisplayManager::PrintStatus("W: config version changed", 4);
			}
		}
		ERROR_HANDLING(parseValue<uint16>(&Settings, "NumFrames", &NumFrames), "E: NumFrames is mandatory");
		ERROR_HANDLING(parseValue<uint16>(&Settings, "NumLeds", &NumLeds), "E: NumLeds is mandatory");

		parseValue<uint8>(&Settings, "Brightness", &Brightness);
		parseValue<uint16>(&Settings, "Framerate", &Framerate);
		parseValue<uint16>(&Settings, "ActiveFrame", &frameCounter);
		parseValue<bool>(&Settings, "AnimationActive", &animationActive);
		parseValue<bool>(&Settings, "DisplayDebugInfo", &debugMode);

		JsonObject Frames;
		ERROR_HANDLING(parseValue<JsonObject>(doc, "Frames", &Frames), "E: No Frame data");

		//Setting all values after all sanity checks are passed to not create an invalid config if something along the line fails
		SettingsManager::NumFrames = NumFrames;
		SettingsManager::NumLeds = NumLeds;
		SettingsManager::Brightness = Brightness;
		SettingsManager::Framerate = Framerate;
		SettingsManager::frameCounter = frameCounter;
		SettingsManager::animationActive = animationActive;

		if(debugMode == true)
		{
			SettingsManager::DisplayDebugInfo = DISPLAY_DEBUG_ENABLED;
		}
		else
		{
			SettingsManager::DisplayDebugInfo = DISPLAY_DEBUG_DISABLED;
		}

		//call all the init methods
		DisplayManager::setDebugOutput(SettingsManager::DisplayDebugInfo);
		ERROR_HANDLING(LEDManager::init(SettingsManager::NumLeds), "E: LED init");
		LEDManager::setFramerate(SettingsManager::Framerate);
		ERROR_HANDLING(FrameBuffer::init(SettingsManager::NumLeds, SettingsManager::NumFrames), "E: Framebuffer init");

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