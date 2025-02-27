# 1 "C:\\Users\\GEBRUI~1\\AppData\\Local\\Temp\\tmpir5_q71k"
#include <Arduino.h>
# 1 "G:/Mijn Drive/platformIO/Amp/src/AmpArduino.ino"
# 17 "G:/Mijn Drive/platformIO/Amp/src/AmpArduino.ino"
const int contrastLevelScreen = 5;
const int resistorLow = 50;
const int resistorHigh = 600;
const float biasResistor = 0.47;
const int biasMultiplier = 2;
const float lowCurBiasFloat = -0.0;
const float highCurBiasFloat = 2.5;
const float lowDCOffsetFloat = -1.0;
const float highDCOffsetFloat = 1.0;
const int highTemp = 60;
const bool startUpAtPower = true;
int startDelayTime = 10;
const int numberOfSensorsCh = 2;
const char* toptekst = "PeWalt, V 1.0";
const char* middleTekst = "     AMP warming up";
const char* bottemTekst = " " ;
const int numberOffDec = 2 ;
const unsigned long timeToShowDetailScreen=30000;
#define correctionOffsetBiasLeft 0
#define correctionOffsetBiasRight 12


#define powerOnOff A0
#define relayOutputLeft A1
#define relayOutputRight A2
#define startUpResistor A3
#define spare A4
#define oneWireLeft 4
#define oneWireRight 5
#define detect230V 2
#define buttonStandby 7
#define ledStandby 11
#define oledReset 12

#define oledAddress 0x3C
#define fontH08 u8g2_font_timB08_tr
#define fontH08fixed u8g2_font_spleen5x8_mr
#define fontH10 u8g2_font_timB10_tr
#define fontgrahp u8g2_font_unifont_t_78_79
#define fontgroot u8g2_font_lubB19_tr
#include <arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C Screen(U8G2_R0);

int aDCLeftI2CAddress = 0x48;
int aDCRightI2CAddress = 0x49;
#define SelectConversionRegister 0b00000000
#define SelectConfigRegister 0b00000001
#define measureDCOfset 0b10010101
#define measureBiasPlus 0b10100101
#define measureBiasMinus 0b10110101
#define numOfSample 0b10100011
float voltageStep = 0.0000625;
float corOffset = 0;
float corBias = 0;

#include <OneWire.h>
#include <DallasTemperature.h>
OneWire wireLeft(oneWireLeft);
OneWire wireRight(oneWireRight);
DallasTemperature tempSensorLeft(&wireLeft);
DallasTemperature tempSensorRight(&wireRight);

#include <ezButton.h>
ezButton button(buttonStandby);
const int shortPressTimeLimit = 1000;
const int longPressTimeLimit = 1000;
unsigned long pressedTime = 0;
unsigned long releasedTime = 0;
bool isPressing = false;
bool isLongDetected = false;
bool isShortDetected = false;

int lowCurBiasInt;
int highCurBiasInt;
int lowDCOffsetInt;
int highDCOffsetInt;
bool aDCOn = true;
bool tempOn = true;
int dCOffsetLeft;
int dCOffsetRight;
int dCOffsetLeftOld;
int dCOffsetRightOld;
int ampsBiasLeft;
int ampsBiasRight;
int tempLeft;
int tempRight;
bool opStateLeftCh = false;
bool opStateRightCh = false;
bool ampInError = false;
int errorCode = 0;
int errorValue = 0;
DeviceAddress addrTempLeftS1;
DeviceAddress addrTempLeftS2;
DeviceAddress addrTempRightS1;
DeviceAddress addrTempRightS2;
const char errorMessages [7][27] = {
  "           No 230V !",
  "DCoffset left (V) : ",
  "DCoffset right (V): ",
  "bias left (A)     : ",
  "bias right (A)    : ",
  "temp left (C)     : ",
  "temp right (C)    : "
};
bool ampPoweredOn = false;
bool showDetailsScreen = true;
unsigned long timeNowplus1s = 0;
unsigned long conversionTime = 0;
#include <digitalWriteFast.h>
void checkButton();
void bypassAllow();
void startAmp();
void shutDownAmp();
void noPower ();
void defineStatusAmp();
void writeValuesScreen();
void oledSchermInit();
void aDCInit();
void tempInit();
void readTempLevels();
void startconversion();
void scanI2CBus();
void printVariables ();
void printAddress(DeviceAddress deviceAddress);
int rightSidePos (int value, int dec);
int leftSidePos (int value);
int readVoltage (int i2cAddress, uint8_t whatToMeasure, float correction);
int measureBias (int i2cAddress);
void waitTillConvReady(int i2cAddress);
void setup();
void loop();
#line 129 "G:/Mijn Drive/platformIO/Amp/src/AmpArduino.ino"
void checkButton() {
  button.loop();
  if(button.isPressed()){
    pressedTime = millis();
    isPressing = true;
  }
  if(button.isReleased()) {
    isPressing = false;
    releasedTime = millis();
    long pressDuration = releasedTime - pressedTime;
    if( pressDuration < shortPressTimeLimit ) {
        isShortDetected = true;
    }
  }
  if((isPressing == true) && (isLongDetected == false)) {
    long pressDuration = millis() - pressedTime;
    if( pressDuration > longPressTimeLimit ) {
      isLongDetected = true;
    }
  }
}

void bypassAllow() {
 #ifdef debugAmp
  Serial.println(F("bypassAllow: waiting "));
 #endif
  bool waitingForPress = true;
  Screen.setFont(fontH08);
  Screen.setCursor(18, 32);
  Screen.print(F("press standby button"));
  Screen.setCursor(6, 44);
  Screen.print(F("protection will be disabled"));
  Screen.sendBuffer();
  while (waitingForPress) {
    button.loop();
    if (button.isPressed()) {
      waitingForPress = false;
    }
  }
  delay(2000);
  Screen.clearBuffer();
  Screen.sendBuffer();
 #ifdef debugAmp
  Serial.println(F("bypassAllow: amp starting, bypass pressed "));
 #endif
}

void startAmp() {
 #ifdef debugAmp
  Serial.println(F("startAmp: starting amp and waiting to stabilize "));
 #endif
  digitalWrite(powerOnOff, HIGH);
  digitalWrite(ledStandby, LOW);
  Screen.setPowerSave(0);
  Screen.clearBuffer();
  Screen.setFont(fontH08);
  Screen.setCursor(0, 8);
  Screen.print(toptekst);
  Screen.setCursor(13, 63);
  Screen.print(bottemTekst);
  Screen.setFont(fontH10);
  Screen.setCursor(0, 38);
  Screen.print(middleTekst);
  Screen.sendBuffer();
  if (startDelayTime < 2) {
    startDelayTime = 2;
  }
  for (int i = startDelayTime; i > 0; i--) {
    delay(1000);
  }
  digitalWrite(startUpResistor, HIGH);
  Screen.clearDisplay();
 #ifdef debugAmp
  Serial.println(F("startAmp: amp started "));
 #endif
}

void shutDownAmp() {
 #ifdef debugAmp
  Serial.println(F("shutDownAmp: shutdown amp "));
 #endif
  digitalWrite(relayOutputLeft, LOW);
  digitalWrite(relayOutputRight, LOW);
  digitalWrite(powerOnOff, LOW);
  digitalWrite(ledStandby, HIGH);
  opStateLeftCh = false;
  opStateRightCh = false;
  Screen.clearBuffer();
  Screen.sendBuffer();
  Screen.setPowerSave(1);
 #ifdef debugAmp
  Serial.println(F("shutDownAmp: amp off "));
 #endif
}

void noPower () {
  if (opStateRightCh && opStateLeftCh) {
    digitalWriteFast(relayOutputLeft, LOW);
    digitalWriteFast(relayOutputRight, LOW);
    digitalWriteFast(powerOnOff, LOW);
    opStateLeftCh = false;
    opStateRightCh = false;
    ampInError = true;
    errorCode=0;
  }
}

void defineStatusAmp() {
 #ifdef debugAmp
  Serial.println(F("defineStatusAmp: checking status "));
 #endif
  bool newStatusLeft = true;
  bool newStatusRight = true;
  if ((aDCOn) && (!ampInError)) {
    if ((dCOffsetLeft < lowDCOffsetInt) | (dCOffsetLeft > highDCOffsetInt)) {
      newStatusLeft=false;
      if (opStateLeftCh) {
        opStateLeftCh=false;
        ampInError = true;
        errorCode=1;
        errorValue = dCOffsetLeft;
      }
    }
    if ((dCOffsetRight < lowDCOffsetInt) | (dCOffsetRight > highDCOffsetInt)) {
      newStatusRight=false;
      if (opStateRightCh) {
        opStateRightCh=false;
        ampInError = true;
        errorCode=2;
        errorValue = dCOffsetRight;
      }
    }
    if ((ampsBiasLeft < lowCurBiasInt) | (ampsBiasLeft > highCurBiasInt)) {
      newStatusLeft=false;
      if (opStateLeftCh) {
        opStateLeftCh=false;
        ampInError = true;
        errorCode=3;
        errorValue = ampsBiasLeft;
      }
    }
    if ((ampsBiasRight < lowCurBiasInt) | (ampsBiasRight > highCurBiasInt)) {
      newStatusRight=false;
      if (opStateRightCh) {
        opStateRightCh=false;
        ampInError = true;
        errorCode=4;
        errorValue = ampsBiasRight;
      }
    }
  }
  if ((tempOn) && (!ampInError)) {
    if (tempLeft > highTemp) {
      newStatusLeft=false;
      if (opStateLeftCh) {
        opStateLeftCh=false;
        ampInError = true;
        errorCode=5;
        errorValue = tempLeft;
      }
    }
    if (tempRight > highTemp) {
      newStatusRight=false;
      if (opStateRightCh) {
        opStateRightCh=false;
        ampInError = true;
        errorCode=6;
        errorValue = tempRight;
      }
    }
  }
  if (ampInError) {
 #ifdef debugAmp
    Serial.println(F("defineStatusAmp: shutdown amp due to error "));
 #endif
    digitalWriteFast(relayOutputLeft, LOW);
    digitalWriteFast(relayOutputRight, LOW);
    digitalWriteFast(powerOnOff, LOW);
    digitalWrite(ledStandby, HIGH);
  }
  if (!(opStateRightCh && opStateLeftCh && (!ampInError))) {
    if ((!opStateLeftCh) && newStatusLeft) {
      opStateLeftCh=true;
    }
    if ((!opStateRightCh) && newStatusRight) {
      opStateRightCh=true;
    }
    if (opStateRightCh && opStateLeftCh) {
      digitalWrite(relayOutputLeft, HIGH);
      digitalWrite(relayOutputRight, HIGH);
    }
  }
  #ifdef debugAmp

    Serial.print(F("defineStatusAmp: newstatus left  : "));
    Serial.println(newStatusLeft);
    Serial.print(F("defineStatusAmp: newstatus right : "));
    Serial.println(newStatusRight);
    printVariables();
 #endif
}

void writeValuesScreen() {
  Screen.clearBuffer();

  if (ampInError) {
    Screen.setFont(fontH08);
    Screen.setCursor(40, 14);
    Screen.print(F("ERROR "));
    Screen.setCursor(15, 44);
    Screen.print(errorMessages[errorCode]);
    if ((errorCode > 0)&&(errorCode < 5)){
      Screen.print(errorValue * 0.01 ,1);
    }
    if (errorCode > 4) {
      Screen.print(errorValue);
    }
    Screen.sendBuffer();
    while (true);
  }

  if ((!ampInError) and (showDetailsScreen)) {
    Screen.setFont(fontH10);
    Screen.setCursor(46, 14);
    Screen.print(F("STATE"));
    Screen.setFont(fontgrahp);
    Screen.setCursor(0, 14);
    if (opStateLeftCh) {Screen.drawGlyph(5, 16 , 0x2714);}
    else {Screen.drawGlyph(5, 16 , 0x2715);}
    if (opStateRightCh) {Screen.drawGlyph(112, 16 , 0x2714);}
    else {Screen.drawGlyph(108, 16 , 0x2715);}
    Screen.setFont(fontH08fixed);
    if (aDCOn) {
      Screen.setCursor(42, 40);
      Screen.print(F("DCoffset(V)"));
      Screen.setCursor(50, 51);
      Screen.print(F("Bias(A)"));
      Screen.setCursor(leftSidePos(dCOffsetLeft), 40);
      Screen.print((dCOffsetLeft*0.01) ,numberOffDec);
      Screen.setCursor(rightSidePos(dCOffsetRight,numberOffDec),40);
      Screen.print((dCOffsetRight*0.01) ,numberOffDec);
      Screen.setCursor(leftSidePos(ampsBiasLeft), 51);
      Screen.print(ampsBiasLeft*0.01, numberOffDec);
      Screen.setCursor(rightSidePos(ampsBiasRight,numberOffDec), 51);
      Screen.print(ampsBiasRight*0.01 ,numberOffDec);
    }
    if (tempOn) {
      Screen.setCursor(50, 63);
      Screen.print(F("Temp(C)"));
      if (tempLeft == -200) {
        Screen.setCursor(0, 63);
        Screen.print(F("err"));
      }
      else {
        Screen.setCursor(leftSidePos(tempLeft*100), 63);
        Screen.print(tempLeft);
      }
      if (tempRight == -200) {
        Screen.setCursor(110, 63);
        Screen.print(F("err"));
      }
      else {
        Screen.setCursor(rightSidePos((tempRight*100) ,numberOffDec), 63);
        Screen.print(tempRight);
      }
    }
  }

  if ((!ampInError) and (!showDetailsScreen)) {
    Screen.setFont(fontgroot);
    Screen.setCursor(10, 35);
    Screen.print(F("ALEPH J"));
    if (tempOn) {
      Screen.setFont(fontH08);
      Screen.setCursor(45, 63);
      Screen.print(F("Temp (C)"));
      Screen.setFont(fontH08fixed);
      if (tempLeft == -200) {
        Screen.setCursor(0, 63);
        Screen.print(F("err"));
      }
      else {
        Screen.setCursor(0, 63);
        Screen.print(tempLeft);
      }
      if (tempRight == -200) {
        Screen.setCursor(110, 63);
        Screen.print(F("err"));
      }
      else {
        Screen.setCursor(117, 63);
        Screen.print(tempRight);
      }
    }
  }
  Screen.sendBuffer();
 #ifdef debugAmp
   Serial.println(F("writeValuesScreen: screen updated "));
 #endif
}

void oledSchermInit() {

  digitalWrite(oledReset, LOW);
  delay(10);
  digitalWrite(oledReset, HIGH);
  delay(110);
  Screen.setI2CAddress(oledAddress * 2);
  Screen.initDisplay();
  Screen.clearDisplay();
  Screen.setContrast((((contrastLevelScreen * 2) + 1) << 4) | 0x0f);
  Screen.setFlipMode(1);
  Screen.clearDisplay();
  Screen.setPowerSave(0);

 #ifdef debugAmp
  Serial.println(F("oledSchermInit: end of procedure"));
 #endif
}

void aDCInit() {
  uint8_t error = 0;
  Wire.beginTransmission(aDCLeftI2CAddress);
  error = Wire.endTransmission();
  if (error != 0) {
    Screen.setFont(fontH10);
    Screen.setCursor(15, 10);
    Screen.print(F("No ADC left side"));
    bypassAllow();
    aDCOn = false;
  }
  Wire.beginTransmission(aDCRightI2CAddress);
  error = Wire.endTransmission();
  if (error != 0) {
    Screen.setFont(fontH10);
    Screen.setCursor(15, 10);
    Screen.print(F("No ADC right side"));
    bypassAllow();
    aDCOn = false;
  }
 #ifdef debugAmp
  Serial.println(F("aDCInit: init of ADC's done"));
 #endif
}

void tempInit() {
  int numberOfSensorsLeft;
  int numberOfSensorsRight;
 #ifdef debugAmp
  Serial.print(F("tempInit: number of sensors / channel defined in code  : "));
  Serial.println(numberOfSensorsCh);
 #endif
  tempSensorLeft.begin();
  numberOfSensorsLeft = tempSensorLeft.getDeviceCount();
  if (numberOfSensorsCh != numberOfSensorsLeft ) {
    Screen.setFont(fontH10);
    Screen.setCursor(15, 10);
    Screen.print(F("No temp left side"));
    bypassAllow();
    tempOn = false;
  }
  tempSensorRight.begin();
  numberOfSensorsRight = tempSensorRight.getDeviceCount();
  if (numberOfSensorsCh != numberOfSensorsRight ) {
    Screen.setFont(fontH10);
    Screen.setCursor(15, 10);
    Screen.print(F("No temp right side"));
    bypassAllow();
    tempOn = false;
  }
  if (tempOn) {
    tempSensorLeft.getAddress(addrTempLeftS1,0);
    tempSensorRight.getAddress(addrTempRightS1,0);
    if (numberOfSensorsCh == 2) {
      tempSensorLeft.getAddress(addrTempLeftS2,1);
      tempSensorRight.getAddress(addrTempRightS2,1);
    }
    tempSensorLeft.setResolution(9);
    tempSensorLeft.setWaitForConversion(false);
    tempSensorRight.setResolution(9);
    tempSensorRight.setWaitForConversion(false);
  }
 #ifdef debugAmp
  Serial.print(F("tempInit: Temperature measurement active               : "));
  Serial.println(tempOn);
  Serial.print(F("tempInit: number of sensors found left side            : "));
  Serial.println(numberOfSensorsLeft);
  if (numberOfSensorsLeft > 0) {
    Serial.print(F("TempInit: sensor address nr 1 : "));
    printAddress(addrTempLeftS1);
  }
  if (numberOfSensorsLeft == 2) {
    Serial.print(F("tempInit: sensor address nr 2 : "));
    printAddress(addrTempLeftS2);
  }
  Serial.print(F("tempInit: number of sensors found right side           : "));
  Serial.println(numberOfSensorsRight);
  if (numberOfSensorsLeft > 0) {
    Serial.print(F("tempInit: sensor address nr 1 : "));
  printAddress(addrTempRightS1);
  }
  if (numberOfSensorsLeft == 2) {
    Serial.print(F("tempInit: sensor address nr 2 : "));
    printAddress(addrTempRightS2);
  }
 #endif
}

void readTempLevels() {
  int temp1;
  int temp2 = 0;
  temp1 = round(tempSensorLeft.getTempC(addrTempLeftS1));
  if (temp1 == DEVICE_DISCONNECTED_C) {
    temp1 = -200;
  }
  if (numberOfSensorsCh == 2) {
    temp2 = round(tempSensorLeft.getTempC(addrTempLeftS2));
    if (temp2 == DEVICE_DISCONNECTED_C) {
      temp2 = -200;
    }
  }
  if ((temp1 == -200) | (temp2 == -200)) {
    tempLeft = -200;
  }
  else {
    if (temp1 > temp2) {tempLeft = temp1;}
    else {tempLeft = temp2;}
  }
 #ifdef debugAmp
    Serial.println(F("readTempLevel: following Temparature levels measured :"));
    Serial.print(F("Temperature level left nr1   (C): "));
    Serial.println(temp1);
    Serial.print(F("Temperature level left nr2   (C): "));
    Serial.println(temp2);
 #endif
 temp1 = round(tempSensorRight.getTempC(addrTempRightS1));
  if (temp1 == DEVICE_DISCONNECTED_C) {
    temp1 = -200;
  }
  if (numberOfSensorsCh == 2) {
    temp2 = round(tempSensorRight.getTempC(addrTempRightS2));
    if (temp2 == DEVICE_DISCONNECTED_C) {
      temp2 = -200;
    }
  }
  if ((temp1 == -200) | (temp2 == -200)) {
    tempRight = -200;
  }
  else {
    if (temp1 > temp2) {tempRight = temp1;}
    else {tempRight = temp2;}
  }
 #ifdef debugAmp
    Serial.print(F("Temperature level right nr1  (C): "));
    Serial.println(temp1);
    Serial.print(F("Temperature level right nr2  (C): "));
    Serial.println(temp2);
 #endif
}

void startconversion() {
  tempSensorLeft.requestTemperatures();
  tempSensorRight.requestTemperatures();
}

 #ifdef debugAmp
void scanI2CBus() {
  uint8_t error;
  uint8_t address;
  int numberOfDevices;
  Serial.println(F("scanI2CBus: I2C addresses defined within the code are : "));
  Serial.print(F("Screen             : 0x"));
  Serial.println(oledAddress, HEX);
  Serial.print(F("ADC board left     : 0x"));
  Serial.println(aDCLeftI2CAddress, HEX);
  Serial.print(F("ADC board right    : 0x"));
  Serial.println(aDCRightI2CAddress, HEX);
  Serial.println(F("ScanI2C: Scanning..."));
  numberOfDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      numberOfDevices++;
    }
  }
  if (numberOfDevices == 0) Serial.println(F("ScanI2C:No I2C devices found"));
  else {
    Serial.print(F("scanI2CBus: done, number of device found : "));
    Serial.println(numberOfDevices);
  }
}
 #endif

 #ifdef debugAmp
void printVariables () {
  Serial.println(F("PrintVariables :"));
  Serial.print(F("temp sensors reachable          : "));
  Serial.println(tempOn);
  Serial.print(F("Are ADC both reachable          : "));
  Serial.println(aDCOn);
  Serial.print(F("Operational state left          : "));
  Serial.println(opStateLeftCh);
  Serial.print(F("Operational state right         : "));
  Serial.println(opStateRightCh);
  Serial.print(F("AMP has an error                : "));
  Serial.println(ampInError);
  Serial.print(F("Errorcode                       : "));
  Serial.println(errorCode);
  Serial.print(F("Value at error                  : "));
  Serial.println(errorValue);
  Serial.print(F("detail screen active            : "));
  Serial.println(showDetailsScreen);

  Serial.println();
}
 #endif

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
  Serial.println();
}

int rightSidePos (int value, int dec) {
  if (value >= 0) {
    if (value < 1000) {
      if (dec == 0) {return(112);}
      if (dec == 1) {return(112);}
      if (dec == 2) {return(107);}
    }
    else {
      if (dec == 0) {return(107);}
      if (dec == 1) {return(107);}
      if (dec == 2) {return(102);}
    }
  }
  else {
    if (value > -1000) {
      if (dec == 0) {return(107);}
      if (dec == 1) {return(107);}
      if (dec == 2) {return(102);}
    }
    else {
      if (dec == 0) {return(102);}
      if (dec == 1) {return(102);}
      if (dec == 2) {return(97);}
    }
  }
  return(92);
}

int leftSidePos (int value) {
  if (value >= 0) {
    if (value < 1000) {return(10);}
    else {return(5);}
  }
  else {
    if (value > -1000) {return(5);}
    else {return(0);}
  }
}

int readVoltage (int i2cAddress, uint8_t whatToMeasure, float correction) {
  int sampleRaw = 0;
  Wire.beginTransmission(i2cAddress);
  Wire.write(SelectConfigRegister);
  Wire.write(whatToMeasure);
  Wire.write(numOfSample);
  Wire.endTransmission();

  waitTillConvReady(i2cAddress);

  Wire.beginTransmission(i2cAddress);
  Wire.write(SelectConversionRegister);
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress, 2);
  sampleRaw = Wire.read() << 8;
  sampleRaw |= Wire.read();
  Wire.endTransmission();
  return(round(sampleRaw*correction));
}

int measureBias (int i2cAddress) {

  int BiasPlusSide=readVoltage(i2cAddress,measureBiasPlus,corBias);
  Serial.println(BiasPlusSide);
  Serial.println(readVoltage(i2cAddress,measureBiasMinus,corBias));
  return(BiasPlusSide-readVoltage(i2cAddress,measureBiasMinus,corBias));
}

void waitTillConvReady(int i2cAddress)
{
  uint8_t busyBit;


  do
  {
    Wire.beginTransmission(i2cAddress);
    Wire.write(SelectConfigRegister);
    Wire.endTransmission();

    Wire.requestFrom(i2cAddress, 1);
    busyBit = Wire.read();
    Wire.endTransmission();
  }
  while ((busyBit & 0x80) == 0);
}

void setup() {
  corOffset = (((resistorLow + resistorHigh)) / resistorLow) * voltageStep * 100.0;
  corBias = biasMultiplier * (corOffset/biasResistor);
  lowCurBiasInt = round(lowCurBiasFloat * 100);
  highCurBiasInt = round(highCurBiasFloat * 100);
  lowDCOffsetInt = round(lowDCOffsetFloat * 100);
  highDCOffsetInt = round(highDCOffsetFloat * 100);


  pinMode(powerOnOff, OUTPUT);
  pinMode(relayOutputLeft, OUTPUT);
  pinMode(relayOutputRight, OUTPUT);
  pinMode(startUpResistor, OUTPUT);
  pinMode(ledStandby, OUTPUT);
  pinMode(oledReset, OUTPUT);
  pinMode(detect230V, INPUT_PULLUP);


  digitalWrite(powerOnOff, LOW);
  digitalWrite(relayOutputLeft, LOW);
  digitalWrite(relayOutputRight, LOW);
  digitalWrite(startUpResistor, LOW);
  digitalWrite(ledStandby, LOW);
  digitalWrite(oledReset, LOW);


 #ifdef debugAmp
  Serial.begin(9600);
  Serial.print(F("corOffset  " ));
  Serial.println(corOffset);
  Serial.print(F("corBias "));
  Serial.println(corBias);
  Serial.print("status detect 230V pin : ");
  Serial.println(digitalRead(detect230V));
 #endif
  Wire.begin();
  delay(100);
  oledSchermInit();
  aDCInit();
  tempInit();
 #ifdef debugAmp
  scanI2CBus();
 #endif
  if (startUpAtPower) {
    startAmp();
    ampPoweredOn = true;
  }
  else {
    shutDownAmp();
  }
 #ifdef debugAmp
  Serial.println(F("setup: end of setup proc"));
 #endif
}

void loop() {
  timeNowplus1s = millis() + 1000;
  while (millis() < timeNowplus1s) {
    if (ampPoweredOn) {
      if (!digitalReadFast(detect230V)) noPower();

      while (showDetailsScreen) {
        if (!digitalReadFast(detect230V)) noPower();
        if (aDCOn) {
          dCOffsetLeft=readVoltage(aDCLeftI2CAddress,measureDCOfset,corOffset);
          dCOffsetRight=readVoltage(aDCRightI2CAddress,measureDCOfset,corOffset);
          ampsBiasLeft=measureBias(aDCLeftI2CAddress)+correctionOffsetBiasLeft;
          ampsBiasRight=measureBias(aDCRightI2CAddress)+correctionOffsetBiasRight;
        }
        if (tempOn) {
          if (conversionTime == 0) {
            startconversion();
            conversionTime=millis() + 2000;
          }
          if (millis() > conversionTime) {
            readTempLevels();
            conversionTime = 0;
          }
        }
        defineStatusAmp();
        writeValuesScreen();
        checkButton();
        if(isShortDetected ) {
          showDetailsScreen = false;
          isShortDetected = false;
        }
        if (isLongDetected) {
          showDetailsScreen = false;
          ampPoweredOn = false;
          shutDownAmp();
          delay(2000);
          isLongDetected = false;
        }

        if (((millis() > (timeNowplus1s + timeToShowDetailScreen)) and (opStateLeftCh) and (opStateRightCh))) {
          showDetailsScreen = false;
        }

      }
      if (aDCOn) {
        dCOffsetLeftOld=dCOffsetLeft;
        dCOffsetRightOld=dCOffsetRight;
        dCOffsetLeft=readVoltage(aDCLeftI2CAddress,measureDCOfset,corOffset);
        dCOffsetRight=readVoltage(aDCRightI2CAddress,measureDCOfset,corOffset);
        dCOffsetLeft= (dCOffsetLeftOld + dCOffsetLeft) / 2;
        dCOffsetRight= (dCOffsetRightOld + dCOffsetRight) / 2;
      }
      if (tempOn) {
        if (conversionTime == 0) {
          startconversion();
          conversionTime=millis() + 2000;
        }
        if (millis() > conversionTime) {
          readTempLevels();
          conversionTime = 0;
        }
      }
      defineStatusAmp();
    }
    checkButton();
    if(isShortDetected ) {
      showDetailsScreen = true;
      isShortDetected = false;
    }
    if (isLongDetected) {
      if (ampPoweredOn) {
        ampPoweredOn = false;
        shutDownAmp();
        delay(2000);
      }
      else {
        ampPoweredOn = true;
        startAmp();
        showDetailsScreen = true;
      }
      isLongDetected = false;
    }
  }
  if (ampPoweredOn) {
    writeValuesScreen();
  }
}