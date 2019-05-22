/* Author:
    kimaurellano
    
    WORKING CALIBRATION VALUES DO NOT CHANGE:
    #define CAL_X 0x003ACE81UL
    #define CAL_Y 0x039F0161UL
    #define CAL_S 0x000EF13FUL
*/

#include <SharpIR.h>
#include <ArduinoJson.h>
#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <SdFat.h>
#include <URTouch.h>
#include <UTFT.h>
#include <UTFT_Buttons.h>

// Adafruit_Fingerprint
Adafruit_Fingerprint fingerScanner = Adafruit_Fingerprint(&Serial3);

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

// SD
SdFat SD;

// File
File sdFile;

// Pins
// HIGH, LOW - COUNTERCLOCKWISE
const int ACTUATOR_TILT_POSITIVE = 10;
const int ACTUATOR_TILT_NEGATIVE = 11;
// HIGH, LOW - DOWNWARD
const int ACTUATOR_LIFT_POSITIVE = 8;
const int ACTUATOR_LIFT_NEGATIVE = 9;
// HIGH, LOW - OUTWARD
const int DC_POSITIVE = 12;
const int DC_NEGATIVE = 13;

const int SD_CHIP_SELECT_PIN = 53;
const int LED_RELAY_PIN = 40;
const int TRIG_PIN = A0;
const int ECHO_PIN = A1;
const int TABLE_LED_PIN = A2;

char sdContent[64];
char keyInput[10];
boolean tilt = false;
boolean lighted = false;
boolean drawerOut = false;

void setup() {
	// Setup serial monitor
	Serial.begin(9600);

	// Setup fingerScanner scanner
	fingerScanner.begin(57600);

	// Setup SD card
	while (!SD.begin(SS)) {
		Serial.print(".");
	}

	// Setup TFT module
	tft.InitLCD();
	tft.clrScr();

	// Setup touch
	touch.InitTouch();
	touch.setPrecision(PREC_MEDIUM);

	// Setup pins
	pinMode(ACTUATOR_LIFT_POSITIVE, OUTPUT);
	pinMode(ACTUATOR_LIFT_NEGATIVE, OUTPUT);
	pinMode(ACTUATOR_TILT_POSITIVE, OUTPUT);
	pinMode(ACTUATOR_TILT_NEGATIVE, OUTPUT);
	pinMode(DC_POSITIVE, OUTPUT);
	pinMode(DC_NEGATIVE, OUTPUT);
	pinMode(LED_RELAY_PIN, OUTPUT);
	pinMode(TRIG_PIN, OUTPUT);
	pinMode(TABLE_LED_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);

	digitalWrite(TABLE_LED_PIN, HIGH);
	digitalWrite(DC_POSITIVE, LOW);
	digitalWrite(DC_NEGATIVE, LOW);

	Serial.println(F("Setting up..."));
	delay(2000);
	while (CheckHeight() >= 17.25) {
		digitalWrite(ACTUATOR_LIFT_POSITIVE, HIGH);
		digitalWrite(ACTUATOR_LIFT_NEGATIVE, LOW);
	}
	digitalWrite(ACTUATOR_LIFT_POSITIVE, LOW);
	digitalWrite(ACTUATOR_LIFT_NEGATIVE, LOW);
	Serial.println(F("OK"));

	if (SD.exists("tilt.txt")) {
		digitalWrite(ACTUATOR_TILT_POSITIVE, LOW);
		digitalWrite(ACTUATOR_TILT_NEGATIVE, HIGH);
		delay(18000);
		digitalWrite(ACTUATOR_TILT_POSITIVE, LOW);
		digitalWrite(ACTUATOR_TILT_NEGATIVE, LOW);

		SD.remove("tilt.txt");
	}

	buttons.setTextFont(SmallFont);
}

void loop() {
	// Start with Home page
	DrawHome();
}

#pragma region Pages
void DrawControl() {
	tft.setColor(255, 255, 255);  // White
	tft.fillRect(0, 0, 319, 239);

	int btnTilt = buttons.addButton(10, 10, 290, 50, "15 or 0 deg");
	int btnLed = buttons.addButton(10, 60, 290, 50, "Led on/off");
	int btnLogout = buttons.addButton(10, 110, 290, 50, "Log out");
	int btnDrawer = buttons.addButton(10, 160, 290, 50, "Drawer open/close");

	buttons.drawButtons();

	int pressedButton = buttons.checkButtons();

	// Create file to cache actuator states
	sdFile = SD.open("log.txt", FILE_WRITE);
	if (!sdFile) {
		Serial.println("Error opening sd file.");
		return;
	}
	
	sdFile.close();

	while (1) {
		if (touch.dataAvailable()) {
			pressedButton = buttons.checkButtons();
			if (pressedButton == btnTilt) {
				Tilt();
			}
			else if (pressedButton == btnLed) {
				LedChangeState();
			}
			else if (pressedButton == btnLogout) {
				// Delete
				SD.remove("log.txt");

				buttons.deleteAllButtons();

				tft.clrScr();

				// Default to lowest lift
				while (CheckHeight() >= 17.25) {
					digitalWrite(ACTUATOR_LIFT_POSITIVE, HIGH);
					digitalWrite(ACTUATOR_LIFT_NEGATIVE, LOW);
				}

				digitalWrite(ACTUATOR_LIFT_POSITIVE, LOW);
				digitalWrite(ACTUATOR_LIFT_NEGATIVE, LOW);

				while (drawerOut) {
					digitalWrite(DC_POSITIVE, LOW);
					digitalWrite(DC_NEGATIVE, HIGH);
					delay(4000);
					digitalWrite(DC_POSITIVE, LOW);
					digitalWrite(DC_NEGATIVE, LOW);
					drawerOut = false;
				}

				DrawHome();

				if (lighted) {
					digitalWrite(TABLE_LED_PIN, HIGH);
					lighted = false;
				}

			}
			else if (pressedButton == btnDrawer) {
				DrawerStateChange();
			}
		}
	}
}

void DrawKeypad() {
	tft.setColor(255, 255, 255);   // White
	tft.fillRect(0, 0, 319, 239);  // Fill the whole screen with white

	//
	int btn9 = buttons.addButton(50, 100, 40, 40, "9");
	int btn5 = buttons.addButton(50, 150, 40, 40, "5");
	int btnDEL = buttons.addButton(10, 200, 40, 40, "CLR");
	//
	int btn8 = buttons.addButton(110, 100, 40, 40, "8");
	int btn4 = buttons.addButton(110, 150, 40, 40, "4");
	int btn0 = buttons.addButton(70, 200, 40, 40, "0");
	int btnP = buttons.addButton(110, 200, 40, 40, ".");
	//
	int btn7 = buttons.addButton(170, 100, 40, 40, "7");
	int btn3 = buttons.addButton(170, 150, 40, 40, "3");
	int btn1 = buttons.addButton(210, 200, 40, 40, "1");
	//
	int btn6 = buttons.addButton(230, 100, 40, 40, "6");
	int btn2 = buttons.addButton(230, 150, 40, 40, "2");
	int btnOK = buttons.addButton(270, 200, 40, 40, "OK");

	buttons.drawButtons();

	int pressedButton;

	char input[20] = { '\0' };
	while (1) {
		if (touch.dataAvailable()) {
			pressedButton = buttons.checkButtons();
			if (pressedButton == btnOK) {
				Serial.println("Height applied");

				// Apply input height
				ApplyHeight(input);

				// Clear screen first
				tft.clrScr();

				buttons.deleteAllButtons();

				//
				DrawControl();
			}
			else if (pressedButton == btn5) {
				strcat(input, "5");
			}
			else if (pressedButton == btn4) {
				strcat(input, "4");
			}
			else if (pressedButton == btn1) {
				strcat(input, "1");
			}
			else if (pressedButton == btn2) {
				strcat(input, "2");
			}
			else if (pressedButton == btn3) {
				strcat(input, "3");
			}
			else if (pressedButton == btn6) {
				strcat(input, "6");
			}
			else if (pressedButton == btn7) {
				strcat(input, "7");
			}
			else if (pressedButton == btn8) {
				strcat(input, "8");
			}
			else if (pressedButton == btn9) {
				strcat(input, "9");
			}
			else if (pressedButton == btn0) {
				strcat(input, "0");
			}
			else if (pressedButton == btnP) {
				strcat(input, ".");
			}
			else if (pressedButton == btnDEL) {
				Serial.println("deleted");

				input[0] = (char)0;

				Serial.println(input);

				tft.setBackColor(255, 255, 255);
				tft.setColor(207, 68, 54);  // Red
				tft.setFont(BigFont);
				tft.print("                  ", CENTER, 25);
			}

			// Avoid garbage display
			if (pressedButton != -1 && pressedButton && pressedButton != btnDEL) {
				tft.setBackColor(255, 255, 255);
				tft.setColor(207, 68, 54);  // Red
				tft.setFont(BigFont);
				tft.print(input, CENTER, 25);
				Serial.println(input);
			}
		}
	}
}

void DrawRegistration() {
	tft.clrScr();
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
}

void DrawRegistrationStart() {
	tft.clrScr();

	tft.setColor(255, 255, 255);   // White
	tft.fillRect(0, 0, 319, 239);  // Fill the whole screen with white
	// Status text
	tft.setBackColor(255, 255, 255);
	tft.setColor(207, 68, 54);  // Red
	tft.setFont(SmallFont);
	tft.print("Place your finger", CENTER, 125);
}

void DrawRegistrationOnRemove() {
	tft.clrScr();
	
	tft.setColor(255, 255, 255);   // White
	tft.fillRect(0, 0, 319, 239);  // Fill the whole screen with white
	// Status text
	tft.setBackColor(255, 255, 255);
	tft.setColor(207, 68, 54);  // Red
	tft.setFont(SmallFont);
	tft.print("Remove finger and wait", CENTER, 125);
}

void DrawRegistrationOnPlace() {
	tft.clrScr();
	
	tft.setColor(255, 255, 255);   // White
	tft.fillRect(0, 0, 319, 239);  // Fill the whole screen with white
	// Status text
	tft.setBackColor(255, 255, 255);
	tft.setColor(207, 68, 54);  // Red
	tft.setFont(SmallFont);
	tft.print("Place finger again", CENTER, 125);
}

void DrawRegistrationDone() {
	tft.clrScr();

	tft.setColor(255, 255, 255);   // White
	tft.fillRect(0, 0, 319, 239);  // Fill the whole screen with white
	// Status text
	tft.setBackColor(255, 255, 255);
	tft.setColor(207, 68, 54);  // Red
	tft.setFont(SmallFont);
	tft.print("Done.", CENTER, 125);

	delay(2000);
}

void DrawOnLogin() {
	tft.clrScr();
	// Title colour
	tft.setColor(255, 255, 255);  // White
	tft.fillRect(0, 0, 319, 239);
	// Title text
	tft.setBackColor(255, 255, 255);
	tft.setColor(207, 68, 54);  // Red
	tft.setFont(BigFont);
	// Display at CENTER at y + 10
	tft.print("LOGIN", CENTER, 10);

	// Prompt text
	tft.setColor(207, 68, 54);  // Red
	tft.setFont(SmallFont);
	// Set location
	tft.print("Please put your finger", CENTER, 60);
	tft.print("to the scanner", CENTER, 70);

	buttons.deleteAllButtons();
}

void DrawOnLoginSuccess() {
	tft.clrScr();
	// Title colour
	tft.setColor(255, 255, 255);  // White
	tft.fillRect(0, 0, 319, 239);
	// Title text
	tft.setBackColor(255, 255, 255);
	tft.setColor(207, 68, 54);  // Red
	tft.setFont(BigFont);
	// Display at CENTER at y + 10
	tft.print("LOGIN", CENTER, 10);

	// Prompt text
	tft.setColor(207, 68, 54);  // Red
	tft.setFont(SmallFont);
	// Set location
	tft.print("Authenticated", CENTER, 60);

	delay(2000);
}

void DrawHome() {
	tft.clrScr();
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
	int btnLogin = buttons.addButton(100, 70, 150, 50, "Login");
	int btnRegister = buttons.addButton(100, 150, 150, 50, "Register");

	buttons.drawButtons();

	int buttonPressed;

	while (1) {
		if (touch.dataAvailable() == true) {
			buttonPressed = buttons.checkButtons();
			if (buttonPressed == btnLogin) {
				Serial.println("pressed");

				DrawOnLogin();

				while (1) {
					if (getFingerprintIDez() != -1) {
						break;
					}
				}

				DrawOnLoginSuccess();

				DrawKeypad();
			}
			else if (buttonPressed == btnRegister) {
				buttons.deleteAllButtons();

				DrawRegistration();

				while (!getFingerprintEnroll());

				DrawHome();
			}
		}
	}
}
#pragma endregion

#pragma region FingerScanner
uint8_t getFingerprintEnroll() {
	srand(millis());

	int id = (rand() % 127) + 1;

	Serial.println(id);
	int p = -1;
	while (p != FINGERPRINT_OK) {
		// Change UI
		DrawRegistrationStart();

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

	DrawRegistrationOnRemove();

	Serial.println("Remove fingerScanner");
	delay(2000);
	p = 0;
	while (p != FINGERPRINT_NOFINGER) {
		p = fingerScanner.getImage();
	}
	Serial.print("ID ");
	Serial.println(id);
	p = -1;

	DrawRegistrationOnPlace();

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
	Serial.println(id);

	p = fingerScanner.createModel();
	if (p == FINGERPRINT_OK) {
		Serial.println("Prints matched!");
	}
	else if (p == FINGERPRINT_PACKETRECIEVEERR) {
		Serial.println("Communication error");
		return p;
	}
	else if (p == FINGERPRINT_ENROLLMISMATCH) {
		Serial.println("Fingerprints did not match");
		return p;
	}
	else {
		Serial.println("Unknown error");
		return p;
	}

	Serial.print("ID ");
	Serial.println(id);
	p = fingerScanner.storeModel(id);
	if (p == FINGERPRINT_OK) {
		Serial.println("Stored!");

		DrawRegistrationDone();

		delay(1500);

		return 1;
	}
	else if (p == FINGERPRINT_PACKETRECIEVEERR) {
		Serial.println("Communication error");
		return p;
	}
	else if (p == FINGERPRINT_BADLOCATION) {
		Serial.println("Could not store in that location");
		return p;
	}
	else if (p == FINGERPRINT_FLASHERR) {
		Serial.println("Error writing to flash");
		return p;
	}
	else {
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

float CheckHeight() {
	// Clears the trigPin
	digitalWrite(TRIG_PIN, LOW);
	delayMicroseconds(2);
	// Sets the trigPin on HIGH state for 10 micro seconds
	digitalWrite(TRIG_PIN, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIG_PIN, LOW);
	// Reads the echoPin, returns the sound wave travel time in microseconds
	long duration = pulseIn(ECHO_PIN, HIGH);
	// Calculating the distance
	float distance = duration * 0.034 / 2;
	// Prints the distance on the Serial Monitor
	Serial.print("Distance: ");
	Serial.println(distance);

	return distance;
}

// h: height
void ApplyHeight(char *input) {
	Serial.println("Applying height");
	if (strstr(input, "4.11") != nullptr) {  // default. do nothing
		Lift(18);
	}
	else if (strstr(input, "5.0") != nullptr || strcmp(input, "5.1") == 0) {
		Lift(20);
	}
	else if (strstr(input, "5.2") != nullptr) {
		Lift(21);
	}
	else if (strstr(input, "5.3") != nullptr || strstr(input, "5.4") != nullptr) {
		Lift(22);
	}
	else if (strstr(input, "5.5") != nullptr || strstr(input, "5.6") != nullptr) {
		Lift(23);
	}
	else if (strstr(input, "5.7") != nullptr || strstr(input, "5.8") != nullptr) {
		Lift(24);
	}
	else if (strstr(input, "5.9") != nullptr || strcmp(input, "5.10") == 0) {
		Lift(25);
	}
	else if (strcmp(input, "5.11") == 0) {
		Lift(26);
	}
}

// until: until the actuator reaches the assigned height in terms of time, h:
// height
void Lift(float until) {
	while (CheckHeight() <= until) {
		Serial.println(CheckHeight());
		digitalWrite(ACTUATOR_LIFT_POSITIVE, LOW);
		digitalWrite(ACTUATOR_LIFT_NEGATIVE, HIGH);
	}

	digitalWrite(ACTUATOR_LIFT_POSITIVE, LOW);
	digitalWrite(ACTUATOR_LIFT_NEGATIVE, LOW);
}

void Tilt() {
	if (tilt) {
		digitalWrite(ACTUATOR_TILT_POSITIVE, LOW);
		digitalWrite(ACTUATOR_TILT_NEGATIVE, HIGH);
		delay(18000);
		digitalWrite(ACTUATOR_TILT_POSITIVE, LOW);
		digitalWrite(ACTUATOR_TILT_NEGATIVE, LOW);

		SD.remove("tilt.txt");
	}
	else {
		digitalWrite(ACTUATOR_TILT_POSITIVE, HIGH);
		digitalWrite(ACTUATOR_TILT_NEGATIVE, LOW);
		delay(18000);
		digitalWrite(ACTUATOR_TILT_POSITIVE, LOW);
		digitalWrite(ACTUATOR_TILT_NEGATIVE, LOW);

		// Set SD card to writing mode/write new data
		sdFile = SD.open("tilt.txt", FILE_WRITE);
		if (!sdFile) {
			Serial.println("Error opening sd file.");
			return;
		}

		sdFile.close();
	}

	tilt = !tilt;
}

void LedChangeState() {
	if (lighted) {
		digitalWrite(TABLE_LED_PIN, HIGH);
	}
	else {
		digitalWrite(TABLE_LED_PIN, LOW);
	}

	lighted = !lighted;
}

void DrawerStateChange() {
	if (drawerOut) {
		digitalWrite(DC_POSITIVE, LOW);
		digitalWrite(DC_NEGATIVE, HIGH);
		delay(2500);
		digitalWrite(DC_POSITIVE, LOW);
		digitalWrite(DC_NEGATIVE, LOW);
	}
	else {
		digitalWrite(DC_POSITIVE, HIGH);
		digitalWrite(DC_NEGATIVE, LOW);
		delay(2500);
		digitalWrite(DC_POSITIVE, LOW);
		digitalWrite(DC_NEGATIVE, LOW);
	}

	drawerOut = !drawerOut;
}
