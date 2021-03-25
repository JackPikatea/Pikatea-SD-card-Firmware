//Arduino 1.8.10
//Pikatea SD card Firmware

//libraries to include
#include "HID-Project.h
#include <Encoder.h>
#include <SPI.h>
#include <SD.h>
#include <Keypad.h>

#define PIKATEA_MACROPAD_GB2

//Constants
//Pikatea GB2
#if defined(PIKATEA_MACROPAD_GB2)
//the buttons
#define ROWS 1
#define COLS 6
#define BUTTON_AMOUNT 6
char keys[ROWS][COLS] = {
  {0, 1, 2, 3, 4, 5}
};
byte rowPins[ROWS] = {A0}; //connect to the row pinouts of the keypad // change to define
byte colPins[COLS] = {7, 5, 9, 8, 6, 2}; //connect to the column pinouts of the keypad //change to define
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS ); //change to define
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

#if defined(PIKATEA_MACROPAD_AFSC)
//the buttons
#define ROWS 2
#define COLS 5
#define BUTTON_AMOUNT 10
char keys[ROWS][COLS] = {
  {0, 1, 2, 3,   4},
  {5, 6, 7,  8,      9}
};
byte rowPins[ROWS] = {A3,A0}; //connect to the row pinouts of the keypad // change to define
byte colPins[COLS] = {9, 7, 8, 16, 2}; //connect to the column pinouts of the keypad //change to define
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS ); //change to define
String buttonStrings[] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
int fakeAnalogSliderValues[] = {1023, 1023, 1023, 1023, 1023, 1023};


//the encoder
#define ENCODER_DO_NOT_USE_INTERRUPTS
const int EncoderPinA[] = {4,5};
const int EncoderPinB[] = {3,6};
Encoder myEnc(EncoderPinA[0], EncoderPinB[0]);
#define encoderConstant 3
int deejSensitivityConstant = 32;
String knobStrings[][2] = {{"", ""},{"",""}};
#endif

//Variables that change
File configFile;
//0 = standard - 1 = deej
int mode = 0;

void setup() {
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
long oldPosition  = myEnc.read();
long position = 0;
int currentButton = -1;
boolean dj = false;

void loop() {
  char key = keypad.getKey();
  unsigned long currentMillis = millis();
  long newPosition = myEnc.read();
  //Serial.println("main loop");
  //encoder has turned
  if (newPosition > oldPosition + encoderConstant) {
    oldPosition = newPosition;
    //standard mode
    if (mode == 0) {
      pressKeys(knobStrings[0][1], false);
      releaseKeys();
      //deej mode
    } else {
      if (currentButton != -1) {
        fakeAnalogSliderValues[currentButton] = fakeAnalogSliderValues[currentButton] - deejSensitivityConstant;
        //Serial.println("adjusting something else and reseting timmer");
        deejPreviousMillis = currentMillis;
      }
    }
  }
  if (newPosition < oldPosition - encoderConstant) {
    oldPosition = newPosition;

    //standard mode
    if (mode == 0) {
      pressKeys(knobStrings[0][0], false);
      releaseKeys();
      //deej mode

    } else {
      if (currentButton != -1) {
        fakeAnalogSliderValues[currentButton] = fakeAnalogSliderValues[currentButton] + deejSensitivityConstant;
        //Serial.println("adjusting something else and reseting timmer");
        deejPreviousMillis = currentMillis;
      }
    }
  }
}

int holdFlag = 0;
// Taking care of some special events.
void keypadEvent(KeypadEvent key) {
  switch (keypad.getState()) {
    case PRESSED:
    Serial.println(key);
      break;

    case RELEASED:
      if (holdFlag == 0) {
        Serial.print("released: ");
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
int InitializeSDCard(boolean makeMultipleAttempts) {

  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(10)) {
    Serial.println(F("initialization failed!"));
    if (!SD.begin(10)) {
      Serial.println(F("initialization failed!"));
      
    }
  }
  Serial.println(F("initialization done."));
  //move this to different function
#if defined(PIKATEA_MACROPAD_GB2)
  knobStrings[0][0] = ExtractSetting(F("KnobCW="), "config.txt");
  knobStrings[0][1] = ExtractSetting(F("KnobCCW="), "config.txt");
  buttonStrings[0] = ExtractSetting(F("Button1="), "config.txt");
  buttonStrings[1] = ExtractSetting(F("Button2="), "config.txt");
  buttonStrings[2] = ExtractSetting(F("Button3="), "config.txt");
  buttonStrings[3] = ExtractSetting(F("Button4="), "config.txt");
  buttonStrings[4] = ExtractSetting(F("Button5="), "config.txt");
  buttonStrings[5] = ExtractSetting(F("KnobButton="), "config.txt");
  buttonStrings[6] = ExtractSetting(F("Button1Hold="), "config.txt");
  buttonStrings[7] = ExtractSetting(F("Button2Hold="), "config.txt");
  buttonStrings[8] = ExtractSetting(F("Button3Hold="), "config.txt");
  buttonStrings[9] = ExtractSetting(F("Button4Hold="), "config.txt");
  buttonStrings[10] = ExtractSetting(F("Button5Hold="), "config.txt");
  buttonStrings[11] = ExtractSettingWithDefault(F("KnobButtonHold="), "config.txt","button hold");
#endif
#if defined(PIKATEA_MACROPAD_AFSC)
  knobStrings[0][0] = ExtractSetting(F("Knob1CW="), F("config.txt"));
  knobStrings[0][1] = ExtractSetting(F("Knob1CCW="), F("config.txt"));
  knobStrings[1][0] = ExtractSetting(F("Knob2CW="), F("config.txt"));
  knobStrings[1][1] = ExtractSetting(F("Knob2CCW="), F("config.txt"));
  buttonStrings[0] = ExtractSetting(F("Button1="), F("config.txt"));
  buttonStrings[1] = ExtractSetting(F("Button2="), F("config.txt"));
  buttonStrings[2] = ExtractSetting(F("Button3="), F("config.txt"));
  buttonStrings[3] = ExtractSetting(F("Button4="), F("config.txt"));
  buttonStrings[4] = ExtractSetting(F("Button5="), F("config.txt"));
  buttonStrings[5] = ExtractSetting(F("Button6="), F("config.txt"));
  buttonStrings[6] = ExtractSetting(F("Button7="), F("config.txt"));
  buttonStrings[7] = ExtractSetting(F("Button8="), F("config.txt"));
  buttonStrings[8] = ExtractSetting(F("KnobButton1="), F("config.txt"));
  buttonStrings[9] = ExtractSetting(F("KnobButton2="), F("config.txt"));
  buttonStrings[10] = ExtractSetting(F("Button1Hold="), F("config.txt"));
  buttonStrings[11] = ExtractSetting(F("Button2Hold="), F("config.txt"));
  buttonStrings[12] = ExtractSetting(F("Button3Hold="), F("config.txt"));
  buttonStrings[13] = ExtractSetting(F("Button4Hold="), F("config.txt"));
  buttonStrings[14] = ExtractSetting(F("Button5Hold="), F("config.txt"));
  buttonStrings[15] = ExtractSetting(F("Button3Hold="), F("config.txt"));
  buttonStrings[16] = ExtractSetting(F("Button4Hold="), F("config.txt"));
  buttonStrings[17] = ExtractSetting(F("Button5Hold="), F("config.txt"));
  buttonStrings[18] = ExtractSettingWithDefault(F("KnobButton1Hold="), F("config.txt"),"button hold1");
  buttonStrings[19] = ExtractSettingWithDefault(F("KnobButton2Hold="), F("config.txt"),"button hold2");
#endif
  deejSensitivityConstant = ExtractSettingWithDefault(F("deejSensitivity="), "config.txt", "32").toInt();

  if (ExtractSetting(F("deejmode="), "config.txt").indexOf("rue") > 0 or ExtractSetting(F("deejMode="), "config.txt").indexOf("rue") > 0) {
    //Serial.println("Deej mode Active");
    mode = 1;
  }


  return 1;
}


String ExtractSettingWithDefault(String setting, String fileName, String defaultValue) {
  String Temp = "";
  configFile = SD.open(fileName);
  if (configFile) {
    Serial.print(F("found and opened the File "));
    Serial.println(fileName);
    char character;
    char previousCharacter = '\n';
    while (configFile.available()) {
      character = configFile.read();
      //if comment
      if (character == '/' && previousCharacter == '\n') {
        //        int count = 0;
        while (character != '\n') {
          character = configFile.read();
          //          count++;
        }
      }
      else {
        Temp.concat(character);
        if (character == '\n') {
          int Length = setting.length();
          if (Temp.lastIndexOf(setting) != -1) {
            Serial.println(Temp.substring(Temp.lastIndexOf(setting) + Length, Temp.indexOf('\n', Temp.lastIndexOf(setting) + Length)));
            return Temp.substring(Temp.lastIndexOf(setting) + Length, Temp.indexOf('\n', Temp.lastIndexOf(setting) + Length));
          }
          Temp = "";
        }
        previousCharacter = character;
      }
    }
    Temp.concat('\n');
  } else {
    Serial.print(F("Error opening the file"));
    Serial.println(fileName);
    return "File Error";
  }
  configFile.close();
  return defaultValue;

}

String ExtractSetting(String setting, String fileName) {
  return ExtractSettingWithDefault(setting, fileName, "");
}

void pressKey(String given, boolean addDelay) {
  //Serial.println("Pressing Key: " + given);
  if (given.indexOf(F("UpArrow")) >= 0) {
    Keyboard.press(KEY_UP_ARROW);
  } else if (given.indexOf(F("DownArrow")) >= 0) {
    Keyboard.press(KEY_DOWN_ARROW);
  } else if (given.indexOf(F("RightArrow")) >= 0) {
    Keyboard.press(KEY_RIGHT_ARROW);
  } else if (given.indexOf(F("LeftArrow")) >= 0) {
    Keyboard.press(KEY_LEFT_ARROW);
  } else if (given.indexOf(F("Delay")) >= 0) {
    delay(given.substring(6, given.length() - 1).toInt());
  } else if (given.indexOf(F("Release")) >= 0) {
    Keyboard.releaseAll();
  } else if (given.indexOf(F("VolumeUp")) >= 0) {
    Consumer.write(MEDIA_VOL_UP);
  } else if (given.indexOf(F("VolumeDown")) >= 0) {
    Consumer.write(MEDIA_VOL_DOWN);
  } else if (given.indexOf(F("Backspace")) >= 0) {
    Keyboard.press(KEY_BACKSPACE);
  } else if (given.indexOf(F("Space")) >= 0) {
    Keyboard.press(' ');
  } else if (given.indexOf(F("PlusSign")) >= 0) {
    Keyboard.press('+');
  } else if (given.indexOf(F("MinusSign")) >= 0) {
    Keyboard.press('-');
  } else if (given.indexOf(F("Tab")) >= 0) {
    Keyboard.press(KEY_TAB);
  } else if (given.indexOf(F("Return")) >= 0) {
    Keyboard.press(KEY_RETURN);
  } else if (given.indexOf(F("Escape")) >= 0) {
    Keyboard.press(KEY_ESC);
  } else if (given.indexOf(F("Insert")) >= 0) {
    Keyboard.press(KEY_INSERT);
  } else if (given.indexOf(F("Delete")) >= 0) {
    Keyboard.press(KEY_DELETE);
  } else if (given.indexOf(F("PageUp")) >= 0) {
    Keyboard.press(KEY_PAGE_UP);
  } else if (given.indexOf(F("PageDown")) >= 0) {
    Keyboard.press(KEY_PAGE_DOWN);
  } else if (given.indexOf(F("Home")) >= 0) {
    Keyboard.press(KEY_HOME);
  } else if (given.indexOf(F("End")) >= 0) {
    Keyboard.press(KEY_END);
  } else if (given.indexOf(F("CapsLock")) >= 0) {
    Keyboard.press(KEY_CAPS_LOCK);
  } else if (given.indexOf(F("PlayPause")) >= 0) {
    Consumer.write(MEDIA_PLAY_PAUSE);
  } else if (given.indexOf(F("Stop")) >= 0) {
    Consumer.write(MEDIA_STOP);
  } else if (given.indexOf(F("Next")) >= 0) {
    Consumer.write(MEDIA_NEXT);
  } else if (given.indexOf(F("Previous")) >= 0) {
    Consumer.write(MEDIA_PREV);
  } else if (given.indexOf(F("FastForward")) >= 0) {
    Consumer.write(MEDIA_FAST_FORWARD);
  } else if (given.indexOf(F("Rewind")) >= 0) {
    Consumer.write(MEDIA_REWIND);
  } else if (given.indexOf(F("Mute")) >= 0) {
    Consumer.write(MEDIA_VOLUME_MUTE);
  } else if (given.indexOf(F("RightShift")) >= 0) {
    Keyboard.press(KEY_RIGHT_SHIFT);
  } else if (given.indexOf(F("RightCtrl")) >= 0) {
    Keyboard.press(KEY_RIGHT_CTRL);
  } else if (given.indexOf(F("RightAlt")) >= 0) {
    Keyboard.press(KEY_RIGHT_ALT);
  } else if (given.indexOf(F("RightMenu")) >= 0) {
    Keyboard.press(KEY_RIGHT_GUI);
  } else if (given.indexOf(F("LeftShift")) >= 0 || given.indexOf(F("Shift")) >= 0) {
    Keyboard.press(KEY_LEFT_SHIFT);
  } else if (given.indexOf(F("LeftCtrl")) >= 0 || given.indexOf(F("Ctrl")) >= 0) {
    Keyboard.press(KEY_LEFT_CTRL);
  } else if (given.indexOf(F("LeftAlt")) >= 0 || given.indexOf(F("Alt")) >= 0) {
    Keyboard.press(KEY_LEFT_ALT);
  } else if (given.indexOf(F("LeftMenu")) >= 0 || given.indexOf(F("Menu")) >= 0) {
    Keyboard.press(KEY_LEFT_GUI);
    //  } else if (given.indexOf(F("F1")) >= 0
    //             && (given.indexOf(F("F1")) != (given.indexOf(F("F11")) && given.indexOf(F("F12")) && given.indexOf(F("F13")) && given.indexOf(F("F14")) && given.indexOf(F("F15")) && given.indexOf(F("F16")) && given.indexOf(F("F17")) && given.indexOf(F("F18")) && given.indexOf(F("F19")) && given.indexOf(F("F10"))))) {
    //    Keyboard.press(KEY_F1);
    //  } else if (given.indexOf(F("F2")) >= 0
    //             && (given.indexOf(F("F2")) != (given.indexOf(F("F21")) && given.indexOf(F("F22")) && given.indexOf(F("F23")) && given.indexOf(F("F24")) && given.indexOf(F("F20"))))) {
    //    Keyboard.press(KEY_F2);
  } else if (given.indexOf(F("F3")) >= 0) {
    Keyboard.press(KEY_F3);
  } else if (given.indexOf(F("F4")) >= 0) {
    Keyboard.press(KEY_F4);
  } else if (given.indexOf(F("F5")) >= 0) {
    Keyboard.press(KEY_F5);
  } else if (given.indexOf(F("F6")) >= 0) {
    Keyboard.press(KEY_F6);
  } else if (given.indexOf(F("F7")) >= 0) {
    Keyboard.press(KEY_F7);
  } else if (given.indexOf(F("F8")) >= 0) {
    Keyboard.press(KEY_F8);
  } else if (given.indexOf(F("F9")) >= 0) {
    Keyboard.press(KEY_F9);
  } else if (given.indexOf(F("F10")) >= 0) {
    Keyboard.press(KEY_F10);
  } else if (given.indexOf(F("F11")) >= 0) {
    Keyboard.press(KEY_F11);
  } else if (given.indexOf(F("F12")) >= 0) {
    Keyboard.press(KEY_F12);
  } else if (given.indexOf(F("F13")) >= 0) {
    Keyboard.press(KEY_F13);
  } else if (given.indexOf(F("F14")) >= 0) {
    Keyboard.press(KEY_F14);
  } else if (given.indexOf(F("F15")) >= 0) {
    Keyboard.press(KEY_F15);
  } else if (given.indexOf(F("F16")) >= 0) {
    Keyboard.press(KEY_F16);
  } else if (given.indexOf(F("F17")) >= 0) {
    Keyboard.press(KEY_F17);
  } else if (given.indexOf(F("F18")) >= 0) {
    Keyboard.press(KEY_F18);
  } else if (given.indexOf(F("F19")) >= 0) {
    Keyboard.press(KEY_F19);
  } else if (given.indexOf(F("F20")) >= 0) {
    Keyboard.press(KEY_F20);
  } else if (given.indexOf(F("F21")) >= 0) {
    Keyboard.press(KEY_F21);
  } else if (given.indexOf(F("F22")) >= 0) {
    Keyboard.press(KEY_F22);
  } else if (given.indexOf(F("F23")) >= 0) {
    Keyboard.press(KEY_F23);
  } else if (given.indexOf(F("F24")) >= 0) {
    Keyboard.press(KEY_F24);
  } else if (given.indexOf(F("MouseLeft")) >= 0) {
    Mouse.click();
  } else if (given.indexOf(F("MouseRight")) >= 0) {
    Mouse.click(MOUSE_RIGHT);
  } else if (given.indexOf(F("MouseMiddle")) >= 0) {
    Mouse.click(MOUSE_MIDDLE);
  } else if (given.indexOf(F("ScrollUp")) >= 0) {
    Mouse.move(0, 0, 1);
  } else if (given.indexOf(F("ScrollDown")) >= 0) {
    Mouse.move(0, 0, -1);
  } else if (given.indexOf(F("ScrollUpFast")) >= 0) {
    Mouse.move(0, 0, 3);
  } else if (given.indexOf(F("ScrollDownFast")) >= 0) {
    Mouse.move(0, 0, -3);
  } else if (given.indexOf(F("ScrollRight")) >= 0) {
    Keyboard.press(KEY_LEFT_SHIFT);
    Mouse.move(0, 0, 1);
    Keyboard.releaseAll();
  } else if (given.indexOf(F("ScrollLeft")) >= 0) {
    Keyboard.press(KEY_LEFT_SHIFT);
    Mouse.move(0, 0, -1);
    Keyboard.releaseAll();
    //  } else if (given.indexOf(F("Power")) >= 0) {
    //    Consumer.write(HID_CONSUMER_POWER);
    //  } else if (given.indexOf(F("Reset")) >= 0) {
    //    Consumer.write(HID_CONSUMER_RESET);
    //  } else if (given.indexOf(F("Sleep")) >= 0) {
    //    Consumer.write(HID_CONSUMER_SLEEP);
  } else if (given.indexOf(F("Keypad1")) >= 0) {
    Keyboard.press(KEYPAD_1);
  } else if (given.indexOf(F("Keypad2")) >= 0) {
    Keyboard.press(KEYPAD_2);
  } else if (given.indexOf(F("Keypad3")) >= 0) {
    Keyboard.press(KEYPAD_3);
  } else if (given.indexOf(F("Keypad4")) >= 0) {
    Keyboard.press(KEYPAD_4);
  } else if (given.indexOf(F("Keypad5")) >= 0) {
    Keyboard.press(KEYPAD_5);
  } else if (given.indexOf(F("Keypad6")) >= 0) {
    Keyboard.press(KEYPAD_6);
  } else if (given.indexOf(F("Keypad7")) >= 0) {
    Keyboard.press(KEYPAD_7);
  } else if (given.indexOf(F("Keypad8")) >= 0) {
    Keyboard.press(KEYPAD_8);
  } else if (given.indexOf(F("Keypad9")) >= 0) {
    Keyboard.press(KEYPAD_9);
  } else if (given.indexOf(F("Keypad0")) >= 0) {
    Keyboard.press(KEYPAD_0);
  } else if (given.indexOf(F("KeypadDot")) >= 0) {
    Keyboard.press(KEYPAD_DOT);
  } else if (given.indexOf(F("KeypadEnter")) >= 0) {
    Keyboard.press(KEYPAD_ENTER);
  } else if (given.indexOf(F("KeypadAdd")) >= 0) {
    Keyboard.press(KEYPAD_ADD);
  } else if (given.indexOf(F("KeypadSubtract")) >= 0) {
    Keyboard.press(KEYPAD_SUBTRACT);
  } else if (given.indexOf(F("KeypadMultiply")) >= 0) {
    Keyboard.press(KEYPAD_MULTIPLY);
  } else if (given.indexOf(F("KeypadDivide")) >= 0) {
    Keyboard.press(KEYPAD_DIVIDE);
  } else if (given.indexOf(F("KeyNumLock")) >= 0) {
    Keyboard.press(KEY_NUM_LOCK);
  } else if (given.indexOf(F("PrintScreen")) >= 0) {
    Keyboard.write(KeyboardKeycode(0x46));
  } else if (given.indexOf(F("RawHex")) >= 0) {
    String temp3 = given.substring(9, given.length() - 1);
    //Keyboard.write(KeyboardKeycode(x2i(temp3)));
  } else {
    // key not found
    //Serial.print("key not found, typing:");

    char c[given.length() + 1];
    given.toCharArray(c, sizeof(c));
    if (given.length() == 1) {
      Keyboard.press(c[0]);
    } else {
      Keyboard.print(c);
    }
  }
  if (addDelay) {
    delay(50);
  }
  else {
    delay (7);
  }
}

void releaseKeys() {
  //  Serial.println("release All");
  Keyboard.releaseAll();
}

void pressKeys(String given, boolean Delay) {
  //Serial.println(given);
  String temp = given;
  while (temp.lastIndexOf("+") > -1) {
    pressKey(temp.substring(0, temp.indexOf("+")), Delay);
    temp = temp.substring(temp.indexOf("+") + 1, temp.length());
  }
  pressKey(temp, Delay);
}

unsigned long DJpreviousMillis = 0;
String prevBuiltString = "";

void sendSliderValues() {
  if (mode == 1) {
    String builtString = String("");

    for (int i = 0; i < 5; i++) {
      builtString += String((int)fakeAnalogSliderValues[i]);

      if (i < 5 - 1) {
        builtString += String("|");
      }
    }
    unsigned long DJcurrentMillis = millis();
    if (DJcurrentMillis - DJpreviousMillis > 10) {
      DJpreviousMillis = DJcurrentMillis;
      if (builtString != prevBuiltString) {
        Serial.println(builtString);
      }
      prevBuiltString = builtString;
    }
  }
}


void correctSliderValues() {
  for (int i = 0; i < 5; i++) {
    if (fakeAnalogSliderValues[i] < 0) {
      fakeAnalogSliderValues[i] = 0;
    }
    if (fakeAnalogSliderValues[i] > 1023) {
      fakeAnalogSliderValues[i] = 1023;
    }
  }
}

int x2i(String s)
{
  int x = 0;
  for (int i = 0; i < s.length(); i++) {
    char c = s.charAt(i);
    if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0';
    }
    else if (c >= 'A' && c <= 'F') {
      x *= 16;
      x += (c - 'A') + 10;
    }
    else break;
  }
  return x;
}
