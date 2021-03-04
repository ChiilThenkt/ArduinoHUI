/************************************************************
ArduinoHUI-master by ChiilThenkt v1.0-alpha
Support boards: All ATmega328p boards, ATmega32u4 boards, etc.


****************Connections (Arduino Uno)****************
* - Arduino PIN13 : SPI_SCK : MAX7219 PIN13
* - Arduino PIN22 : SPI_MOSI : MAX7219 PIN1 (The first driver only)
* - Arduino PIN8 : CS : MAX7219 PIN12
* 
* - Arduino PIN5 : LATCH : 74HC595 PIN12
* - Arduino PIN6 : CLOCK : 74HC595 PIN11
* - Arduino PIN7 : DATA : 74HC595 PIN14 (The first driver only)
*
* - Arduino PIN0(RX) : MIDI_IN
* - Arduino PIN1(TX) : MIDI_OUT
*
* --------USING STANDARD MIDI I/O, BAUDRATE AT 31250--------
* -------CHANGE TO DESIRED RATE IF YOU USE SERIALMIDI-------
* ---OR FOLLOW COMMENTS TO USE USB MIDI ON SUPPORT BOARDS---
*********************************************************
*
*
*Wirtten by ChiilThenkt
*GitHub Page: https://github.com/ChiilThenkt/ArduinoHUI
*
*
***********************************************************/
#include <MIDI.h>
//Un-comment below to enable USB MIDI support
//#include <USB-MIDI.h>
//NOTICE: This is not the offical MIDIUSB library, instead, Arduino-USBMIDI by lathoub.
//Link: https://github.com/lathoub/Arduino-USBMIDI
#include <SPI.h>

#define CS 8
#define dispCount 2


MIDI_CREATE_DEFAULT_INSTANCE();

//For Leonardo, Pro Micro or other ATmega32u4 microcontrollers, 
//comment above and uncomment below to use USB MIDI.
//USBMIDI_CREATE_DEFAULT_INSTANCE(); 

//---------ShiftRegisterPins-------------
int clockPin = 5; //Pin connected to SH_CP of 74HC595
int latchPin = 6; //Pin connected to ST_CP of 74HC595
int dataPin = 7; ////Pin connected to DS of 74HC595

// -----------------------------------------------------------------------------

//----------------SysEx to Displays----------------
void SysExHandler(byte* sysexDat, unsigned sysexSize){
  if (sysexDat[6] == 0x11){ //Received time packets
    counterHandler(sysexDat, sysexSize);
  }
}

//----------------Reply to HUI pings----------------
void noteOffHandler(byte inChannel, byte inNote, byte inVelocity){
  if ((inChannel == 0x01)&&(inNote == 0x00)&&(inVelocity == 0x00)) MIDI.sendNoteOn(0x00, 0x7f, 0x01);    
}

//----------------CC to LEDS----------------
byte zone = 0x1e; //Initialize a null zone
void ccHandler(byte inChannel, byte cmd, byte para){
  switch (cmd) {
    case 0x0c:
      zone = para;
      break;
    case 0x2c:
      outputHandler(zone, para);
      break;
  }
}



//----------------Display Elements----------------

byte counterDigit[8] = {0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F};
byte counterAddr[8] = {0x05, 0x01, 0x07, 0x03, 0x02, 0x06, 0x08, 0x04};
byte counterLED = 0x00;

byte tcDigit[8] = {0,0x80,0,0x80,0,0x80,0,0};
byte tcAddr[8] = {0x07, 0x05, 0x01, 0x03, 0x02, 0x06, 0x08, 0x04};
byte tcLED = 0x00;

byte led1 = 0x00;

//----------------HUI Counter----------------
void counterHandler(byte* sysexDat, unsigned sysexSize){
  int timeCursor;
  if (sysexDat[6] == 0x11){ //Received time packets
    for (timeCursor = 7;timeCursor < sysexSize - 1;timeCursor++){
      if (sysexDat[timeCursor] < 0x10) counterDigit[14 - timeCursor] = sysexDat[timeCursor]; //keep valid numbers only
      else if (sysexDat[timeCursor] < 0x20)counterDigit[14 - timeCursor] = sysexDat[timeCursor] + 0x80; //with dots
      else counterDigit[14 - timeCursor] = 0x0F; //turn digit off
    }
  }
}

//----------------MTC Display----------------
byte h, m, s, f;
byte tcString[8];
int byteIndex, fmt;
void mtcHandler(byte qf){
  byteIndex = qf >> 4;
  tcString[byteIndex] = qf & 0x0F;
  if (byteIndex >= 0x07) {  //recalculate timecode once FRAMES LSB quarter-frame received
    tcDigit[0] = ((tcString[7] & 0x01)*16 + tcString[6])/10;
    tcDigit[1] = ((tcString[7] & 0x01)*16 + tcString[6])%10+0x80;
    tcDigit[2] = (tcString[5]*16 + tcString[4])/10;
    tcDigit[3] = (tcString[5]*16 + tcString[4])%10+0x80;
    tcDigit[4] = (tcString[3]*16 + tcString[2])/10;
    tcDigit[5] = (tcString[3]*16 + tcString[2])%10+0x80;
    tcDigit[6] = (tcString[1]*16 + tcString[0])/10;
    tcDigit[7] = (tcString[1]*16 + tcString[0])%10;
    fmt = (tcString[7] >> 1);
    switch (fmt) {
      case 0x00: tcLED = B00000001; break;
      case 0x01: tcLED = B00000010; break;
      case 0x02: tcLED = B00001100; break;
      case 0x03: tcLED = B00000100; break;
    }
  }
}




//----------------LED----------------
void outputHandler(byte zone, byte para){
  switch (zone){
    case 0x16:
      setCounterLED((int)(para & 0x0f), para >> 6);
      break;
    default:
      break;
     
  }
}


void setCounterLED(int port, bool state){
  if (state) counterLED |= (0x01 << port);
  else counterLED &= ~(0x01 << port);
}



//------------Update all display elements------------

void ledRefresh(){
  //Re-organize led bits
  led1 = (counterLED & 0x0f) + ((tcLED & 0x0f) << 4);
  //ground latchPin and hold low for as long as you are transmitting
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, led1);
  //return the latch pin high
  digitalWrite(latchPin, HIGH);
}

int refCsr;
void digRefresh(){
  for (refCsr = 0; refCsr < 8; refCsr++){
    displayWrite(counterAddr[refCsr],counterDigit[refCsr],tcAddr[refCsr],tcDigit[refCsr]);
  }
}

//----------------DigitDisplayFunctions----------------
/*  Write VALUE to register ADDRESS on the MAX7219. */
void displayWrite(uint8_t addr1, uint8_t value1,uint8_t addr2, uint8_t value2) {
  digitalWrite(CS, LOW); //Toggle enable pin to load MAX7219 shift register
  SPI.transfer(addr2); //Transfer 16 bits of TC to the 2nd MAX7219
  SPI.transfer(value2);
  SPI.transfer(addr1); //Transfer 16 bits of counter to the 1st MAX7219
  SPI.transfer(value1);
  digitalWrite(CS, HIGH); //End transfer
}

void singleDispWrite(uint8_t addr, uint8_t value, uint8_t destnation) {
  int dispID = dispCount;
  digitalWrite(CS, LOW); //Toggle enable pin to load MAX7219 shift register
  while (dispID > destnation) {
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    dispID--;
  }
  SPI.transfer(addr);
  SPI.transfer(value);
  dispID--;
  while (dispID > 0) {
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    dispID--;
  }
  digitalWrite(CS, HIGH); //End transfer
}

void allDispWrite(uint8_t addr, uint8_t value) {
  int dispID = dispCount;
  digitalWrite(CS, LOW); //Toggle enable pin to load MAX7219 shift register
  while (dispID > 0) {
    SPI.transfer(addr);
    SPI.transfer(value);
    dispID--;
  }
  digitalWrite(CS, HIGH); //End transfer
}





// -----------------------------------------------------------------------------

void setup()
{
    //Handle SysEx
    MIDI.setHandleSystemExclusive(SysExHandler);
    MIDI.setHandleTimeCodeQuarterFrame(mtcHandler);
    MIDI.setHandleNoteOff(noteOffHandler);
    MIDI.setHandleControlChange(ccHandler);
    
    SPI.setBitOrder(MSBFIRST);
    SPI.begin();
    
    //Set pins as outputs
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);
    pinMode(CS, OUTPUT);
    delay(50);
    allDispWrite(0x09, 0xFF);  //enable onboard bit decode (Mode B)
    allDispWrite(0x0A, 0x0F);  //max intensity
    allDispWrite(0x0B, 0x07);  //display all digits
    allDispWrite(0x0C, 0x01);  //turn on chip
    delay(50);
    digRefresh();

    // Initiate MIDI communications, listen to all channels
    
    MIDI.begin(MIDI_CHANNEL_OMNI);

    //USB MIDI doesn't come with thru, comment it if necessary.
    MIDI.turnThruOff();
}

void loop()
{
    // Call MIDI.read the fastest you can for real-time performance.
    MIDI.read();
    digRefresh();
    ledRefresh();
    

}
