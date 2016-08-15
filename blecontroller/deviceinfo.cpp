#include "deviceinfo.h"
#include <QDebug>
#include <qbluetoothuuid.h>

DeviceInfo::DeviceInfo(const QBluetoothDeviceInfo &info):
    QObject(), m_device(info)
{
    emit deviceUpdated();
}

QString DeviceInfo::address() const
{
#ifdef Q_OS_MAC
    // workaround for Core Bluetooth:
    return m_device.deviceUuid().toString();
#else
    return m_device.address().toString();
#endif
}
