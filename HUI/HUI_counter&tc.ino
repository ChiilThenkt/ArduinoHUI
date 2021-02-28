#include <MIDI.h>
#include <LiquidCrystal_I2C.h>
MIDI_CREATE_DEFAULT_INSTANCE();

LiquidCrystal_I2C lcd(0x27,16,2);

// -----------------------------------------------------------------------------
char timeDisp[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int validNums=1,thisValidNums=0,chkValidCsr;
void SysExHandler(byte* sysexDat, unsigned sysexSize){
  if (sysexDat[6] == 0x11){ //Received time packets
    char tempChar[2] = {0,0};
    int timeCursor;
    for (timeCursor = 7;timeCursor < sysexSize - 1;timeCursor++){
      if (sysexDat[timeCursor] < 0x10){ //keep valid numbers only
        tempChar[0] = sysexDat[timeCursor] + 0x30;
        tempChar[1] = 0;
        //char tempChar[2] = {sysexDat[timeCursor] + 0x30, 0};
      }else if (sysexDat[timeCursor] < 0x20){ //with dots
        tempChar[0] = sysexDat[timeCursor] + 0x20;
        tempChar[1] = '.';
        //char tempChar[2] = {sysexDat[timeCursor] + 0x20, '.'};
      }else {
        tempChar[0] = 0;
        tempChar[1] = 0;
        //char tempChar[2] = {0, 0};
      }
      timeDisp[29-2*timeCursor] = tempChar[1];
      timeDisp[28-2*timeCursor] = tempChar[0];
    }
    thisValidNums=0;
    for (int chkValidCsr=0; chkValidCsr<16; chkValidCsr++){
      if (timeDisp[chkValidCsr]) thisValidNums++;
    }
    if (thisValidNums < validNums) lcd.clear();
    validNums = thisValidNums;
    //Begin outputs
    lcd.setCursor(0,0);
    for (int printCursor = 0; printCursor < 16; printCursor++){
      if (timeDisp[printCursor]) lcd.print(timeDisp[printCursor]);
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
    h = (tcString[7] & 0x01)*16 + tcString[6];
    m = tcString[5]*16 + tcString[4];
    s = tcString[3]*16 + tcString[2];
    f = tcString[1]*16 + tcString[0];
    fmt = (tcString[7] >> 1);
    mtcRefresh();
  }
}

void mtcRefresh(){
  lcd.setCursor(0,1);
  lcd.print(h/10);
  lcd.print(h%10);
  lcd.print(':');
  lcd.print(m/10);
  lcd.print(m%10);
  lcd.print(':');
  lcd.print(s/10);
  lcd.print(s%10);
  if (fmt==2) {
    lcd.print(';');
  }else lcd.print(':');
  lcd.print(f/10);
  lcd.print(f%10);
}

// -----------------------------------------------------------------------------

void setup()
{
    //Handle SysEx
    MIDI.setHandleSystemExclusive(SysExHandler);
    MIDI.setHandleTimeCodeQuarterFrame(mtcHandler);

    // Initiate MIDI communications, listen to all channels
    MIDI.begin(MIDI_CHANNEL_OMNI);

    lcd.begin();
    
}

void loop()
{
    // Call MIDI.read the fastest you can for real-time performance.
    MIDI.read();

    // There is no need to check if there are messages incoming
    // if they are bound to a Callback function.
    // The attached method will be called automatically
    // when the corresponding message has been received.
}
