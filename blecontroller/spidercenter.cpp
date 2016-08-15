#include "spidercenter.h"
#include <math.h>

MyQmlController::MyQmlController(QObject *parent) : SingleScanner(parent)
{
    m_lastJoyX=0;
    m_lastJoyY=0;
    m_updateJoystick=false;

    m_pstJoystickTimer = new QTimer();
    connect(m_pstJoystickTimer,SIGNAL(timeout()),
            this,SLOT(onTimeoutJoystickUpdate()));
}

void MyQmlController::setJoystickPosition(float x, float y){
    unsigned char value_x = SPIDER_JOYSTICK_RANGE*(x+1.0f);
    unsigned char value_y = SPIDER_JOYSTICK_RANGE*(y+1.0f);
    m_updateJoystick = false;
    //qDebug()<<tr("float pos:%1,%2").arg(x).arg(y);
    //qDebug()<<tr("char pos:%1,%2").arg((int)value_x).arg((int)value_y);
    if(abs(value_x-m_lastJoyX)>=SPIDER_JOYSTICK_TRIGGER){
        m_lastJoyX = value_x;
        m_updateJoystick=true;
    }
    if(abs(value_y-m_lastJoyY)>=SPIDER_JOYSTICK_TRIGGER){
        m_lastJoyY = value_y;
        m_updateJoystick=true;
    }
}
void MyQmlController::onTimeoutJoystickUpdate()
{
    if(m_updateJoystick){
        //qDebug()<<tr("pos:%1,%2").arg((int)m_lastJoyX).arg((int)m_lastJoyY);
        QByteArray buffer;
        buffer.append(m_lastJoyX);
        buffer.append(m_lastJoyY);
        QLowEnergyService* service = getCurrentService();
        if(service ){//write to joystick
            service->writeCharacteristic(m_charJoystick,buffer);
        }
        m_updateJoystick =false;
    }
}
QLowEnergyService* MyQmlController::checkService(){
    QLowEnergyService* service =  getService(SPIDER_UUID);
    if(service==NULL){
        qDebug()<<tr("FUCKKKKKK!");
    }
    return service;
}

bool MyQmlController::checkServiceDetailDiscovered(QLowEnergyService *service){
    bool ret = true;
    if(!service){
        return false;
    }

    //get characteristic list
    QList<QLowEnergyCharacteristic> list =  service->characteristics();
    if(list.isEmpty()){
        return false;
    }

    do {
        qDebug()<<tr("%1 characteristics found.").arg(list.size());
        for(int i=0;i<list.size();i++){
            QLowEnergyCharacteristic c = list.at(i);
            if(!c.isValid()){
                qDebug()<<tr("characteristic_[%1] is not valid.").arg(i);
                continue;
            }

            QString id = uuidToString(c.uuid());
            qDebug()<<tr("characteristic_[%1] uuid is %2.").arg(i).arg(id);
            if(id==QString(SPIDER_UUID_JOY)){
                /**
                 * joystick
                 **/
                qDebug()<<tr("joystick character is found.");
                m_charJoystick = c;
                m_pstJoystickTimer->start(SPIDER_JOYSTICK_UPDATE);
                //nothing else to do
            }
            if(id==QString(SPIDER_UUID_AUTOPILOT)){
                /**
                 * autopilot
                 **/
                qDebug()<<tr("autopilot character is found.");
                m_charAutopilot = c;
            }
            if(id==QString(SPIDER_UUID_STEP)){
                /**
                 * step counter
                 **/
                qDebug()<<tr("step counter character is found.");
                m_charStepCounter = c;
                //enable the notify;
                enableCharacteristicNotify(m_charStepCounter,true);
            }
            if(id==QString(SPIDER_UUID_MADGWIDCK)){
                /**
                 * madgwick filter
                 **/
                qDebug()<<tr("madgwick counter character is found.");
                m_charHeading = c;
                //enable the notify;
                enableCharacteristicNotify(m_charHeading,true);
            }
        }

        //init property
        m_heading = 0.0f;
        m_step = 0;
    }while(0);



    return ret;
}


void MyQmlController::processCharacteristicValueUpdated(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    if(c==m_charStepCounter){
        long counter;
        char * p = (char*)&counter;
        memcpy(p,value.constData(),sizeof(long));
        //qDebug()<<tr("step:%1").arg(counter);
        m_step += counter;
        emit stepCounterUpdated(m_step,counter);
    }

    if(c==m_charHeading){
        float heading;
        char * p = (char*)&heading;
        memcpy(p,value.constData(),sizeof(heading));

#if 0
        qDebug()<<tr("heading:%1").arg(heading);
#endif
        if(abs(heading-m_heading)>1.0){
            m_heading = heading;
            emit headingUpdated(heading);
        }
    }
}


void MyQmlController::setAutopilotTarget(float degree, int distance)
{
    m_dstDegree = degree;
    m_dstRadius = distance;

    m_dstDegree =((int)m_dstDegree+720)%360;
#if 0
    //calc the dist arc
    float arc = (m_dstDegree-m_heading)*3.1415f/180.0f;
    double rotate = sin(arc)*1.1f;
    double forward = cos(arc)*1.2f;

    if(rotate>1.0f){rotate=1.0f;}
    if(rotate<-1.0f){rotate=-1.0f;}
    if(forward>1.0f){forward=1.0f;}
    if(forward<-1.0f){forward=-1.0f;}
#endif
    long dstDegree = (long)m_dstDegree;
    //setJoystickPosition(rotate,forward);
    QByteArray buffer;
    unsigned char * p = (unsigned char*)(&dstDegree);
    buffer.append(p[0]);
    buffer.append(p[1]);
    buffer.append(p[2]);
    buffer.append(p[3]);
    if(distance>=SPIDER_AUTOPILOT_MIN_RADIUS){
        buffer.append((char)0x01);
    }else{
        buffer.append((char)0x00);
    }

    //qDebug()<<tr("dst_degree:%1").arg(m_dstDegree);

    QLowEnergyService* service = getCurrentService();
    if(service ){//write to joystick
        service->writeCharacteristic(m_charAutopilot,buffer);
    }
}
