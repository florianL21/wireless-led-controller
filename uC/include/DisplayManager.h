#include <Arduino.h>
#include <U8g2lib.h>

#pragma once

#define DISPLAY_DEBUG_OUTPUT true

class DisplayManager
{
private:
	static char lines[4][22];
	static U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C* u8g2;
	static const char loadingCharSequence[4];
	static uint8 loadingAnimationCouter;
	
	DisplayManager();
public:
	static void init();

	static void PrintStatus(const char status[], uint8 line, bool debug = false);
	static void PrintStatus(String message, uint8 line, bool debug = false);
	static char getLoadingAnimation();
};

