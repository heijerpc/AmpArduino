//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// v0.1, base used content of preAmpArduino

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// below definitions could be change by user depending on setup, no code changes needed
#define DEBUG                            // Comment this line when DEBUG mode is not needed
const int ContrastLevelScreen = 5;                  // contrastlevel of screen, value 1 - 7
const int ResistorLow = 50;                         // low value resistor in voltage divider in Kohm
const int ResistorHigh = 600;                       // high value resistor in voltage divider in Kohm
const float BiasResistor = 1;                         // value of bias resistor in Ohm   
const float LowCurBias = 1.00;                      // low cutoff value bias in A, 2 decimals
const float HighCurBias = 1.50;                     // high cutoff value bias in A, 2 decimals
const float LowDcOffset = -2.0;                     // low cutoff value DC offset in V
const float HighDcOffset = 2.0;                     // high cutoff value DC offset n V
const int HighTemp = 70;                            // high temp cutoff in C
const bool StartUpAtPower = true;                   // if true amp starts if power applied, if false it will be in standby mode
int StartDelayTime = 5;                             // delay after power on of AMP before monitoring will start and speakers could be connected 
const int NumberOfSensorsCh = 1;                    // number of temp sensors / side. value 1 or 2
const char* Toptekst = "PeWalt, V 0.1";             // current version of the code, shown in startscreen top, content could be changed
const char* MiddleTekst = "          please wait";  //as an example const char* MiddleTekst = "Cristian, please wait";
const char* BottemTekst = " " ;                     //as an example const char* BottemTekst = "design by: Walter Widmer" ;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pin definitions
#define PowerOnOff A0         // pin connected to the relay handeling power on/off of the amp
#define RelayOutputLeft A1    // pin connected to the relay handeling left output 
#define RelayOutputRight A2   // pin connected to the relay handeling right output
#define StartResistor A3      // pin connected relay handeling bypass of resiister used when starting the AMP
#define Spare A4              // spare
#define OneWireLeft 3         // pin connected to the sensors measuring temp of left amp
#define OneWireRight 4        // pin connected to the sensors measuring temp of rigt amp
volatile int Detect230V = 5;  // pin connected to relay which monitors 230V
#define ButtonStandby 7       // pin connected to button to switch between on and standby
#define LedStandby 11         // connected to a led that is on if amp is in standby mode
#define OledReset 12          // connected to the reset port of Oled screen, used to reset Oled screen
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// definitions for the oled screen
#define OLED_Address 0x3C                            // 3C is address used by oled controler
#define fontH08 u8g2_font_timB08_tr                  // 11w x 11h, char 7h
#define fontH08fixed u8g2_font_spleen5x8_mr          // 15w x 14h, char 10h
#define fontH10 u8g2_font_timB10_tr                  // 15w x 14h, char 10h
#define fontH10figure u8g2_font_spleen8x16_mn        //  8w x 13h, char 12h
#define fontH14 u8g2_font_timB14_tr                  // 21w x 18h, char 13h
#define fontgrahp u8g2_font_open_iconic_play_2x_t    // 16w x 16h pixels
#define fontH21cijfer u8g2_font_timB24_tn            // 17w x 31h, char 23h
#include <U8g2lib.h>                                 // include graphical based character mode library
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C Screen(U8G2_R2);  // define the screen type used.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// definitions for the ADC controlers
//#include<ADS1015_WE.h> 
#define ADCLeftI2CAddress 0x48                          // 48 is address used by left ADC controler
#define ADCRightI2CAddress 0x49                         // 49 is address used by right ADC controler
#include <OneWire.h>
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1015 ADCLeft;                               // create left instance
Adafruit_ADS1015 ADCRight;                              // create right instance
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// definitions for the temp sensors
#include <DallasTemperature.h>
OneWire WireLeft(OneWireLeft);
OneWire WireRight(OneWireRight);
DallasTemperature TempSensorLeft(&WireLeft);
DallasTemperature TempSensorRight(&WireRight);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// general definitions
#include <Wire.h>                          // include functions for i2c
#include <ezButton.h>                      // include functions for debounce
ezButton button(ButtonStandby);   
struct Data {                              // definition of the data stored
  bool MeasureVolt = true;                 // measure voltage ?
  float DCoffsetLeft;                        // voltage level output channel left
  float DCoffsetRight;                       // voltage level output channel left
  float AmpsBiasLeft;                     // voltage level bias left channel
  float AmpsBiasRight;                    // voltage level bias left channel
  int TempLeft;                            // tempature of left channel
  int TempRight;                           // tempature of right channel
  bool OperationalStateLeftCh = false;     // boolean defines if channel is on or off
  bool OperationalStateRightCh = false;    // boolean defines if channel is on or off
  bool AmpInError = false;                 // Amp in error
  int ErrorCode = 0;                       // which issue caused the AMP to stop, temp, bias, dcoffset
  int ErrorValue = 0;                      // value of the parameter at time of errort
  bool MeasureTemp = true;                 // measure temperature ?
  DeviceAddress SensorTempLeft1;           // address temp sensor
  DeviceAddress SensorTempLeft2;           // address temp sensor
  DeviceAddress SensorTempRight1;          // address temp sensor
  DeviceAddress SensorTempRight2;          // address temp sensor
};
Data Measurements; 
const char ErrorMessages [7][27] = {       // error codes starting with 0
  "No error",
  "DC offset left ch (V) : ",
  "DC offset right ch (V): ",
  "bias left (A)  : ",
  "bias right (A) : ",
  "temp left (C)  : ",
  "temp right (C) : "
};
bool AmpPoweredOn = false;                  // defines if amp is powered on or off
float VoltCor = 0.0;                        // voltage correction 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// definitions for the compiler
void WriteValuesScreen();                                 // write info on left side screen (mute, passive, input)
void OledSchermInit();                                    // init procedure for Oledscherm
void ADCInit();                                           // init proceduree for analog digital converters 
void TempInit();                                          // init the temp sensors
void StartAmp();                                          // start the amp
void ShutdownAmp();                                       // stop the amp
void ReadVoltageLevels();                                 // read voltaga levels
void ReadTempLevels();                                    // read temparature levels  
void DefineStatusAmp();                                   // define the status of the amp according to voltage and temp levels
void BypassAllow();                                       // bypass an error
void ScanI2CBus();                                        // used in debug mode to scan the i2c bus
void PrintVariables ();                                   // used in debug mode to print values
void printAddress(DeviceAddress deviceAddress);           // used in debug mode to print address tempsensors
void NoPower ();                                          // runs when mains fails, cuts of speakers to prevent any thunp 
int XPosition (int value);                                // defines position on the sreen for right side values
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  VoltCor = ((ResistorLow + ResistorHigh) / ResistorLow)/100.0;
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // pin modes
  pinMode(PowerOnOff, OUTPUT);             // control the relay that provides power to the rest of the amp
  pinMode(RelayOutputLeft, OUTPUT);        // control the relay that connects the amp to the left output port
  pinMode(RelayOutputRight, OUTPUT);       // control the relay that connects the amp to the right output port
  pinMode(StartResistor, OUTPUT);          // control the relay that shortcuts the startup resistor
  pinMode(OledReset, OUTPUT);              // set reset of oled screen
  pinMode(Detect230V, INPUT_PULLUP);       // pin Detect230V is high and its an input port

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // write init state to output pins
  digitalWrite(PowerOnOff, LOW);           // keep amp turned off
  digitalWrite(RelayOutputLeft, LOW);      // output port left is disconnected
  digitalWrite(RelayOutputRight, LOW);     // output port right is disconnected
  digitalWrite(StartResistor, LOW);        // startup resistor is not shortcutted
  digitalWrite(LedStandby, LOW);           // turn off standby led to indicate device is becoming active
  digitalWrite(OledReset, LOW);            // keep the Oled screen in reset mode
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // attach interupt 
  attachInterrupt(digitalPinToInterrupt(Detect230V), NoPower, FALLING);  // if pin encoderA changes run RotaryTurn
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // init screen, adc and temperature sensors
 #ifdef DEBUG
  Serial.begin(9600);                                 // if debuglevel on start monitor screen
 #endif
  Wire.begin();                                       // start i2c communication
  delay(100);
 #ifdef DEBUG
  ScanI2CBus();                                       // in debug mode show i2c addresses used
 #endif
  OledSchermInit();                                   // intialize the Oled screen
  ADCInit();                                          // intialize the analog to digital converters
  TempInit();                                         // initialize the temp sensors
 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
 /// startup amp
  if (StartUpAtPower) {
    StartAmp();
    AmpPoweredOn = true;
  }
  else {
    ShutdownAmp();
  }
 #ifdef DEBUG
  Serial.println(F("Setup: end of setup proc"));
 #endif
}
//////////////////////////////////////////////////////////////////////////////////////////////
// Main loop
//////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  if (AmpPoweredOn) {
    if (Measurements.MeasureVolt) {
      ReadVoltageLevels();
    }
    if (Measurements.MeasureTemp) {
      ReadTempLevels();
    }
    DefineStatusAmp();
    WriteValuesScreen();
    delay(1000);
  }
  button.loop();
  if (button.isPressed()) {
    if (AmpPoweredOn) {
      AmpPoweredOn = false;
      ShutdownAmp();
    }
    else {
      AmpPoweredOn = true;
      StartAmp();
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// bypass procedure to start amp although their is an issue
void BypassAllow() {
 #ifdef DEBUG               // if debug enabled write message
  Serial.println(F("BypassAllow: waiting "));
 #endif
  bool WaitingForPress = true;
  Screen.setFont(fontH08);                                            // set font
  Screen.setCursor(18, 32);
  Screen.print(F("press standby button"));
  Screen.setCursor(6, 44);
  Screen.print(F("protection will be disabled"));
  Screen.sendBuffer();
  while (WaitingForPress) {
    button.loop();
    if (button.isPressed()) {
      WaitingForPress = false;
    }  
  }
 #ifdef DEBUG               // if debug enabled write message
  Serial.println(F("BypassAllow: amp starting, bypass pressed "));
 #endif
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// startup amp, arduino itself and sensors are already active.Measurements
void StartAmp() {
 #ifdef DEBUG               // if debug enabled write message
  Serial.println(F("StartAmp: starting amp and waiting to stabilize "));
 #endif
  digitalWrite(PowerOnOff, HIGH);             // start amp
  digitalWrite(LedStandby, LOW);              //
  Screen.setPowerSave(0); 
  Screen.clearBuffer();                       // clear the internal memory and screen
  Screen.setFont(fontH08);                    // choose a suitable font
  Screen.setCursor(0, 8);                     // set cursur in correct position
  Screen.print(Toptekst);                     // write tekst to buffer
  Screen.setCursor(13, 63);                   // set cursur in correct position
  Screen.print(BottemTekst);                  // write tekst to buffer
  Screen.setFont(fontH10);                    // choose a suitable font
  Screen.setCursor(5, 28);                    // set cursur in correct position
  Screen.print(MiddleTekst);                  // write please wait
  if (StartDelayTime < 3) {
    StartDelayTime = 3;
  }
  for (int i = StartDelayTime; i > 0; i--) {  // run for startdelaytime times
    Screen.setDrawColor(0);                              // clean channel name  part in buffer
    Screen.drawBox(65, 31, 30, 14);
    Screen.setDrawColor(1);
    Screen.setCursor(65, 45);               // set cursur in correct position
    Screen.print(i);                          // print char array
    Screen.sendBuffer();                      // transfer internal memory to the display
    delay(1000);                              // delay for a second
  }
  digitalWrite(StartResistor, HIGH);          // bypass startup resistor
  Screen.clearDisplay();                      // clear screen
 #ifdef DEBUG                                 // if debug enabled write message
  Serial.println(F("StartAmp: preamp started "));
 #endif
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// turn amp off, arduino itself and sensors stay active
void ShutdownAmp() {
#ifdef DEBUG               // if debug enabled write message
  Serial.println(F("ShutdownAmp: shutdown amp "));
 #endif
  digitalWrite(PowerOnOff, LOW);             // start amp
  digitalWrite(LedStandby, HIGH);            //
  digitalWrite(RelayOutputLeft, LOW);
  digitalWrite(RelayOutputRight, LOW);
  Measurements.OperationalStateLeftCh = false;
  Measurements.OperationalStateRightCh = false;
  Screen.clearBuffer();  
  Screen.sendBuffer();  
  Screen.setPowerSave(1); 
 #ifdef DEBUG               // if debug enabled write message
  Serial.println(F("ShutdownAmp: amp off "));
 #endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // procedure to prevent thump if power fails
void NoPower () {
  digitalWrite(RelayOutputLeft, LOW);
  digitalWrite(RelayOutputRight, LOW);
  Measurements.AmpInError = true;
 #ifdef DEBUG               // if debug enabled write message
  Serial.println(F("NoPower: amp off "));
 #endif
}
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // procedure to verify status
void DefineStatusAmp() {
 #ifdef DEBUG               // if debug enabled write message
  Serial.println(F("DefineStatusAmp: Starting "));
 #endif
  bool NewStatusLeft = true;
  bool NewStatusRight = true;
  if (Measurements.MeasureVolt) {
    if ((Measurements.DCoffsetLeft < (LowDcOffset))|(Measurements.DCoffsetLeft > (HighDcOffset))) {
      NewStatusLeft=false;
      if (Measurements.OperationalStateLeftCh) {
        Measurements.AmpInError = true;
        Measurements.ErrorCode=1;
        Measurements.ErrorValue = Measurements.DCoffsetLeft;
      }
    }
    if ((Measurements.DCoffsetRight < (LowDcOffset))|(Measurements.DCoffsetRight > (HighDcOffset))) {
      NewStatusRight=false;
      if (Measurements.OperationalStateRightCh) {
        Measurements.AmpInError = true;
        Measurements.ErrorCode=2;
        Measurements.ErrorValue = Measurements.DCoffsetRight;
      }
    }
    if ((Measurements.AmpsBiasLeft<LowCurBias) | (Measurements.AmpsBiasLeft>HighCurBias)){
      NewStatusLeft=false;
      if (Measurements.OperationalStateLeftCh) {
        Measurements.AmpInError = true;
        Measurements.ErrorCode=3;
        Measurements.ErrorValue = Measurements.AmpsBiasLeft;
      }
    } 
    if ((Measurements.AmpsBiasRight<LowCurBias)|(Measurements.AmpsBiasRight>HighCurBias)) {
      NewStatusRight=false;
      if (Measurements.OperationalStateRightCh) {
        Measurements.AmpInError = true;
        Measurements.ErrorCode=4;
        Measurements.ErrorValue = Measurements.AmpsBiasRight;
      }
    }
  }
  if (Measurements.MeasureTemp) {
    if (Measurements.TempLeft>HighTemp) {
      NewStatusLeft=false;
      if (Measurements.OperationalStateLeftCh) {
        Measurements.AmpInError = true;
        Measurements.ErrorCode=5;
        Measurements.ErrorValue = Measurements.TempLeft;
      }
    }
    if (Measurements.TempRight>HighTemp) {
      NewStatusRight=false;
      if (Measurements.OperationalStateRightCh) {
        Measurements.AmpInError = true;
        Measurements.ErrorCode=6;
        Measurements.ErrorValue = Measurements.TempRight;
      }
    }
  }

  if (Measurements.AmpInError) {
 #ifdef DEBUG               // if debug enabled write message
   Serial.println(F("DefineStatusAmp: shutdown amp due to error "));
 #endif    
    digitalWrite(RelayOutputLeft, LOW);
    digitalWrite(RelayOutputRight, LOW);
    digitalWrite(PowerOnOff, LOW);             // start amp
    digitalWrite(LedStandby, HIGH);            //
  }
  if ((!Measurements.OperationalStateLeftCh) && (NewStatusLeft) && (!Measurements.AmpInError)) { // turn channel on if all conditions met
    digitalWrite(RelayOutputLeft, HIGH);
    Measurements.OperationalStateLeftCh=true;
  }
  if ((!Measurements.OperationalStateRightCh) && (NewStatusRight) && (!Measurements.AmpInError)) {
    digitalWrite(RelayOutputRight, HIGH);
    Measurements.OperationalStateRightCh=true;
  }
 #ifdef DEBUG               // if debug enabled write message
    Serial.print(F("DefineStatusAmp: newstatus left  : "));
    Serial.println(NewStatusLeft);
    Serial.print(F("DefineStatusAmp: newstatus right : "));
    Serial.println(NewStatusRight);
    PrintVariables();
 #endif  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write values on the screen (state, voltage levels and temperature)
void WriteValuesScreen() {
  Screen.clearBuffer();                                                // clear buffer
  Screen.setFont(fontH08);                                            // set font
  if (Measurements.AmpInError) {
    Screen.setCursor(0, 14);
    Screen.print(F("ERROR "));
    Screen.print(ErrorMessages[Measurements.ErrorCode]); 
    Screen.print(Measurements.ErrorValue);
  }
  else{
  Screen.setCursor(45, 14);
  Screen.print(F("STATE"));
  Screen.setCursor(0, 14);
  if (Measurements.OperationalStateLeftCh) {Screen.print(F("On"));}
    else {Screen.print(F("OFF"));}
    Screen.setCursor(100, 14);
    if (Measurements.OperationalStateRightCh) {
      Screen.setCursor(108, 14);
      Screen.print(F("On"));}
    else {Screen.print(F("OFF"));}
  }
  Screen.setFont(fontH08);;
  if (Measurements.MeasureVolt) {
    Screen.setCursor(0, 40);
    Screen.print(Measurements.DCoffsetLeft,1);
    Screen.setCursor(38, 40);
    Screen.print(F("DC offset (V)"));
    Screen.setCursor((XPosition(Measurements.DCoffsetRight)-5),40);
    Screen.print(Measurements.DCoffsetRight,1);
    Screen.setCursor(0, 51); 
    Screen.print(Measurements.AmpsBiasLeft,2);
    Screen.setCursor(50, 51); 
    Screen.print(F("Bias (A)"));
    Screen.setCursor(XPosition(Measurements.AmpsBiasRight), 51); 
    Screen.print(Measurements.AmpsBiasRight,2);
  }
  if (Measurements.MeasureTemp) {
    if (Measurements.TempLeft == -200) {
      Screen.setCursor(0, 63);
      Screen.print(F("err"));
    }
    else {
      Screen.setCursor(0, 63);
      Screen.print(Measurements.TempLeft);
    }     
    Screen.setCursor(50, 62);  
    Screen.print(F("Temp (C)")); 

    Screen.setCursor(113, 63);
    if (Measurements.TempRight == -200) {
      Screen.setCursor(105, 63);
      Screen.print(F("err"));
    }
    else {
      Screen.setCursor(XPosition(Measurements.TempRight), 63);
      Screen.print(Measurements.TempRight);
    }   
  }  
  Screen.sendBuffer();
 #ifdef DEBUG               // if debug enabled write message
   Serial.println(F("WriteValuesScreen: screen updated "));
 #endif  


}
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// intialisation of the screen after powerup of screen.
void OledSchermInit() {
  digitalWrite(OledReset, LOW);                                        // set screen in reset mode
  delay(15);                                                           // wait to stabilize
  digitalWrite(OledReset, HIGH);                                       // set screen active
  delay(15);                                                           // wait to stabilize
  Screen.setI2CAddress(OLED_Address * 2);                              // set oled I2C address
  Screen.begin();                                                      // init the screen
  Screen.setContrast((((ContrastLevelScreen * 2) + 1) << 4) | 0x0f);   // set contrast level, reduce number of options
  Screen.setPowerSave(0); 
 #ifdef DEBUG
  Serial.println(F("OledSchermInit: end of procedure"));
 #endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// intialisation of Analoog Digital converter left and right channel
void ADCInit() {
  if (!ADCLeft.begin(ADCLeftI2CAddress)) {
    Screen.setFont(fontH10);
    Screen.setCursor(0, 10);
    Screen.print(F("No ADC left side"));
    Screen.sendBuffer();
    BypassAllow();
    Measurements.MeasureVolt = false;
  }
  ADCLeft.setGain(GAIN_TWO);
  if (!ADCRight.begin(ADCRightI2CAddress)) {
    Screen.setFont(fontH10);
    Screen.setCursor(0, 10);
    Screen.print(F("No ADC left side"));
    Screen.sendBuffer();
    BypassAllow();
    Measurements.MeasureVolt = false;
  }
  ADCRight.setGain(GAIN_TWO);
 #ifdef DEBUG
  Serial.println(F("ADCInit: init of ADC's done"));
 #endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// intialisation of temparature sensors
void TempInit() {
  int NumberOfSensorsLeft;
  int NumberOfSensorsRight;
 #ifdef DEBUG
  Serial.print(F("TempInit: number of sensors / channel defined : "));
  Serial.println(NumberOfSensorsCh);
 #endif
  TempSensorLeft.begin();                             // intialize the tempsensors left channel
  NumberOfSensorsLeft = TempSensorLeft.getDeviceCount();
  if (NumberOfSensorsCh != NumberOfSensorsLeft ) {
    Screen.setFont(fontH10);
    Screen.setCursor(0, 10);
    Screen.print(F("No temp left side"));
    Screen.sendBuffer();
    BypassAllow();
    Measurements.MeasureTemp = false;
  }
  TempSensorRight.begin();                             // intialize the tempsensors right channel
  NumberOfSensorsRight = TempSensorRight.getDeviceCount();
  if (NumberOfSensorsCh != NumberOfSensorsRight ) {
    Screen.setFont(fontH10);
    Screen.setCursor(0, 10);
    Screen.print(F("No temp right side"));
    Screen.sendBuffer();
    BypassAllow();
    Measurements.MeasureTemp = false;
  }
  if (Measurements.MeasureTemp) {
    if (NumberOfSensorsCh > 0) {
      TempSensorLeft.getAddress(Measurements.SensorTempLeft1,0);
      TempSensorRight.getAddress(Measurements.SensorTempRight1,0);
    }
    if (NumberOfSensorsCh == 2) {
      TempSensorLeft.getAddress(Measurements.SensorTempLeft2,1);
      TempSensorRight.getAddress(Measurements.SensorTempRight2,1);
    }
    TempSensorLeft.setResolution(9);
    TempSensorRight.setResolution(9);
  }
 #ifdef DEBUG
  Serial.print(F("TempInit: Temperature measurement 1 = oke, 0 = not oke : "));
  Serial.println(Measurements.MeasureTemp);
  Serial.print(F("TempInit: number of sensors found left side            : "));
  Serial.println(NumberOfSensorsLeft);
  if (NumberOfSensorsLeft > 0) {
    Serial.print(F("TempInit: sensor address nr 1 : "));
    printAddress(Measurements.SensorTempLeft1);
    Serial.println();
  }
  if (NumberOfSensorsLeft == 2) {
    Serial.print(F("TempInit: sensor address nr 2 : "));
    printAddress(Measurements.SensorTempLeft2);
    Serial.println(); 
  }
    Serial.print(F("TempInit: number of sensors found right side : "));
  Serial.println(NumberOfSensorsRight);
  if (NumberOfSensorsLeft > 0) {
    Serial.print(F("TempInit: sensor address nr 1 : "));
    printAddress(Measurements.SensorTempRight1);
    Serial.println(); 
  }
  if (NumberOfSensorsLeft == 2) {
    Serial.print(F("TempInit: sensor address nr 2 : "));
    printAddress(Measurements.SensorTempRight2); 
    Serial.println(); 
  }
 #endif  
}

void ReadVoltageLevels() {
  Measurements.DCoffsetLeft=((ADCLeft.readADC_Differential_0_1()) * VoltCor);
  Measurements.AmpsBiasLeft=(((ADCLeft.readADC_Differential_2_3()) * VoltCor)/BiasResistor);
  Measurements.DCoffsetRight=((ADCRight.readADC_Differential_0_1()) * VoltCor);
  Measurements.AmpsBiasRight=(((ADCRight.readADC_Differential_2_3()) * VoltCor)/BiasResistor);
 #ifdef DEBUG               // if debug enabled write message
  Serial.println(F("ReadVoltageLevels: following Voltage levels measured :"));
  Serial.print(F("DC offset left        (V): "));
  Serial.println(Measurements.DCoffsetLeft);
  Serial.print(F("DC offset right       (V): "));
  Serial.println(Measurements.DCoffsetRight);
  Serial.print(F("Bias left channel    (A): "));
  Serial.println(Measurements.AmpsBiasLeft);
  Serial.print(F("Bias right channel   (A): "));
  Serial.println(Measurements.AmpsBiasRight);
 #endif
}
////////////////////////////////////////////////////////////////////////////////////////////////
// read the temparature levels
void ReadTempLevels() {
  int temp1;
  int temp2 = 0; 
  TempSensorLeft.requestTemperatures();
  temp1 = round(TempSensorLeft.getTempC(Measurements.SensorTempLeft1));
  if (temp1 == DEVICE_DISCONNECTED_C) {
    temp1 = -200;
  }
  if (NumberOfSensorsCh == 2) {
    temp2 = round(TempSensorLeft.getTempC(Measurements.SensorTempLeft2));
    if (temp2 == DEVICE_DISCONNECTED_C) {
      temp2 = -200;
    }
  }
  if ((temp1 == -200) | (temp2 == -200)) {
    Measurements.TempLeft = -200;
  }
  else {
    if (temp1 > temp2) {Measurements.TempLeft = temp1;}
    else {Measurements.TempLeft = temp2;} 
  }
 #ifdef DEBUG               // if debug enabled write message
    Serial.println(F("ReadTempLevel: following Temparature levels measured :"));
    Serial.print(F("Temperature level left nr1   (C): "));
    Serial.println(temp1);  
    Serial.print(F("Temperature level left nr2   (C): "));
    Serial.println(temp2); 
 #endif
   TempSensorRight.requestTemperatures();
  temp1 = round(TempSensorRight.getTempC(Measurements.SensorTempRight1));
  if (temp1 == DEVICE_DISCONNECTED_C) {
    temp1 = -200;
  }
  if (NumberOfSensorsCh == 2) {
    temp2 = round(TempSensorRight.getTempC(Measurements.SensorTempRight2));
    if (temp2 == DEVICE_DISCONNECTED_C) {
      temp2 = -200;
    }
  }
  if ((temp1 == -200) | (temp2 == -200)) {
    Measurements.TempRight = -200;
  }
  else {
    if (temp1 > temp2) {Measurements.TempRight = temp1;}
    else {Measurements.TempRight = temp2;} 
  }
 #ifdef DEBUG 
    Serial.print(F("Temperature level right nr1  (C): "));
    Serial.println(temp1);  
    Serial.print(F("Temperature level right nr2  (C): "));
    Serial.println(temp2); 
 #endif       
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function to check i2c bus
  #ifdef DEBUG
void ScanI2CBus() {
  uint8_t error;                                                      // error code
  uint8_t address;                                                    // address to be tested
  int NumberOfDevices;                                                // number of devices found
  Serial.println(F("ScanI2CBus: I2C addresses defined within the code are : ")); // print content of code
  Serial.print(F("Screen             : 0x"));
  Serial.println(OLED_Address, HEX);
  Serial.print(F("ADC board left     : 0x"));
  Serial.println(ADCLeftI2CAddress, HEX);
  Serial.print(F("ADC board right    : 0x"));
  Serial.println(ADCRightI2CAddress, HEX);
  Serial.println(F("ScanI2C: Scanning..."));
  NumberOfDevices = 0;
  for (address = 1; address < 127; address++) {                                   // loop through addresses
    Wire.beginTransmission(address);                                              // test address
    error = Wire.endTransmission();                                               // resolve errorcode
    if (error == 0) {                                                             // if address exist code is 0
      Serial.print("I2C device found at address 0x");                             // print address info
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      NumberOfDevices++;
    }
  }
  if (NumberOfDevices == 0) Serial.println(F("ScanI2C:No I2C devices found"));
  else {
    Serial.print(F("ScanI2CBus: done, number of device found : "));
    Serial.println(NumberOfDevices);
  }
}
void PrintVariables () {
  Serial.println(F("PrintVariables :"));
  Serial.print(F("Are ADC both reachable          : "));
  Serial.println(Measurements.MeasureVolt);
  Serial.print(F("DC offset left                  : "));
  Serial.println(Measurements.DCoffsetLeft);
  Serial.print(F("DC offset right                 : "));
  Serial.println(Measurements.DCoffsetRight);
  Serial.print(F("Bias left                       : "));
  Serial.println(Measurements.AmpsBiasLeft);
  Serial.print(F("Bias right                      : "));
  Serial.println(Measurements.AmpsBiasRight);  
  Serial.print(F("Temperature left                : "));
  Serial.println(Measurements.TempLeft);
  Serial.print(F("Temperature right               : "));
  Serial.println(Measurements.TempRight);
  Serial.print(F("Operational state left          : "));
  Serial.println(Measurements.OperationalStateLeftCh);
  Serial.print(F("Operational state right         : "));
  Serial.println(Measurements.OperationalStateRightCh);
  Serial.print(F("AMP has an error                : "));
  Serial.println(Measurements.AmpInError);
  Serial.print(F("Errorcode                       : "));
  Serial.println(Measurements.ErrorCode);
  Serial.print(F("Value at error                  : "));
  Serial.println(Measurements.ErrorValue);
  Serial.print(F("temp sensors reachable          : "));
  Serial.println(Measurements.MeasureTemp);
  Serial.print(F("address temp sensors left nr 1  : "));
  printAddress(Measurements.SensorTempLeft1);
  Serial.print(F("address temp sensors left nr 2  : "));
  printAddress(Measurements.SensorTempLeft2);
  Serial.print(F("address temp sensors right nr 1 : "));
  printAddress(Measurements.SensorTempRight1);
  Serial.print(F("address temp sensors right nr 2 : "));
  printAddress(Measurements.SensorTempRight2); 
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

int XPosition (int value) {
  if (value == 0) {
    return(122);
  }
  if (value > 0) {
    if (value < 10) {return(122);}
    if (value < 100) {return(117);}
    if (value < 1000) {return(112);}
    return(108);
  }
  else {
    if (value > -10) {return(122);}
    if (value > -100) {return(117);}
    if (value > -1000) {return(112);}
    return(108);  
  }
}
