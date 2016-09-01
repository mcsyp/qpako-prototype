#ifndef _SPIDER_CTL_H_
#define _SPIDER_CTL_H_

#include "pin.h"

class SpiderMotor{
public:
  SpiderMotor();
  void walk(int pwm);
  void rotate(int pwm);

  void stopWalking();
  void stopRotating();

  void brakeWaking();
  void brakeRotating();

  int dir(){return m_dir;}
private:
  int m_dir;
};

#endif







