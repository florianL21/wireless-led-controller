#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

/*************************
 * LED Configuration:
 *************************/

//LED Data pin
#define LED_DATA_PIN			D8
//LED Type
#define LED_TYPE                WS2812B
//LED Color mode
#define LED_COLOR_MODE          GRB

/*************************
 * Default values:
 *************************/
#define DEFAULT_NUM_LED			5
#define DEFAULT_NUM_FRAMES		2
#define DEFAULT_ANIM_ENABLE		true
#define DEFAULT_BRIGHTNESS		255
#define DEFAULT_FRAMERATE		5

/*************************
 * WIFI configuration:
 *************************/

#define WIFI_SSID				"***REMOVED***"
#define WIFI_PASSWORD			"***REMOVED***"

//can be changed to improve startup speed of the programm
#define WIFI_RETRY_DELAY		500
#define MAX_WIFI_INIT_RETRY		50
//should not be changed unless you have a good reason to do so
#define HTTP_REST_PORT			80

/*************************
 * Display configuration:
 *************************/


#endif