#include "settingwidget.h"
#include <QPainter>
#include <QPixmap>
#include <QPaintEvent>
#include <QApplication>
SettingWidget::SettingWidget(QWidget *parent):
    QWidget(parent),
   m_settings(new QSettings(CONFIG_PATH, QSettings::IniFormat))
{
    m_settings->setIniCodec("UTF-8");
    this->initQss();
    this->initUI();

    if(isConfigExists())
    {
        this->loadConfigToUI();
    }
}

SettingWidget::~SettingWidget()
{
    delete m_settings;
}

bool SettingWidget::isConfigExists()
{
    QFile file(CONFIG_PATH);
    return file.exists();
}


void SettingWidget::initUI()
{
    this->setWindowTitle("设置界面");
    this->setFixedSize(700,500);


    this->serialPortLabel=new QLabel("串口选择:",this);
    serialPortLabel->setStyleSheet("color: white; font-size: 17px;");
    this->serialPortCombo=new QComboBox(this);
    this->serialPortCombo->setMinimumWidth(250);

    //视频路径
    this->videoPathLabel=new QLabel("视频存储位置:",this);
    videoPathLabel->setStyleSheet("color: white; font-size: 17px;");
    this->videoPathEdit=new QLineEdit(this);
    this->videoPathEdit->setReadOnly(true);//禁止手动输入路径
    this->videoPathEdit->setMinimumWidth(300);
    this->selectVideoPathBtn=new QPushButton("...",this);
    this->selectVideoPathBtn->setMinimumWidth(100);


    //采集间隔
    this->captureIntervalLabel=new QLabel("视频采集间隔:",this);
    captureIntervalLabel->setStyleSheet("color: white; font-size: 17px;");
    this->captureIntervalCombo=new QComboBox(this);
    this->captureIntervalCombo->addItems({"30秒","40秒","1分钟"});
    this->captureIntervalCombo->setMinimumWidth(200);
    //摄像头
    this->cameraLabel=new QLabel("摄像头配置",this);
    cameraLabel->setStyleSheet("color: white; font-size: 17px;");
    this->cameraCombo=new QComboBox(this);
    this->cameraCombo->setMinimumWidth(250);
    loadCameras();

    // 告警阈值分组框
    alarmThresholdGroup = new QGroupBox("告警阈值设置", this);
    alarmThresholdGroup->setMinimumHeight(200);

    tempGeneralLabel = new QLabel("温度一般告警（℃）：", this);
    tempGeneralLabel->setStyleSheet("color: white; font-size: 17px;");
    tempGeneralSpin = new QDoubleSpinBox(this);
    tempGeneralSpin->setRange(0, 100); // 温度范围0-100℃
    tempGeneralSpin->setDecimals(1);   // 保留1位小数

    tempSeriousLabel = new QLabel("温度严重告警（℃）：", this);
     tempSeriousLabel->setStyleSheet("color: white; font-size: 17px;");
    tempSeriousSpin = new QDoubleSpinBox(this);
    tempSeriousSpin->setRange(0, 100);
    tempSeriousSpin->setDecimals(1);

    humidityGeneralLabel = new QLabel("湿度一般告警（%）：", this);
    humidityGeneralLabel->setStyleSheet("color: white; font-size: 17px;");
    humidityGeneralSpin = new QDoubleSpinBox(this);
    humidityGeneralSpin->setRange(0, 100);
    humidityGeneralSpin->setDecimals(1);

    humiditySeriousLabel = new QLabel("湿度严重告警（%）：", this);
    humiditySeriousLabel->setStyleSheet("color: white; font-size: 17px;");
    humiditySeriousSpin = new QDoubleSpinBox(this);
    humiditySeriousSpin->setRange(0, 100);
    humiditySeriousSpin->setDecimals(1);

    lightGeneralLabel = new QLabel("光照一般告警（lux）：", this);
    lightGeneralLabel->setStyleSheet("color: white; font-size: 17px;");
    lightGeneralSpin = new QSpinBox(this);
    lightGeneralSpin->setRange(0, 1000); // 光照范围0-1000lux

    lightSeriousLabel = new QLabel("光照严重告警（lux）：", this);
    lightSeriousLabel->setStyleSheet("color: white; font-size: 17px;");
    lightSeriousSpin = new QSpinBox(this);
    lightSeriousSpin->setRange(0, 1000);

    //底部按钮
    this->confirmBtn=new QPushButton("确定",this);
    this->cancelBtn=new QPushButton("取消",this);
    this->confirmBtn->setMinimumWidth(100);
    //布局
    this->serialLayout=new QHBoxLayout();
    this->serialLayout->addWidget(this->serialPortLabel);
    this->serialLayout->addWidget(this->serialPortCombo);
    this->serialLayout->addStretch();

    this->intervalLayout=new QHBoxLayout();
    this->intervalLayout->addWidget(this->captureIntervalLabel);
    this->intervalLayout->addWidget(this->captureIntervalCombo);
    this->intervalLayout->addStretch();

    this->cameraLayout=new QHBoxLayout();
    this->cameraLayout->addWidget(this->cameraLabel);
    cameraLayout->addWidget(cameraCombo);
    cameraLayout->addStretch();

    // 告警阈值网格布局（2列6行，表格式排列）
    this->alarmLayout = new QGridLayout(alarmThresholdGroup); // 绑定到分组框
    this->alarmLayout->addWidget(tempGeneralLabel, 0, 0);     // 第0行第0列
    this->alarmLayout->addWidget(tempGeneralSpin, 0, 1);     // 第0行第1列
    this->alarmLayout->addWidget(tempSeriousLabel, 1, 0);    // 第1行第0列
    this->alarmLayout->addWidget(tempSeriousSpin, 1, 1);    // 第1行第1列
    this->alarmLayout->addWidget(humidityGeneralLabel, 2, 0); // 第2行第0列
    this->alarmLayout->addWidget(humidityGeneralSpin, 2, 1); // 第2行第1列
    this->alarmLayout->addWidget(humiditySeriousLabel, 3, 0); // 第3行第0列
    this->alarmLayout->addWidget(humiditySeriousSpin, 3, 1); // 第3行第1列
    this->alarmLayout->addWidget(lightGeneralLabel, 4, 0);   // 第4行第0列
    this->alarmLayout->addWidget(lightGeneralSpin, 4, 1);   // 第4行第1列
    this->alarmLayout->addWidget(lightSeriousLabel, 5, 0);   // 第5行第0列
    this->alarmLayout->addWidget(lightSeriousSpin, 5, 1);   // 第5行第1列
    this->alarmLayout->setColumnStretch(1, 1); // 第1列拉伸，适配窗口宽度



    btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(confirmBtn);
    btnLayout->addSpacing(20); // 按钮之间间距
    btnLayout->addWidget(cancelBtn);
    btnLayout->addStretch();

    this->videoPathLayout = new QHBoxLayout();
    this->videoPathLayout->addWidget(this->videoPathLabel);
    this->videoPathLayout->addWidget(this->videoPathEdit);
    this->videoPathLayout->addWidget(this->selectVideoPathBtn);
    this->videoPathLayout->addStretch();
    // 主布局
   mainLayout = new QVBoxLayout(this);
   mainLayout->addLayout(serialLayout);
   mainLayout->addSpacing(15);
   mainLayout->addLayout(videoPathLayout);
   mainLayout->addLayout(intervalLayout);
   mainLayout->addSpacing(15);
   mainLayout->addLayout(cameraLayout);
   mainLayout->addSpacing(15);
   mainLayout->addWidget(alarmThresholdGroup);
   mainLayout->addStretch();               // 中间留白，按钮靠下
   mainLayout->addLayout(btnLayout);

   //绑定信号槽
   connect(selectVideoPathBtn, &QPushButton::clicked, this, &SettingWidget::on_selectVideoPathBtn);
   connect(confirmBtn, &QPushButton::clicked, this, &SettingWidget::on_confirmBtn);
   connect(cancelBtn, &QPushButton::clicked, this, &SettingWidget::on_cancelBtn);

   loadSerialPorts();

}

void SettingWidget::loadSerialPorts()
{
    this->serialPortCombo->clear();
    QList<QSerialPortInfo> portInfos=QSerialPortInfo::availablePorts();
    if(portInfos.isEmpty())
    {
        this->serialPortCombo->addItem("无可用串口");
        this->serialPortCombo->setEnabled(false);
    }
    else {
        foreach(QSerialPortInfo info,portInfos)
        {
            QString portText=QString("%1 (%2)").arg(info.portName()).arg(info.description());
            this->serialPortCombo->addItem(portText,info.portName());
        }
    }
}

void SettingWidget::loadCameras()
{
    this->cameraCombo->clear();
    QList<QCameraInfo> cameraInfos=QCameraInfo::availableCameras();
    if(cameraInfos.isEmpty())
    {
        this->cameraCombo->addItem("无可用摄像头");
        this->cameraCombo->setEnabled(false);
    }
    else {
        foreach(QCameraInfo info ,cameraInfos)
        {
            this->cameraCombo->addItem(info.description(), info.description());
        }

    }
    // 加载已保存的友好名称
    QString savedCamera = m_settings->value("VideoConfig/DefaultCamera").toString();
    int index = this->cameraCombo->findData(savedCamera);
    if (index != -1)
    {
        this->cameraCombo->setCurrentIndex(index);
    }
}

void SettingWidget::loadConfigToUI()
{
    //1. 串口配置
    QString saveSerialPort=m_settings->value("SerialConfig/SerialPort","COM3").toString();
    for(int i=0;i<this->serialPortCombo->count();i++)
    {
        if(this->serialPortCombo->itemData(i).toString()==saveSerialPort)
        {
            this->serialPortCombo->setCurrentIndex(i);
            break;
        }
    }
     // 2. 视频路径配置
    QString saveVideoPath=m_settings->value("VideoConfig/VideoSavePath", "/../fileOut/Video").toString();
    this->videoPathEdit->setText(saveVideoPath);

    //3. 采集间隔配置（秒数转文本）
    int savedIntervalSec=m_settings->value("VideoConfig/CaptureInterval",30).toInt();
    this->captureIntervalCombo->setCurrentText(convertSecToCaptureInterval(savedIntervalSec));

    //4. 摄像头配置

    QString saveCamera = m_settings->value("VideoConfig/DefaultCamera", "").toString();
    bool cameraFound = false;
    for(int i = 0; i < cameraCombo->count(); i++)
    {
        if(this->cameraCombo->itemData(i).toString() == saveCamera)
        {
            this->cameraCombo->setCurrentIndex(i);
            cameraFound = true;
            break;
        }
    }
    // 未找到匹配项时，强制设置为第0项（避免无效索引）
    if (!cameraFound && cameraCombo->count() > 0)
    {
        cameraCombo->setCurrentIndex(0);
    }

    // 5. 告警阈值配置
   tempGeneralSpin->setValue(m_settings->value("AlarmThreshold/TempGeneral", 30.0).toDouble());
   tempSeriousSpin->setValue(m_settings->value("AlarmThreshold/TempSerious", 35.0).toDouble());
   humidityGeneralSpin->setValue(m_settings->value("AlarmThreshold/HumidityGeneral", 65.0).toDouble());
   humiditySeriousSpin->setValue(m_settings->value("AlarmThreshold/HumiditySerious", 70.0).toDouble());
   lightGeneralSpin->setValue(m_settings->value("AlarmThreshold/LightGeneral", 200).toInt());
   lightSeriousSpin->setValue(m_settings->value("AlarmThreshold/LightSerious", 100).toInt());

}

bool SettingWidget::validateConfig(QString &errorMsg)
{
    errorMsg.clear();
    // 1. 验证视频存储路径
    QString videoPath=videoPathEdit->text().trimmed();
    QDir videoDir(videoPath);
    if(!videoDir.exists())
    {
        if(!videoDir.mkpath(videoPath))
        {
            errorMsg+="视频存储路径无效：无法创建或访问该路径\n";
        }
    }
    // 2. 验证摄像头配置
    if(this->cameraCombo->currentText()=="无可用摄像头")
    {
        errorMsg+="摄像头配置无效：未检测到可用摄像头\n";
    }

    // 3. 验证告警阈值逻辑
    double tempGeneral = tempGeneralSpin->value();
    double tempSerious = tempSeriousSpin->value();
    if (tempSerious <= tempGeneral)
    {
        errorMsg += "温度阈值无效：严重告警值应大于一般告警值\n";
    }

    double humidityGeneral = humidityGeneralSpin->value();
    double humiditySerious = humiditySeriousSpin->value();
    if (humiditySerious <= humidityGeneral)
    {
        errorMsg += "湿度阈值无效：严重告警值应大于一般告警值\n";
    }

    int lightGeneral = lightGeneralSpin->value();
    int lightSerious = lightSeriousSpin->value();
    if (lightSerious >= lightGeneral)
    {
        errorMsg += "光照阈值无效：严重告警值应小于一般告警值\n";
    }

    return errorMsg.isEmpty();
}

void SettingWidget::saveUIToConfig()
{
     // 1. 串口配置
    QString selectedSerial=this->serialPortCombo->currentData().toString();
    m_settings->setValue("SerialConfig/SerialPort",selectedSerial);

    // 2. 视频配置
    m_settings->setValue("VideoConfig/VideoSavePath",videoPathEdit->text().trimmed());
    int selectedIntervalSec=this->convertCaptureIntervalToSec(this->captureIntervalCombo->currentText());
    m_settings->setValue("VideoConfig/CaptureInterval",selectedIntervalSec);
    QString selectedCamera = this->cameraCombo->currentData().toString();
    m_settings->setValue("VideoConfig/DefaultCamera", selectedCamera);

    // 3. 告警阈值配置
   m_settings->setValue("AlarmThreshold/TempGeneral", tempGeneralSpin->value());
   m_settings->setValue("AlarmThreshold/TempSerious", tempSeriousSpin->value());
   m_settings->setValue("AlarmThreshold/HumidityGeneral", humidityGeneralSpin->value());
   m_settings->setValue("AlarmThreshold/HumiditySerious", humiditySeriousSpin->value());
   m_settings->setValue("AlarmThreshold/LightGeneral", lightGeneralSpin->value());
   m_settings->setValue("AlarmThreshold/LightSerious", lightSeriousSpin->value());

   // 强制写入文件（避免缓存延迟）
   m_settings->sync();

}

int SettingWidget::convertCaptureIntervalToSec(const QString &text)
{
    if(text=="30秒") return 30;
    if(text=="40秒") return 40;
    if(text=="1分钟") return 60;
    return  30;
}

QString SettingWidget::convertSecToCaptureInterval(int sec)
{
    if (sec == 30) return "30秒";
    if (sec == 40) return "40秒";
    if (sec == 60) return "1分钟";
    return "30秒";
}


void SettingWidget::on_selectVideoPathBtn()
{
    QString initPath=videoPathEdit->text().trimmed();
    if(initPath.isEmpty())
    {
        initPath=QDir::currentPath()+ "/../fileOut/Video";
    }
    QString selectedPath=QFileDialog::getExistingDirectory(
                this,"选择视频存储路径",initPath,QFileDialog::ShowDirsOnly
                );
    if(!selectedPath.isEmpty())
    {
        this->videoPathEdit->setText(selectedPath);
    }
}

void SettingWidget::on_confirmBtn()
{
    int ret=QMessageBox::question(this,"确认保存","是否确认保存当前设置？保存后将跳转至主界面"
                                  ,QMessageBox::Yes|QMessageBox::No,QMessageBox::No);
    if(ret!=QMessageBox::Yes)
    {
        return;
    }

    QString errorMsg;
    if(!this->validateConfig(errorMsg))
    {
        QMessageBox::warning(this,"配置无效",errorMsg);
        return;
    }
    this->saveUIToConfig();
    emit configSaved();
    this->close();
}


void SettingWidget::on_cancelBtn()
{
    // 无配置文件时提示"退出后需重新设置"
    if (!isConfigExists())
    {
        int ret = QMessageBox::warning(
            this, "警告", "未保存配置，退出后下次启动需重新设置！是否退出？",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No
        );
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    this->close();
}

void SettingWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QPixmap bgPixmap(":/images/back2.png");
    // 按窗口大小缩放图片（覆盖整个窗口，可能拉伸）
    painter.drawPixmap(rect(), bgPixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void SettingWidget::initQss()
{
    QFile file(":/setwin.qss");  // 确保资源文件路径正确
    qDebug() << "QSS文件打开成功：";
   if (!file.open(QFile::ReadOnly | QFile::Text)) {  // 增加错误判断
       qDebug() << "QSS文件打开失败：" << file.errorString();
       return;
   }
   QString styleSheet = QLatin1String(file.readAll());
   this->setStyleSheet(styleSheet);
   qApp->setStyleSheet(styleSheet);
   file.close();  // 确保文件关闭
}


