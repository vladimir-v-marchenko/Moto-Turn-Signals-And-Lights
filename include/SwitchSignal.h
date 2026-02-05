#ifndef SwitchSignal_h
#define SwitchSignal_h

#include <Arduino.h>

class SwitchSignal  {
  enum { debounceTime = 15, noSwitch = -1 };
  typedef void (*handlerFunction) (const byte newState,
                                   const unsigned long interval,
                                   const byte whichSwitch);

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


void handleLHPress(const byte newState, const unsigned long interval, const byte whichPin)
{
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
}

void handleRHPress(const byte newState, 
                   const unsigned long interval, 
                   const byte whichPin)
{
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
} 

#endif