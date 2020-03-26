#include "DisplayManager.h"

char DisplayManager::lines[4][22] = {"", "", "", ""};
U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C* DisplayManager::u8g2 = NULL;
const char DisplayManager::loadingCharSequence[4] = {'|', '/', '-', '\\'};
uint8 DisplayManager::loadingAnimationCouter = 0;

void DisplayManager::init()
{
    u8g2 = new U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // Adafruit ESP8266/32u4/ARM Boards + FeatherWing OLED
    u8g2->begin();
	u8g2->setFont(u8g2_font_courR08_tf);
}

void DisplayManager::PrintStatus(const char status[], uint8 line)
{
	if(line <= 4 && line > 0)
	{
		uint8 len = 0;
		while(status[len++] != '\0'); //find str end
		if(len >= 22)
			len = 21;
		memcpy(lines[line - 1], status, len);
		u8g2->firstPage();
		do {
			for(uint8 i = 0; i < 4; i++)
			{
				u8g2->setCursor(0, 8 * (i + 1));
				u8g2->print(lines[i]);
			}
		} while ( u8g2->nextPage() );
	}
}

void DisplayManager::PrintStatus(String message, uint8 line)
{
	char *buffer = new char[message.length()];
	message.toCharArray(buffer, message.length() + 1);
	PrintStatus(buffer, line);
}

char DisplayManager::getLoadingAnimation()
{
	if(loadingAnimationCouter == 4)
		loadingAnimationCouter = 0;
	return loadingCharSequence[loadingAnimationCouter++];
}
