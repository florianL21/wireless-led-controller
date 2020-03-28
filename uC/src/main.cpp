#include <Arduino.h>
#include "DisplayManager.h"
#include "LEDManager.h"
#include "FrameBuffer.h"
#include "HTTPServer.h"

void setup(void) 
{
	Serial.begin(9600);
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
}

void loop(void)
{
	LEDManager::update();
	HTTPServer::processRequests();
	DisplayManager::update();
}