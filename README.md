# GPS tracker
---
Yet another GPS tracker based on arduino/nodemcu platform. The main reason for publishing this project is to provide a complete, step by step, no doubts/investigation process for building and programing a complete and fully functional GPS tracker for ~~everyone~~ ...well mostly sailors.

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

| NEO 6-M | NodemCU v2|
|---------|----------:|
| VCC     | 3V        |   
| GND     | G         |
