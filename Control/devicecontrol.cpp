#include "devicecontrol.h"
DeviceControl* DeviceControl::instance = nullptr;

DeviceControl *DeviceControl::getInstance()
{
    if(DeviceControl::instance==nullptr)
    {
        DeviceControl::instance=new DeviceControl();
    }
    return DeviceControl::instance;
}

void DeviceControl::loadDevices()
{
    DeviceModel::getInstance()->loadDevices();
}

void DeviceControl::updateDeviceStatus(int deviceId, bool status)
{
    DeviceModel::getInstance()->updateDeviceStatus(deviceId,status);
}


DeviceControl::DeviceControl(QObject *parent)
{
    m_model = DeviceModel::getInstance();
}

DeviceModel* DeviceControl::getModel()
{
    return m_model;
}
