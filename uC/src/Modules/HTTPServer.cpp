#include "HTTPServer.h"

ESP8266WebServer* HTTPServer::http_rest_server = NULL;

void HTTPServer::init(int port)
{
    http_rest_server = new ESP8266WebServer(port);
    config_rest_server_routing();
    http_rest_server->begin();
}

int HTTPServer::init_wifi(const char* wifi_ssid, const char* wifi_passwd, uint8 numRetrys, uint16 retryDelay)
{
	int retries = 0;
	DisplayManager::PrintStatus("Connecting to", 1);
	DisplayManager::PrintStatus(String(wifi_ssid) + DisplayManager::getLoadingAnimation(), 2);
	WiFi.mode(WIFI_STA);
	WiFi.begin(wifi_ssid, wifi_passwd);
	// check the status of WiFi connection to be WL_CONNECTED
	while ((WiFi.status() != WL_CONNECTED) && (retries < numRetrys)) 
	{
		retries++;
		delay(retryDelay);
		DisplayManager::PrintStatus(String(wifi_ssid) + " " + DisplayManager::getLoadingAnimation(), 2);
	}
	return WiFi.status(); // return the WiFi connection status
}

void HTTPServer::getAll()
{
	DisplayManager::PrintStatus("Sending config ...", 4, DISPLAY_DEBUG_OUTPUT);
	//							settings+headroom		the 2 nested arrays						all led arrays												single led array length					frames array length							general headroom for copy operations
	const size_t FrameCapacity = JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(2) + SettingsManager::NumFrames*JSON_ARRAY_SIZE(SettingsManager::NumLeds) + JSON_OBJECT_SIZE(SettingsManager::NumLeds) + JSON_OBJECT_SIZE(SettingsManager::NumFrames) + 130 + 40 * SettingsManager::NumFrames;
	DynamicJsonDocument doc(FrameCapacity);
	JsonObject root = doc.to<JsonObject>();
	JsonObject Settings = root.createNestedObject("Settings");
	Settings["Brightness"] = SettingsManager::Brightness;
	Settings["Framerate"] = SettingsManager::Framerate;
	Settings["NumFrames"] = SettingsManager::NumFrames;
	Settings["NumLeds"] = SettingsManager::NumLeds;
	Settings["ActiveFrame"] = SettingsManager::frameCounter;
	Settings["AnimationActive"] = SettingsManager::animationActive;

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
	http_rest_server->send(length, "application/json", JSONmessageBuffer);
	DisplayManager::PrintStatus("Sending config done", 4, DISPLAY_DEBUG_OUTPUT);
}

void HTTPServer::getSettings()
{
	DisplayManager::PrintStatus("Sending settings ...", 4, DISPLAY_DEBUG_OUTPUT);
	const size_t FrameCapacity = JSON_OBJECT_SIZE(4);
	DynamicJsonDocument doc(FrameCapacity);
	JsonObject root = doc.to<JsonObject>();
	root["Brightness"] = SettingsManager::Brightness;
	root["Framerate"] = SettingsManager::Framerate;
	root["NumFrames"] = SettingsManager::NumFrames;
	root["NumLeds"] = SettingsManager::NumLeds;
	uint32 length = measureJson(doc) + 1;
	char *JSONmessageBuffer = new char[length];
	serializeJson(doc, JSONmessageBuffer, length);
	http_rest_server->send(length, "application/json", JSONmessageBuffer);
	DisplayManager::PrintStatus("Sending settings done", 4, DISPLAY_DEBUG_OUTPUT);
}

void HTTPServer::postLed()
{
	const size_t FrameCapacity = JSON_OBJECT_SIZE(5);
	DynamicJsonDocument jsonBody(FrameCapacity);
	DeserializationError error = deserializeJson(jsonBody, http_rest_server->arg("plain"));

	if (error) 
    {
		DisplayManager::PrintStatus("Error parsing json", 4);
		http_rest_server->send(400);
	}
	else
	{
		DisplayManager::PrintStatus("Updateing single LED", 4, DISPLAY_DEBUG_OUTPUT);

		uint16 frame = 0;
		JsonVariant jFrame = jsonBody["Frame"];
		if (jFrame.isNull())
		{
			DisplayManager::PrintStatus("E: Frame num required", 4);
			http_rest_server->send(204);
			return;
		}
		if(!jFrame.is<uint16>())
		{
			DisplayManager::PrintStatus("E: Frame not uint16", 4);
			http_rest_server->send(204);
			return;
		}
		frame = jFrame.as<uint16>();

		if(frame >= SettingsManager::NumFrames)
		{
			DisplayManager::PrintStatus("Invalid frame " + String(frame) + "max = " + String(SettingsManager::NumFrames), 4);
			http_rest_server->send(204);
			return;
		}

		uint16 id = 0;
		JsonVariant jId = jsonBody["id"];
		if (jId.isNull())
		{
			DisplayManager::PrintStatus("E: LED ID required", 4);
			http_rest_server->send(204);
			return;
		}
		if(!jId.is<uint16>())
		{
			DisplayManager::PrintStatus("E: ID not uint16", 4);
			http_rest_server->send(204);
			return;
		}
		id = jId.as<uint16>();

		if(id >= SettingsManager::NumLeds)
		{
			DisplayManager::PrintStatus("Invalid id " + String(id) + "max = " + String(SettingsManager::NumLeds), 4);
			http_rest_server->send(204);
			return;
		}

		uint32 color = 0;
		JsonVariant jColor = jsonBody["color"];
		if (jColor.isNull())
		{
			DisplayManager::PrintStatus("E: LED ID required", 4);
			http_rest_server->send(204);
			return;
		}
		if(!jColor.is<uint32>())
		{
			DisplayManager::PrintStatus("E: Color not uint32", 4);
			http_rest_server->send(204);
			return;
		}
		color = jColor.as<uint32>();
        
        FrameBuffer::setLED(frame, id, color);
		http_rest_server->sendHeader("Location", "/led/" + String(id));
		http_rest_server->send(200);
		DisplayManager::PrintStatus("LED update done", 4, DISPLAY_DEBUG_OUTPUT);
	}
}

void HTTPServer::postFrame()
{
	const size_t FrameCapacity = JSON_ARRAY_SIZE(SettingsManager::NumLeds) + JSON_OBJECT_SIZE(2) + 10 * SettingsManager::NumLeds;
	DynamicJsonDocument jsonBody(FrameCapacity);
	DeserializationError error = deserializeJson(jsonBody, http_rest_server->arg("plain"));

	if (error) 
    {
		DisplayManager::PrintStatus("Error parsing json", 4);
		http_rest_server->send(400);
	}
	else
	{
		uint16 frame = 0;
		JsonVariant jFrame = jsonBody["Frame"];
		if (jFrame.isNull())
		{
			DisplayManager::PrintStatus("E: Frame num required", 4);
			http_rest_server->send(204);
			return;
		}
		if(!jFrame.is<uint16>())
		{
			DisplayManager::PrintStatus("E: Frame not uint16", 4);
			http_rest_server->send(204);
			return;
		}
		frame = jFrame.as<uint16>();
		DisplayManager::PrintStatus("Updateing frame " + String(frame), 4, DISPLAY_DEBUG_OUTPUT);
		if(frame >= SettingsManager::NumFrames)
		{
			DisplayManager::PrintStatus("Invalid frame " + String(frame) + "max = " + String(SettingsManager::NumFrames), 4);
			http_rest_server->send(204);
			return;
		}
		JsonArray Leds = jsonBody["Leds"];
		if (Leds.isNull())
		{
			DisplayManager::PrintStatus("E: LED array required", 4);
			http_rest_server->send(204);
			return;
		}
		uint16 i = 0;
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
					FrameBuffer::setLED(frame, i, v.as<uint32>());
				}
			}
			else
			{
				DisplayManager::PrintStatus("Led id " + String(i) + " ignored. Too high", 4);
			}
			i++;
		}
		http_rest_server->sendHeader("Location", "/leds/" + String(frame));
		http_rest_server->send(200);
		DisplayManager::PrintStatus("Frame " + String(frame) + " updated", 4, DISPLAY_DEBUG_OUTPUT);
	}
}

void HTTPServer::postSettings()
{
	const size_t FrameCapacity = JSON_OBJECT_SIZE(10);
	DynamicJsonDocument jsonBody(FrameCapacity);
	DeserializationError error = deserializeJson(jsonBody, http_rest_server->arg("plain"));

	DisplayManager::PrintStatus("Updateing settings", 4, DISPLAY_DEBUG_OUTPUT);
	
	if (error) {
		DisplayManager::PrintStatus("Error parsing json", 4);
		http_rest_server->send(400);
	}
	else 
	{
		bool numLedsChanged = false;
		bool numFramesChanged = false;
		JsonVariant brightness = jsonBody["Brightness"];
		if (!brightness.isNull())
		{
			if(!brightness.is<uint8>())
			{
				DisplayManager::PrintStatus("E: Brightness not uint8", 4);
				http_rest_server->send(204);
				return;
			}
			SettingsManager::Brightness = brightness.as<uint8>();
		}
		JsonVariant framerate = jsonBody["Framerate"];
		if (!framerate.isNull())
		{
			if(!framerate.is<uint16>())
			{
				DisplayManager::PrintStatus("E: Framerate not uint16", 4);
				http_rest_server->send(204);
				return;
			}
			LEDManager::setFramerate(framerate.as<uint16>());
		}
		
		JsonVariant animationRunning = jsonBody["AnimationActive"];
		if (!animationRunning.isNull())
		{
			if(!animationRunning.is<bool>())
			{
				DisplayManager::PrintStatus("E: AnimationActive not bool", 4);
				http_rest_server->send(204);
				return;
			}
			SettingsManager::animationActive = animationRunning.as<bool>();
		}

		JsonVariant activeFrame = jsonBody["ActiveFrame"];
		if (!activeFrame.isNull())
		{
			if(!activeFrame.is<uint16>())
			{
				DisplayManager::PrintStatus("E: ActiveFrame not uint16", 4);
				http_rest_server->send(204);
				return;
			}
			SettingsManager::frameCounter = activeFrame.as<uint16>();
		}

		JsonVariant numFrames = jsonBody["NumFrames"];
		if (!numFrames.isNull()) 
		{
			if(!numFrames.is<uint16>())
			{
				DisplayManager::PrintStatus("E: NumFrames not uint16", 4);
				http_rest_server->send(204);
				return;
			}
			if(SettingsManager::NumFrames != numFrames.as<uint16>())
			{
				SettingsManager::NumFrames = numFrames.as<uint16>();
				numFramesChanged = true;
			}
		}
		JsonVariant numLeds = jsonBody["NumLeds"];
		if (!numLeds.isNull())
		{
			if(!numLeds.is<uint16>())
			{
				DisplayManager::PrintStatus("E: NumLeds not uint16", 4);
				http_rest_server->send(204);
				return;
			}
			if(SettingsManager::NumLeds != numLeds.as<uint16>())
			{
				SettingsManager::NumLeds = numLeds.as<uint16>();
				numLedsChanged = true;
			}
		}

		if(numLedsChanged)
		{
            LEDManager::init(SettingsManager::NumLeds);
		}
		if(numLedsChanged || numFramesChanged)
		{
            FrameBuffer::init(SettingsManager::NumLeds, SettingsManager::NumFrames);
		}
		http_rest_server->sendHeader("Location", "/settings");
		http_rest_server->send(200);
		DisplayManager::PrintStatus("Settings update done", 4, DISPLAY_DEBUG_OUTPUT);
	}
}

void HTTPServer::config_rest_server_routing() 
{
	http_rest_server->on("/", HTTP_GET, getAll);
	http_rest_server->on("/led", HTTP_POST, postLed);
	http_rest_server->on("/leds", HTTP_POST, postFrame);
	http_rest_server->on("/settings", HTTP_GET, getSettings);
	http_rest_server->on("/settings", HTTP_POST, postSettings);
}

void HTTPServer::processRequests()
{
    http_rest_server->handleClient();
}