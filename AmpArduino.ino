//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// v0.1, base used content of preAmpArduino
// v0.2,  optimized voltage readings as all was way to slow.
//        changed naming convention
//        split voltage readings

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// below definitions could be change by user depending on setup, no code changes needed
#define debugAmp                                    // Comment this line when debugAmp mode is not needed
const int contrastLevelScreen = 5;                  // contrastlevel of screen, value 1 - 7
const int resistorLow = 50;                         // low value resistor in voltage divider in Kohm
const int resistorHigh = 600;                       // high value resistor in voltage divider in Kohm
const int biasResistor = 1;                         // value of bias resistor in Ohm   
const float lowCurBiasFloat = 0.0;                  // low cutoff value bias in A, 1 decimals
const float highCurBiasFloat = 2.5;                 // high cutoff value bias in A, 1 decimals
const float lowDCOffsetFloat = -12.0;               // low cutoff value DC offset in V, 1 decimal
const float highDCOffsetFloat = 21.0;               // high cutoff value DC offset in V, 1 decimal
const int highTemp = 70;                            // high temp cutoff in Celcius
const bool startUpAtPower = true;                   // if true amp starts if power applied, if false it will be in standby mode
int startDelayTime = 2;                             // delay after power on of AMP, startup resistor is active, monitoring will start after this time and speakers could be connected 
const int numberOfSensorsCh = 1;                    // number of temp sensors / side. value 1 or 2
const char* toptekst = "";                          // toptext, could be changed
const char* middleTekst = "          PeWalt, V 0.2";  // Version of the code";
const char* bottemTekst = " " ;                     //as an example const char*bBottemTekst = "design by: Walter Widmer" ;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pin definitions
#define powerOnOff A0         // pin connected to the relay handeling power on/off of the amp
#define relayOutputLeft A1    // pin connected to the relay handeling left output to speaker
#define relayOutputRight A2   // pin connected to the relay handeling right output to speaker
#define startUpResistor A3    // pin connected relay handeling bypass of resister used when starting the AMP
#define spare A4              // spare
#define oneWireLeft 3         // pin connected to the sensors measuring temp of left amp
#define oneWireRight 4        // pin connected to the sensors measuring temp of rigt amp
volatile int detect230V = 5;  // pin connected to relay which monitors 230V
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
#define fontgraph2 u8g2_font_unifont_t_0_76
#define fontgroot u8g2_font_lubB19_tr                // 29w x26, car 19H
#include <U8g2lib.h>                                 // include graphical based character mode library
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C Screen(U8G2_R2);  // define the screen type used.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// definitions for the ADC controlers
//#include<ADS1015_WE.h> 
#define aDCLeftI2CAddress 0x48                          // 48 is address used by left ADC controler
#define aDCRightI2CAddress 0x49                         // 49 is address used by right ADC controler
#include <OneWire.h>
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1015 aDCLeft;                               // create left instance
Adafruit_ADS1015 aDCRight;                              // create right instance
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// definitions for the temp sensors
#include <DallasTemperature.h>
OneWire wireLeft(oneWireLeft);
OneWire wireRight(oneWireRight);
DallasTemperature tempSensorLeft(&wireLeft);
DallasTemperature tempSensorRight(&wireRight);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// general definitions
#include <Wire.h>                          // include functions for i2c
#include <ezButton.h>                      // include functions for debounce
ezButton button(buttonStandby);

const int shortPressTimeLimit = 1000;           // 1000 milliseconds
const int longPressTimeLimit  = 1000;           // 1000 milliseconds
int  lowCurBias;           // convert to int
int  highCurBias;          // convert to int
int  lowDCOffset;          // convert to int
int  highDCOffset;         // convert to int
unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;
bool isPressing = false;
bool isLongDetected = false;


struct Data {                              // definition of the data stored
  bool Volt = true;                        // measure voltage ?
  int DCOffsetLeft;                        // voltage level output channel left * 10
  int DCOffsetRight;                       // voltage level output channel left * 10
  int AmpsBiasLeft;                        // voltage level bias left channel * 100
  int AmpsBiasRight;                       // voltage level bias left channel * 100
  int TempLeft;                            // tempature of left channel
  int TempRight;                           // tempature of right channel
  bool OperationalStateLeftCh = false;     // boolean defines if channel is on or off
  bool OperationalStateRightCh = false;    // boolean defines if channel is on or off
  bool AmpInError = false;                 // Amp in error
  int ErrorCode = 0;                       // which issue caused the AMP to stop, temp, bias, dcoffset
  int ErrorValue = 0;                      // value of the parameter at time of errort
  bool Temp = true;                        // measure temperature ?
  DeviceAddress SensorTempLeft1;           // address temp sensor
  DeviceAddress SensorTempLeft2;           // address temp sensor
  DeviceAddress SensorTempRight1;          // address temp sensor
  DeviceAddress SensorTempRight2;          // address temp sensor
};
Data measure; 
const char errorMessages [7][27] = {       // error codes starting with 0
  "No error",
  "DC offset left ch (V) : ",
  "DC offset right ch (V): ",
  "bias left (A)  : ",
  "bias right (A) : ",
  "temp left (C)  : ",
  "temp right (C) : "
};
bool ampPoweredOn = false;                  // defines if amp is powered, depending on state it will have speakers on or off
int voltCorOffset = 0;                      // voltage correction
int corBias = 0; 
int showDetailsScreen = 10;                 // number of seconds detail screen is shown.
unsigned long timeNow = 0;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// definitions for the compiler
void writeValuesScreen();                                 // write info on left side screen (mute, passive, input)
void oledSchermInit();                                    // init procedure for Oledscherm
void aDCInit();                                           // init proceduree for analog digital converters 
void tempInit();                                          // init the temp sensors
void startAmp();                                          // start the amp
void shutDownAmp();                                       // stop the amp
void readVoltageLevels();                                 // read voltaga levels on the output of the speakers
void readBiasA();                                         // read the bias amperage
void readTempLevels();                                    // read temparature levels  
void defineStatusAmp();                                   // define the status of the amp according to voltage and temp levels
void bypassAllow();                                       // bypass an error
void scanI2CBus();                                        // used in debugAmp mode to scan the i2c bus
void printVariables ();                                   // used in debugAmp mode to print values
void printAddress(DeviceAddress deviceAddress);           // used in debugAmp mode to print address tempsensors
void noPower ();                                          // runs when mains fails, cuts of speakers to prevent any thunp 
int xPosition (int value);                                // defines position on the sreen for right side values
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
 // measure.dCOffsetLeft=measure.dCOffsetLeft*10;                       // voltage level output channel left * 10
 // measure.dCOffsetRight=measure.dCOffsetRight*10;
  voltCorOffset = round(((resistorLow + resistorHigh) * 10) / resistorLow);  
  corBias =  round((((resistorLow + resistorHigh) * 10) / resistorLow)/biasResistor);
  lowCurBias = round(lowCurBiasFloat * 10);              // convert to int
  highCurBias = round(highCurBiasFloat * 10);            // convert to int
  lowDCOffset = round(lowDCOffsetFloat * 10);           // convert to int
  highDCOffset = round(highDCOffsetFloat * 10);          // convert to int
  //button.setDebounceTime(50);                // set debounce time to 50 milliseconds
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // pin modes
  pinMode(powerOnOff, OUTPUT);             // control the relay that provides power to the rest of the amp
  pinMode(relayOutputLeft, OUTPUT);        // control the relay that connects the amp to the left output port
  pinMode(relayOutputRight, OUTPUT);       // control the relay that connects the amp to the right output port
  pinMode(startUpResistor, OUTPUT);        // control the relay that shortcuts the startup resistor
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
  // attach interupt 
  attachInterrupt(digitalPinToInterrupt(detect230V), noPower, FALLING);  // if pin encoderA changes run RotaryTurn
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // init screen, adc and temperature sensors
 #ifdef debugAmp
  Serial.begin(9600);                                 // if debuglevel on start monitor screen
 #endif
  Serial.print(F("voltCorOffset  " )); 
  Serial.println(voltCorOffset);  
  Serial.print(F("corBias ")); 
  Serial.println(corBias);
  Wire.begin();                                       // start i2c communication
  delay(100);
 #ifdef debugAmp
  scanI2CBus();                                       // in debugAmp mode show i2c addresses used
 #endif
  oledSchermInit();                                   // intialize the Oled screen
  aDCInit();                                          // intialize the analog to digital converters
  tempInit();                                         // initialize the temp sensors
 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
 /// startup amp
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
  timeNow = millis();
  while (millis() < (timeNow + 1000)) {
    if (ampPoweredOn) {
      if (measure.Volt) {
        readVoltageLevels();
      }
      if (measure.Temp) {
        readTempLevels();
      }
      defineStatusAmp();
    }
    button.loop(); 
    if(button.isPressed()){
      pressedTime = millis();
      isPressing = true;
      isLongDetected = false;
    }
    if(button.isReleased()) {
      isPressing = false;
      releasedTime = millis();
      long pressDuration = releasedTime - pressedTime;
      if( pressDuration < shortPressTimeLimit ) {
        showDetailsScreen=10;
      }
    }
    if((isPressing == true) && (isLongDetected == false)) {
      long pressDuration = millis() - pressedTime;
      if( pressDuration > longPressTimeLimit ) {
        isLongDetected = true;
        if (ampPoweredOn) {
          ampPoweredOn = false;
          shutDownAmp();
        }
        else {
          ampPoweredOn = true;
          startAmp();
          showDetailsScreen=10;
        }
      }
    }
  }
  if (ampPoweredOn) {
    if (measure.Volt) {
      readBiasA();
    }
    writeValuesScreen();
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// bypass procedure to start amp although their is an issue
void bypassAllow() {
 #ifdef debugAmp               // if debugAmp enabled write message
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
  Screen.clearBuffer();                       // clear the internal memory and screen
  Screen.setCursor(5, 28);                    // set cursur in correct position
  Screen.print(middleTekst);                  // write please wait
  Screen.sendBuffer();                        // transfer internal memory to the display
  delay(2000);
  Screen.clearBuffer();                       // clear the internal memory and screen
  Screen.sendBuffer();  
 #ifdef debugAmp                                 // if debugAmp enabled write message
  Serial.println(F("bypassAllow: amp starting, bypass pressed "));
 #endif
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// startup amp, arduino itself and sensors are already active.measurements
void startAmp() {
 #ifdef debugAmp               // if debugAmp enabled write message
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
  if (startDelayTime < 2) {
    startDelayTime = 2;
  }
  for (int i = startDelayTime; i > 0; i--) {  // run for startdelaytime times
    delay(1000);                              // delay for a second
  }
  digitalWrite(startUpResistor, HIGH);        // bypass startup resistor
  Screen.clearDisplay();                      // clear screen
 #ifdef debugAmp                                 // if debugAmp enabled write message
  Serial.println(F("startAmp: preamp started "));
 #endif
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// turn amp off, arduino itself and sensors stay active
void shutDownAmp() {
#ifdef debugAmp               // if debugAmp enabled write message
  Serial.println(F("shutDownAmp: shutdown amp "));
 #endif
  digitalWrite(relayOutputLeft, LOW);
  digitalWrite(relayOutputRight, LOW);
  digitalWrite(powerOnOff, LOW);             // start amp
  digitalWrite(ledStandby, HIGH);            //
  measure.OperationalStateLeftCh = false;
  measure.OperationalStateRightCh = false;
  Screen.clearBuffer();  
  Screen.sendBuffer();  
  Screen.setPowerSave(1); 
 #ifdef debugAmp               // if debugAmp enabled write message
  Serial.println(F("shutDownAmp: amp off "));
 #endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // procedure to prevent thump if power fails
void noPower () {
  shutDownAmp();
 #ifdef debugAmp               // if debugAmp enabled write message
  Serial.println(F("noPower: amp off "));
 #endif
}
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // procedure to verify status
void defineStatusAmp() {
 #ifdef debugAmp               // if debugAmp enabled write message
  Serial.println(F("defineStatusAmp: Starting "));
 #endif
  bool newStatusLeft = true;
  bool newStatusRight = true;
  if ((measure.Volt) && (!measure.AmpInError)) {
    if ((measure.DCOffsetLeft < (lowDCOffset)) | (measure.DCOffsetLeft > (highDCOffset))) {
      newStatusLeft=false;
      if (measure.OperationalStateLeftCh) {
        measure.AmpInError = true;
        measure.ErrorCode=1;
        measure.ErrorValue = measure.DCOffsetLeft;
      }
    }
    if ((measure.DCOffsetRight < (lowDCOffset)) | (measure.DCOffsetRight > (highDCOffset))) {
      newStatusRight=false;
      if (measure.OperationalStateRightCh) {
        measure.AmpInError = true;
        measure.ErrorCode=2;
        measure.ErrorValue = measure.DCOffsetRight;
      }
    }
    if ((measure.AmpsBiasLeft < lowCurBias) | (measure.AmpsBiasLeft > highCurBias)) {
      newStatusLeft=false;
      if (measure.OperationalStateLeftCh) {
        measure.AmpInError = true;
        measure.ErrorCode=3;
        measure.ErrorValue = measure.AmpsBiasLeft;
      }
    } 
    if ((measure.AmpsBiasRight < lowCurBias) | (measure.AmpsBiasRight > highCurBias)) {
      newStatusRight=false;
      if (measure.OperationalStateRightCh) {
        measure.AmpInError = true;
        measure.ErrorCode=4;
        measure.ErrorValue = measure.AmpsBiasRight;
      }
    }
  }
  if ((measure.Temp) && (!measure.AmpInError)) {
    if (measure.TempLeft > highTemp) {
      newStatusLeft=false;
      if (measure.OperationalStateLeftCh) {
        measure.AmpInError = true;
        measure.ErrorCode=5;
        measure.ErrorValue = measure.TempLeft;
      }
    }
    if (measure.TempRight > highTemp) {
      newStatusRight=false;
      if (measure.OperationalStateRightCh) {
        measure.AmpInError = true;
        measure.ErrorCode=6;
        measure.ErrorValue = measure.TempRight;
      }
    }
  }

  if (measure.AmpInError) {
 #ifdef debugAmp               // if debugAmp enabled write message
    Serial.println(F("defineStatusAmp: shutdown amp due to error "));
 #endif    
    digitalWrite(relayOutputLeft, LOW);
    digitalWrite(relayOutputRight, LOW);
    digitalWrite(powerOnOff, LOW);             // start amp
    digitalWrite(ledStandby, HIGH);            //
  }
  if ((!measure.OperationalStateLeftCh) && (newStatusLeft) && (!measure.AmpInError)) { // turn channel on if all conditions met
    measure.OperationalStateLeftCh=true;
  }
  if ((!measure.OperationalStateRightCh) && (newStatusRight) && (!measure.AmpInError)) {
    measure.OperationalStateRightCh=true;
  }
  if ((measure.OperationalStateRightCh==true) and (measure.OperationalStateLeftCh==true)) {
    digitalWrite(relayOutputLeft, HIGH);
    digitalWrite(relayOutputRight, HIGH);
  } 
  else {
    showDetailsScreen = 10;
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
void writeValuesScreen() {
  if (showDetailsScreen > 0) {
    showDetailsScreen = showDetailsScreen -1;
  }
  Screen.clearBuffer();                                                // clear buffer
  Screen.setFont(fontH08);                                            // set font
  if (measure.AmpInError) {
    Screen.setCursor(0, 14);
    Screen.print(F("ERROR "));
    Screen.print(errorMessages[measure.ErrorCode]); 
    Screen.print(measure.ErrorValue);
  }
  if ((!measure.AmpInError) and (showDetailsScreen > 0)){
    Screen.setFont(fontH10);  
    Screen.setCursor(42, 14);
    Screen.print(F("STATE"));
    Screen.setFont(fontgrahp);    
    Screen.setCursor(0, 14);
    if (measure.OperationalStateLeftCh) {Screen.drawGlyph(5, 16 , 0x2714);}
    else {Screen.drawGlyph(5, 16 , 0x2715);}
    if (measure.OperationalStateRightCh) {
      Screen.drawGlyph(112, 16 , 0x2714);}
    else {Screen.drawGlyph(108, 16 , 0x2715);}
    Screen.setFont(fontH08);
    if (measure.Volt) {
      Screen.setCursor(38, 40);
      Screen.print(F("DC offset (V)"));
      Screen.setCursor(50, 51); 
      Screen.print(F("Bias (A)"));
      Screen.setFont(fontH08fixed);
      Screen.setCursor(0, 40);
      Screen.print(measure.DCOffsetLeft / 1000.0,1);
      Screen.setCursor(XPosition(measure.DCOffsetRight,1),40);
      Screen.print(measure.DCOffsetRight / 1000.0,1);
      Screen.setCursor(0, 51); 
      Screen.print(measure.AmpsBiasLeft/100,2);
      Screen.setCursor(XPosition(measure.AmpsBiasRight,2), 51); 
      Screen.print(measure.AmpsBiasRight/100,2);
    }
    if (measure.Temp) {
      Screen.setCursor(45, 62);  
      Screen.print(F("Temp (C)")); 
      Screen.setFont(fontH08fixed);  
      if (measure.TempLeft == -200) {
        Screen.setCursor(0, 63);
        Screen.print(F("err"));
      }
      else {
        Screen.setCursor(0, 63);
        Screen.print(measure.TempLeft);
      }     
      if (measure.TempRight == -200) {
        Screen.setCursor(105, 63);
        Screen.print(F("err"));
      }
      else {
        Screen.setCursor(XPosition(measure.TempRight,0), 63);
        Screen.print(measure.TempRight);
      }   
    }
  }
  if ((!measure.AmpInError) && (showDetailsScreen == 0)){
    Screen.setFont(fontgroot);
    Screen.setCursor(10, 35);
    Screen.print(F("ALEPH J"));
    if (measure.Temp) {
      Screen.setFont(fontH08);
      Screen.setCursor(45, 62);  
      Screen.print(F("Temp (C)")); 
      Screen.setFont(fontH08fixed);  
      if (measure.TempLeft == -200) {
        Screen.setCursor(0, 63);
        Screen.print(F("err"));
      }
      else {
        Screen.setCursor(0, 63);
        Screen.print(measure.TempLeft);
      }     
      if (measure.TempRight == -200) {
        Screen.setCursor(105, 63);
        Screen.print(F("err"));
      }
      else {
        Screen.setCursor(XPosition(measure.TempRight,0), 63);
        Screen.print(measure.TempRight);
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
void oledSchermInit() {
  digitalWrite(oledReset, LOW);                                        // set screen in reset mode
  delay(15);                                                           // wait to stabilize
  digitalWrite(oledReset, HIGH);                                       // set screen active
  delay(15);                                                           // wait to stabilize
  Screen.setI2CAddress(oledAddress * 2);                              // set oled I2C address
  Screen.begin();                                                      // init the screen
  Screen.setContrast((((contrastLevelScreen * 2) + 1) << 4) | 0x0f);   // set contrast level, reduce number of options
  Screen.setPowerSave(0); 
 #ifdef Debug
  Serial.println(F("oledSchermInit: end of procedure"));
 #endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// intialisation of Analoog Digital converter left and right channel
void aDCInit() {
  if (!aDCLeft.begin(aDCLeftI2CAddress)) {
    Screen.setFont(fontH10);
    Screen.setCursor(0, 10);
    Screen.print(F("No ADC left side"));
    Screen.sendBuffer();
    bypassAllow();
    measure.Volt = false;
  }
  aDCLeft.setGain(GAIN_TWO);
  if (!aDCRight.begin(aDCRightI2CAddress)) {  
    Screen.setFont(fontH10);
    Screen.setCursor(0, 10);
    Screen.print(F("No ADC left side"));
    Screen.sendBuffer();
    bypassAllow();
    measure.Volt = false;
  }
  aDCRight.setGain(GAIN_TWO);
 #ifdef debugAmp
  Serial.println(F("aDCInit: init of ADC's done"));
 #endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// intialisation of temparature sensors
void tempInit() {
  int numberOfSensorsLeft;
  int numberOfSensorsRight;
 #ifdef debugAmp
  Serial.print(F("tempInit: number of sensors / channel defined : "));
  Serial.println(numberOfSensorsCh);
 #endif
  tempSensorLeft.begin();                             // intialize the tempsensors left channel
  numberOfSensorsLeft = tempSensorLeft.getDeviceCount();
  if (numberOfSensorsCh != numberOfSensorsLeft ) {
    Screen.setFont(fontH10);
    Screen.setCursor(15, 10);
    Screen.print(F("No temp left side"));
    Screen.sendBuffer();
    bypassAllow();
    measure.Temp = false;
  }
  tempSensorRight.begin();                             // intialize the tempsensors right channel
  numberOfSensorsRight = tempSensorRight.getDeviceCount();
  if (numberOfSensorsCh != numberOfSensorsRight ) {
    Screen.setFont(fontH10);
    Screen.setCursor(15, 10);
    Screen.print(F("No temp right side"));
    Screen.sendBuffer();
    bypassAllow();
    measure.Temp = false;
  }
  if (measure.Temp) {
    tempSensorLeft.getAddress(measure.SensorTempLeft1,0);
    tempSensorRight.getAddress(measure.SensorTempRight1,0);
    if (numberOfSensorsCh == 2) {
      tempSensorLeft.getAddress(measure.SensorTempLeft2,1);
      tempSensorRight.getAddress(measure.SensorTempRight2,1);
    }
    tempSensorLeft.setResolution(9);
    tempSensorRight.setResolution(9);
  }
 #ifdef debugAmp
  Serial.print(F("tempInit: Temperature measurement 1 = oke, 0 = not oke : "));
  Serial.println(measure.Temp);
  Serial.print(F("tempInit: number of sensors found left side            : "));
  Serial.println(numberOfSensorsLeft);
  if (numberOfSensorsLeft > 0) {
    Serial.print(F("TempInit: sensor address nr 1 : "));
    printAddress(measure.SensorTempLeft1);
    Serial.println();
  }
  if (numberOfSensorsLeft == 2) {
    Serial.print(F("tempInit: sensor address nr 2 : "));
    printAddress(measure.SensorTempLeft2);
    Serial.println(); 
  }
    Serial.print(F("tempInit: number of sensors found right side : "));
    Serial.println(numberOfSensorsRight);
    if (numberOfSensorsLeft > 0) {
    Serial.print(F("tempInit: sensor address nr 1 : "));
    printAddress(measure.SensorTempRight1);
    Serial.println(); 
  }
  if (numberOfSensorsLeft == 2) {
    Serial.print(F("tempInit: sensor address nr 2 : "));
    printAddress(measure.SensorTempRight2); 
    Serial.println(); 
  }
 #endif  
}

void readVoltageLevels() {
  measure.DCOffsetLeft=(aDCLeft.readADC_Differential_0_1()) * voltCorOffset;
  measure.DCOffsetRight=(aDCRight.readADC_Differential_0_1()) * voltCorOffset;
 #ifdef debugAmp               // if debugAmp enabled write message
  Serial.println(F("readVoltageLevels: following Voltage levels measured :"));
  Serial.print(F("DC offset left        (V) x 10 : "));
  Serial.println(measure.DCOffsetLeft);
  Serial.print(F("DC offset right       (V) x 10 : "));
  Serial.println(measure.DCOffsetRight);
 #endif
}

void readBiasA() {
  measure.AmpsBiasLeft=(aDCLeft.readADC_Differential_2_3()) * corBias;
  measure.AmpsBiasRight=(aDCRight.readADC_Differential_2_3()) * corBias;
 #ifdef debugAmp               // if debugAmp enabled write message
  Serial.println(F("ReadVoltageLevels: following amperage levels measured :"));
  Serial.println(measure.DCOffsetRight);
  Serial.print(F("Bias left channel    (A) x 1000 : "));
  Serial.println(measure.AmpsBiasLeft);
  Serial.print(F("Bias right channel   (A) x 1000 : "));
  Serial.println(measure.AmpsBiasRight);
 #endif
}
////////////////////////////////////////////////////////////////////////////////////////////////
// read the temparature levels
void readTempLevels() {
  int temp1;
  int temp2 = 0; 
  tempSensorLeft.requestTemperatures();
  temp1 = round(tempSensorLeft.getTempC(measure.SensorTempLeft1));
  if (temp1 == DEVICE_DISCONNECTED_C) {
    temp1 = -200;
  }
  if (numberOfSensorsCh == 2) {
    temp2 = round(tempSensorLeft.getTempC(measure.SensorTempLeft2));
    if (temp2 == DEVICE_DISCONNECTED_C) {
      temp2 = -200;
    }
  }
  if ((temp1 == -200) | (temp2 == -200)) {
    measure.TempLeft = -200;
  }
  else {
    if (temp1 > temp2) {measure.TempLeft = temp1;}
    else {measure.TempLeft = temp2;} 
  }
 #ifdef Debug               // if debugAmp enabled write message
    Serial.println(F("readTempLevel: following Temparature levels measured :"));
    Serial.print(F("Temperature level left nr1   (C): "));
    Serial.println(temp1);  
    Serial.print(F("Temperature level left nr2   (C): "));
    Serial.println(temp2); 
 #endif
   tempSensorRight.requestTemperatures();
  temp1 = round(tempSensorRight.getTempC(measure.SensorTempRight1));
  if (temp1 == DEVICE_DISCONNECTED_C) {
    temp1 = -200;
  }
  if (numberOfSensorsCh == 2) {
    temp2 = round(tempSensorRight.getTempC(measure.SensorTempRight2));
    if (temp2 == DEVICE_DISCONNECTED_C) {
      temp2 = -200;
    }
  }
  if ((temp1 == -200) | (temp2 == -200)) {
    measure.TempRight = -200;
  }
  else {
    if (temp1 > temp2) {measure.TempRight = temp1;}
    else {measure.TempRight = temp2;} 
  }
 #ifdef debugAmp 
    Serial.print(F("Temperature level right nr1  (C): "));
    Serial.println(temp1);  
    Serial.print(F("Temperature level right nr2  (C): "));
    Serial.println(temp2); 
 #endif       
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function to check i2c bus
  #ifdef debugAmp
void scanI2CBus() {
  uint8_t error;                                                      // error code
  uint8_t address;                                                    // address to be tested
  int numberOfDevices;                                                // number of devices found
  Serial.println(F("scanI2CBus: I2C addresses defined within the code are : ")); // print content of code
  Serial.print(F("Screen             : 0x"));
  Serial.print(oledAddress, HEX);
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
void printVariables () {
  Serial.println(F("PrintVariables :"));
  Serial.print(F("Are ADC both reachable          : "));
  Serial.println(measure.Volt);
  Serial.print(F("DC offset left                  : "));
  Serial.println(measure.DCOffsetLeft);
  Serial.print(F("DC offset right                 : "));
  Serial.println(measure.DCOffsetRight);
  Serial.print(F("Bias left                       : "));
  Serial.println(measure.AmpsBiasLeft);
  Serial.print(F("Bias right                      : "));
  Serial.println(measure.AmpsBiasRight);  
  Serial.print(F("Temperature left                : "));
  Serial.println(measure.TempLeft);
  Serial.print(F("Temperature right               : "));
  Serial.println(measure.TempRight);
  Serial.print(F("Operational state left          : "));
  Serial.println(measure.OperationalStateLeftCh);
  Serial.print(F("Operational state right         : "));
  Serial.println(measure.OperationalStateRightCh);
  Serial.print(F("AMP has an error                : "));
  Serial.println(measure.AmpInError);
  Serial.print(F("Errorcode                       : "));
  Serial.println(measure.ErrorCode);
  Serial.print(F("Value at error                  : "));
  Serial.println(measure.ErrorValue);
  Serial.print(F("temp sensors reachable          : "));
  Serial.println(measure.Temp);
  Serial.print(F("address temp sensors left nr 1  : "));
  printAddress(measure.SensorTempLeft1);
  Serial.print(F("address temp sensors left nr 2  : "));
  printAddress(measure.SensorTempLeft2);
  Serial.print(F("address temp sensors right nr 1 : "));
  printAddress(measure.SensorTempRight1);
  Serial.print(F("address temp sensors right nr 2 : "));
  printAddress(measure.SensorTempRight2); 
  Serial.println();
} 
 #endif

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
  Serial.println();
}

int XPosition (int value, int dec) {
  if (value == 0) {
    return(122);
  }
  if (value > 0) {
    if (value < 10) {
      if (dec == 0) {return(122);}
      if (dec == 1) {return(112);}
      if (dec == 2) {return(108);}
    }
    if (value < 100) {
      if (dec == 0) {return(117);}
      if (dec == 1) {return(107);}
      if (dec == 2) {return(102);}
    }
    return(100);
  }
  else {
    if (value > -10) {
      if (dec == 0) {return(117);}
      if (dec == 1) {return(107);}
      if (dec == 2) {return(103);}
    }
    if (value > -100) {
      if (dec == 0) {return(112);}
      if (dec == 1) {return(102);}
      if (dec == 2) {return(97);}
    }
    return(95);  
  }
}
