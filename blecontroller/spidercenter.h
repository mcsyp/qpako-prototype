#ifndef SPIDERCENTER_H
#define SPIDERCENTER_H

#include <QObject>
#include <QTimer>
#include "singlescanner.h"

#define SPIDER_UUID "19b10000-e8f2-537e-4f6c-d10476ab9339"
#define SPIDER_UUID_JOY "19b10000-e8f2-537e-4f6c-d10476ab9340"
#define SPIDER_UUID_STEP "19b10000-e8f2-537e-4f6c-d10476ab9341"
#define SPIDER_UUID_AUTOPILOT "19b10000-e8f2-537e-4f6c-d10476ab9349"
#define SPIDER_UUID_MADGWIDCK "19b10000-e8f2-537e-4f6c-d10476ab9342"

#define SPIDER_JOYSTICK_TRIGGER 5 //trigger bigger than 5;
#define SPIDER_JOYSTICK_RANGE 127 //move the value within this range
#define SPIDER_JOYSTICK_UPDATE 60 //update joystick value every 60ms;

#define SPIDER_AUTOPILOT_MIN_RADIUS 200

class MyQmlController : public SingleScanner
{
    Q_OBJECT
public:
    explicit MyQmlController(QObject *parent = 0);

    Q_INVOKABLE void setJoystickPosition(float x,float y);
    Q_INVOKABLE void setAutopilotTarget(float degree,int distance);
    Q_INVOKABLE int getAutopilotMinRadius(){return SPIDER_AUTOPILOT_MIN_RADIUS;}
    virtual void processCharacteristicValueUpdated(const QLowEnergyCharacteristic &c, const QByteArray &value);

protected:
    virtual QLowEnergyService* checkService();
    virtual bool checkServiceDetailDiscovered(QLowEnergyService *service);

signals:
    void stepCounterUpdated(int step,int delta);
    void headingUpdated(short heading);

public slots:
    void onTimeoutJoystickUpdate();
private:
    bool m_updateJoystick;
    unsigned char m_lastJoyX;//last joysitck position X
    unsigned char m_lastJoyY;//last joystick position Y
    QLowEnergyCharacteristic m_charJoystick;//read|write characteristic
    QLowEnergyCharacteristic m_charStepCounter;//read|notify sensor data
    QLowEnergyCharacteristic m_charHeading;//read|notify
    QLowEnergyCharacteristic m_charAutopilot;//read|write characteristic
    QTimer *m_pstJoystickTimer;


    float m_heading;
    long m_step;
    int m_dstRadius;//distance from current to the target.
    int m_dstDegree;//degree of the target
};

#endif // SPIDERCENTER_H
