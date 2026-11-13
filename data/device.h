#ifndef DEVICE_H
#define DEVICE_H
#include <QString>
#include <QDateTime>

class Device
{
public:
    Device();
    Device(int deviceId, const QString &deviceName, const QString &deviceType,
           const QDateTime &accessTime, bool status);
    int getDeviceId() const;         // 获取设备ID
    QString getDeviceName() const;   // 获取设备名称
    QString getDeviceType() const;   // 获取设备类型（这里都是“camera”）
    QDateTime getAccessTime() const; // 获取接入时间
    bool getStatus() const;          // 获取状态（true=开启，false=关闭）
    QString getStatusText() const;   // 把状态转成文字（true→“开启”，false→“关闭”）
    void setStatus(bool status);
private:
    int m_deviceId;
    QString m_deviceName;
    QString m_deviceType;
    QDateTime m_accessTime;
    bool m_status;
};

#endif // DEVICE_H
