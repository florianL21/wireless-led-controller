#pragma once

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "FrameBuffer.h"
#include "DisplayManager.h"
#include "SettingsManager.h"
#include "LEDManager.h"


class HTTPServer
{
private:
    static ESP8266WebServer* http_rest_server;

    HTTPServer();
    static void getAll();
    static void postLed();
    static void postFrame();
    static void postSettings();
    static void getSettings();
    static void config_rest_server_routing();
    static void saveSettingsToMemory();
public:
    static bool init(int port);
    ~HTTPServer();

    static int init_wifi(const char* wifi_ssid, const char* wifi_passwd, uint8 numRetrys, uint16 retryDelay);
    static void processRequests();
};