# wireless-led-controller
This is a small project that allows the creation of RGB LED animations.
The core idea behind this is that an ESP8266 is controlling a number of WS2812B LED's. 
The ESP can be in a remote location but it would still be able to be configured remotly via an HTTP request.
The main goal was to keep all the LED's individally addressable and still be able to create new animations on the fly.

## Used Hardware:
 - ESP8266 (nodeMCU)
 - A number of WS2812B LED's
 - OLED Display 128x32 (SSD1306 controller)

# Features
This is split into two main parts:
## The microcontroller:
### current features:
 - Support for a variying amount of LED's (changable during runtime without a restart)
 - All settings can be changed remotly via an HTTP JSON request. Changable settings as of now:
    - Number of LED's
    - Number of frames
    - Framerate
  	- Brightness
    - currently active frame
    - Enable/Disable the animation
    - Debug output visible on screen
 - Configuration can be stored on the internal file system of the ESP as a JSON file and automatically restored during startup
 - Debug output as well as device IP address and other status information is shown on an OLED screen which turns black after a 
    set amount of time if there is no new information.
### Planned features:
 - OTA update
 - MAYBE an option to fade between frames or something like frame interpolation
 - MAYBE support for more than one configuration and being able to cycle through them?

## The Windows program
Written in .NET framework and used to set up and transmit the data to the ESP
### current features:
 - Visual representation of the LED's in their currently selected frame
 - It is possible to change most of the values using the GUI
 - The visual representation of the LED's can be clicked on and the color can be changed. Multi select modes etc. are also working.
 - Transmission of the data to the ESP is working.
### Planned features:
 - More robustness and less bugs in the GUI, this is very much just a proof of concept implementation for now.
 - Being able to download the current effect from the ESP
 - Live view mode: Having the ESP update the LED's live while configuring the effects on the GUI.

