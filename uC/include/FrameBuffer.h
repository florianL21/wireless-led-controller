
#pragma once

#include <Arduino.h>
#include "DisplayManager.h"

class FrameBuffer
{
private:
	static uint32 **frameBuffer;
	static bool isInitialized;
	static uint16 numFrames;
	static uint16 numFramesOld;
	static uint16 numLeds;

	FrameBuffer();
public:
	static void init(uint16 numOfLEDs, uint16 numOfFrames);

	static uint32 getLED(uint16 frame, uint16 index);
	static void setLED(uint16 frame, uint16 index, uint32 color);
};
