#include <Arduino.h>
#include <Wire.h> 
#include <OneWire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Debug and Test options
//#define _DEBUG_
//#define _TEST_

#ifdef _DEBUG_
#define _PP(a) Serial.print(a);
#define _PL(a) Serial.println(a);
#else
#define _PP(a)
#define _PL(a)
#endif

// Configuration
#define BLINK_INTERVAL      500
#define BEEP_FREQUENCY     1000
#define BUZZER_PIN           12
#define EMERGENCY_HEAD_LIGHT 11
#define RH_LIGHT             10
#define LH_LIGHT              9
#define RH_SWITCH_PIN         8
#define LH_SWITCH_PIN         7
#define NEUTRAL_SENSOR_PIN   A0
// Display W:H
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SSD1306_WHITE  1

Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

enum class State {
  NONE,
  LH_DOWN,
  RH_DOWN,
  LH_LIGHT_ON,
  RH_LIGHT_ON,
  EMERGENCY
};

volatile State signal_state = State::NONE;

#include "SwitchSignal.h"

SwitchSignal LHswitch;
SwitchSignal RHswitch;

unsigned long lastBlink;
bool onCycle;

int16_t x1;
int16_t y1;
uint16_t width;
uint16_t height;
int8_t sig_count = -1;
volatile int8_t neutral_sensor;

void displayTitle(const String &str) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.getTextBounds(str, 0, 0, &x1, &y1, &width, &height);
  display.setCursor((SCREEN_WIDTH - width) / 2, 0);
  display.println(str);
}

void displayLeftSig(void) {
  _PL(F("Signal: Left"));
  digitalWrite(LH_LIGHT, HIGH);
  digitalWrite(RH_LIGHT, LOW);
  tone(BUZZER_PIN, BEEP_FREQUENCY);
  displayTitle("LEFT");
  display.fillTriangle(32, 40, 96, 16, 96, 64, SSD1306_WHITE);
  display.display();
}

void displayRightSig(void) {
  _PL(F("Signal: Right"));
  digitalWrite(RH_LIGHT, HIGH);
  digitalWrite(LH_LIGHT, LOW);
  tone(BUZZER_PIN, BEEP_FREQUENCY);
  displayTitle("RIGHT");
  display.fillTriangle(32, 16, 96, 40, 32, 64, SSD1306_WHITE);
  display.display();
}

void displayEmergencySig(void) {
  _PL(F("Signal: Emergency"));
  digitalWrite(LH_LIGHT, HIGH);
  digitalWrite(RH_LIGHT, HIGH);
  digitalWrite(EMERGENCY_HEAD_LIGHT, HIGH);
  tone(BUZZER_PIN, BEEP_FREQUENCY);
  displayTitle("EMERGENCY");
  display.setTextSize(4);
  display.setTextColor(SSD1306_WHITE);
  display.getTextBounds(F("!"), 0, 0, &x1, &y1, &width, &height);
  display.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT + 14 - height) / 2 ); 
  display.drawCircle(62, 39, 22, 1);
  display.println(F("!"));
  display.display();
}

void displayClear(void) {
//  digitalWrite(LH_LIGHT, LOW);
//  digitalWrite(RH_LIGHT, LOW);
  digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);
  noTone(BUZZER_PIN);
  display.clearDisplay();
  display.display();
}

// ------------------------- SETUP -------------------------
void setup() {
  #if defined(_DEBUG_) || defined(_TEST_)
    Serial.begin(9600);
  _PL("Automotive signal controller - DEBUG mode\n");
  #endif
  //Wire.begin();
  _PP(F("Setup: Display"));
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  displayTitle("LOADING");
  display.println(F("Press LH enter setup"));
  display.display();
  delay(1000);

  _PP(F(", NeutralSensor"));
  pinMode(NEUTRAL_SENSOR_PIN, INPUT);
  
  _PP(F(", LHswitch"));
  LHswitch.begin(LH_SWITCH_PIN, handleLHPress);
  pinMode(LH_LIGHT, OUTPUT);
  digitalWrite(LH_LIGHT, LOW);

  _PP(F(", RHswitch"));
  RHswitch.begin(RH_SWITCH_PIN, handleRHPress);
  pinMode(RH_LIGHT, OUTPUT);
  digitalWrite(RH_LIGHT, LOW);
  
  _PP(F(", Emergency"));
  pinMode(EMERGENCY_HEAD_LIGHT, OUTPUT);
  digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);

  _PP(F(", Buzzer"));
  pinMode(BUZZER_PIN, OUTPUT);
  tone(BUZZER_PIN, BEEP_FREQUENCY); delay(500);
  noTone(BUZZER_PIN);
  _PL(". Complete!");
}

void display_gear(void) {
  String gear = "DRIVE";
  if (neutral_sensor == 1) { gear="NEUTRAL"; } else gear = "DRIVE";
  displayTitle(gear);
  display.setTextSize(4);
  display.setTextColor(SSD1306_WHITE);
  display.getTextBounds(gear.substring(0,1), 0, 0, &x1, &y1, &width, &height);
  display.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT + 14 - height) / 2 ); 
  display.drawCircle(62, 39, 22, 1);
  display.print(gear.substring(0,1));
  display.display();
}


// ------------------------- LOOP ---------------------------
void loop() {
  LHswitch.check();  // check for left signal button presses
  RHswitch.check();  // check for right signal button presses
  neutral_sensor = digitalRead(NEUTRAL_SENSOR_PIN);
  if (millis() - lastBlink >= BLINK_INTERVAL) {
     lastBlink = millis();
     onCycle = !onCycle;
      if (!onCycle) {
        if (signal_state != State::NONE) {
        displayClear();
        }
        return;
      }
    // blink light
    switch (signal_state) {
      case State::NONE:
        display_gear();
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
   }
  // end of loop
};