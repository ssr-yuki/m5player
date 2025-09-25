#include "solenoid.h"
#define PIN_SDA (38)
#define PIN_SCL (39)

Solenoid::Solenoid() {
  Wire.end();
  Wire1.end();
  Wire1.begin(PIN_SDA, PIN_SCL, 10000);
};

void Solenoid::SetSolenoid(int id, int on_off) {
  if (id < 0 || id >= SOLENOID_NUM) {
    return;
  }
  solenoid_state[id] = on_off;

  int unit_id = (int)(id / 12);
  int board_id;
  int board_addr;

  uint8_t data;

  if(id%12<5){
    board_id=unit_id*2;
    board_addr=0x20+unit_id;
    data =  solenoid_state[unit_id*12+0] << 0 |
            solenoid_state[unit_id*12+1] << 1 |
            solenoid_state[unit_id*12+2] << 2 |
            solenoid_state[unit_id*12+3] << 3 |
            solenoid_state[unit_id*12+4] << 4 ;
  }else{
    board_id=unit_id*2+1;
    board_addr=0x38+unit_id;
    data =  solenoid_state[unit_id*12+5] << 0 |
            solenoid_state[unit_id*12+6] << 1 |
            solenoid_state[unit_id*12+7] << 2 |
            solenoid_state[unit_id*12+8] << 3 |
            solenoid_state[unit_id*12+9] << 4 |
            solenoid_state[unit_id*12+10] << 5 |
            solenoid_state[unit_id*12+11] << 6 ;
  }

  Wire1.beginTransmission(board_addr);
  Wire1.write(data);
  Wire1.endTransmission();
}

void Solenoid::ResetSolenoid() {
  for (int i = 0; i < SOLENOID_NUM; i++) {
    SetSolenoid(i, 0);
  }
}
