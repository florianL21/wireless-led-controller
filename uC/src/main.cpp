#include <Arduino.h>
#include "DisplayManager.h"
#include "LEDManager.h"
#include "FrameBuffer.h"
#include "HTTPServer.h"

String test = "{'Settings':{'Brightness':255,'Framerate':1,'NumFrames':20,'NumLeds':20,'ActiveFrame':17,'AnimationActive':true,'ConfigFileVersion':'0.0.0','DisplayDebugInfo':false},'Frames':{'0':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'1':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'2':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'3':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'4':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'5':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'6':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'7':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'8':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'9':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'10':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'11':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'12':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'13':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'14':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'15':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'16':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'17':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'18':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],'19':[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}}";

void setup(void) 
{
	Serial.begin(115200);
	DisplayManager::init(DEFAULT_DEBUG_MODE, DEFAULT_DISPLAY_TIMEOUT);
	DisplayManager::PrintStatus("Starting...", 1);
	LEDManager::init(DEFAULT_NUM_LED);
	LEDManager::setFramerate(DEFAULT_FRAMERATE);
	FrameBuffer::init(DEFAULT_NUM_LED, DEFAULT_NUM_FRAMES);

	if (HTTPServer::init_wifi(WIFI_SSID, WIFI_PASSWORD, MAX_WIFI_INIT_RETRY, WIFI_RETRY_DELAY) == WL_CONNECTED) 
	{
		DisplayManager::PrintStatus("Connected to:", 1);
		DisplayManager::PrintStatus(WIFI_SSID, 2);
		DisplayManager::PrintStatus("IP: " + WiFi.localIP().toString(), 3);
	}
	else 
	{
		DisplayManager::PrintStatus("Error connecting to:", 1);
		DisplayManager::PrintStatus(WIFI_SSID, 2);
	}
	HTTPServer::init(HTTP_REST_PORT);
	SettingsManager::deserializeConfiguration(test);
}

void loop(void)
{
	LEDManager::update();
	HTTPServer::processRequests();
	DisplayManager::update();
}