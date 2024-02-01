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
  unsigned long _switchPressTime;  // when the switch last changed state
  unsigned long _lastLowTime;
  unsigned long _lastHighTime;

  public:
     // constructor
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
      }  // end of begin

     void check() {
       // we need a valid pin number and a valid function to call
       if (_pin == noSwitch || _func == NULL)
         return;
        // see if switch is open or closed
        byte switchState = digitalRead(_pin);
        // has it changed since last time?
        if (switchState != _oldSwitchState) {
          // debounce
          if (millis() - _switchPressTime >= debounceTime) {
             _switchPressTime = millis();  // when we closed the switch
             _oldSwitchState =  switchState;  // remember for next time
             if (switchState == LOW) {
               _lastLowTime = _switchPressTime;
               _func(LOW, _lastLowTime -  _lastHighTime, _pin);
              } else {
               _lastHighTime = _switchPressTime;
               _func(HIGH, _lastHighTime - _lastLowTime, _pin);
              }
             }  // end if debounce time up
          }  // end of state change
       }  // end of operator ()
  };  // class SwitchSignal



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

void handleRHPress(const byte newState, const unsigned long interval, const byte whichPin)
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


#endif