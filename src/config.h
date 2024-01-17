// Flasher Blink delay
#define BLINK_INTERVAL 500
#define BEEP_FREQUENCY 1000

// D12 Buzzer connection pin
#define BUZZER_PIN 12
// Emergency Signal head light pin
#define EMERGENCY_HEAD_LIGHT 13 // A1
// Left Signal switch pin & Left Signal light pin
#define LH_SWITCH_PIN 7
#define LH_LIGHT 9
// Right Signal switch pin & Right Signal light pin
#define RH_SWITCH_PIN 8
#define RH_LIGHT 10

#define CONTROL_PANEL_LIGHT 11

#define Set_SIG_NONE() digitalWrite(RH_LIGHT, LOW);digitalWrite(LH_LIGHT, LOW);digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);
#define Set_SIG_LEFT() digitalWrite(RH_LIGHT, LOW);digitalWrite(LH_LIGHT, HIGH);digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);
#define Set_SIG_RIGHT() digitalWrite(RH_LIGHT, HIGH);digitalWrite(LH_LIGHT, LOW);digitalWrite(EMERGENCY_HEAD_LIGHT, LOW);
#define Set_SIG_EMERGENCY() digitalWrite(RH_LIGHT, HIGH);digitalWrite(LH_LIGHT, HIGH);digitalWrite(EMERGENCY_HEAD_LIGHT, HIGH;

byte _leftTop[8] = {
  0x00,
  0x00,
  0x00,
  0x01,
  0x03,
  0x07,
  0x0F,
  0x1F
};

byte _leftBottom[8] = {
  0x1F,
  0x0F,
  0x07,
  0x03,
  0x01,
  0x00,
  0x00,
  0x00
};

byte _rightTop[8] = {
  0x00,
  0x00,
  0x00,
  0x10,
  0x18,
  0x1C,
  0x1E,
  0x1F
};
byte _rightBottom[8] = {
  0x1F,
  0x1E,
  0x1C,
  0x18,
  0x10,
  0x00,
  0x00,
  0x00
};

byte _baseTop[8] = {
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x1F,
  0x1F,
  0x1F
};


byte _baseBottom[8] = {
  0x1F,
  0x1F,
  0x1F,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00
};