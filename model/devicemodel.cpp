#include "devicemodel.h"
DeviceModel *DeviceModel::instance=nullptr;

DeviceModel::DeviceModel(QObject *parent)
{

}

DeviceModel *DeviceModel::getInstance()
{
    if(DeviceModel::instance==nullptr)
    {
        DeviceModel::instance=new DeviceModel();
    }
    return DeviceModel::instance;
}

DeviceModel::~DeviceModel()
{
    qDeleteAll(m_devices);// 清空之前的设备列表
     m_devices.clear();
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        return 0;
    }
    return m_devices.count();
}

int DeviceModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        return 0;
    }
    return COLUMN_COUNT;
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()||index.row()>=m_devices.count())
    {
        return  QVariant();
    }
    Device * device=m_devices[index.row()];
    if(role==Qt::DisplayRole)
    {
        switch (index.column())
        {
        case ID_COLUMN:
           return device->getDeviceId();
       case NAME_COLUMN:
           return device->getDeviceName();
       case TYPE_COLUMN:
           return device->getDeviceType();
       case ACCESS_TIME_COLUMN:
           return device->getAccessTime().toString("yyyy-MM-dd HH:mm:ss");
       case STATUS_COLUMN:
           return device->getStatusText();
       default:
           return QVariant();
        }
    }
    else if(role==Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }
     return QVariant();
}
//设置表格表头
// section：列号（横向表头）或行号（纵向表头）
// orientation：表头方向（Qt::Horizontal=横向，Qt::Vertical=纵向）
// role：数据角色（这里只关心“显示文本”）
QVariant DeviceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation==Qt::Horizontal&&role==Qt::DisplayRole)
    {
        switch(section)
        {
        case ID_COLUMN:
            return "设备ID";
        case NAME_COLUMN:
            return "设备名称";
        case TYPE_COLUMN:
            return "设备类型";
        case ACCESS_TIME_COLUMN:
            return "接入时间";
        case STATUS_COLUMN:
            return "状态";
        default:
            return QVariant();
        }
    }
    return QVariant();
}

void DeviceModel::loadDevices()
{
    beginResetModel();//表格刷新数据
    qDeleteAll(m_devices);
    m_devices.clear();
    QSettings settings("../config.ini",QSettings::IniFormat);
    QString realCameraName=settings.value("VideoConfig/DefaultCamera", "USB2.0 PC CAMERA").toString();
    this->initVirtualDevices(realCameraName);
    QString sql="select device_id,status from device";
    char **qres=nullptr;
    int row=0,col=0;
    if(SingletonDB::getInstance()->selectSql(sql,qres,row,col)==0&&row>0)
    {
        for(int i=0;i<row;i++)
        {
            int deviceId=atoi(qres[i*col]);//取当前0列device_id
            bool status=atoi(qres[i*col+1])==1;//status（1=开启，0=关闭）
            for(Device* device:m_devices)
            {
                if(device->getDeviceId()==deviceId)
                {
                    device->setStatus(status);
                    break;
                }
            }
        }
        sqlite3_free_table(qres);
    }
    this->saveDevicesToDatabase();
    endResetModel();//更新完可以刷新
}

void DeviceModel::updateDeviceStatus(int deviceId, bool status)
{
    for (int i = 0; i < m_devices.count(); i++)
    {
        if (m_devices[i]->getDeviceId() == deviceId)
        {
            m_devices[i]->setStatus(status);
            QString sql = QString("UPDATE device SET status = %1 WHERE device_id = %2")
                        .arg(status ? 1 : 0)
                        .arg(deviceId);
            SingletonDB::getInstance()->execSql(sql);
            QModelIndex index = createIndex(i, STATUS_COLUMN);
            emit dataChanged(index, index);
            break;
        }
    }
}

bool DeviceModel::saveDevicesToDatabase()
{
   for(Device * device :m_devices)
   {
       if(isDeviceExistsInDb(device->getDeviceId()))
       {
           QString sql = QString("UPDATE device SET "
                         "device_name = '%1', "
                         "status = %2, "
                          "access_time = '%3' "
                          "WHERE device_id = %4")
                       .arg(device->getDeviceName().replace("'", "''"))
                       .arg(device->getStatus() ? 1 : 0)  // bool转1/0
                       .arg(device->getAccessTime().toString("yyyy-MM-dd HH:mm:ss"))
                       .arg(device->getDeviceId());
           SingletonDB::getInstance()->execSql(sql);
       }
       else
       {
           QString sql = QString("INSERT INTO device "
                               "(device_id, device_name, device_type, access_time, status) "
                               "VALUES (%1, '%2', '%3', '%4', %5)")
                          .arg(device->getDeviceId())
                          .arg(device->getDeviceName().replace("'", "''"))
                          .arg(device->getDeviceType())
                          .arg(device->getAccessTime().toString("yyyy-MM-dd HH:mm:ss"))
                          .arg(device->getStatus() ? 1 : 0);
          SingletonDB::getInstance()->execSql(sql);
       }
   }
    return true;
}


void DeviceModel::initVirtualDevices(const QString &realCameraName)
{
    QDateTime accessTime = QDateTime::currentDateTime();
    m_devices.append(new Device(1, realCameraName, "camera", accessTime, true));
    m_devices.append(new Device(2, "模拟摄像头2", "camera", accessTime, false));
    m_devices.append(new Device(3, "模拟摄像头3", "camera", accessTime, false));
    m_devices.append(new Device(4, "模拟摄像头4", "camera", accessTime, false));
}

//判断设备ID是否已经存在于数据库中,避免重复插入
bool DeviceModel::isDeviceExistsInDb(int deviceId)
{
    QString sql=QString("select count(*) from device where device_id=%1").arg(deviceId);
    char **qres=nullptr;
    int row=0,col=0;
    bool exits=false;
    if(SingletonDB::getInstance()->selectSql(sql,qres,row,col)==0&&row>0)
    {
        exits=(atoi(qres[0])>0);
        sqlite3_free_table(qres);
    }
    return  exits;
}
