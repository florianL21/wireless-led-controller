#include <Arduino.h>
#include <U8g2lib.h>
#include "Configuration.h"

#pragma once

enum debugOutput {DISPLAY_DEBUG_ENABLED = true, DISPLAY_DEBUG_DISABLED = false};

class DisplayManager
{
private:
	static char lines[DISPLAY_NUM_ROWS][22];
	static DISPLAY_TYPE* u8g2;
	static const char loadingCharSequence[4];
	static uint8 loadingAnimationCouter;
	static debugOutput isDebug;
	static uint32 displayTimeout;
	static bool diplayInSleep;
	static uint32 lastMessageTimestamp;
	
	DisplayManager();
public:
	static void init(debugOutput isInDebugMode, uint16 diplayTimeoutInSeconds = 0);

	static void PrintStatus(const char status[], uint8 line, debugOutput debug = DISPLAY_DEBUG_DISABLED);
	static void PrintStatus(String message, uint8 line, debugOutput debug = DISPLAY_DEBUG_DISABLED);
	static char getLoadingAnimation();
	static void setDebugOutput(debugOutput printDebugMessages);
	static void update();
};

