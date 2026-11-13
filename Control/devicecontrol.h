#ifndef DEVICECONTROL_H
#define DEVICECONTROL_H
#include <QObject>
#include "model/devicemodel.h"

class DeviceControl: public QObject
{
    Q_OBJECT
public:
    static DeviceControl* getInstance();
    void loadDevices();
    void updateDeviceStatus(int deviceId,bool status);
    DeviceModel* getModel();
private:
    explicit DeviceControl(QObject *parent = nullptr);
    static DeviceControl* instance;
    DeviceModel* m_model;
};

#endif // DEVICECONTROL_H
