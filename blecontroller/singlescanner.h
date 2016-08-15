#ifndef SCANNER_H
#define SCANNER_H

#include <qbluetoothglobal.h>
#include <qbluetoothlocaldevice.h>
#include <QObject>
#include <QVariant>
#include <QList>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>
#include <QBluetoothServiceInfo>

#include "deviceinfo.h"


class SingleScanner : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant deviceList READ getDeviceList NOTIFY deviceListUpdated)
public:
    explicit SingleScanner(QObject *parent = 0);
    ~SingleScanner();
    /*************************************************
     * call this function to start BLE scan
     * @return true to be successfull.
     *************************************************/
    Q_INVOKABLE bool start();
    Q_INVOKABLE void stop();

    /***
     * connect to the device with specific address
     * @address the mac address string shown in the qml interface.
     * @return true, if connect first call is success. but it doesn't mean connectiong is done.
     */
    Q_INVOKABLE bool connectToDevice(const QString & address);
    Q_INVOKABLE void disconnectFromDevice();

    /**************************************************
     * return all scanned devices
     **************************************************/
    QVariant getDeviceList();
    DeviceInfo* getDevice(const QString& address);
    QLowEnergyService* getService(const QString & uuid);
    QLowEnergyService* getCurrentService();


    Q_INVOKABLE bool isDeviceConnected();
    Q_INVOKABLE bool isServiceDiscovered();

protected:
    void resetDeviceList();
    void resetServiceList();
    /************************************************
     * convert UUID class into readable uuid string.
     ************************************************/
    QString uuidToString(const QBluetoothUuid & id);
    void enableCharacteristicNotify(QLowEnergyCharacteristic &c, bool b=true);

    /*************************************************************
     * check if the required service & characteristics exists.
     *************************************************************/
    /**
     * check if the required service exists
     * @return the service, if NULL means not found.
     */
    virtual QLowEnergyService* checkService()=0;
    /**
     * process the discovered characteristics
     * @service, the detals discovered service.
     * @return true if the characterlist is OK.
     **/
    virtual bool checkServiceDetailDiscovered(QLowEnergyService* service)=0;

    /*
     * process your characteristic message here
     * TODO:inherit it and process it here.
     */
    virtual void processCharacteristicValueUpdated(const QLowEnergyCharacteristic &c,const QByteArray &value)=0;
signals:
    //emit when device list content changed.
    void deviceListUpdated();

    /***********************************************
     * controller state related signal
     **********************************************/
    void stateDeviceDiscoverStarted();
    void stateDeviceDiscoverDone();
    void stateDeviceConnected(const QString & address);
    void stateDeviceDisconnected(const QString & address);
    void stateServiceDiscoverStarted();
    void stateServiceDiscoverDone();

    /*
     * update message to the signal receiver.
     * @message, the transmitting message.
     */
    void updateMessage(const QString & message);

protected slots:
    // QBluetoothDeviceDiscoveryAgent related
    void onScanDeviceDiscovered(const QBluetoothDeviceInfo&);
    void onScanError(QBluetoothDeviceDiscoveryAgent::Error);
    void onScanFinished();

    void onDeviceConnected();
    void onDeviceDisconnected();

    /************************************************
     * service related functions
     ***********************************************/
    void onServiceDiscovered(const QBluetoothUuid &newService);
    void onServiceStateChanged(QLowEnergyService::ServiceState);
    void onServiceDiscoveryFinished();
    void onServiceErrorReceived(QLowEnergyController::Error);

    void onCharacteristicValueUpdated(const QLowEnergyCharacteristic &c,const QByteArray &value);
private:
    /***************************************
     * local centeral device scan agent
     ***************************************/
    QBluetoothDeviceDiscoveryAgent *m_pstAgent;

    /***************************
     * device related members
     ***************************/
    QLowEnergyController* m_pstController;
    DeviceInfo* m_pstCurrentDevice;
    QLowEnergyService* m_pstCurrentService;
    QList<QObject*> m_deviceList;
    QList<QObject*> m_serviceList;

};

#endif // SCANNER_H
