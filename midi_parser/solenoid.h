#ifndef _SOLENOID_H_
#define _SOLENOID_H_
#include <Wire.h>

class Solenoid{
public:
  Solenoid();
  void ResetSolenoid();
  void SetSolenoid(int id,int on_off);
  const static int SOLENOID_NUM=40;
private:
  uint8_t solenoid_state[SOLENOID_NUM]={0};
  
};

#endif //_SOLENOID_H_