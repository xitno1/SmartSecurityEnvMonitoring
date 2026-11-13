#include "myview.h"
#include<QBrush>
#include<QPixmap>
#include<QDebug>
#include<QFile>
MyView::MyView()
{
    //1.设置窗口大小，背景
    this->setFixedSize(550,350);
    this->setWindowTitle("欢迎界面");
    this->setBackgroundBrush(QBrush(QPixmap(":/images/openimg.jfif")));
    //创建场景
    this->scene=new QGraphicsScene();
    this->setScene(this->scene);
    //坐标系统一
    this->setSceneRect(0,0,this->width()-2,this->height()-2);
    //创建图元
    this->item1=new MyItem(":/images/item1.png",true,1);
    this->item2=new MyItem(":/images/item2.png",true,-1);
    this->item3=new MyItem(":/images/item3.png",false,1);
    //添加到场景中
    this->scene->addItem(this->item1);
    this->scene->addItem(this->item2);
    //设置站位
    this->item1->setPos(50,this->height()/2);
    this->item2->setPos(500,this->height()/2);
    //创建定时器
    mytimer=new QTimer(this);
    //设置间隔时间
    mytimer->setInterval(40);
    this->mytimer->start();//也可以直接 this->mytimer->start(40);
    //每隔40ms，场景会调用advance槽函数，也会调用自己管辖内的多有item的advance函数


    delayTime=new QTimer(this);
    delayTime->setInterval(1500);
    connect(mytimer,SIGNAL(timeout()),this->scene,SLOT(advance()));
    connect(this->item1,SIGNAL(boom()),this,SLOT(stopTimer()));
    connect(this->item2,SIGNAL(boom()),this,SLOT(stopTimer()));
    connect(delayTime,SIGNAL(timeout()),this,SLOT(DelayFinish()));
}

void MyView::stopTimer()
{

    if (!mytimer->isActive())
    {
        return; // 若定时器已停止，直接返回，避免重复操作
    }

    mytimer->stop();
    qDebug() << "碰撞发生，等待1.5秒后打开设置界面...";

    if (item3->scene() == nullptr) {
        scene->addItem(item3);
        item3->setPos(width()/2, height()/2);
    }

    delayTime->start();
}

void MyView::DelayFinish()
{
    delayTime->stop();
    this->hide();

    if (isConfigValid()) {
        qDebug() << "配置有效且硬件未变动，直接进入主界面";
        mainWin = new MainWindow();
        mainWin->show();
    } else {
        qDebug() << "配置无效或硬件变动，打开设置界面";
        settingWin = new SettingWidget(nullptr);
        connect(settingWin, SIGNAL(configSaved()), this, SLOT(onConfigSaved()), Qt::QueuedConnection);
        settingWin->show();
    }
}

void MyView::onConfigSaved()
{
    if (settingWin)
   {
       disconnect(settingWin, SIGNAL(configSaved()), this, SLOT(onConfigSaved()));
       settingWin->close();
       delete settingWin;
       settingWin = nullptr;
   }

   // 直接进入主界面，不做额外验证
   mainWin = new MainWindow();
   mainWin->show();
}

bool MyView::isConfigValid()
{
    const QString configPath = "../config.ini";
    QFile file(configPath);
    if (!file.exists()) {
        qDebug() << "config.ini 不存在，需要设置";
        return false;
    }

    QSettings settings(configPath, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    // 检查已配置的串口是否仍然可用
    QString savedPort = settings.value("SerialConfig/SerialPort").toString();
    if (savedPort.isEmpty()) {
        qDebug() << "配置中无串口记录，需要设置";
        return false;
    }
    bool portFound = false;
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &info, ports) {
        if (info.portName() == savedPort) {
            portFound = true;
            break;
        }
    }
    if (!portFound) {
        qDebug() << "已配置的串口" << savedPort << "未找到，需要重新设置";
        return false;
    }

    // 检查已配置的摄像头是否仍然可用
    QString savedCamera = settings.value("VideoConfig/DefaultCamera").toString();
    if (savedCamera.isEmpty()) {
        qDebug() << "配置中无摄像头记录，需要设置";
        return false;
    }
    bool cameraFound = false;
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &info, cameras) {
        if (info.description() == savedCamera) {
            cameraFound = true;
            break;
        }
    }
    if (!cameraFound) {
        qDebug() << "已配置的摄像头" << savedCamera << "未找到，需要重新设置";
        return false;
    }

    qDebug() << "配置有效：串口=" << savedPort << "，摄像头=" << savedCamera;
    return true;
}
