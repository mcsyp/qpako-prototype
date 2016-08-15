
#include <qbluetoothaddress.h>
#include <qbluetoothdevicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothservicediscoveryagent.h>
#include <QDebug>
#include <QList>
#include <QTimer>

#include "singlescanner.h"

SingleScanner::SingleScanner(QObject *parent) : QObject(parent)
{
    //init the bluetooth agent.
    m_pstAgent = new QBluetoothDeviceDiscoveryAgent();
    connect(m_pstAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)),
            this, SLOT(onScanDeviceDiscovered(const QBluetoothDeviceInfo&)));
    connect(m_pstAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(onScanError(QBluetoothDeviceDiscoveryAgent::Error)));
    connect(m_pstAgent, SIGNAL(finished()), this, SLOT(onScanFinished()));

    m_pstController = NULL;
    resetDeviceList();
    resetServiceList();
}
SingleScanner::~SingleScanner()
{
    resetServiceList();
    resetDeviceList();
    if(m_pstAgent){
        delete m_pstAgent;
    }
}

void SingleScanner::resetDeviceList(){
    qDeleteAll(m_deviceList);
    m_deviceList.clear();
    m_pstCurrentDevice=NULL;
    emit deviceListUpdated();
}
void SingleScanner::resetServiceList(){
    qDeleteAll(m_serviceList);
    m_serviceList.clear();
    m_pstCurrentService = NULL;
}

bool SingleScanner::start()
{
    bool ret = false;

    //release the resources
    resetDeviceList();

    //start the scanner.
    m_pstAgent->start();
    ret = m_pstAgent->isActive();
    if(ret){
        //syp debug
        qDebug()<<tr("Start scanning BLE devices.\n");
        emit stateDeviceDiscoverStarted();
    }
    return ret;
}
void SingleScanner::stop()
{
    m_pstAgent->stop();
    onScanFinished();
}
void SingleScanner::onScanDeviceDiscovered(const QBluetoothDeviceInfo & info)
{
    if (info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        DeviceInfo *d = new DeviceInfo(info);
        m_deviceList.append(d);
        emit deviceListUpdated();
    }
}
void SingleScanner::onScanError(QBluetoothDeviceDiscoveryAgent::Error)
{
    qDebug()<<tr("Error happens at scanning phase.");
    /**************************************************
     *TODO: add some scan error processing code here.
     **************************************************/
}
void SingleScanner::onScanFinished()
{
    emit stateDeviceDiscoverDone();//emit a scan finished signal
}

bool SingleScanner::connectToDevice(const QString &address)
{
    DeviceInfo * dev = getDevice(address);
    if(dev==NULL)
        return false;

    // We need the current device for service discovery.
    if (!dev->device().isValid()) {
        qDebug()<<tr("Not a valid device\n");
        return false;
    }

    resetServiceList();
    if (m_pstController) {
        m_pstController->disconnectFromDevice();
        delete m_pstController;
        m_pstController = 0;
    }

    if (!m_pstController) {
        // Connecting signals and slots for connecting to LE services.
        m_pstController = new QLowEnergyController(dev->device());
        connect(m_pstController, SIGNAL(connected()),
                this, SLOT(onDeviceConnected()));
        connect(m_pstController, SIGNAL(error(QLowEnergyController::Error)),
                this, SLOT(onServiceErrorReceived(QLowEnergyController::Error)));
        connect(m_pstController, SIGNAL(disconnected()),
                this, SLOT(onDeviceDisconnected()));

        connect(m_pstController, SIGNAL(serviceDiscovered(const QBluetoothUuid&)),
                this, SLOT(onServiceDiscovered(const QBluetoothUuid&)));
        connect(m_pstController, SIGNAL(discoveryFinished()),
                this, SLOT(onServiceDiscoveryFinished()));
    }
    m_pstCurrentDevice=dev;//set device to current device;
    m_pstController->setRemoteAddressType(QLowEnergyController::PublicAddress);
    m_pstController->connectToDevice();
    return true;
}

void SingleScanner::disconnectFromDevice()
{
    resetServiceList();
    if(m_pstController){
        m_pstController->disconnectFromDevice();
        delete m_pstController;
        m_pstController = NULL;
    }

    //clear the current device
    if(m_pstCurrentDevice){
        emit stateDeviceDisconnected(m_pstCurrentDevice->address());
    }else{
        emit stateDeviceDisconnected("Unknown Address");
    }
    m_pstCurrentDevice=NULL;
}

void SingleScanner::onDeviceConnected(){
    if(m_pstCurrentDevice){
        emit stateDeviceConnected(m_pstCurrentDevice->address());
    }else{
        emit stateDeviceConnected("Unkown Address");
    }
    //if is conneccted, scan all services
    if(m_pstController)
        m_pstController->discoverServices();
    emit stateServiceDiscoverStarted();
}

void SingleScanner::onDeviceDisconnected()
{
    disconnectFromDevice();
}

void SingleScanner::onServiceErrorReceived(QLowEnergyController::Error err)
{
    int id = (int)err;
    switch(id){
    case QLowEnergyController::UnknownRemoteDeviceError:
    case QLowEnergyController::InvalidBluetoothAdapterError:
        disconnectFromDevice();
        break;
    }
}
void SingleScanner::onServiceDiscovered(const QBluetoothUuid& serviceUuid)
{
    QLowEnergyService *service = m_pstController->createServiceObject(serviceUuid);
    if (!service) {
        qWarning() << "Cannot create service for uuid";
        return;
    }
    m_serviceList.append(service);
}

void SingleScanner::onServiceDiscoveryFinished()
{
   if(m_pstCurrentDevice){//cast it to a correct
       QLowEnergyService* service = checkService();//let user check if the required service exits.
       if(service){
           //clear all connected call back
           disconnect(service,0,0,0);
           //connect service
           connect(service,SIGNAL(characteristicChanged(const QLowEnergyCharacteristic &, const QByteArray &)),
                   this,SLOT(onCharacteristicValueUpdated(const QLowEnergyCharacteristic &, const QByteArray & )));
           connect(service, SIGNAL(stateChanged(QLowEnergyService::ServiceState)),
                   this, SLOT(onServiceStateChanged(QLowEnergyService::ServiceState)));
           service->discoverDetails();
           m_pstCurrentService = service;
       }else{
           //if the service is not found.
           disconnectFromDevice();//tell the system that the serivce is not found.
       }
   }
   emit stateServiceDiscoverDone();
}
void SingleScanner::onCharacteristicValueUpdated(const QLowEnergyCharacteristic &c, const QByteArray &value){
    processCharacteristicValueUpdated(c,value);
}

void SingleScanner::onServiceStateChanged(QLowEnergyService::ServiceState state)
{
    QLowEnergyService* service = qobject_cast<QLowEnergyService*>(sender());
    switch(state){
    case QLowEnergyService::DiscoveryRequired:
        if(service){
            service->discoverDetails();
        }
        break;
    case QLowEnergyService::DiscoveringServices:
        break;
    case QLowEnergyService::ServiceDiscovered:
        if(!service || !checkServiceDetailDiscovered(service)){
            //if service or detail is not found
            qDebug()<<tr("Service or characteristic not found.");
            disconnectFromDevice();//tell the system that the service is not found and disconnect the device.
        }
        break;
    }
}
void SingleScanner::enableCharacteristicNotify(QLowEnergyCharacteristic &c, bool b){
    if(!c.isValid() || m_pstCurrentService==NULL){
        qDebug()<<tr("Fail to update charactoristic property.");
        return;
    }

    const QLowEnergyDescriptor notificationDesc = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
    if (!notificationDesc.isValid()){
        qDebug()<<tr("Descriptor is not valid.");
        return;
    }
    if(b){
        m_pstCurrentService->writeDescriptor(notificationDesc, QByteArray::fromHex("FFFF"));
    }else{
        m_pstCurrentService->writeDescriptor(notificationDesc, QByteArray::fromHex("0000"));
    }
}

QVariant SingleScanner::getDeviceList(){
    return QVariant::fromValue(m_deviceList);
}

DeviceInfo* SingleScanner::getDevice(const QString& address)
{
    //1.search for device
    for(int i=0;i<m_deviceList.size();i++){
        DeviceInfo* dev = (DeviceInfo*)m_deviceList.at(i);
        if(dev->address()==address){
            return dev;
        }
    }
    return NULL;
}

QLowEnergyService* SingleScanner::getService(const QString &uuid){
    for(int i=0;i<m_serviceList.size();i++){
        QLowEnergyService* s = qobject_cast<QLowEnergyService*>(m_serviceList.at(i));
        if(s && uuidToString(s->serviceUuid())==uuid){
            return s;
        }
    }
    return NULL;
}
QLowEnergyService* SingleScanner::getCurrentService(){
    return m_pstCurrentService;
}

QString SingleScanner::uuidToString(const QBluetoothUuid & id)
{
    bool success = false;
    quint16 result16 = id.toUInt16(&success);
    if (success)
        return QStringLiteral("0x") + QString::number(result16, 16);

    quint32 result32 = id.toUInt32(&success);
    if (success)
        return QStringLiteral("0x") + QString::number(result32, 16);

    return id.toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
}

bool SingleScanner::isDeviceConnected(){
    return !(m_pstCurrentDevice==NULL);
}
bool SingleScanner::isServiceDiscovered(){
    return !(m_pstCurrentService==NULL);
}
