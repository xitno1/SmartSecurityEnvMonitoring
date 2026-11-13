#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H
#include<QAbstractTableModel>//qt的表格数据类型
#include<QList>
#include"data/device.h"
#include"data/singletondb.h"
#include"view/settingwidget.h"
#include <QDateTime>
#include <QDebug>
#include <cstring>
class DeviceModel: public QAbstractTableModel
{
    Q_OBJECT
public:
    enum  DeviceColum
    {
        ID_COLUMN,          // 第0列：设备ID
        NAME_COLUMN,        // 第1列：设备名称
        TYPE_COLUMN,        // 第2列：设备类型
        ACCESS_TIME_COLUMN, // 第3列：接入时间
        STATUS_COLUMN,      // 第4列：状态
        COLUMN_COUNT        // 列数（5列）
    };
    static DeviceModel* getInstance();
     ~DeviceModel() override;
    //表格类要有的函数
    int rowCount(const QModelIndex &parent=QModelIndex()) const override;//表行数
    int columnCount(const QModelIndex &parent=QModelIndex())const override;//表列数
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override; // 单元格显示什么数据
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override; // 表格表头

    void loadDevices();//加载设备列表
    void updateDeviceStatus(int deviceId,bool status);//更新设备状态
    bool saveDevicesToDatabase();

private:
   explicit DeviceModel(QObject *parent = nullptr);
    static DeviceModel* instance;

    QList<Device *>m_devices;//存储所有设备列表
    void initVirtualDevices(const QString &realCameraName);
    bool isDeviceExistsInDb(int deviceId);

};

#endif // DEVICEMODEL_H
