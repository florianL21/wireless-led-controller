#include <Arduino.h>
#include <U8g2lib.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <stdio.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#define DEFAULT_NUM_LED 		5
#define DEFAULT_NUM_FRAMES 		2
#define DEFAULT_ANIM_ENABLE 	true
#define DEFAULT_BRIGHTNESS 		255
#define DEFAULT_FRAMERATE 		5

#define DATA_PIN D8

#define HTTP_REST_PORT 80
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50

struct Settings {
	uint16 Framerate;
	uint8 Brightness;
	uint16 NumLeds;
	uint16 NumFrames;
	uint16 frameCounter;
	bool animationActive;
} setting_resource;

CRGB *leds = NULL;
CFastLED FastLedConfig;
U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // Adafruit ESP8266/32u4/ARM Boards + FeatherWing OLED
uint32 **frameBuffer = NULL;

uint64 oldMillis = 0;
uint16 refreshIntervall = 10;

//WIFI settings
const char* wifi_ssid = "***REMOVED***";
const char* wifi_passwd = "***REMOVED***";

ESP8266WebServer http_rest_server(HTTP_REST_PORT);

const char loadingCharSequence[4] = {'|', '/', '-', '\\'};

void PrintStatus(const char status[], uint8 line)
{
	static char lines[4][22] = {"", "", "", ""};
	if(line <= 4 && line > 0)
	{
		uint8 len = 0;
		while(status[len++] != '\0'); //find str end
		if(len >= 22)
			len = 21;
		memcpy(lines[line - 1], status, len);
		u8g2.firstPage();
		do {
			for(uint8 i = 0; i < 4; i++)
			{
				u8g2.setCursor(0, 8 * (i + 1));
				u8g2.print(lines[i]);
			}
		} while ( u8g2.nextPage() );
	}
}

void PrintStatus(String message, uint8 line)
{
	char *buffer = new char[message.length()];
	message.toCharArray(buffer, message.length() + 1);
	PrintStatus(buffer, line);
}

void initFrameBuffer(uint16 numLEDs, uint16 numFrames)
{
	static uint16 numFramesOld = 0;
	if(frameBuffer != NULL)
	{
		for (uint16 i = 0; i < numFramesOld; i++)
		{
			free(frameBuffer[i]);
		}
	}
	frameBuffer = new uint32*[numFrames];
	if(frameBuffer == NULL)
	{
		PrintStatus("Not enought memory", 1);
		return;
	}
	for (uint16 i = 0; i < numFrames; i++)
	{
		frameBuffer[i] = new uint32[numLEDs];
		if(frameBuffer[i] == NULL)
		{
			PrintStatus("Not enought memory", 1);
			return;
		}
		for (uint16 j = 0; j < numLEDs; j++)
		{
			frameBuffer[i][j] = 0;
		}
	}
}

void setFramerate(uint16 Framerate)
{
	if(Framerate == 0)
	{
		setting_resource.Framerate = 1;
		setting_resource.animationActive = false;
	}
	else
	{
		setting_resource.Framerate = Framerate;
	}
	refreshIntervall = 1000 / Framerate;
}

void FastLEDSetup(uint16 numLEDs)
{
	if(leds != NULL)
	{
		free(leds);
	}
	leds = new CRGB[numLEDs];
	if(leds == NULL)
	{
		PrintStatus("Not enought memory", 1);
	}
	else
	{
		FastLedConfig.addLeds<WS2812B, DATA_PIN, GRB>(leds, numLEDs);
	}
}

void FastLEDPrint(uint16 frameNum)
{
	for (uint16 i = 0; i < setting_resource.NumLeds; i++)
	{
		leds[i] = frameBuffer[frameNum][i];
	}
	FastLED.show();
}

char getLoadingAnimation()
{
	static uint8 index = 0;
	if(index == 4)
		index = 0;
	return loadingCharSequence[index++];
}

uint32 getColorCode(CRGB led)
{
	uint32 colorCode = 0x0;
	colorCode |= led.red;
	colorCode |= (led.green << 8);
	colorCode |= (led.blue << 16);
	return colorCode;
}

int init_wifi() {
	int retries = 0;
	PrintStatus("Connecting to", 1);
	PrintStatus(String(wifi_ssid) + getLoadingAnimation(), 2);
	WiFi.mode(WIFI_STA);
	WiFi.begin(wifi_ssid, wifi_passwd);
	// check the status of WiFi connection to be WL_CONNECTED
	while ((WiFi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY)) {
		retries++;
		delay(WIFI_RETRY_DELAY);
		PrintStatus(String(wifi_ssid) + " " + getLoadingAnimation(), 2);
	}
	return WiFi.status(); // return the WiFi connection status
}

void get_leds()
{
	const size_t FrameCapacity = setting_resource.NumFrames*JSON_ARRAY_SIZE(setting_resource.NumLeds) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(setting_resource.NumLeds) + JSON_OBJECT_SIZE(setting_resource.NumFrames) + 130 + 40 * setting_resource.NumFrames;
	DynamicJsonDocument doc(FrameCapacity);
	JsonObject root = doc.to<JsonObject>();
	JsonObject Settings = root.createNestedObject("Settings");
	Settings["Brightness"] = setting_resource.Brightness;
	Settings["Framerate"] = setting_resource.Framerate;
	Settings["NumFrames"] = setting_resource.NumFrames;
	Settings["NumLeds"] = setting_resource.NumLeds;
	JsonObject Frames = root.createNestedObject("Frames");
	for(uint16 f = 0; f < setting_resource.NumFrames; f++)
	{
		JsonArray FrameArray= Frames.createNestedArray(String(f));
		for(uint16 l = 0; l < setting_resource.NumLeds; l++)
		{
			uint32 color = getColorCode(leds[l]);
			FrameArray.add(color);
		}
	}
	uint32 length = measureJson(doc) + 1;
	char *JSONmessageBuffer = new char[length];
	serializeJson(doc, JSONmessageBuffer, length);
	http_rest_server.send(length, "application/json", JSONmessageBuffer);
}

void post_put_led() 
{
	DynamicJsonDocument jsonBody(500);
	DeserializationError error = deserializeJson(jsonBody, http_rest_server.arg("plain"));

	PrintStatus("HTTP Method: " + String(http_rest_server.method()), 4);
	
	if (error) {
		PrintStatus("Error parsing json", 4);
		http_rest_server.send(400);
	}
	else
	{
		uint16 frame = jsonBody["Frame"];
		uint16 id = jsonBody["id"];
		if(frame >= setting_resource.NumFrames)
		{
			PrintStatus("Invalid frame " + String(frame) + "max = " + String(setting_resource.NumFrames), 4);
			http_rest_server.send(204);
			return;
		}
		if(id >= setting_resource.NumLeds)
		{
			PrintStatus("Invalid id " + String(id) + "max = " + String(setting_resource.NumLeds), 4);
			http_rest_server.send(204);
			return;
		}
		frameBuffer[frame][id] = jsonBody["color"];
		http_rest_server.sendHeader("Location", "/led/" + String(id));
		http_rest_server.send(200);
	}
}

void post_put_leds()
{
	const size_t FrameCapacity = JSON_ARRAY_SIZE(setting_resource.NumLeds) + JSON_OBJECT_SIZE(2) + 10 * setting_resource.NumLeds;
	DynamicJsonDocument jsonBody(FrameCapacity);
	DeserializationError error = deserializeJson(jsonBody, http_rest_server.arg("plain"));

	PrintStatus("HTTP Method: " + String(http_rest_server.method()), 4);
	
	if (error) {
		PrintStatus("Error parsing json", 4);
		http_rest_server.send(400);
	}
	else
	{
		uint16 frame = jsonBody["Frame"];
		JsonArray Leds = jsonBody["Leds"];
		if(frame >= setting_resource.NumFrames)
		{
			PrintStatus("Invalid frame " + String(frame) + "max = " + String(setting_resource.NumFrames), 4);
			http_rest_server.send(204);
			return;
		}
		uint16 i = 0;
		for(JsonVariant v : Leds) 
		{
			if(i < setting_resource.NumLeds)
			{
				frameBuffer[frame][i] = v.as<int>();
			}
			else
			{
				PrintStatus("Led id " + String(i) + " ignored. Too high", 4);
			}
			i++;
		}
		http_rest_server.sendHeader("Location", "/led/" + String(frame));
		http_rest_server.send(200);
	}
}

void postSettings()
{
	DynamicJsonDocument jsonBody(500);
	DeserializationError error = deserializeJson(jsonBody, http_rest_server.arg("plain"));

	PrintStatus("HTTP Method: " + String(http_rest_server.method()), 4);
	
	if (error) {
		PrintStatus("Error parsing json", 4);
		http_rest_server.send(400);
	}
	else 
	{
		bool numLedsChanged = false;
		bool numFramesChanged = false;
		setting_resource.Brightness = jsonBody["Brightness"];
		setFramerate(jsonBody["Framerate"]);
		if(setting_resource.NumFrames != jsonBody["NumFrames"])
		{
			setting_resource.NumFrames = jsonBody["NumFrames"];
			numFramesChanged = true;
		}
		if(setting_resource.NumLeds != jsonBody["NumLeds"])
		{
			setting_resource.NumLeds = jsonBody["NumLeds"];
			numLedsChanged = true;
		}
		if(numLedsChanged)
		{
			FastLEDSetup(setting_resource.NumLeds);
		}
		if(numLedsChanged || numFramesChanged)
		{
			initFrameBuffer(setting_resource.NumLeds, setting_resource.NumFrames);
		}
		http_rest_server.sendHeader("Location", "/settings");
		http_rest_server.send(200);
	}
}

void getSettings()
{
	const size_t FrameCapacity = JSON_OBJECT_SIZE(4);
	DynamicJsonDocument doc(FrameCapacity);
	JsonObject root = doc.to<JsonObject>();
	root["Brightness"] = setting_resource.Brightness;
	root["Framerate"] = setting_resource.Framerate;
	root["NumFrames"] = setting_resource.NumFrames;
	root["NumLeds"] = setting_resource.NumLeds;
	uint32 length = measureJson(doc) + 1;
	char *JSONmessageBuffer = new char[length];
	serializeJson(doc, JSONmessageBuffer, length);
	http_rest_server.send(length, "application/json", JSONmessageBuffer);
}

void config_rest_server_routing() 
{
	http_rest_server.on("/", HTTP_GET, get_leds);
	http_rest_server.on("/led", HTTP_POST, post_put_led);
	http_rest_server.on("/leds", HTTP_POST, post_put_leds);
	// http_rest_server.on("/leds", HTTP_PUT, post_put_leds);
	http_rest_server.on("/settings", HTTP_GET, getSettings);
	http_rest_server.on("/settings", HTTP_POST, postSettings);
}

void setup(void) 
{
	Serial.begin(9600);
	u8g2.begin(); 
	u8g2.setFont(u8g2_font_courR08_tf);
	PrintStatus("Starting...", 1);
	setting_resource.NumLeds = DEFAULT_NUM_LED;
	setting_resource.NumFrames = DEFAULT_NUM_FRAMES;
	setting_resource.Brightness = DEFAULT_BRIGHTNESS;
	setting_resource.animationActive = DEFAULT_ANIM_ENABLE;
	setting_resource.frameCounter = 0;
	setFramerate(DEFAULT_FRAMERATE);
	FastLEDSetup(setting_resource.NumLeds);
	initFrameBuffer(setting_resource.NumLeds, setting_resource.NumFrames);

	if (init_wifi() == WL_CONNECTED) 
	{
		PrintStatus("Connected to:", 1);
		PrintStatus(wifi_ssid, 2);
		PrintStatus("IP: " + WiFi.localIP().toString(), 3);
	}
	else {
		PrintStatus("Error connecting to:", 1);
		PrintStatus(wifi_ssid, 2);
	}

	config_rest_server_routing();

	http_rest_server.begin();
}

void loop(void)
{
	if(oldMillis + refreshIntervall < millis())
	{
		FastLEDPrint(setting_resource.frameCounter);
		if(setting_resource.animationActive)
		{
			setting_resource.frameCounter++;
			if(setting_resource.frameCounter >= setting_resource.NumFrames)
			{
				setting_resource.frameCounter = 0;
			}
		}
		oldMillis = millis();
	}
	http_rest_server.handleClient();
}