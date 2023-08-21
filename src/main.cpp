#include <Arduino.h>
#include "SwitchSignal.h"

#define BLINK_INTERVAL  500
// Left Signal switch
#define LH_SWITCH_PIN   6
#define LH_LIGHT        7
// Right Signal switch
#define RH_SWITCH_PIN   8
#define RH_LIGHT        9
// Beam light switch
#define BEAM_SWITCH_PIN 4
// D5 PWM pin: on Timer0 at 976Hz for beam light
#define BEAM_LIGHT      5

//#define BRAKE_SWITCH_PIN 7 // Brake sensor switch pin
//#define BRAKE_LIGHT 11

typedef enum {
    NONE,
    LH_DOWN,
    RH_DOWN,
    LH_LIGHT_ON,
    RH_LIGHT_ON,
    BOTH
} signal_state_t;

typedef enum {
    BEAM_LIGHT_OFF ,
    BEAM_LIGHT_LOW,
    BEAM_LIGHT_HIGH
} beam_state_t;

signal_state_t state = NONE;
beam_state_t beam_state = BEAM_LIGHT_OFF;

byte beam_val = 0;

SwitchSignal LHswitch;
SwitchSignal RHswitch;
SwitchSignal BMswitch;

void handleBMPress(const byte newState, const unsigned long interval, const byte whichPin) {
  if (newState == LOW) {
     switch (beam_state) {
        // if beam was off, turn on beam low
        case BEAM_LIGHT_OFF:
          beam_state = BEAM_LIGHT_LOW;
          analogWrite(BEAM_LIGHT, 128);
          break;
        // if beam low was on, turn on beam high 
        case BEAM_LIGHT_LOW:
          beam_state = BEAM_LIGHT_HIGH;
          analogWrite(BEAM_LIGHT, 255);
          break;
        // if beam high was on, turn beam off
        case BEAM_LIGHT_HIGH:
          beam_state = BEAM_LIGHT_OFF;
          analogWrite(BEAM_LIGHT, 0);
          break;
     }
     return;
  }
}

void handleLHPress(const byte newState, const unsigned long interval, const byte whichPin) {
  // switch down?
  if (newState == LOW) {
     switch (state) {
       // if other switch down, switch to warning mode
       case RH_DOWN:
         state = BOTH;
         break;
       // if already on or warning signal, turn all off
       case LH_LIGHT_ON:
       case BOTH:
         state = NONE;
         break;
       // otherwise switch is now down, but not yet released
       default:
         state = LH_DOWN;
         break;
       }  // end of switch
     return;
     }  // end of LH switch down
  // switch must be up
  if (state == LH_DOWN)  // if down, switch to down-and-released mode
    state = LH_LIGHT_ON;
}  // end of handleLHPress


void handleRHPress(const byte newState, const unsigned long interval, const byte whichPin) {
  // switch down?
  if (newState == LOW) {
     switch(state) {
       // if other switch down, switch to warning mode
       case LH_DOWN:
         state = BOTH;
         break;
       // if already on or warning signal, turn all off
       case RH_LIGHT_ON:
       case BOTH:
         state = NONE;
         break;
       // otherwise switch is now down, but not yet released
       default:
         state = RH_DOWN;
         break;
       }  // end of switch
     return;
     }  // end of RH switch down
  // switch must be up
  if (state == RH_DOWN)  // if down, switch to down-and-released mode
    state = RH_LIGHT_ON;
}  // end of handleRHPress

unsigned long lastBlink;
bool onCycle;

void blinkLights() {
  lastBlink = millis();
  onCycle = !onCycle;
  // default to off
  digitalWrite(LH_LIGHT, LOW);
  digitalWrite(RH_LIGHT, LOW);
  // every second time, turn them all off
  if (!onCycle)
    return;
  // blink light
  switch (state) {
    case NONE:
      break;
    case LH_DOWN:
    case LH_LIGHT_ON:
      digitalWrite(LH_LIGHT, HIGH);
      break;
    case RH_DOWN:
    case RH_LIGHT_ON:
      digitalWrite(RH_LIGHT, HIGH);
      break;
    case BOTH:
      digitalWrite(LH_LIGHT, HIGH);
      digitalWrite(RH_LIGHT, HIGH);
      break;
    }  // end of switch on state
}  // end of blinkLights

// ------------------------- SETUP -------------------------
void setup() {
  LHswitch.begin(LH_SWITCH_PIN, handleLHPress);
  pinMode(LH_LIGHT, OUTPUT);
  
  RHswitch.begin(RH_SWITCH_PIN, handleRHPress);
  pinMode(RH_LIGHT, OUTPUT);

  BMswitch.begin(BEAM_SWITCH_PIN, handleBMPress);
  pinMode(BEAM_LIGHT, OUTPUT);
  
  //pinMode(BRAKE_SWITCH_PIN, INPUT_PULLUP);
  //pinMode(BRAKE_LIGHT, OUTPUT);
}  // end of setup

// ------------------------- LOOP ---------------------------
void loop() {
  LHswitch.check();  // check for left signal button presses
  RHswitch.check();  // check for right signal button presses
  BMswitch.check();  // check for beam light button presses
  /* if (digitalRead(BRAKE_SWITCH_PIN) == LOW)
    digitalWrite(BRAKE_LIGHT, HIGH);
  else
    digitalWrite(BRAKE_LIGHT, LOW);
  */
  if (millis() - lastBlink >= BLINK_INTERVAL)
    blinkLights();
}  // end of loop
