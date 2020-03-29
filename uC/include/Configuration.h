#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

/*************************
 * LED Configuration:
 *************************/

//LED Data pin
#define LED_DATA_PIN			D8
//LED Type
#define LED_TYPE				WS2812B
//LED Color mode
#define LED_COLOR_MODE			GRB

/*************************
 * Default values:
 *************************/
#define DEFAULT_NUM_LED			5
#define DEFAULT_NUM_FRAMES		2
#define DEFAULT_ANIM_ENABLE		true
#define DEFAULT_BRIGHTNESS		255
#define DEFAULT_FRAMERATE		5
#define DEFAULT_DEBUG_MODE		DISPLAY_DEBUG_DISABLED
#define DEFAULT_DISPLAY_TIMEOUT	60

/*************************
 * WIFI configuration:
 *************************/
#ifndef WIFI_SSID
    #define WIFI_SSID			"YOUR_WIFI_NAME"
#endif
#ifndef WIFI_PASSWORD
    #define WIFI_PASSWORD		"YOUR_WIFI_PASSWORD"
#endif

//can be changed to improve startup speed of the programm
#define WIFI_RETRY_DELAY		500
#define MAX_WIFI_INIT_RETRY		50
//should not be changed unless you have a good reason to do so
#define HTTP_REST_PORT			80

/*************************
 * Display configuration:
 *************************/
#define DISPLAY_TYPE			U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C
#define DISPLAY_DATA_PIN		SDA
#define DISPLAY_CLOCK_PIN		SCL
#define DISPLAY_RESET_PIN		U8X8_PIN_NONE
#define DISPLAY_ROTATION		U8G2_R0
#define DISPLAY_FONT			u8g2_font_courR08_tf
#define DISPLAY_FONT_SIZE		8
#define DISPLAY_NUM_ROWS		4
#define DISPLAY_MAX_LENGTH		22


#endif