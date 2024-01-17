//#include <SPI.h>
#include <Wire.h> 
#include <OneWire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <DallasTemperature.h>
#include "SwitchSignal.h"
#include "config.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RTC_DS1307 rtc;

void displayTextCenter(String text) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  // display on horizontal and vertical center
  display.clearDisplay(); // clear display
  display.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT - height) / 2);
  display.println(text); // text to display
  display.display();
}
void displayLeftSig(void) {
  display.clearDisplay();
  display.fillTriangle(62, 0, 0, 32, 62, 64, WHITE);
  display.display();
}

void displayRightSig(void) {
  display.clearDisplay();
  display.fillTriangle(66, 0, 128, 32, 66, 64, WHITE);
  display.display();
}

void displayEmergencySig(void) {
  display.clearDisplay();
  display.fillTriangle(62, 0, 0, 32, 62, 64, WHITE);
  display.fillTriangle(66, 0, 128, 32, 66, 64, WHITE);
  display.display();  
}

void displayClear(void) {
  display.clearDisplay();
  display.display();
}

enum class State {
  NONE,
  LH_DOWN,
  RH_DOWN,
  LH_LIGHT_ON,
  RH_LIGHT_ON,
  EMERGENCY
};

volatile State signal_state = State::NONE;
byte gear = 0;
SwitchSignal LHswitch;
SwitchSignal RHswitch;

void handleLHPress(const byte newState, const unsigned long interval, const byte whichPin)
{
  // switch down?
  if (newState == LOW) {
     switch (signal_state) {
       // if other switch down, switch to warning mode
       case State::RH_DOWN:
         signal_state = State::EMERGENCY;
         break;
       // if already on or warning signal, turn all off
       case State::LH_LIGHT_ON:
       case State::EMERGENCY:
         signal_state = State::NONE;
        break;
       // otherwise switch is now down, but not yet released
       default:
         signal_state = State::LH_DOWN;
         break;
       }  // end of switch
     return;
     }  // end of LH switch down
  // switch must be up
     if (signal_state == State::LH_DOWN) { // if down, switch to down-and-released mode
      signal_state = State::LH_LIGHT_ON;
     }
} // end of handleLHPress

void handleRHPress(const byte newState,
              const unsigned long interval,
              const byte whichPin)
{
     // switch down?
     if (newState == LOW) {
     switch(signal_state) {
       // if other switch down, switch to warning mode
       case State::LH_DOWN:
         signal_state = State::EMERGENCY;
         break;
       // if already on or warning signal, turn all off
       case State::RH_LIGHT_ON:
       case State::EMERGENCY:
         signal_state = State::NONE;
         break;
       // otherwise switch is now down, but not yet released
       default:
         signal_state = State::RH_DOWN;
         break;
       }  // end of switch
     return;
     }  // end of RH switch down
  // switch must be up
     if (signal_state == State::RH_DOWN) { // if down, switch to down-and-released mode
     signal_state = State::RH_LIGHT_ON;
     }
} // end of handleRHPress

unsigned long lastBlink;
bool onCycle;

void blinkLights() {
  lastBlink = millis();
  onCycle = !onCycle;
  // default to off
/*  if (signal_state == State::LH_LIGHT_ON) cleanLeftSig();
  if (signal_state == State::RH_LIGHT_ON) cleanRightSig();
  if (signal_state == State::EMERGENCY) cleanEmergency();
*/
  //lcd.clear();

  // digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LH_LIGHT, LOW);
  digitalWrite(RH_LIGHT, LOW);
  digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);
  noTone(BUZZER_PIN);
  // every second time, turn them all off
  if (!onCycle) {
     return;
  }
  // blink light
  switch (signal_state) {
    case State::NONE:
      break;
    case State::LH_DOWN:
    case State::LH_LIGHT_ON:
      displayLeftSig();
      //digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LH_LIGHT, HIGH);
      digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);
      tone(BUZZER_PIN, BEEP_FREQUENCY);
      break;
    case State::RH_DOWN:
    case State::RH_LIGHT_ON:
      displayRightSig();
      //digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(RH_LIGHT, HIGH);
      digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);
      tone(BUZZER_PIN, BEEP_FREQUENCY);
      break;
    case State::EMERGENCY:
      displayEmergencySig();
      //digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LH_LIGHT, HIGH);
      digitalWrite(RH_LIGHT, HIGH);
      digitalWrite(EMERGENCY_HEAD_LIGHT, HIGH);
      tone(BUZZER_PIN, BEEP_FREQUENCY);
      break;
    }  // end of switch on state
}  // end of blinkLights

void dashboard() {

}
// ------------------------- SETUP -------------------------
void setup() {
  Serial.begin (9600);

  // initialize OLED display with address 0x3C for 128x64
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  // displayClear();
    Serial.println(F("Loading..."));
    displayTextCenter("Loading...");
    delay(1000);
  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, LOW);
 
  LHswitch.begin(LH_SWITCH_PIN, handleLHPress);
  pinMode(LH_LIGHT, OUTPUT);
  digitalWrite(LH_LIGHT, LOW);


  RHswitch.begin(RH_SWITCH_PIN, handleRHPress);
  pinMode(RH_LIGHT, OUTPUT);
  digitalWrite(RH_LIGHT, LOW);

  
  pinMode(EMERGENCY_HEAD_LIGHT, OUTPUT);
  digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);


  pinMode(CONTROL_PANEL_LIGHT, OUTPUT);
  digitalWrite(CONTROL_PANEL_LIGHT, LOW);
  // for (int i=0; i<256; i++) {
  //   analogWrite(CONTROL_PANEL_LIGHT, i);
  //   if (i%10) { lcd.print('.'); }
  //   delay(5);
  // }
  
  pinMode(BUZZER_PIN, OUTPUT);
  tone(BUZZER_PIN, BEEP_FREQUENCY);
  delay(500);
  noTone(BUZZER_PIN);

  // lcd.setCursor(5,0);
  // lcd.print("DONE");

  //delay(5000);
 // digitalWrite(LED_BUILTIN, HIGH);

}  // end of setup


// ------------------------- LOOP ---------------------------
void loop() {
  LHswitch.check();  // check for left signal button presses
  RHswitch.check();  // check for right signal button presses

/*
  if (digitalRead(HORN_SWITCH_PIN) == HIGH) 
  {
    digitalWrite(HORN_SIGNAL, HIGH);
  } 
    else {
      digitalWrite(HORN_SIGNAL, LOW);
  }
*/
  if (millis() - lastBlink >= BLINK_INTERVAL) {
    blinkLights();
  }
}  // end of loop
