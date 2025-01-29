#include <Arduino.h>
#ifdef DEBUG
#define _state Serial.println(getSignalState());
#else
#define _state
#endif

#define BLINK_INTERVAL      500
#define BEEP_FREQUENCY     1000
#define LH_SWITCH_PIN         2
#define RH_SWITCH_PIN         3
#define LH_LIGHT              4
#define RH_LIGHT              5
#define EMERGENCY_HEAD_LIGHT  6
#define BUZZER_PIN            7

enum class State {
  NONE,
  LH_DOWN,
  RH_DOWN,
  LH_LIGHT_ON,
  RH_LIGHT_ON,
  EMERGENCY
};
volatile State signal_state = State::NONE;

class SwitchSignal  {
  enum { debounceTime = 15, noSwitch = -1 };
  typedef void (*handlerFunction) (const byte newState, const unsigned long interval, const byte whichSwitch);
  int _pin;
  handlerFunction _func;
  byte _oldSwitchState;
  unsigned long _switchPressTime;
  unsigned long _lastLowTime;
  unsigned long _lastHighTime;
  public:
     SwitchSignal() {
       _pin = noSwitch;
       _func = NULL;
       _oldSwitchState  = HIGH;
       _switchPressTime = 0;
       _lastLowTime  = 0;
       _lastHighTime = 0;
      }
     void begin (const int pin, handlerFunction f) {
       _pin = pin;
       _func = f;
       if (_pin != noSwitch)
         pinMode(_pin, INPUT_PULLUP);
      }
     void check() {
       if (_pin == noSwitch || _func == NULL)
         return;
        byte switchState = digitalRead(_pin);
        if (switchState != _oldSwitchState) {
          if (millis() - _switchPressTime >= debounceTime) {
             _switchPressTime = millis();
             _oldSwitchState =  switchState;
             if (switchState == LOW) { 
                _lastLowTime = _switchPressTime;
                _func(LOW, _lastLowTime - _lastHighTime, _pin);
               } else {
                _lastHighTime = _switchPressTime;
                _func(HIGH, _lastHighTime - _lastLowTime, _pin);
               }
             }
          }
       } 
  }; 
void handleLHPress(const byte newState, const unsigned long interval, const byte whichPin) {
  if (newState == LOW) {
     switch (signal_state) {
       case State::RH_DOWN:
         signal_state = State::EMERGENCY;
         break;
       case State::LH_LIGHT_ON:
       case State::EMERGENCY:
         signal_state = State::NONE;
        break;
       default:
         signal_state = State::LH_DOWN;
         break;
       }
     return;
  }
  if (signal_state == State::LH_DOWN) signal_state = State::LH_LIGHT_ON;
  _state;
}
void handleRHPress(const byte newState, const unsigned long interval, const byte whichPin) {
  if (newState == LOW) {
     switch(signal_state) {
       case State::LH_DOWN:
         signal_state = State::EMERGENCY;
         break;
       case State::RH_LIGHT_ON:
       case State::EMERGENCY:
         signal_state = State::NONE;
         break;
       default:
         signal_state = State::RH_DOWN;
         break;
       }
     return;
  }
  if (signal_state == State::RH_DOWN) signal_state = State::RH_LIGHT_ON;
  _state;
}

SwitchSignal LHswitch;
SwitchSignal RHswitch;
unsigned long lastBlink;
bool onCycle;

void setup() {
  #if defined(_DEBUG_) || defined(TEST)
    Serial.begin(9600);
  #endif
  pinMode(LED_BUILTIN, OUTPUT);  digitalWrite(LED_BUILTIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);  tone(BUZZER_PIN, BEEP_FREQUENCY); 
  LHswitch.begin(LH_SWITCH_PIN, handleLHPress);
  RHswitch.begin(RH_SWITCH_PIN, handleRHPress);
  pinMode(LH_LIGHT, OUTPUT);  digitalWrite(LH_LIGHT, LOW);
  pinMode(RH_LIGHT, OUTPUT);  digitalWrite(RH_LIGHT, LOW);
  pinMode(EMERGENCY_HEAD_LIGHT, OUTPUT);  digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);
  delay(500);
  noTone(BUZZER_PIN);
  attachInterrupt(digitalPinToInterrupt(LH_SWITCH_PIN), L_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RH_SWITCH_PIN), R_ISR, CHANGE);
 _state;
}
void L_ISR() {
  LHswitch.check();
  setSignalState();
}
void R_ISR() {
  RHswitch.check();
  setSignalState();
}
byte getSignalState() {
   byte code;
   switch (signal_state) {
      case State::NONE:
        code=0;
        break;
      case State::LH_LIGHT_ON:
        code=1;
        break;
      case State::RH_LIGHT_ON:
        code=2;
        break;
      case State::EMERGENCY:
        code=3;
        break;
   }
   return code;
}
void setSignalState()
{
   switch (signal_state) {
      case State::NONE:
        digitalWrite(LH_LIGHT, LOW);
        digitalWrite(RH_LIGHT, LOW);
        digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);
        digitalWrite(LED_BUILTIN, LOW);
        noTone(BUZZER_PIN);
        break;
      case State::LH_LIGHT_ON:
        digitalWrite(LH_LIGHT, HIGH);
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(RH_LIGHT, LOW);
        digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);
        tone(BUZZER_PIN, BEEP_FREQUENCY);
        break;
      case State::RH_LIGHT_ON:
        digitalWrite(RH_LIGHT, HIGH);
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(LH_LIGHT, LOW);
        digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);
        tone(BUZZER_PIN, BEEP_FREQUENCY);
        break;
      case State::EMERGENCY:
        digitalWrite(LH_LIGHT, HIGH);
        digitalWrite(RH_LIGHT, HIGH);
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(EMERGENCY_HEAD_LIGHT, HIGH);        
        tone(BUZZER_PIN, BEEP_FREQUENCY);
        break;
    }
}
void loop() {
  if (millis() - lastBlink >= BLINK_INTERVAL) {
    lastBlink = millis();
    onCycle = !onCycle;
    if (!onCycle) {
      digitalWrite(LED_BUILTIN, LOW);
      noTone(BUZZER_PIN);
      return;
    } else {
      if (signal_state != State::NONE) {
         tone(BUZZER_PIN, BEEP_FREQUENCY);
      }
    }
  }
};
