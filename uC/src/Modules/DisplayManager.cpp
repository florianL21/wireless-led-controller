#include "DisplayManager.h"

char DisplayManager::lines[DISPLAY_NUM_ROWS][DISPLAY_MAX_LENGTH];
DISPLAY_TYPE* DisplayManager::u8g2 = NULL;
const char DisplayManager::loadingCharSequence[4] = {'|', '/', '-', '\\'};
uint8 DisplayManager::loadingAnimationCouter = 0;
debugOutput DisplayManager::isDebug = DISPLAY_DEBUG_DISABLED;
uint32 DisplayManager::displayTimeout = 0;
bool DisplayManager::diplayInSleep = false;
uint32 DisplayManager::lastMessageTimestamp = 0;

void DisplayManager::init(debugOutput isInDebugMode, uint16 diplayTimeoutInSeconds)
{
    u8g2 = new DISPLAY_TYPE(DISPLAY_ROTATION, DISPLAY_RESET_PIN, DISPLAY_CLOCK_PIN, DISPLAY_DATA_PIN);
    u8g2->begin();
	u8g2->setFont(DISPLAY_FONT);
	isDebug = isInDebugMode;
	for (uint16 i = 0; i < DISPLAY_NUM_ROWS; i++)
	{
		lines[i][0] = '\0';
	}
	displayTimeout = diplayTimeoutInSeconds * 1000;
	lastMessageTimestamp = millis();
	diplayInSleep = false;
}

void DisplayManager::PrintStatus(const char status[], uint8 line, debugOutput debug)
{
	if(debug == DISPLAY_DEBUG_ENABLED && isDebug == DISPLAY_DEBUG_DISABLED)
	{
		return;
	}
	if(line <= DISPLAY_NUM_ROWS && line > 0)
	{
		uint8 len = 0;
		while(status[len++] != '\0'); //find str end
		if(len >= DISPLAY_MAX_LENGTH)
		{
			len = DISPLAY_MAX_LENGTH - 1;
		}
		memcpy(lines[line - 1], status, len);
		u8g2->firstPage();
		do {
			for(uint8 i = 0; i < DISPLAY_NUM_ROWS; i++)
			{
				u8g2->setCursor(0, DISPLAY_FONT_SIZE * (i + 1));
				u8g2->print(lines[i]);
			}
		} while ( u8g2->nextPage() );
		lastMessageTimestamp = millis();
		diplayInSleep = false;
	}
}

void DisplayManager::PrintStatus(String message, uint8 line, debugOutput debug)
{
	char *buffer = new char[message.length()];
	message.toCharArray(buffer, message.length() + 1);
	PrintStatus(buffer, line, debug);
}

char DisplayManager::getLoadingAnimation()
{
	if(loadingAnimationCouter == 4)
		loadingAnimationCouter = 0;
	return loadingCharSequence[loadingAnimationCouter++];
}

void DisplayManager::setDebugOutput(debugOutput printDebugMessages)
{
	isDebug = printDebugMessages;
}

void DisplayManager::update()
{
	if(displayTimeout == 0)
	{
		return;
	}
	if(lastMessageTimestamp + displayTimeout < millis() && diplayInSleep == false)
	{
		diplayInSleep = true;
		u8g2->clear();
	}
}