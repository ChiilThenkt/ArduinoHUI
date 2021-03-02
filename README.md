# ArduinoHUI
 Mackie HUI Emulation on Arduino
 
 ## Software
 Currently I've added two counter/timecode display functions including modes (TC/Feets/Beats, on HUI) display and framerate indicators (which are embeded in MTC packets).
 MIDI signal with HUI protocol and MTC data feeds from serial, to fake an "online" HUI device, the program will reply the host pings.
 
 ## Hardware
 Two MAX7219 8-digit display driver are daisy chained together, communicate with Arduino using SPI, and a 74HC595 shift register to drive 8 mode / framerate indication LEDs.
 More display or LED drivers can be daisy-chained but mods on codes are needed.
 
 ## TODO / What's next
 To be honest I have no new ideas yet, feel free tell me what function you want to add to this project. All suggestions are welcomed.
 

## 
**Big thanks to theageman's reverse engineering on Mackie HUI protocol, this project won't be possible without his contribution.**
