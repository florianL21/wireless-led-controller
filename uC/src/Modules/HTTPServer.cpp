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

void HTTPServer::get_all()
{
	const size_t FrameCapacity = SettingsManager::NumFrames*JSON_ARRAY_SIZE(SettingsManager::NumLeds) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(SettingsManager::NumLeds) + JSON_OBJECT_SIZE(SettingsManager::NumFrames) + 130 + 40 * SettingsManager::NumFrames;
	DynamicJsonDocument doc(FrameCapacity);
	JsonObject root = doc.to<JsonObject>();
	JsonObject Settings = root.createNestedObject("Settings");
	Settings["Brightness"] = SettingsManager::Brightness;
	Settings["Framerate"] = SettingsManager::Framerate;
	Settings["NumFrames"] = SettingsManager::NumFrames;
	Settings["NumLeds"] = SettingsManager::NumLeds;
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
}

void HTTPServer::post_put_led() 
{
	DynamicJsonDocument jsonBody(500);
	DeserializationError error = deserializeJson(jsonBody, http_rest_server->arg("plain"));

	DisplayManager::PrintStatus("HTTP Method: " + String(http_rest_server->method()), 4);
	
	if (error) 
    {
		DisplayManager::PrintStatus("Error parsing json", 4);
		http_rest_server->send(400);
	}
	else
	{
		uint16 frame = jsonBody["Frame"];
		uint16 id = jsonBody["id"];
		if(frame >= SettingsManager::NumFrames)
		{
			DisplayManager::PrintStatus("Invalid frame " + String(frame) + "max = " + String(SettingsManager::NumFrames), 4);
			http_rest_server->send(204);
			return;
		}
		if(id >= SettingsManager::NumLeds)
		{
			DisplayManager::PrintStatus("Invalid id " + String(id) + "max = " + String(SettingsManager::NumLeds), 4);
			http_rest_server->send(204);
			return;
		}
        uint32 color = jsonBody["color"];
        FrameBuffer::setLED(frame, id, color);
		http_rest_server->sendHeader("Location", "/led/" + String(id));
		http_rest_server->send(200);
	}
}

void HTTPServer::post_put_leds()
{
	const size_t FrameCapacity = JSON_ARRAY_SIZE(SettingsManager::NumLeds) + JSON_OBJECT_SIZE(2) + 10 * SettingsManager::NumLeds;
	DynamicJsonDocument jsonBody(FrameCapacity);
	DeserializationError error = deserializeJson(jsonBody, http_rest_server->arg("plain"));

	DisplayManager::PrintStatus("HTTP Method: " + String(http_rest_server->method()), 4);
	
	if (error) 
    {
		DisplayManager::PrintStatus("Error parsing json", 4);
		http_rest_server->send(400);
	}
	else
	{
		uint16 frame = jsonBody["Frame"];
		JsonArray Leds = jsonBody["Leds"];
		if(frame >= SettingsManager::NumFrames)
		{
			DisplayManager::PrintStatus("Invalid frame " + String(frame) + "max = " + String(SettingsManager::NumFrames), 4);
			http_rest_server->send(204);
			return;
		}
		uint16 i = 0;
		for(JsonVariant v : Leds) 
		{
			if(i < SettingsManager::NumLeds)
			{
                FrameBuffer::setLED(frame, i, v.as<int>());
			}
			else
			{
				DisplayManager::PrintStatus("Led id " + String(i) + " ignored. Too high", 4);
			}
			i++;
		}
		http_rest_server->sendHeader("Location", "/led/" + String(frame));
		http_rest_server->send(200);
	}
}

void HTTPServer::postSettings()
{
	DynamicJsonDocument jsonBody(500);
	DeserializationError error = deserializeJson(jsonBody, http_rest_server->arg("plain"));

	DisplayManager::PrintStatus("HTTP Method: " + String(http_rest_server->method()), 4);
	
	if (error) {
		DisplayManager::PrintStatus("Error parsing json", 4);
		http_rest_server->send(400);
	}
	else 
	{
		bool numLedsChanged = false;
		bool numFramesChanged = false;
		SettingsManager::Brightness = jsonBody["Brightness"];
        LEDManager::setFramerate(jsonBody["Framerate"]);
		if(SettingsManager::NumFrames != jsonBody["NumFrames"])
		{
			SettingsManager::NumFrames = jsonBody["NumFrames"];
			numFramesChanged = true;
		}
		if(SettingsManager::NumLeds != jsonBody["NumLeds"])
		{
			SettingsManager::NumLeds = jsonBody["NumLeds"];
			numLedsChanged = true;
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
	}
}

void HTTPServer::getSettings()
{
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
}

void HTTPServer::config_rest_server_routing() 
{
	http_rest_server->on("/", HTTP_GET, get_all);
	http_rest_server->on("/led", HTTP_POST, post_put_led);
	http_rest_server->on("/leds", HTTP_POST, post_put_leds);
	http_rest_server->on("/settings", HTTP_GET, getSettings);
	http_rest_server->on("/settings", HTTP_POST, postSettings);
}

void HTTPServer::processRequests()
{
    http_rest_server->handleClient();
}