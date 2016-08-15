#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QObject>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothaddress.h>
#include <QLowEnergyController>
#include <QLowEnergyCharacteristic>
#include <QList>

#include <QString>
#include <QObject>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothaddress.h>

class DeviceInfo: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString deviceName READ name NOTIFY deviceUpdated)
    Q_PROPERTY(QString deviceAddress READ address NOTIFY deviceUpdated)
    Q_PROPERTY(int deviceRSSI READ rssi NOTIFY deviceUpdated)

public:
    DeviceInfo(const QBluetoothDeviceInfo &device);
    QString address() const;
    QString name() const{return m_device.name();}
    int rssi() const{return m_device.rssi();}

    QBluetoothDeviceInfo& device() {return m_device;}

signals:
    void deviceUpdated();

private:
    QBluetoothDeviceInfo m_device;
};


#endif // BLEDEVICEINFO_H
