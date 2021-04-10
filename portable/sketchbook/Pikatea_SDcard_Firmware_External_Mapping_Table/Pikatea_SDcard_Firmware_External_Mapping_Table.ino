//Arduino 1.8.10
//Pikatea SD card Firmware

//libraries to include
#include "HID-Project.h"
#include <Encoder.h>
#include <SPI.h>
#include <SD.h>
#include <Keypad.h>

#define PIKATEA_MACROPAD_GB2
//#define PIKATEA_MACROPAD_AFSC

//Constants
//Pikatea GB2
#if defined(PIKATEA_MACROPAD_GB2)

//the buttons
#define ROWS 1
#define COLS 6
#define BUTTON_AMOUNT 61
char keys[ROWS][COLS] = {
    {0, 1, 2, 3, 4, 5}};
byte rowPins[ROWS] = {A0};                                              //connect to the row pinouts of the keypad // change to define
byte colPins[COLS] = {7, 5, 9, 8, 6, 2};                                //connect to the column pinouts of the keypad //change to define
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); //change to define
String buttonStrings[] = {"", "", "", "", "", "", "", "", "", "", "", ""};
int fakeAnalogSliderValues[] = {1023, 1023, 1023, 1023, 1023, 1023};

//the encoder
#define ENCODER_DO_NOT_USE_INTERRUPTS
const int EncoderPinA[] = {4};
const int EncoderPinB[] = {3};
Encoder myEnc(EncoderPinA[0], EncoderPinB[0]);
#define encoderConstant 3
int deejSensitivityConstant = 32;
String knobStrings[][2] = {{"", ""}};
#endif

//desktop macropad
#if defined(PIKATEA_MACROPAD_AFSC)
//the buttons
#define ROWS 2
#define COLS 5
#define BUTTON_AMOUNT 10
char keys[ROWS][COLS] = {
    {0, 1, 2, 3, 4},
    {5, 6, 7, 8, 9}};
byte rowPins[ROWS] = {A3, A0};                                          //connect to the row pinouts of the keypad // change to define
byte colPins[COLS] = {9, 7, 8, 16, 2};                                  //connect to the column pinouts of the keypad //change to define
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); //change to define
String buttonStrings[] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
int fakeAnalogSliderValues[] = {1023, 1023, 1023, 1023, 1023, 1023};

//the encoder
#define ENCODER_DO_NOT_USE_INTERRUPTS
const int EncoderPinA[] = {4, 5};
const int EncoderPinB[] = {3, 6};
Encoder myEnc(EncoderPinA[0], EncoderPinB[0]);
#define encoderConstant 3
int deejSensitivityConstant = 32;
String knobStrings[][2] = {{"", ""}, {"", ""}};
#endif

//Variables that change

//0 = standard - 1 = deej
int mode = 0;

void setup()
{
  Serial.begin(9600);
  //  while (!Serial) {
  //    ; // wait for serial port to connect.
  //  }
  delay(4000); // wait for serial port to connect.
  InitializeSDCard(false);

  //setup keyboard and keypad
  Keyboard.begin();
  Mouse.begin();
  keypad.addEventListener(keypadEvent); // Add an event listener for this keypad

  pinMode(EncoderPinA[0], INPUT_PULLUP);
  pinMode(EncoderPinB[1], INPUT_PULLUP);
}

unsigned long previousMillis = 0;
unsigned long deejPreviousMillis = 0;
long oldPosition = myEnc.read();
long position = 0;
int currentButton = -1;
boolean dj = false;

void loop()
{
  char key = keypad.getKey();
  unsigned long currentMillis = millis();
  long newPosition = myEnc.read();
  //Serial.println("main loop");
  //encoder has turned
  if (newPosition > oldPosition + encoderConstant)
  {
    oldPosition = newPosition;
    //standard mode
    if (!dj)
    {
      pressKeys(knobStrings[0][1], false);
      releaseKeys();
      //deej mode
    }
    else
    {
      if (currentButton != -1)
      {
        fakeAnalogSliderValues[currentButton] = fakeAnalogSliderValues[currentButton] - deejSensitivityConstant;
        //Serial.println("adjusting something else and reseting timmer");
        deejPreviousMillis = currentMillis;
      }
    }
  }
  if (newPosition < oldPosition - encoderConstant)
  {
    oldPosition = newPosition;

    //standard mode
    if (!dj)
    {
      pressKeys(knobStrings[0][0], false);
      releaseKeys();
      //deej mode
    }
    else
    {
      if (currentButton != -1)
      {
        fakeAnalogSliderValues[currentButton] = fakeAnalogSliderValues[currentButton] + deejSensitivityConstant;
        //Serial.println("adjusting something else and reseting timmer");
        deejPreviousMillis = currentMillis;
      }
    }
  }
}

int holdFlag = 0;
//when a button is pressed
void keypadEvent(KeypadEvent key)
{
  switch (keypad.getState())
  {
  case PRESSED:
    dj = false;
    Serial.println(key);
    break;

  case RELEASED:
    //if the button has not been held
    if (holdFlag == 0)
    {
      //short press
      Serial.print("shortpress: ");
      Serial.println(buttonStrings[(int)key]);
      Keyboard.release(key);
      pressKeys(buttonStrings[(int)key], true);
      releaseKeys();
    }
    holdFlag = 0;
    break;

  case HOLD:
    Serial.print("hold: ");
    Serial.println(buttonStrings[((int)key) + BUTTON_AMOUNT]);
    pressKeys(buttonStrings[((int)key) + BUTTON_AMOUNT], true);
    releaseKeys();
    holdFlag = 1;
    //break;
  }
}

//Initialize the SD card and return 1 if successful
//TODO - add support for making multiple attempts
int InitializeSDCard(boolean makeMultipleAttempts)
{

  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(10))
  {
    Serial.println(F("initialization failed!"));
    if (!SD.begin(10))
    {
      Serial.println(F("initialization failed!"));
    }
  }
  Serial.println(F("initialization done."));
  String FileName = F("config.txt");
  //move this to different function
#if defined(PIKATEA_MACROPAD_GB2)
  knobStrings[0][0] = ConvertValuesToKeycodes(ExtractSetting(F("KnobCW="), FileName));
  knobStrings[0][1] = ConvertValuesToKeycodes(ExtractSetting(F("KnobCCW="), FileName));
  buttonStrings[0] = ConvertValuesToKeycodes(ExtractSetting(F("Button1="), FileName));
  buttonStrings[1] = ConvertValuesToKeycodes(ExtractSetting(F("Button2="), FileName));
  buttonStrings[2] = ConvertValuesToKeycodes(ExtractSetting(F("Button3="), FileName));
  buttonStrings[3] = ConvertValuesToKeycodes(ExtractSetting(F("Button4="), FileName));
  buttonStrings[4] = ConvertValuesToKeycodes(ExtractSetting(F("Button5="), FileName));
  buttonStrings[5] = ConvertValuesToKeycodes(ExtractSetting(F("KnobButton="), FileName));
  buttonStrings[6] = ConvertValuesToKeycodes(ExtractSetting(F("Button1Hold="), FileName));
  buttonStrings[7] = ConvertValuesToKeycodes(ExtractSetting(F("Button2Hold="), FileName));
  buttonStrings[8] = ConvertValuesToKeycodes(ExtractSetting(F("Button3Hold="), FileName));
  buttonStrings[9] = ConvertValuesToKeycodes(ExtractSetting(F("Button4Hold="), FileName));
  buttonStrings[10] = ConvertValuesToKeycodes(ExtractSetting(F("Button5Hold="), FileName));
  buttonStrings[11] = ConvertValuesToKeycodes(ExtractSetting(F("KnobButtonHold="), FileName));
#endif
#if defined(PIKATEA_MACROPAD_AFSC)
  knobStrings[0][0] = ConvertValuesToKeycodes(ExtractSetting(F("Knob1CW="), FileName));
  knobStrings[0][1] = ConvertValuesToKeycodes(ExtractSetting(F("Knob1CCW="), FileName));
  knobStrings[1][0] = ConvertValuesToKeycodes(ExtractSetting(F("Knob2CW="), FileName));
  knobStrings[1][1] = ConvertValuesToKeycodes(ExtractSetting(F("Knob2CCW="), FileName));
  buttonStrings[0] = ConvertValuesToKeycodes(ExtractSetting(F("Button1="), FileName));
  buttonStrings[1] = ConvertValuesToKeycodes(ExtractSetting(F("Button2="), FileName));
  buttonStrings[2] = ConvertValuesToKeycodes(ExtractSetting(F("Button3="), FileName));
  buttonStrings[3] = ConvertValuesToKeycodes(ExtractSetting(F("Button4="), FileName));
  buttonStrings[4] = ConvertValuesToKeycodes(ExtractSetting(F("Button5="), FileName));
  buttonStrings[5] = ConvertValuesToKeycodes(ExtractSetting(F("Button6="), FileName));
  buttonStrings[6] = ConvertValuesToKeycodes(ExtractSetting(F("Button7="), FileName));
  buttonStrings[7] = ConvertValuesToKeycodes(ExtractSetting(F("Button8="), FileName));
  buttonStrings[8] = ConvertValuesToKeycodes(ExtractSetting(F("KnobButton1="), FileName));
  buttonStrings[9] = ConvertValuesToKeycodes(ExtractSetting(F("KnobButton2="), FileName));
  buttonStrings[10] = ConvertValuesToKeycodes(ExtractSetting(F("Button1Hold="), FileName));
  buttonStrings[11] = ConvertValuesToKeycodes(ExtractSetting(F("Button2Hold="), FileName));
  buttonStrings[12] = ConvertValuesToKeycodes(ExtractSetting(F("Button3Hold="), FileName));
  buttonStrings[13] = ConvertValuesToKeycodes(ExtractSetting(F("Button4Hold="), FileName));
  buttonStrings[14] = ConvertValuesToKeycodes(ExtractSetting(F("Button5Hold="), FileName));
  buttonStrings[15] = ConvertValuesToKeycodes(ExtractSetting(F("Button3Hold="), FileName));
  buttonStrings[16] = ConvertValuesToKeycodes(ExtractSetting(F("Button4Hold="), FileName));
  buttonStrings[17] = ConvertValuesToKeycodes(ExtractSetting(F("Button5Hold="), FileName));
  buttonStrings[18] = ConvertValuesToKeycodes(ExtractSetting(F("KnobButton1Hold="), FileName));
  buttonStrings[19] = ConvertValuesToKeycodes(ExtractSetting(F("KnobButton2Hold="), FileName));
#endif
  deejSensitivityConstant = ExtractSettingWithDefault(F("deejSensitivity="), FileName, "32").toInt();

  if (ExtractSetting(F("deejmode="), FileName).indexOf("rue") > 0 or ExtractSetting(F("deejMode = "), FileName).indexOf("rue") > 0)
  {
    //Serial.println("Deej mode Active");
    mode = 1;
  }
  return 1;
}

String ExtractSetting(String setting, String fileName)
{
  return ExtractSettingWithDefault(setting, fileName, "");
}
String ExtractSettingWithDefault(String setting, String fileName, String defaultValue)
{
  String Temp = "";
  File configFile;
  configFile = SD.open(fileName);
  if (configFile)
  {
    //    Serial.print(F("found and opened the File "));
    //    Serial.println(fileName);
    char character;
    char previousCharacter = '\n';
    while (configFile.available())
    {
      character = configFile.read();
      //if comment
      if (character == '/' && previousCharacter == '\n')
      {
        while (character != '\n')
        {
          character = configFile.read();
        }
      }
      else
      {        
        if (character == '\n')
        {
          int Length = setting.length(); //length of the setting eg. button1=
          if (Temp.lastIndexOf(setting) != -1)
          { // if the setting is in the file
            Serial.print(F("From: "));
            Serial.println(fileName + " - " + setting.substring(0, Length - 1) + ": " + Temp.substring(Temp.lastIndexOf(setting) + Length, Temp.indexOf('\n', Temp.lastIndexOf(setting) + Length)));
            configFile.close();
            return Temp.substring(Temp.lastIndexOf(setting) + Length, Temp.indexOf('\n', Temp.lastIndexOf(setting) + Length));
          }
          Temp = "";
        }
        else
        {
          Temp.concat(character);
        }        
        previousCharacter = character;
      }
    }
    Temp.concat('\n');
  }
  else
  {
    Serial.print(F("Error opening the file"));
    Serial.println(fileName);
    return "File Error";
  }
  configFile.close();
  return defaultValue;
}

String ConvertValuesToKeycodes(String given)
{
  given.trim();
  if (given.length() == 0)
  {
    return "";
  }
  String temp2 = "";
  while (given.lastIndexOf("+") > -1)
  {    
    temp2.concat(ExtractSettingWithDefault(given.substring(0, given.indexOf("+")) + "=", "TMF.txt", "w(" + given.substring(0, given.indexOf("+")) + ")"));
    temp2.concat('+');
    given = given.substring(given.indexOf("+") + 1, given.length());
  }  
  temp2.concat(ExtractSettingWithDefault(given + "=", "TMF.txt", "w(" + given + ")"));
  
  return temp2;
}

void pressKey(String given, boolean addDelay)
{
  //if "w(khkjgldf whatever)" is found
  if (given.indexOf(F("w")) >= 0)
  {
    //chop of the first 2 and last 1
    String temp = given.substring(2, given.length() - 1);
    //print out a single char or write out a string
    char c[temp.length() + 1];
    temp.toCharArray(c, sizeof(c));
    if (temp.length() == 1)
    {
      Keyboard.press(c[0]);
    }
    else
    {
      Keyboard.print(c);
    }
  }
  else
  {
    Keyboard.write(KeyboardKeycode(given.toInt()));
  }
}

void releaseKeys()
{
  //  Serial.println("release All");
  Keyboard.releaseAll();
}

void pressKeys(String given, boolean Delay)
{
  //Serial.println(given);
  //todo remove temp = given;
  String temp = given;
  //if a "+" is found
  while (temp.lastIndexOf("+") > -1)
  {
    pressKey(temp.substring(0, temp.indexOf("+")), Delay);
    temp = temp.substring(temp.indexOf("+") + 1, temp.length());
  }
  pressKey(temp, Delay);
}

unsigned long DJpreviousMillis = 0;
String prevBuiltString = "";

void sendSliderValues()
{
  if (mode == 1)
  {
    String builtString = String("");

    for (int i = 0; i < 5; i++)
    {
      builtString += String((int)fakeAnalogSliderValues[i]);

      if (i < 5 - 1)
      {
        builtString += String("|");
      }
    }
    unsigned long DJcurrentMillis = millis();
    if (DJcurrentMillis - DJpreviousMillis > 10)
    {
      DJpreviousMillis = DJcurrentMillis;
      if (builtString != prevBuiltString)
      {
        Serial.println(builtString);
      }
      prevBuiltString = builtString;
    }
  }
}

void correctSliderValues()
{
  for (int i = 0; i < 5; i++)
  {
    if (fakeAnalogSliderValues[i] < 0)
    {
      fakeAnalogSliderValues[i] = 0;
    }
    if (fakeAnalogSliderValues[i] > 1023)
    {
      fakeAnalogSliderValues[i] = 1023;
    }
  }
}
