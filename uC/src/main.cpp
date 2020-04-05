#include <Arduino.h>
#include "DisplayManager.h"
#include "LEDManager.h"
#include "FrameBuffer.h"
#include "HTTPServer.h"

bool initOK = true;

#define CHECK_FOR_ERRORS(x, msg) if(x == false) { systemError(msg); }

void systemError(String location)
{
	DisplayManager::PrintStatus("System will not start", 1);
	DisplayManager::PrintStatus("ERROR at:", 2);
	DisplayManager::PrintStatus(location, 3);
	
	while(1) // endless loop to keep the system from going any further
	{
		wdt_reset(); //prevent the watchdog from resetting the system
	} 
}

void setup(void) 
{
	Serial.begin(115200);
	DisplayManager::init(DEFAULT_DEBUG_MODE, DEFAULT_DISPLAY_TIMEOUT);
	DisplayManager::PrintStatus("Starting...", 1);
	DisplayManager::PrintStatus("Loading settings...", 2);
	if(LOAD_CONFIG_ON_STARTUP)
	{
		CHECK_FOR_ERRORS(SettingsManager::init(), "Memory init");
		// To be enabled if you break the configuration stored in memory somehow and the system keeps resetting
		// SettingsManager::saveConfigToMemory();
		CHECK_FOR_ERRORS(SettingsManager::loadConfigFromMemory(), "Config load");
	}
	else
	{
		SettingsManager::initWithDefaults();
	}

	DisplayManager::setDebugOutput(SettingsManager::DisplayDebugInfo);
	CHECK_FOR_ERRORS(LEDManager::init(SettingsManager::NumLeds), "LED init");
	LEDManager::setFramerate(SettingsManager::Framerate);
	CHECK_FOR_ERRORS(FrameBuffer::init(SettingsManager::NumLeds, SettingsManager::NumFrames), "Framebuffer init");

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

	CHECK_FOR_ERRORS(HTTPServer::init(HTTP_REST_PORT), "HTTP init");
	//if this point is reached everything was initialized without any errors
}

void loop(void)
{
	LEDManager::update();
	HTTPServer::processRequests();
	DisplayManager::update();
}