/* NOTE:
    - If touch not working disconnect and connect usb connection
    - Or modify the calibration library

    #define CAL_X 0x00C4CE68UL
    #define CAL_Y 0x03A74343UL
    #define CAL_S 0x000EF13FUL
 */

#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <SdFat.h>
#include <URTouch.h>
#include <UTFT.h>
#include <UTFT_Buttons.h>

// Adafruit_Fingerprint
Adafruit_Fingerprint fingerScanner = Adafruit_Fingerprint(&Serial1);
int id;

// Touch, touch coordinates
URTouch touch(6, 5, 4, 3, 2);
int xCoord;
int yCoord;

// TFT, TFT size, buttons, and fonts
UTFT tft(ILI9341_16, 38, 39, 40, 41);
const int XMAX = 300;
const int YMAX = 239;
UTFT_Buttons buttons(&tft, &touch);
extern uint8_t BigFont[];
extern uint8_t SmallFont[];

// SD card
//SdFat SD;

// File
File sdFile;

// Pins
const int SD_CHIP_SELECT_PIN = 53;
const int ACTUATOR_TILT_POSITIVE = 5;
const int ACTUATOR_TILT_NEGATIVE = 6;
const int ACTUATOR_LIFT_POSITIVE = 7;
const int ACTUATOR_LIFT_NEGATIVE = 8;
const int DC_POSITIVE = 9;
const int DC_NEGATIVE = 10;
const int LED_RELAY_PIN = 11;
const int TRIG_PIN = 12;
const int ECHO_PIN = 13;

// Page current at(default)
char *currentPage = "Home";

char keyInput[10];
long randNumber;

void setup() {
  // Setup serial monitor
  Serial.begin(9600);

  // Setup fingerScanner scanner
  fingerScanner.begin(57600);

  // Setup TFT module
  tft.InitLCD();
  tft.clrScr();

  // Setup touch
  touch.InitTouch();
  touch.setPrecision(PREC_MEDIUM);

  // Setup sdFat
  // Serial.print(F("Starting SD "));
  // while (!SD.begin(SD_CHIP_SELECT_PIN)) {
  //   Serial.print(F("."));
  // }
  // Serial.println(F("OK"));

  // Setup pins
//  pinMode(ACTUATOR_LIFT_POSITIVE, OUTPUT);
//  pinMode(ACTUATOR_LIFT_NEGATIVE, OUTPUT);
//  pinMode(ACTUATOR_TILT_POSITIVE, OUTPUT);
//  pinMode(ACTUATOR_TILT_NEGATIVE, OUTPUT);
//  pinMode(DC_POSITIVE, OUTPUT);
//  pinMode(DC_NEGATIVE, OUTPUT);
//  pinMode(LED_RELAY_PIN, OUTPUT);

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  buttons.setTextFont(SmallFont);

  drawHome();
}

void loop() {
  if (touch.dataAvailable()) {
    touch.read();
    xCoord = touch.getX();
    yCoord = touch.getY();
    Serial.print(F("X:"));
    Serial.print(xCoord);
    Serial.print(F(","));
    Serial.print(F("Y:"));
    Serial.println(yCoord);

    // x1:70, y1:150, x2:250, x2:190
    if (xCoord >= 70 && xCoord <= 250 && yCoord >= 150 && yCoord <= 190 &&
        currentPage == "Home") {
      tft.clrScr();
      drawRegistration();

      // Returns -1 when failed
      // while (!getFingerprintEnroll())
      //   ;
      delay(1500);

      drawRegistrationDone();
      delay(2000);
      drawHome();
    }
    // x1:70, y1:215, x2:250, y2:235
    if (xCoord >= 70 && xCoord <= 250 && yCoord >= 215 && yCoord <= 235 &&
        currentPage == "Register") {
      tft.clrScr();
      drawHome();
    }
    // x1:70, y1:170, x2:250, y2:110... 70, 70, 250, 110
    if (xCoord >= 70 && xCoord <= 250 && yCoord >= 70 && yCoord <= 110 &&
        currentPage == "Home") {
      tft.clrScr();
      drawKeypad();
    }
  }
}

#pragma region Pages
void drawInput(char *input) {
  currentPage = "UserInput";

  if (strstr(input, "OK") != nullptr) {
    // Clear content
    memset(&keyInput[0], 0, sizeof(keyInput));

    int inputHeight = atoi(keyInput);

    while(1){
//      if(ReadUltrasonic() > inputHeight){
//        digitalWrite(ACTUATOR_LIFT_NEGATIVE, HIGH);
//        digitalWrite(ACTUATOR_LIFT_POSITIVE, LOW);
//      }else if(ReadUltrasonic() < inputHeight){
//        digitalWrite(ACTUATOR_LIFT_NEGATIVE, LOW);
//        digitalWrite(ACTUATOR_LIFT_POSITIVE, HIGH);
//      }else if(ReadUltrasonic() == inputHeight){
//        digitalWrite(ACTUATOR_LIFT_NEGATIVE, LOW);
//        digitalWrite(ACTUATOR_LIFT_POSITIVE, LOW);
//        break;
//      }
    }
  }

  // Append input
  strcat(keyInput, input);

  tft.setBackColor(255, 255, 255);
  tft.setColor(207, 68, 54);  // Red
  tft.setFont(BigFont);
  tft.print(keyInput, CENTER, 20);
}

void drawControl() {
  tft.setBackColor(255, 255, 255);

  int btnTilt0 = buttons.addButton(20, 60, 40, 40, "0°");
  int btnTilt15 = buttons.addButton(40, 60, 40, 40, "15°");
  int btnLed = buttons.addButton(20, 90, 40, 40, "Led");
  int btnLogout = buttons.addButton(40, 90, 40, 40, "Exit");

  buttons.drawButtons();

  int pressedButton = buttons.checkButtons();

  if(pressedButton == btnTilt0){

  }else if(pressedButton == btnTilt15){

  }else if(pressedButton == btnLed){

  }else if(pressedButton == btnLogout){

  }
}

void drawKeypad() {
  tft.setColor(255, 255, 255);  // White
  tft.fillRect(0, 0, 319, 239);

  // ROW 4
  int btn9 = buttons.addButton(50, 100, 40, 40, "9");
  int btn5 = buttons.addButton(50, 150, 40, 40, "5");
  int btnDEL = buttons.addButton(50, 200, 40, 40, "DEL");
  // ROW 3
  int btn8 = buttons.addButton(110, 100, 40, 40, "8");
  int btn4 = buttons.addButton(110, 150, 40, 40, "4");
  int btn0 = buttons.addButton(110, 200, 40, 40, "0");
  // ROW 2
  int btn7 = buttons.addButton(170, 100, 40, 40, "7");
  int btn3 = buttons.addButton(170, 150, 40, 40, "3");
  int btn1 = buttons.addButton(170, 200, 40, 40, "1");
  // ROW 1
  int btn6 = buttons.addButton(230, 100, 40, 40, "6");
  int btn2 = buttons.addButton(230, 150, 40, 40, "2");
  int btnOK = buttons.addButton(230, 200, 40, 40, "OK");

  buttons.drawButtons();

  int pressedButton;

  char input[20] = { '\0' };
  while(1){
    if(touch.dataAvailable()){
      pressedButton = buttons.checkButtons();
      if(pressedButton == btnOK){
        input[20] = 0;
        tft.clrScr();
        drawHome();
      }else if(pressedButton == btn5){
        strcat(input, "5");
      }else if (pressedButton == btn4) {
        strcat(input, "4");
      }

      // Avoid garbage display
      if(pressedButton != -1){ 
        tft.setBackColor(255, 255, 255);
        tft.setColor(207, 68, 54);  // Red  
        tft.setFont(BigFont);
        tft.print(input, CENTER, 25);
        Serial.println(input);
      }
    }
  }
}

void drawRegistration() {
  currentPage = "Register";

  // Title colour
  tft.setColor(255, 255, 255);
  tft.fillRect(0, 0, 319, 239);
  // Title text
  tft.setBackColor(255, 255, 255);
  tft.setColor(207, 68, 54);  // Red
  tft.setFont(BigFont);
  // Display at CENTER at y + 10
  tft.print("REGISTER", CENTER, 10);

  // Prompt text
  tft.setColor(207, 68, 54);  // Red
  tft.setFont(SmallFont);
  // Set location
  tft.print("Please put your fingerScanner", CENTER, 60);
  tft.print("to the scanner", CENTER, 70);

  // Button back
  tft.setColor(207, 68, 54);
  tft.fillRoundRect(70, 215, 250, 235);
  // Button back text
  tft.setBackColor(207, 68, 54);  // Red
  tft.setColor(255, 255, 255);
  tft.setFont(SmallFont);
  tft.print("Back", CENTER, 220);
}

void drawRegistrationStart() {
  // Status text
  tft.setBackColor(255, 255, 255);
  tft.setColor(207, 68, 54);  // Red
  tft.setFont(SmallFont);
  tft.print("Scanning...", CENTER, 125);
}

void drawRegistrationDone() {
  // Status text
  tft.setBackColor(255, 255, 255);
  tft.setColor(207, 68, 54);  // Red
  tft.setFont(SmallFont);
  tft.print("Done.", CENTER, 125);
}

void drawHome() {
  currentPage = "Home";

  // Title colour
  tft.setColor(255, 255, 255);  // White
  tft.fillRect(0, 0, 319, 239);
  // Title text
  tft.setBackColor(255, 255, 255);
  tft.setColor(207, 68, 54);  // Red
  tft.setFont(BigFont);
  // Display at CENTER at y + 10
  tft.print("WELCOME!", CENTER, 10);

  // Button login
  tft.setColor(207, 68, 54);  // Red
  tft.fillRoundRect(70, 70, 250, 110);
  // Button login text
  tft.setBackColor(207, 68, 54);  // Red
  tft.setColor(255, 255, 255);
  tft.setFont(BigFont);
  tft.print("Login", CENTER, 80);

  // Button register
  tft.setColor(207, 68, 54);  // Red
  tft.fillRoundRect(70, 150, 250, 190);
  // Button register text
  tft.setBackColor(207, 68, 54);  // Red
  tft.setColor(255, 255, 255);
  tft.setFont(BigFont);
  tft.print("Register", CENTER, 160);
}
#pragma endregion

#pragma region FingerScanner
uint8_t getFingerprintEnroll() {
  // Generate random number for ID
  randNumber = random(1000);

  int p = -1;
  while (p != FINGERPRINT_OK) {
    // Change UI
    drawRegistrationStart();

    p = fingerScanner.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println(F("Image taken"));
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(F("."));
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println(F("Communication error"));
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println(F("Imaging error"));
        break;
      default:
        Serial.println(F("Unknown error"));
        break;
    }
  }

  // OK success!

  p = fingerScanner.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println(F("Image converted"));
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println(F("Image too messy"));
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove fingerScanner");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = fingerScanner.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same fingerScanner again");
  while (p != FINGERPRINT_OK) {
    p = fingerScanner.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = fingerScanner.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(randNumber);

  p = fingerScanner.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(randNumber);
  p = fingerScanner.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
}

int getFingerprintIDez() {
  uint8_t p = fingerScanner.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = fingerScanner.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = fingerScanner.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  // found a match!
  Serial.print("Found ID #");
  Serial.print(fingerScanner.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(fingerScanner.confidence);
  return fingerScanner.fingerID;
}
#pragma endregion

int ReadUltrasonic(){
  // Clears the trigPin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  float duration = pulseIn(ECHO_PIN, HIGH);
  // Calculating the distance
  int distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  Serial.print(F("Distance: "));
  Serial.println(distance);

  return distance;
}