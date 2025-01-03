//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// v0.1,  base used content of preAmpArduino
// v0.2,  optimized voltage readings as all was way to slow.
//        changed naming convention
//        split voltage readings
// v0.3   further optimized voltage reading by changing them to async
//        fixed error in biasreading
// v0.4   lots of optimalisations, as the code was way to slow
// v0.5   removed interupt as is was triggered incorrectly,
//        changed bias and offset reading to change in hardware

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// below definitions could be change by user depending on setup, no code changes needed
//#define debugAmp                                    // Comment this line when debugAmp mode is not needed
const int contrastLevelScreen = 5;                  // contrastlevel of screen, value 1 - 7
const int resistorLow = 50;                         // low value resistor in voltage divider in Kohm
const int resistorHigh = 600;                       // high value resistor in voltage divider in Kohm
const int biasResistor = 1;                      // value of bias resistor in Ohm   
const float lowCurBiasFloat = -2.0;                 // low cutoff value bias in A, 1 decimals
const float highCurBiasFloat = 2.0;                 // high cutoff value bias in A, 1 decimals
const float lowDCOffsetFloat = -1.0;                // low cutoff value DC offset in V, 1 decimal
const float highDCOffsetFloat = 1.0;                // high cutoff value DC offset in V, 1 decimal
const int highTemp = 70;                            // high temp cutoff in Celcius
const bool startUpAtPower = true;                   // if true amp starts if power applied, if false it will be in standby mode
int startDelayTime = 2;                             // delay after power on of AMP, startup resistor is active, monitoring will start after this time and speakers could be connected 
const int numberOfSensorsCh = 1;                    // number of temp sensors / side. value 1 or 2
const char* toptekst = "";                          // toptext, could be changed
const char* middleTekst = "        PeWalt, V 0.4";  // Version of the code";
const char* bottemTekst = " " ;                     // as an example const char*bBottemTekst = "design by: Walter Widmer" ;
const int numberOffDec = 2 ;                        // number of dec on the screen, could be 1 or 2
#define timeToShowDetailScreen 30000                // time in mS to show detail screen if button pushed
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pin definitions
#define powerOnOff A0         // pin connected to the relay handeling power on/off of the amp
#define relayOutputLeft A1    // pin connected to the relay handeling left output to speaker
#define relayOutputRight A2   // pin connected to the relay handeling right output to speaker
#define startUpResistor A3    // pin connected relay handeling bypass of resister used when starting the AMP
#define spare A4              // spare
#define oneWireLeft 4         // pin connected to the sensors measuring temp of left amp
#define oneWireRight 5        // pin connected to the sensors measuring temp of rigt amp
#define detect230V 2          // pin connected to relay which monitors 230V
#define buttonStandby 7       // pin connected to button to switch between on and standby
#define ledStandby 11         // connected to a led that is on if amp is in standby mode
#define oledReset 12          // connected to the reset port of Oled screen, used to reset Oled screen
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// definitions for the oled screen
#define oledAddress 0x3C                             // 3C is address used by oled controler
#define fontH08 u8g2_font_timB08_tr                  // 11w x 11h, char 7h
#define fontH08fixed u8g2_font_spleen5x8_mr          // 5w x 8h, char 6h
#define fontH10 u8g2_font_timB10_tr                  // 15w x 14h, char 10h
#define fontgrahp u8g2_font_unifont_t_78_79          // 16w x 16h pixels
#define fontgroot u8g2_font_lubB19_tr                // 29w x26, car 19H
#include <arduino.h>
#include <U8g2lib.h>                                 // include graphical based character mode library
#include <Wire.h>
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C Screen(U8G2_R2,oledReset);  // define the screen type used.
//U8G2_SSD1309_128X64_NONAME0_F_2ND_HW_I2C Screen(U8G2_R2);  // define the screen type used.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// definitions for the ADC controlers
int aDCLeftI2CAddress = 0x48;                    // 48 is I2C address used by left ADC controler
int aDCRightI2CAddress = 0x49;                   // 49 is I2C address used by right ADC controler
#define SelectConversionRegister 0b00000000      // address of conversion register
#define SelectConfigRegister 0b00000001          // address of config register
#define measureDCOfset 0b10010101                //Read dif ain0-3, 2.048V, Single Shot
#define measureBiasPlus 0b10100101               //Read dif ain1-3, 2.048V, Single Shot
#define measureBiasMinus 0b10110101              //Read dif ain2-3, 2.048V, Single Shot
#define numOfSample 0b10100011                   //250 samples/sec
float voltageStep = 0.0000625;                   // 2.048 / 32768(15 bits)
float corOffset = 0;                             // number to convert from measured v to actual V
float corBias = 0;                               // number to convert from measured v to I
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// definitions for the temp sensors
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire wireLeft(oneWireLeft);                    // install onewire instance on the port  
OneWire wireRight(oneWireRight);                  // install onewire instance on the port
DallasTemperature tempSensorLeft(&wireLeft);      // create left instance
DallasTemperature tempSensorRight(&wireRight);    // create righ instance
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// button definitions
#include <ezButton.h>                    // include functions for debounce
ezButton button(buttonStandby);          // generate an instance of ezButton
const int shortPressTimeLimit = 1000;    // used to detect short press
const int longPressTimeLimit  = 1000;    // used to detect long press
unsigned long pressedTime  = 0;          // used to detect pressing of button
unsigned long releasedTime = 0;          // used to detect pressing of button
bool isPressing = false;                 // status of button
bool isLongDetected = false;             // status of button
bool isShortDetected = false;
////////////////////////////////////////////////////////////////////////////////////
// general definitions
int lowCurBiasInt;                       // convert limit value to int
int highCurBiasInt;                      // convert limit value to int
int lowDCOffsetInt;                      // convert limit value to int
int highDCOffsetInt;                     // convert limit value to int
bool aDCOn = true;                       // measure voltage ?
bool tempOn = true;                      // measure temperature ?
int dCOffsetLeft;                        // voltage level output channel left * 100
int dCOffsetRight;                       // voltage level output channel left * 100
int ampsBiasLeft;                        // voltage level bias left channel * 100
int ampsBiasRight;                       // voltage level bias left channel * 100
int tempLeft;                            // tempature of left channel
int tempRight;                           // tempature of right channel
bool opStateLeftCh = false;              // boolean defines if channel is on or off
bool opStateRightCh = false;             // boolean defines if channel is on or off
bool ampInError = false;                 // Amp in error
int errorCode = 0;                       // which issue caused the AMP to stop, temp, bias, dcoffset
int errorValue = 0;                      // value of the parameter at time of errort
DeviceAddress addrTempLeftS1;            // address temp sensor
DeviceAddress addrTempLeftS2;            // address temp sensor
DeviceAddress addrTempRightS1;           // address temp sensor
DeviceAddress addrTempRightS2;           // address temp sensor
const char errorMessages [7][27] = {     // error codes starting with 0
  "           No 230V !",
  "DCoffset left (V) : ",
  "DCoffset right (V): ",
  "bias left (A)     : ",
  "bias right (A)    : ",
  "temp left (C)     : ",
  "temp right (C)    : "
};
bool ampPoweredOn = false;                        // defines if amp is powered on. 
bool showDetailsScreen = true;                    // number of seconds detail screen is shown.
unsigned long timeNowplus1s = 0;                  // used to keep track of time for screen update
unsigned long conversionTime = 0;                 // used to keep track of time temp conversion
#include <digitalWriteFast.h>                     // include fast read used within interrupt routine
/////////////////////////////////////////////////////////////////////////////////
// detect short and long press 
/////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// bypass procedure to start amp although their is an issue
/////////////////////////////////////////////////////////////////////////////////////////////
void bypassAllow() {
 #ifdef debugAmp                                   // if debugAmp enabled write message
  Serial.println(F("bypassAllow: waiting "));
 #endif
  bool waitingForPress = true;
  Screen.setFont(fontH08);                                            // set font
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
  Screen.clearBuffer();                       // clear the internal memory and screen
  Screen.sendBuffer();  
 #ifdef debugAmp                                 // if debugAmp enabled write message
  Serial.println(F("bypassAllow: amp starting, bypass pressed "));
 #endif
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// startup amp, arduino itself and sensors are already active
///////////////////////////////////////////////////////////////////////////////////////////////
void startAmp() {    
 #ifdef debugAmp                              // if debugAmp enabled write message
  Serial.println(F("startAmp: starting amp and waiting to stabilize "));
 #endif
  digitalWrite(powerOnOff, HIGH);             // start amp
  digitalWrite(ledStandby, LOW);              //
  Screen.setPowerSave(0); 
  Screen.clearBuffer();                       // clear the internal memory and screen
  Screen.setFont(fontH08);                    // choose a suitable font
  Screen.setCursor(0, 8);                     // set cursur in correct position
  Screen.print(toptekst);                     // write tekst to buffer
  Screen.setCursor(13, 63);                   // set cursur in correct position
  Screen.print(bottemTekst);                  // write tekst to buffer
  Screen.setFont(fontH10);                    // choose a suitable font
  Screen.setCursor(5, 28);                    // set cursur in correct position
  Screen.print(middleTekst);                  // write please wait
  Screen.sendBuffer();
  if (startDelayTime < 2) {
    startDelayTime = 2;
  }
  for (int i = startDelayTime; i > 0; i--) {  // run for startdelaytime times
    delay(1000);                              // delay for a second
  }
  digitalWrite(startUpResistor, HIGH);        // bypass startup resistor
  Screen.clearDisplay();                      // clear screen
 #ifdef debugAmp                              // if debugAmp enabled write message
  Serial.println(F("startAmp: amp started "));
 #endif
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// turn amp off, arduino itself and sensors stay active
///////////////////////////////////////////////////////////////////////////////////////////
void shutDownAmp() {
 #ifdef debugAmp                              // if debugAmp enabled write message
  Serial.println(F("shutDownAmp: shutdown amp "));
 #endif
  digitalWrite(relayOutputLeft, LOW);
  digitalWrite(relayOutputRight, LOW);
  digitalWrite(powerOnOff, LOW);             // shut down power amp
  digitalWrite(ledStandby, HIGH);            // 
  opStateLeftCh = false;
  opStateRightCh = false;
  Screen.clearBuffer();  
  Screen.sendBuffer();  
  Screen.setPowerSave(1); 
 #ifdef debugAmp               // if debugAmp enabled write message
  Serial.println(F("shutDownAmp: amp off "));
 #endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   procedure to prevent thump if power fails
///////////////////////////////////////////////////////////////////////////////////////////////
void noPower () {
  if (opStateRightCh && opStateLeftCh) {
    digitalWriteFast(relayOutputLeft, LOW);            // turn left channel off
    digitalWriteFast(relayOutputRight, LOW);           // turn right channel off
    digitalWriteFast(powerOnOff, LOW); 
    opStateLeftCh = false;
    opStateRightCh = false;
    ampInError = true;                            // amp is in error
    errorCode=0;
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// procedure to verify status
//////////////////////////////////////////////////////////////////////////////////////////
void defineStatusAmp() {
 #ifdef debugAmp                                 // if debugAmp enabled write message
  Serial.println(F("defineStatusAmp: Starting "));
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
 #ifdef debugAmp               // if debugAmp enabled write message
    Serial.println(F("defineStatusAmp: shutdown amp due to error "));
 #endif    
    digitalWriteFast(relayOutputLeft, LOW);
    digitalWriteFast(relayOutputRight, LOW);
    digitalWriteFast(powerOnOff, LOW);            
    digitalWrite(ledStandby, HIGH);            
  }
  if (!(opStateRightCh && opStateLeftCh && (!ampInError))) {
    if ((!opStateLeftCh) && newStatusLeft) { // turn channel on if all conditions met
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
  else {
    showDetailsScreen = true;
  }
  #ifdef debugAmp               // if debugAmp enabled write message
    
    Serial.print(F("defineStatusAmp: newstatus left  : "));
    Serial.println(newStatusLeft);
    Serial.print(F("defineStatusAmp: newstatus right : "));
    Serial.println(newStatusRight);
    printVariables();
 #endif  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write values on the screen (state, voltage levels and temperature)
//////////////////////////////////////////////////////////////////////////////////////////
void writeValuesScreen() {
  Screen.clearBuffer();         
  // if amp in error display error message 
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
    while (true);   
  }
  // display detail screen
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
  // display general screen
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
 #ifdef debugAmp               // if debugAmp enabled write message
   Serial.println(F("writeValuesScreen: screen updated "));
 #endif  
}
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// intialisation of the screen after powerup of screen.
/////////////////////////////////////////////////////////////////////////////////////////////
void oledSchermInit() {
  Screen.setI2CAddress(oledAddress * 2);                               // set oled I2C address
  digitalWrite(oledReset, LOW);                                        // set screen in reset mode
  delay(10);                                                           // wait to stabilize
  digitalWrite(oledReset, HIGH);                                       // set screen active
  delay(110);  
  Screen.initDisplay();
  delay(5);
  Screen.clearDisplay();
  delay(5);
  Screen.setPowerSave(0);
  Screen.setContrast((((contrastLevelScreen * 2) + 1) << 4) | 0x0f);   // set contrast level, reduce number of options
  
 #ifdef debugAmp
  Serial.println(F("oledSchermInit: end of procedure"));
 #endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check Analoog Digital converter left and right channel
/////////////////////////////////////////////////////////////////////////////////////////////
void aDCInit() {
  uint8_t error = 0;
  Wire.beginTransmission(aDCLeftI2CAddress);                                              // test address
  error = Wire.endTransmission();                                               // resolve errorcode
  if (error != 0) {                                                             // if address exist code is 0
    Screen.setFont(fontH10);
    Screen.setCursor(15, 10);
    Screen.print(F("No ADC left side"));
    bypassAllow();
    aDCOn = false;
  }
  Wire.beginTransmission(aDCRightI2CAddress);                                              // test address
  error = Wire.endTransmission();                                               // resolve errorcode
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
}/////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
void tempInit() {
  int numberOfSensorsLeft;
  int numberOfSensorsRight;
 #ifdef debugAmp
  Serial.print(F("tempInit: number of sensors / channel defined in code  : "));
  Serial.println(numberOfSensorsCh);
 #endif
  tempSensorLeft.begin();                             // intialize the tempsensors left channel
  numberOfSensorsLeft = tempSensorLeft.getDeviceCount();
  if (numberOfSensorsCh != numberOfSensorsLeft ) {
    Screen.setFont(fontH10);
    Screen.setCursor(15, 10);
    Screen.print(F("No temp left side"));
    bypassAllow();
    tempOn = false;
  }
  tempSensorRight.begin();                             // intialize the tempsensors right channel
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
///////////////////////////////////////////////////////////////////////////////////////
// read the temperature sensors, request to do conversion is already done
///////////////////////////////////////////////////////////////////////////////////////
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
 #ifdef debugAmp               // if debugAmp enabled write message
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

//////////////////////////////////////////////////////////////////////////////////////////
// start conversion of temp sensors
//////////////////////////////////////////////////////////////////////////////////////////////
void startconversion() { 
  tempSensorLeft.requestTemperatures();
  tempSensorRight.requestTemperatures();
}
//////////////////////////////////////////////////////////////////////////////////////////
// function to check i2c bus
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 #ifdef debugAmp
void scanI2CBus() {
  uint8_t error;                                                      // error code
  uint8_t address;                                                    // address to be tested
  int numberOfDevices;                                                // number of devices found
  Serial.println(F("scanI2CBus: I2C addresses defined within the code are : ")); // print content of code
  Serial.print(F("Screen             : 0x"));
  Serial.println(oledAddress, HEX);
  Serial.print(F("ADC board left     : 0x"));
  Serial.println(aDCLeftI2CAddress, HEX);
  Serial.print(F("ADC board right    : 0x"));
  Serial.println(aDCRightI2CAddress, HEX);
  Serial.println(F("ScanI2C: Scanning..."));
  numberOfDevices = 0;
  for (address = 1; address < 127; address++) {                                   // loop through addresses
    Wire.beginTransmission(address);                                              // test address
    error = Wire.endTransmission();                                               // resolve errorcode
    if (error == 0) {                                                             // if address exist code is 0
      Serial.print("I2C device found at address 0x");                             // print address info
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function to print variables in debug mode
///////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function to print a device address
//////////////////////////////////////////////////////////////////////////////////////////
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
  Serial.println();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// define the position of the cursor right side depending on the length and sign of the value
///////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////
// define the position of the cursor left side depending on the length and sign of the value
///////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////
// measure voltage
////////////////////////////////////////////////////////////////////////////////////////////
int readVoltage (int i2cAddress, uint8_t whatToMeasure, float correction) {
  int sampleRaw = 0;
  Wire.beginTransmission(i2cAddress);
  Wire.write(SelectConfigRegister);
  Wire.write(whatToMeasure);   // OS=1 (Start Conversion), 2.048, Single Shot
  Wire.write(numOfSample);     // 250 SPS, Comparitor Disabled
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
////////////////////////////////////////////////////////////////////////////////////////////
// measure voltage
////////////////////////////////////////////////////////////////////////////////////////////
int measureBias (int i2cAddress) {
  int BiasPlusSide=readVoltage(i2cAddress,measureBiasPlus,corBias);
  return(BiasPlusSide-readVoltage(i2cAddress,measureBiasMinus,corBias));
}
///////////////////////////////////////////////////////////////////////////////////////
// wait till de conversion of the ADC is done
///////////////////////////////////////////////////////////////////////////////////////
void waitTillConvReady(int i2cAddress)
{
  uint8_t busyBit;

  // Wait for the Operating State bit to go LOW
  do
  {
    Wire.beginTransmission(i2cAddress);
    Wire.write(SelectConfigRegister);
    Wire.endTransmission();

    Wire.requestFrom(i2cAddress, 1);
    busyBit = Wire.read();
    Wire.endTransmission();
  }
  while ((busyBit & 0x80) == 0);  // Check for Busy flag
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  corOffset = (((resistorLow + resistorHigh)) / resistorLow) * voltageStep * 100.0;   
  corBias =  (corOffset/biasResistor);
  lowCurBiasInt = round(lowCurBiasFloat * 100);              // convert to int
  highCurBiasInt = round(highCurBiasFloat * 100);            // convert to int
  lowDCOffsetInt = round(lowDCOffsetFloat * 100);            // convert to int
  highDCOffsetInt = round(highDCOffsetFloat * 100);          // convert to int
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // pin modes
  pinMode(powerOnOff, OUTPUT);             // control the relay that provides power to the rest of the amp
  pinMode(relayOutputLeft, OUTPUT);        // control the relay that connects the amp to the left output port
  pinMode(relayOutputRight, OUTPUT);       // control the relay that connects the amp to the right output port
  pinMode(startUpResistor, OUTPUT);        // control the relay that shortcuts the startup resistor
  pinMode(ledStandby, OUTPUT);             // led output
  pinMode(oledReset, OUTPUT);              // set reset of oled screen
  pinMode(detect230V, INPUT_PULLUP);       // pin detect230V is high and its an input port
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // write init state to output pins
  digitalWrite(powerOnOff, LOW);           // keep amp turned off
  digitalWrite(relayOutputLeft, LOW);      // output port left is disconnected
  digitalWrite(relayOutputRight, LOW);     // output port right is disconnected
  digitalWrite(startUpResistor, LOW);      // startup resistor is not shortcutted
  digitalWrite(ledStandby, LOW);           // turn off standby led to indicate device is becoming active
  digitalWrite(oledReset, LOW);            // keep the Oled screen in reset mode
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // init screen, adc and temperature sensors
 #ifdef debugAmp
  Serial.begin(9600);                                 // if debuglevel on start monitor screen
  Serial.print(F("corOffset  " )); 
  Serial.println(corOffset); 
  Serial.print(F("corBias ")); 
  Serial.println(corBias);
  Serial.print("status detect 230V pin : ");
  Serial.println(digitalRead(detect230V));
 #endif
  Wire.begin();                                       // start i2c communication
  delay(100);
  oledSchermInit();                                   // intialize the Oled screen
  aDCInit();                                          // intialize the analog to digital converters
  tempInit();                                         // initialize the temp sensors
 #ifdef debugAmp
  scanI2CBus();                                       // in debugAmp mode show i2c addresses used
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
//////////////////////////////////////////////////////////////////////////////////////////////
// Main loop
//////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  timeNowplus1s = millis() + 1000;
  while (millis() < timeNowplus1s) {      //only update the screen every second
    if (ampPoweredOn) {
      if (!digitalReadFast(detect230V)) noPower();
      ////loop through the detail screen
      while (showDetailsScreen)  {           //detail screen only active in specific cases
        if (!digitalReadFast(detect230V)) noPower();
        if (aDCOn) {                         //if we can measure voltages
          dCOffsetLeft=readVoltage(aDCLeftI2CAddress,measureDCOfset,corOffset);
          dCOffsetRight=readVoltage(aDCRightI2CAddress,measureDCOfset,corOffset);
          ampsBiasLeft=measureBias(aDCLeftI2CAddress);
          ampsBiasRight=measureBias(aDCRightI2CAddress);
        }
        if (tempOn) {                        //if we can read temp
          if (conversionTime == 0) { 
            startconversion();               // request temp sensors to start conversion
            conversionTime=millis() + 2000;  //only update temp every 2 seconds
          }
          if (millis() > conversionTime) { 
            readTempLevels();                // read the outcome of temp sensor conversion
            conversionTime = 0;
          }                    
        }
        defineStatusAmp();                   //define status of the amp 
        writeValuesScreen();                 //in case of detail screen we update asap
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
        // only leave detail screen after timeToShowDetailScreen sec and if amp is fully operational
        if ((millis() > (timeNowplus1s + timeToShowDetailScreen)) and (opStateLeftCh) and (opStateRightCh)) {
          showDetailsScreen = false;
        }
      // or loop through overview screen measure dc offset and temp
      }
      if (aDCOn) {
        dCOffsetLeft=readVoltage(aDCLeftI2CAddress,measureDCOfset,corOffset);
        dCOffsetRight=readVoltage(aDCRightI2CAddress,measureDCOfset,corOffset);
      }
      if (tempOn) {
        if (conversionTime == 0) { 
          startconversion();
          conversionTime=millis() + 2000; //only update temp every 2 seconds
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




