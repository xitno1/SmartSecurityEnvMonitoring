#include "device.h"


Device::Device()
{

}

Device::Device(int deviceId, const QString &deviceName, const QString &deviceType, const QDateTime &accessTime, bool status)
{
    this->m_deviceId=deviceId;
    this->m_deviceName=deviceName;
    this->m_deviceType=deviceType;
    this->m_accessTime=accessTime;
    this->m_status=status;
}

int Device::getDeviceId() const
{
    return m_deviceId;
}

QString Device::getDeviceName() const
{
     return m_deviceName;
}

QString Device::getDeviceType() const
{
    return m_deviceType;
}

QDateTime Device::getAccessTime() const
{
    return m_accessTime;
}

bool Device::getStatus() const
{
    return m_status;
}

QString Device::getStatusText() const
{
     return m_status ? "开启" : "关闭";
}

void Device::setStatus(bool status)
{
     m_status = status;
}
