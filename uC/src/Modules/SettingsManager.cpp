#include "SettingsManager.h"

uint16 SettingsManager::Framerate				= DEFAULT_FRAMERATE;
uint8 SettingsManager::Brightness				= DEFAULT_BRIGHTNESS;
uint16 SettingsManager::NumLeds 				= DEFAULT_NUM_LED;
uint16 SettingsManager::NumFrames				= DEFAULT_NUM_FRAMES;
uint16 SettingsManager::frameCounter			= 0;
bool SettingsManager::animationActive			= DEFAULT_ANIM_ENABLE;
debugOutput SettingsManager::DisplayDebugInfo	= DEFAULT_DEBUG_MODE;