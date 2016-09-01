#include  "SpiderMotor.h"
#include <arduino.h>

SpiderMotor::SpiderMotor(){
  pinMode(MOTOR_F_1,OUTPUT);
  pinMode(MOTOR_F_2,OUTPUT);
  pinMode(MOTOR_F_PWM,OUTPUT);

  pinMode(MOTOR_R_1,OUTPUT);
  pinMode(MOTOR_R_2,OUTPUT);
  pinMode(MOTOR_R_PWM,OUTPUT);

  stopWalking();
  stopRotating();

  m_dir=0;
}
void SpiderMotor::walk(int pwm){
  int v = abs(pwm);
  if(v>255) return;

  if(pwm>0){
    digitalWrite(MOTOR_F_1,1);
    digitalWrite(MOTOR_F_2,0);
  }else if(pwm<0){
    digitalWrite(MOTOR_F_1,0);
    digitalWrite(MOTOR_F_2,1);
  }else{//pwm=0;
    stopWalking();
  }
  //write speed
  analogWrite(MOTOR_F_PWM,v);
  if(pwm>0){
    m_dir =1;
  }else if(pwm<0){
    m_dir=-1;
  }else{
    m_dir=0;
  }
}

void SpiderMotor::rotate(int pwm){
   int v = abs(pwm);
  if(v>255) return;

  if(pwm>0){
    digitalWrite(MOTOR_R_1,1);
    digitalWrite(MOTOR_R_2,0);
  }else if(pwm<0){
    digitalWrite(MOTOR_R_1,0);
    digitalWrite(MOTOR_R_2,1);
  }else{//pwm=0;
    stopRotating();
  }
  //write speed
  analogWrite(MOTOR_R_PWM,v);
}
void SpiderMotor::stopWalking(){
  digitalWrite(MOTOR_F_1,0);
  digitalWrite(MOTOR_F_2,0);
  digitalWrite(MOTOR_F_PWM,0);
  m_dir=0;
}
void SpiderMotor::stopRotating(){  
  digitalWrite(MOTOR_R_1,0);
  digitalWrite(MOTOR_R_2,0);
  digitalWrite(MOTOR_R_PWM,0);
}

void SpiderMotor::brakeWaking(){
  digitalWrite(MOTOR_F_PWM,0);
  digitalWrite(MOTOR_F_1,1);
  digitalWrite(MOTOR_F_2,1);
   m_dir=0;
}
void SpiderMotor::brakeRotating(){
  analogWrite(MOTOR_R_PWM,0);
  digitalWrite(MOTOR_R_1,1);
  digitalWrite(MOTOR_R_2,1);
}







