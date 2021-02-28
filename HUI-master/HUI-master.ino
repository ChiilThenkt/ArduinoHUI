#include <MIDI.h>
#include <SPI.h>

#define CS 7
#define dispCount 2


MIDI_CREATE_DEFAULT_INSTANCE();


// -----------------------------------------------------------------------------

void SysExHandler(byte* sysexDat, unsigned sysexSize){
  if (sysexDat[6] == 0x11){ //Received time packets
    counterHandler(sysexDat, sysexSize);
  }
}


byte counterDigit[8] = {0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F};
byte counterAddr[8] = {0x01, 0x05, 0x07, 0x03, 0x04, 0x08, 0x06, 0x02};

byte tcDigit[8] = {0,0x80,0,0x80,0,0x80,0,0};
byte tcAddr[8] = {0x01, 0x05, 0x07, 0x03, 0x04, 0x08, 0x06, 0x02};




//int validNums=1, thisValidNums=0, chkValidCsr;
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

    //tcRefresh();
  }
}


int refCsr;
void digRefresh(){
  for (refCsr = 0; refCsr < 8; refCsr++){
    displayWrite(counterAddr[refCsr],counterDigit[refCsr],tcAddr[refCsr],tcDigit[refCsr]);
  }
}


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





void noteOffHandler(byte inChannel, byte inNote, byte inVelocity){
  if ((inChannel == 0x01)&&(inNote == 0x00)&&(inVelocity == 0x00)) MIDI.sendNoteOn(127, 0, 1);    
}

// -----------------------------------------------------------------------------

void setup()
{
    //Handle SysEx
    MIDI.setHandleSystemExclusive(SysExHandler);
    MIDI.setHandleTimeCodeQuarterFrame(mtcHandler);
    MIDI.setHandleNoteOff(noteOffHandler);
    
    SPI.setBitOrder(MSBFIRST);
    SPI.begin();

    // Initiate MIDI communications, listen to all channels
    
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.turnThruOff();

    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH);

    allDispWrite(0x09, 0xFF);  //enable onboard bit decode (Mode B)
    allDispWrite(0x0A, 0x0F);  //max intensity
    allDispWrite(0x0B, 0x07);  //display all digits
    allDispWrite(0x0C, 0x01);  //turn on chip

    digRefresh();
}

void loop()
{
    // Call MIDI.read the fastest you can for real-time performance.
    MIDI.read();
    digRefresh();

    // There is no need to check if there are messages incoming
    // if they are bound to a Callback function.
    // The attached method will be called automatically
    // when the corresponding message has been received.
}
