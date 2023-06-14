#include <Arduino.h>
#include <MsTimer2.h>

typedef enum { SIG_OFF=0, SIG_LEFT, SIG_RIGHT, SIG_EMERGENCY } state_t;
volatile state_t state = SIG_OFF;

#define LEFT_SIG_BUTTON_PIN        2
#define RIGHT_SIG_BUTTON_PIN       3
#define EMERGENCY_SIG_BUTTON_PIN   4
#define LEFT_SIG_PIN               5
#define RIGHT_SIG_PIN              6
#define HEAD_EMERGENCY_SIG_PIN     7

#define TIMEBUTTON 15
// Description of the button signal processing class
class Button {
  public:
    Button (byte pin, byte timeButton = TIMEBUTTON); // description of the constructor
    boolean flagPress; // flag - the button is pressed
    boolean flagClick; // flag - the button was clicked
    void scanState(); // method for checking the signal state
    void setPinTime(byte pin, byte timeButton = TIMEBUTTON); // method of setting the pin number and time (number) of state acknowledgments
  private:
    byte _buttonCount; // counter of button state acknowledgments
    byte _timeButton = TIMEBUTTON; // time of button state acknowledgments
    byte _pin; // pin number
};

void Button::scanState() {
  if ( flagPress == (! digitalRead(_pin)) ) {
    _buttonCount= 0;
  } else {
    _buttonCount++;
    if ( _buttonCount >= _timeButton ) {
      flagPress = ! flagPress;
      _buttonCount = 0;
      if (flagPress == true) flagClick = true;
    }
  }
}

// method of setting the pin number and time of acknowledgments
void Button::setPinTime(byte pin, byte timeButton = TIMEBUTTON) {
  _pin = pin;
  _timeButton = timeButton;
  pinMode(_pin, INPUT_PULLUP);
}

Button::Button(byte pin, byte timeButton = TIMEBUTTON) {
  _pin = pin;
  _timeButton = timeButton;
  pinMode(_pin, INPUT_PULLUP);
}


int button_pins[3] = { LEFT_SIG_BUTTON_PIN, RIGHT_SIG_BUTTON_PIN, EMERGENCY_SIG_BUTTON_PIN };
int signal_pins[3] = { LEFT_SIG_PIN, RIGHT_SIG_PIN, HEAD_EMERGENCY_SIG_PIN };

Button button_left_sig(LEFT_SIG_BUTTON_PIN);
Button button_right_sig(RIGHT_SIG_BUTTON_PIN);
Button button_emergency_sig(EMERGENCY_SIG_BUTTON_PIN);

void flash() {
  static boolean output = HIGH;
  switch (state) {
    case SIG_LEFT:
      digitalWrite(LEFT_SIG_PIN, output);
      digitalWrite(RIGHT_SIG_PIN, LOW);
      digitalWrite(HEAD_EMERGENCY_SIG_PIN, LOW);
      digitalWrite(LED_BUILTIN, output);
      break;
    case SIG_RIGHT:
      digitalWrite(RIGHT_SIG_PIN, output);
      digitalWrite(LEFT_SIG_PIN, LOW);
      digitalWrite(HEAD_EMERGENCY_SIG_PIN, LOW);
      digitalWrite(LED_BUILTIN, output);
      break;
    case SIG_EMERGENCY:;
      digitalWrite(LEFT_SIG_PIN, output);
      digitalWrite(RIGHT_SIG_PIN, output);
      digitalWrite(HEAD_EMERGENCY_SIG_PIN, output);
      digitalWrite(LED_BUILTIN, output);
      break;
    case SIG_OFF:
      digitalWrite(LEFT_SIG_PIN, LOW);
      digitalWrite(RIGHT_SIG_PIN, LOW);
      digitalWrite(HEAD_EMERGENCY_SIG_PIN, LOW);
      digitalWrite(LED_BUILTIN, LOW);
     break;
    }
  output = !output;
  }


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  for(int i = 0; i < sizeof(button_pins)/sizeof(int); i++) {
    pinMode(i, INPUT_PULLUP);
  }

  for(int i = 0; i < sizeof(signal_pins)/sizeof(int); i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  MsTimer2::set(500, flash); // 500ms period
  MsTimer2::start();

  digitalWrite(LED_BUILTIN, HIGH);
}x

void loop() {  // Turn EMERGENCY Signal
  if (button_left_sig.flagPress == true && button_right_sig.flagPress == true)
  {
    button_left_sig.flagPress = false;
    button_right_sig.flagPress = false;
    button_left_sig.flagClick = false;
    button_right_sig.flagClick = false;
    if (state == SIG_EMERGENCY ) {
      state = SIG_OFF;
    } else {
      state = SIG_EMERGENCY;
    }
  }  // Turn LEFT Signal
  else if (button_left_sig.flagClick == true && button_right_sig.flagClick == false) 
  {
    button_left_sig.flagClick = false;
    if (state != SIG_EMERGENCY) {
      if (state == SIG_LEFT) {
        state = SIG_OFF;
      } else {
        state = SIG_LEFT;
      }
    }
  }  // Turn RIGHT Signal
  else if (button_left_sig.flagClick == false && button_right_sig.flagClick == true) 
  {
    button_right_sig.flagClick = false;
    if (state != SIG_EMERGENCY) {
      if (state == SIG_RIGHT) {
        state = SIG_OFF;
      } else {
        state = SIG_RIGHT;
      }
    }
  }

}
