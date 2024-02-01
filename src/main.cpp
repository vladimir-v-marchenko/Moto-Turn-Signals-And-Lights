#include <Arduino.h>
#include <Wire.h> 
#include <OneWire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Flasher Blink delay
#define BLINK_INTERVAL      500
#define BEEP_FREQUENCY     1000
// D12 Buzzer connection pin
#define BUZZER_PIN           12
// Emergency Signal head light pin
#define EMERGENCY_HEAD_LIGHT 11
// Right Signal light pin
#define RH_LIGHT             10
// Left Signal light pin 
#define LH_LIGHT              9
// Right Signal switch pin 
#define RH_SWITCH_PIN         8
// Left Signal switch pin 
#define LH_SWITCH_PIN         7
#define CONTROL_PANEL_LIGHT  11
// Display W:H
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SSD1306_WHITE   1

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire, -1);

enum class State {
  NONE,
  LH_DOWN,
  RH_DOWN,
  LH_LIGHT_ON,
  RH_LIGHT_ON,
  EMERGENCY
};

volatile State signal_state = State::NONE;

#include <SwitchSignal.h>

SwitchSignal LHswitch;
SwitchSignal RHswitch;

unsigned long lastBlink;
bool onCycle;

int16_t x1;
int16_t y1;
uint16_t width;
uint16_t height;
int8_t sig_count = -1;

void displayLeftSig(void) {
  Serial.println(F("LEFT"));
  digitalWrite(LH_LIGHT, HIGH);
  tone(BUZZER_PIN, BEEP_FREQUENCY);
  display.getTextBounds(F("LEFT"), 0, 0, &x1, &y1, &width, &height);
  display.clearDisplay();
  display.setCursor((SCREEN_WIDTH - width) / 2, 0);     
  display.print(F("LEFT"));
  display.fillTriangle(32, 40, 96, 16, 96, 64, SSD1306_WHITE);
  display.display();
}

void displayRightSig(void) {
  Serial.println(F("RIGHT"));
  digitalWrite(RH_LIGHT, HIGH);
  tone(BUZZER_PIN, BEEP_FREQUENCY);
  display.clearDisplay();
  display.getTextBounds(F("RIGHT"), 0, 0, &x1, &y1, &width, &height);
  display.setCursor((SCREEN_WIDTH - width) / 2, 0);   
  display.print(F("RIGHT"));
  display.fillTriangle(32, 16, 96, 40, 32, 64, SSD1306_WHITE);
  display.display();
}

void displayEmergencySig(void) {
  Serial.println(F("EMERGENCY"));
  digitalWrite(LH_LIGHT, HIGH);
  digitalWrite(RH_LIGHT, HIGH);
  digitalWrite(EMERGENCY_HEAD_LIGHT, HIGH);
  tone(BUZZER_PIN, BEEP_FREQUENCY);
  display.clearDisplay();
  display.getTextBounds(F("EMERGENCY"), 0, 0, &x1, &y1, &width, &height);
  display.setCursor((SCREEN_WIDTH - width) / 2, 0);
  display.print(F("EMERGENCY"));
  //display.fillTriangle(62, 16, 0, 48, 62, 64, WHITE);
  //display.fillTriangle(66, 16, 128, 48, 66, 64, WHITE);
  display.display();  
}

void displayClear(void) {
  digitalWrite(LH_LIGHT, LOW);
  digitalWrite(RH_LIGHT, LOW);
  digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);
  noTone(BUZZER_PIN);
  display.clearDisplay();
  display.display();
}

void blinkLights() {
  lastBlink = millis();
  onCycle = !onCycle;

  // every second time, turn them all off
  if (!onCycle) {
     displayClear();
     return;
  }
  // blink light
  switch (signal_state) {
    case State::NONE:
      digitalWrite(LH_LIGHT, LOW);
      digitalWrite(RH_LIGHT, LOW);
      digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);
      break;
    case State::LH_DOWN:
    case State::LH_LIGHT_ON:
      displayLeftSig();
      break;
    case State::RH_DOWN:
    case State::RH_LIGHT_ON:
      displayRightSig();
      break;
    case State::EMERGENCY:
      displayEmergencySig();
      break;
    }  // end of switch on state
}  // end of blinkLights


void i2c_scanner()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(5000);           // wait 5 seconds for next scan
}// ------------------------- SETUP -------------------------
void setup() {
  Wire.begin();
  Serial.begin(9600);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  Serial.println(F("Initialize: Display"));
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor((SCREEN_WIDTH - width) / 2, 0); 
  display.clearDisplay();
  Serial.print(F("Loading"));
  display.println(F("Loading"));
  display.display();
  
  //i2c_scanner();

  LHswitch.begin(LH_SWITCH_PIN, handleLHPress);
  pinMode(LH_LIGHT, OUTPUT);
  digitalWrite(LH_LIGHT, LOW);

  RHswitch.begin(RH_SWITCH_PIN, handleRHPress);
  pinMode(RH_LIGHT, OUTPUT);
  digitalWrite(RH_LIGHT, LOW);

  pinMode(EMERGENCY_HEAD_LIGHT, OUTPUT);
  digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  tone(BUZZER_PIN, BEEP_FREQUENCY);
  delay(500);
  noTone(BUZZER_PIN);

  Serial.println(F("Ready"));
  display.clearDisplay();
  display.setCursor((SCREEN_WIDTH - width) / 2, 0);     
  display.println(F("Ready"));
  display.display();
}  // end of setup


// ------------------------- LOOP ---------------------------
void loop() {
  LHswitch.check();  // check for left signal button presses
  RHswitch.check();  // check for right signal button presses
  if (millis() - lastBlink >= BLINK_INTERVAL) {
    blinkLights();
  }
}  // end of loop
