#include <BLEAttribute.h>
#include <BLECentral.h>
#include <BLECharacteristic.h>
#include <BLECommon.h>
#include <BLEDescriptor.h>
#include <BLEPeripheral.h>
#include <BLEService.h>
#include <BLETypedCharacteristic.h>
#include <BLETypedCharacteristics.h>
#include <BLEUuid.h>
#include <CurieBLE.h>
#include <CurieIMU.h>

#include "pin.h"
#include "SpiderMotor.h"
#include "task.h"
#include "math.h"


#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"


#define M_PI 3.1415926f


MPU6050 mpu;
SpiderMotor motor;
enum TASK_ID {
	TASK_JOYSTICK=0,
	TASK_STEPCOUNT=1,
	TASK_HEADING=2,
  TASK_AUTOPILOT,
	TASK_NUM
};
Task g_task[TASK_NUM];

/****************************
 * BLE related section START
 ****************************/
#define SPIDER_BLE_NAME "QPako"
#define SPIDER_UUID "19B10000-E8F2-537E-4F6C-D10476AB9339"
#define SPIDER_UUID_JOY "19B10000-E8F2-537E-4F6C-D10476AB9340"
#define SPIDER_UUID_STEP "19B10000-E8F2-537E-4F6C-D10476AB9341"
#define SPIDER_UUID_HEADING "19B10000-E8F2-537E-4F6C-D10476AB9342"
#define SPIDER_UUID_AUTOPILOT "19B10000-E8F2-537E-4F6C-D10476AB9349"
#define SPIDER_MAX_JOYSTICK_CHAR 4
#define SPIDER_MAX_IMU_CHAR 12
#define SPIDER_MAX_AUTOPILOT_CHAR 6
BLEPeripheral blePeripheral;
BLEService spiderService(SPIDER_UUID);
BLECharacteristic joystickChar(SPIDER_UUID_JOY,BLERead | BLEWrite,SPIDER_MAX_AUTOPILOT_CHAR);
BLECharacteristic autopilotChar(SPIDER_UUID_AUTOPILOT,BLERead | BLEWrite,8);
BLELongCharacteristic stepCounterChar(SPIDER_UUID_STEP,BLERead | BLENotify);
BLEFloatCharacteristic headingChar(SPIDER_UUID_HEADING,BLERead | BLENotify);
/****************************
 * BLE related section END
 ****************************/
 
#define TASK_INTERVAL_JOYSTICK 50000
void taskReceiveJoystickMessage(){
  const unsigned char *rx_buffer;
  int forward;
  int rotate;
  short rx_len=0;
  if(joystickChar.written()){
    rx_buffer = joystickChar.value();         
	  //TODO: process the joystick code here.
    rotate = ((int)rx_buffer[0]-127)*2;
    //Serial.print("rotate:");Serial.println(rotate);
    forward = ((int)rx_buffer[1]-127)*2;
    //Serial.print("forward:");Serial.println(forward);
    //process joystick control
    if(forward){
      motor.walk(forward);
    }else{
      motor.stopWalking();
    }
    if(rotate){
      motor.rotate(rotate);
    }else{
      motor.stopRotating();
    }
  }
  
}
#define TASK_INTERVAL_STEPCOUNTER 33000
static long g_lastStep=0;
static long g_stepCounter=0;
void taskStepCounter()
{
  long d=0;
  if(g_stepCounter!=g_lastStep){
    d = motor.dir()*(g_stepCounter - g_lastStep);
    stepCounterChar.setValue(d);
    g_lastStep= g_stepCounter;
  }
}
void isr_step_count(){
  g_stepCounter+=1;
}

/*DMP related TASK*/
#define TASK_INTERVAL_HEADING 100000 //100ms
#define HEADING_THRESHOLD_MIN 1.5
#define HEADING_THRESHOLD_MAX 25
#if (I2CDEV_IMPLEMENTATION==I2CDEV_SOFTWARE)
void soft_i2c_delay(void) {
	delayMicroseconds(5);
}
void soft_i2c_sda_mode(uint8_t value) {
	if (value) {
		pinMode(SOFT_I2C_SDA, OUTPUT);
	}
	else {
		pinMode(SOFT_I2C_SDA, INPUT);
	}
}
void soft_i2c_sda_write(uint8_t value) {
	digitalWrite(SOFT_I2C_SDA, value);
}
uint8_t soft_i2c_sda_read(void) {
	return digitalRead(SOFT_I2C_SDA);
}
void soft_i2c_scl_write(uint8_t value) {
	digitalWrite(SOFT_I2C_SCL, value);
}

#endif

bool g_dmpReady = false;
int g_packetSize = 0;
uint16_t g_fifoCount=0;     // count of all bytes currently in FIFO
uint8_t g_fifoBuffer[64]; // FIFO storage buffer
Quaternion g_q;
VectorFloat g_gravity;
float g_ypr[3];
float g_heading;
float g_lastHeading=0;
bool g_mpuInterrupt = false;
void taskHeadingUpdate() {
  headingChar.setValue(g_heading);
  //Serial.print("heading:");Serial.println(g_heading);
  #if 0
  float d = abs(g_heading - g_lastHeading);
  //Serial.print(" delta:"); Serial.print(d);Serial.print(" heading:"); Serial.println(g_heading);
	if (d>HEADING_THRESHOLD_MIN) {
		headingChar.setValue(g_heading);
    g_lastHeading = g_heading;
		//Serial.println(" updated");
	}
 #endif
}
void dmpDataReady()
{
	g_mpuInterrupt = true;
}

#define TASK_INTERVAL_BLE_CENTRAL 100000 //100 ms
bool g_bleConnected=false;
Task g_bleCentralCheckTask ;
void taskBleCentralCheck(){
	g_bleConnected = false;
	BLECentral central = blePeripheral.central();
	if (central && central.connected()) {
		g_bleConnected = true;
	}
};

#define TASK_INTERVAL_AUTOPILOT 40000 //40ms 25hz
long g_autopilotDegree=0;
void taskAutopilot(){
  const unsigned char *rx_buffer;
  unsigned char * p = (unsigned char*)(&g_autopilotDegree);
  unsigned char isRunning=0;
  //update the dst degree
  if(autopilotChar.written()){
    rx_buffer = autopilotChar.value();
    p[0] = rx_buffer[0];
    p[1] = rx_buffer[1];
    p[2] = rx_buffer[2]; 
    p[3] = rx_buffer[3]; 
    isRunning = rx_buffer[4];
    //Serial.print("dstDegree:");Serial.println(g_autopilotDegree);
    if(!isRunning){
      motor.stopRotating();
      motor.stopWalking();
    }
  }
  //TODO:to rotate to the target degree
  if(isRunning){
    //calc the dist ar
    int heading = ((int)g_heading+720)%360;
    float arc = (g_autopilotDegree-heading)*3.1415f/180.0f;
    double rotate = sin(arc)*1.1f;
    double forward = cos(arc)*1.2f;
  
    if(rotate>1.0f){rotate=1.0f;}
    if(rotate<-1.0f){rotate=-1.0f;}
    if(forward>1.0f){forward=1.0f;}
    if(forward<-1.0f){forward=-1.0f;}
    
    if(forward){
      motor.walk(forward*255);
    }else{
      motor.stopWalking();
    }
    if(rotate){
      motor.rotate(rotate*255);
    }else{
      motor.stopRotating();
    }
  }
}
void setup() {
  Serial.begin(9600);
  Serial.println("Hello world!");

  blePeripheral.setLocalName(SPIDER_BLE_NAME);
  blePeripheral.setAdvertisedServiceUuid(spiderService.uuid());

  //add service and characteristic
  blePeripheral.addAttribute(spiderService);
  blePeripheral.addAttribute(autopilotChar);
  blePeripheral.addAttribute(joystickChar);
  blePeripheral.addAttribute(stepCounterChar);
  blePeripheral.addAttribute(headingChar);

  stepCounterChar.setValue(0);
  blePeripheral.begin();
  

  //setup task
  g_task[TASK_JOYSTICK].init(TASK_INTERVAL_JOYSTICK,taskReceiveJoystickMessage);
  g_task[TASK_STEPCOUNT].init(TASK_INTERVAL_STEPCOUNTER,taskStepCounter);
  g_task[TASK_HEADING].init(TASK_INTERVAL_HEADING, taskHeadingUpdate);
  g_task[TASK_AUTOPILOT].init(TASK_INTERVAL_AUTOPILOT,taskAutopilot);
  g_bleCentralCheckTask.init(TASK_INTERVAL_BLE_CENTRAL, taskBleCentralCheck);
  
  //init the dmp6050
#if I2CDEV_IMPLEMENTATION == I2CDEV_SOFTWARE
  pinMode(SOFT_I2C_SDA, OUTPUT);
  pinMode(SOFT_I2C_SCL, OUTPUT);
  soft_i2c_init();
#endif
  do{
	  mpu.initialize();
	  // load and configure the DMP
	  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

	  Serial.println(F("Initializing DMP..."));
	  if (mpu.dmpInitialize()) {
			g_dmpReady = false;
			break;
	  }
	  g_dmpReady = true;
	  
	  // supply your own gyro offsets here, scaled for min sensitivit
	  mpu.setXGyroOffset(220);
	  mpu.setYGyroOffset(76);
	  mpu.setZGyroOffset(-85);
	  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip
	  //mpu.setRate(9);//50hz
	  if (g_dmpReady) {
		  // turn on the DMP, now that it's ready
		  Serial.println(F("Enabling DMP..."));
		  mpu.setDMPEnabled(true);

		  attachInterrupt(MPU6050_INT_RDY, dmpDataReady, RISING);
		  mpu.getIntStatus();
  
		  // set our DMP Ready flag so the main loop() function knows it's okay to use it
		  Serial.println(F("DMP ready! Waiting for first interrupt..."));
		 
		  // get expected DMP packet size for later comparison
		  g_packetSize = mpu.dmpGetFIFOPacketSize();
		}
  }while(0);

  //init hall sensor 
  do{
    pinMode(STEP_HALL_INT,INPUT);
    attachInterrupt(STEP_HALL_INT,isr_step_count,FALLING);
  }while(0);
}

void schedule_tasks() {
	int i = 0;
	unsigned long us = micros();
	for (i = 0; i<TASK_NUM; i++) {
		g_task[i].trigger(us);
	}
}
void schedule_dmp() {
	//while (!g_mpuInterrupt && g_fifoCount < g_packetSize) {
	//}
	// reset interrupt flag and get INT_STATUS byte
	g_mpuInterrupt = false;
    uint8_t mpuIntStatus = mpu.getIntStatus();

	// get current FIFO count
	g_fifoCount = mpu.getFIFOCount();

	// check for overflow (this should never happen unless our code is too inefficient)
	if ((mpuIntStatus & 0x10) || g_fifoCount == 1024) {
		// reset so we can continue cleanly
		mpu.resetFIFO();
		//Serial.println(F("FIFO overflow!"));

		// otherwise, check for DMP data ready interrupt (this should happen frequently)
	}
	else if (mpuIntStatus & 0x02) {
		
		// wait for correct available data length, should be a VERY short wait
		while (g_fifoCount < g_packetSize) g_fifoCount = mpu.getFIFOCount();

		// read a packet from FIFO
		mpu.getFIFOBytes(g_fifoBuffer, g_packetSize);

		// track FIFO count here in case there is > 1 packet available
		// (this lets us immediately read more without waiting for an interrupt)
		g_fifoCount -= g_packetSize;


		// display Euler angles in degrees
		mpu.dmpGetQuaternion(&g_q, g_fifoBuffer);
		mpu.dmpGetGravity(&g_gravity, &g_q);
		mpu.dmpGetYawPitchRoll(g_ypr, &g_q, &g_gravity);
		g_heading = g_ypr[0] * 180 / M_PI;
    //Serial.print("heading:");Serial.println(g_heading);
	}
}

void loop() {
	//Serial.println("hello");
#if 1
  do {
	  //check the dmp result
	  schedule_dmp();

	  //check the tasks
    g_bleCentralCheckTask.trigger(micros());
	  if (g_bleConnected) {
		  schedule_tasks();
	  }
  } while (g_bleConnected);  
#else
  schedule_dmp();
  //delay(40);
#endif
}







