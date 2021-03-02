# ArduinoHUI
 Mackie HUI Emulation on Arduino
 
 ## Software
 Currently I've add two counter/timecode display functions including modes (TC/Feets/Beats, on HUI) display and framerate indicators (which are embeded in MTC packets).
 MIDI signal with HUI protocol and MTC data feeds from serial, to fake an "online" HUI device, the program will reply the host pings.
 
 ## Hardware
 Two 8-digit display driver are daisy chained together, and a 74HC595 shift register to drive 8 mode / framerate indication LEDs.
 
 
 
## More details on the road!

**Big thanks to theageman's reverse engineering on Mackie HUI protocol, this project won't be possible without his contribution.**
