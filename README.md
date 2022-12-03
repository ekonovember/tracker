# GPS tracker
---
Yet another GPS tracker based on arduino/nodemcu platform. The main reason for publishing this project is to provide a complete, step by step, no doubts/investigation process for building and programing a complete and fully functional GPS tracker for ~~everyone~~ ...well mostly sailors.

### How it works


### Environment
This project has been created using *PlatformIO* and *VS Code*. Dependencies (libraries) needed for this project to run are defined in *platformio.ini* file. After the project is built the libraries will be restored in *.pio/libdeps/nodemcuv2/* folder. Look [here](https://www.youtube.com/watch?v=dany7ae_0ks) for IDE setup. Find below the dependencies listed:
| dependencies                                                          |
|-----------------------------------------------------------------------|
| thingpulse/ESP8266 and ESP32 OLED driver for SSD1306 displays@^4.3.0  |
| plerup/EspSoftwareSerial@^6.16.1                                      |  
| slashdevin/NeoGPS@^4.2.9                                              |
| ivanseidel/LinkedList@0.0.0-alpha+sha.dac3874d28                      |

### Components
* NodemCU v3 board (ESP8266 controller),
* NEO 6-M GPS module (with antenna),
* HW-125 SD card module (**remove / unsolder voltage regulator to make it usable with 3.3V - google that one!**),
* SH1106 based SPI OLED 128x64 display - it can be SSD1306 but you will need some code changes (**most probably you will get a random one when shopping for OLED display 128x64**),
* switch button,
* 10k ohm resistor, 
* female to female breadboard wires (pack),
* usb-a to micro cable with data lines,
* powerbank.

#### optionally:
* breadboard,
* male to male, female to male wires.

### Circuit connections

| NEO 6-M | NodemCU v3 |
|---------|-----------:|
| VCC     | 3V         |  
| GND     | G          |
| RX      | D4         |
| TX      | D3         |

| SH1106  | NodemCU v3 |
|---------|-----------:|
| VDD     | 3V         |  
| GND     | G          |
| SCK     | D1         |
| SDA     | D2         |

| HW-125  | NodemCU v3 |
|---------|-----------:|
| VCC     | 3V         |  
| GND     | G          |
| CS      | D8         |
| SCK     | D5         |
| MOSI    | D7         |
| MISO    | D6         |

| switch / button     | NodemCU v3 |
|---------------------|-----------:|
| gnd before resistor | D0         |
| gnd after resistor  | G          |
| other leg           | 3V         |

### HW-125 modification
You need to remove voltage controller (unsolder) and then short (solder together) marked pins. This is only required for NodemCU as it is 3.3V. For Arduino it can stay as is.

![alt text][sd_picture]

[sd_picture]: ./pictures/hw_125_picture.png "SD module picture"

### Changes in libraries
Some GPS settings need to be changed directly in files installed from dependencies. Namely (after build) in folder *.pio/libdeps/nodemcuv2/NeoGPS/src*:

```c++
//in file NMEAGPS_cfg.h uncomment:
#define NMEAGPS_PARSE_GGA
#define NMEAGPS_PARSE_GLL
#define NMEAGPS_PARSE_GSA
#define NMEAGPS_PARSE_GSV
//#define NMEAGPS_PARSE_GST
#define NMEAGPS_PARSE_RMC
#define NMEAGPS_PARSE_VTG
#define NMEAGPS_PARSE_ZDA

//and set

#define LAST_SENTENCE_IN_INTERVAL NMEAGPS::NMEA_GLL
//instead of
//#define LAST_SENTENCE_IN_INTERVAL NMEAGPS::NMEA_RMC
```

### Settings in code
For "production" mode - if you want logger to work only when there is movement.

```c++
//source file: src/main.cpp
bool VALID_FIX_ONLY_WITH_HEADING = true;
bool ENABLE_SERIAL_DEBUGGING     = false;
```