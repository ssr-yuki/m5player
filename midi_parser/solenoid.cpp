#include "solenoid.h"
#define PIN_SDA (38)
#define PIN_SCL (39)

Solenoid::Solenoid() {
  Wire.end();
  Wire1.end();
  Wire1.begin(PIN_SDA, PIN_SCL, 10000);
};

void Solenoid::SetSolenoid(int id, int on_off) {
  if (id < 0 || id > SOLENOID_NUM) {
    return;
  }

  int ic_id = (int)(id / 8);
  solenoid_state[id] = on_off;
  uint8_t data = solenoid_state[0 + 8 * ic_id] << 0 |
                 solenoid_state[1 + 8 * ic_id] << 1 |
                 solenoid_state[2 + 8 * ic_id] << 2 |
                 solenoid_state[3 + 8 * ic_id] << 3 |
                 solenoid_state[4 + 8 * ic_id] << 4 |
                 solenoid_state[5 + 8 * ic_id] << 5 |
                 solenoid_state[6 + 8 * ic_id] << 6 |
                 solenoid_state[7 + 8 * ic_id] << 7;
  Wire1.beginTransmission(0x20 + ic_id);
  Wire1.write(data);
  Wire1.endTransmission();
}

void Solenoid::ResetSolenoid() {
  for (int i = 0; i < SOLENOID_NUM; i++) {
    SetSolenoid(i, 0);
  }
}
