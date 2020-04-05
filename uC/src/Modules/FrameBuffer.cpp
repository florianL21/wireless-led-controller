#include "FrameBuffer.h"

uint32 **FrameBuffer::frameBuffer = NULL;
bool FrameBuffer::isInitialized = false;
uint16 FrameBuffer::numFrames = 0;
uint16 FrameBuffer::numFramesOld = 0;
uint16 FrameBuffer::numLeds = 0;

bool FrameBuffer::init(uint16 numOfLEDs, uint16 numOfFrames)
{
	numFrames = numOfFrames;
	numLeds = numOfLEDs;
	if(isInitialized == true)
	{
		for (uint16 i = 0; i < numFramesOld; i++)
		{
			free(frameBuffer[i]);
		}
	}
	frameBuffer = new uint32*[numFrames];
	if(frameBuffer == NULL)
	{
		DisplayManager::PrintStatus("Not enought memory", 1);
		return false;
	}
	for (uint16 i = 0; i < numFrames; i++)
	{
		frameBuffer[i] = new uint32[numLeds];
		if(frameBuffer[i] == NULL)
		{
			DisplayManager::PrintStatus("Not enought memory", 1);
			return false;
		}
		for (uint16 j = 0; j < numLeds; j++)
		{
			frameBuffer[i][j] = 0;
		}
	}
	isInitialized = true;
	numFramesOld = numFrames;
	return true;
}

uint32 FrameBuffer::getLED(uint16 frame, uint16 index)
{
	// if(frameBuffer != NULL && frame < numFrames && index < numLeds)
	{
		return frameBuffer[frame][index];
	}
	return 0;
}

void FrameBuffer::setLED(uint16 frame, uint16 index, uint32 color)
{
	// if(frameBuffer != NULL && frame < numFrames && index < numLeds)
	{
		frameBuffer[frame][index] = color;
	}
}