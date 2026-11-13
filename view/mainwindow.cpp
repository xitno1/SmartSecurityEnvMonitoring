#include "mainwindow.h"
#include <QStyle>
#include <QCameraInfo>
#include <QDir>
#include<QApplication>

MainWindow::MainWindow(QWidget *parent)
{
    // 所有指针成员初始化为nullptr，防止析构时访问野指针
    this->cameraThread = nullptr;
    this->playbackWidget = nullptr;
    this->loginWidget = nullptr;
    this->renderTimer = nullptr;
    this->chartUpdateTimer = nullptr;
    this->alarmCheckTimer = nullptr;
    this->alarmPageTimer = nullptr;
    this->serial = nullptr;
    this->initUI();
    this->seria_port_init();
    this->initConect();
    this->initCharts();
    this->initDeviceList();
    this->onSettingSaved();
}

MainWindow::~MainWindow()
{
    // 停止所有定时器，避免在析构期间触发信号
    if (renderTimer) {
        renderTimer->stop();
    }
    if (chartUpdateTimer) {
        chartUpdateTimer->stop();
    }
    if (alarmCheckTimer) {
        alarmCheckTimer->stop();
    }
    if (alarmPageTimer) {
        alarmPageTimer->stop();
    }

    if (serial)
    {
        serial->close();
        delete serial;
        serial = nullptr;
    }

    if (cameraThread)
    {
        cameraThread->requestInterruption();
        // 等待最多5秒让FFmpeg正常退出，避免强制terminate
        if (!cameraThread->wait(5000)) {
            qWarning() << "CameraThread 未能正常退出，强制终止";
            cameraThread->terminate();
            cameraThread->wait(1000);
        }
        delete cameraThread;
        cameraThread = nullptr;
    }
}

void MainWindow::initUI()
{
    this->setWindowTitle("监控主界面");
    this->resize(1920,1080);
    this->mainLayout=new QHBoxLayout;
    this->leftWidget=new QWidget();
    this->midWidget=new QWidget();
    this->rightWidget=new QWidget();

    this->mainLayout->addWidget(this->leftWidget,3);
    this->mainLayout->addWidget(this->midWidget,4);
    this->mainLayout->addWidget(this->rightWidget,3);
    this->mainLayout->setSpacing(0);
   // 清除主布局的内边距（默认是9）
   this->mainLayout->setContentsMargins(0, 0, 0, 0);

    this->leftLayout=new QVBoxLayout;
    this->leftLayout->setSpacing(5);
        this->leftTopWidget=new QWidget();
        this->leftTopWidget->setStyleSheet("background-color:#0F3460");
        this->leftTopLayout=new QVBoxLayout;
        this->leftTopLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
            //------光照部分：图片+数值
            QHBoxLayout *luxLayout=new QHBoxLayout();
            this->luxLabel=new QLabel;
            this->luxLabel->setMinimumSize(50,50);
            this->luxLabel->setPixmap(QPixmap(":/images/lux.png"));
            this->luxValueLabel=new QLabel("光照：--lux");
            this->luxValueLabel->setStyleSheet("color:white; font-size:16px;");
            luxLayout->addWidget(this->luxLabel);
            luxLayout->addWidget(this->luxValueLabel);
            this->leftTopLayout->addLayout(luxLayout);
            //-----------温度部分---------------------------
            QHBoxLayout* tempLayout=new QHBoxLayout;
            this->tempLabel=new QLabel;
            this->tempLabel->setMinimumSize(50,50);
            this->tempLabel->setPixmap(QPixmap(":/images/temp.png"));
            this->tempValueLabel=new QLabel("温度：--℃");
            this->tempValueLabel->setStyleSheet("color:white; font-size:16px;");
            tempLayout->addWidget(this->tempLabel);
            tempLayout->addWidget(this->tempValueLabel);
            this->leftTopLayout->addLayout(tempLayout);
            //---------湿度部分-----------------
            QHBoxLayout * humLayout=new QHBoxLayout;
            this->humLabel=new QLabel;
            this->humLabel->setMinimumSize(50,50);
            this->humLabel->setPixmap(QPixmap(":/images/hum.png"));
            this->humValueLabel = new QLabel("湿度：-- %");  // 初始化湿度数值标签
            this->humValueLabel->setStyleSheet("color:white; font-size:16px;");
            humLayout->addWidget(this->humLabel);
            humLayout->addWidget(this->humValueLabel);
            this->leftTopLayout->addLayout(humLayout);
            //-------灯控按钮和风扇控制---------------------
            QHBoxLayout *controlLayout = new QHBoxLayout();
            controlLayout->setSpacing(5); // 两个按钮之间的间距
            // 灯控制按钮
            this->ledControlBtn = new QPushButton("开灯");
            this->ledControlBtn->setStyleSheet("color:white; font-size:16px;"
            "background-color:#1A73E8; padding:8px; border-radius:4px;");
            this->isLedOn = false;
            controlLayout->addWidget(ledControlBtn);

            // 风扇控制按钮
            this->fanControlBtn = new QPushButton("开风扇");
            this->fanControlBtn->setStyleSheet("color:white; font-size:16px; "
            "background-color:#1A73E8; padding:8px; border-radius:4px;");
            this->isFanOn = false;
            controlLayout->addWidget(fanControlBtn);
            this->leftTopLayout->addLayout(controlLayout);

        this->leftTopWidget->setLayout(this->leftTopLayout);
        //------左侧中间图表区域----------------------------------
        this->leftMidWidget=new QWidget();
        this->leftMidWidget->setStyleSheet("background-color:#0F3460");
         //为图表创建布局
        QVBoxLayout *chartLayout=new QVBoxLayout(leftMidWidget);
        chartLayout->setContentsMargins(1,1,1,1);

        //------------左下方柱状图---------------
        this->leftButtonWidget=new QWidget();
        this->leftButtonWidget->setStyleSheet("background-color:#0F3460");
        QVBoxLayout *weekChartLayout=new QVBoxLayout(leftButtonWidget);
        weekChartLayout->setContentsMargins(1,1,1,1);
        this->initWeekTempChart();
        weekChartLayout->addWidget(this->weekTempChartView);


    this->leftLayout->addWidget(this->leftTopWidget,1);
    this->leftLayout->addWidget(this->leftMidWidget,2);
    this->leftLayout->addWidget(this->leftButtonWidget,2);
    this->leftWidget->setLayout(this->leftLayout);

    this->midLayout=new QVBoxLayout;
    this->midLayout->setSpacing(5);
        this->midTittleWidget=new QWidget();
        this->midTittleWidget->setStyleSheet("background-color:#0F3460");
        this->midtopLayout=new QHBoxLayout;
        this->title=new QLabel("智能机房环境监测系统",this);
        this->title->setStyleSheet(R"(color: white;font-weight: bold;font-size: 30px;font-family: "Microsoft YaHei";
        padding: 10px;)");
        this->title->setAlignment(Qt::AlignCenter);
        this->midtopLayout->addWidget(this->title);
        this->midTittleWidget->setLayout(this->midtopLayout);

        this->midButtonWidget=new QWidget();
        this->midButtonWidget->setStyleSheet("background-color:#0F3460");
        this->midButtonLayout=new QHBoxLayout;
        this->midButtonLayout->setContentsMargins(5, 5, 5, 5);
        this->midButtonLayout->setSpacing(5); // 控件之间的间距
        // 先添加伸缩空间，将后续控件推到右边
        this->midButtonLayout->addStretch();
        this->oneLabel=new QLabel;
        this->oneLabel->setMinimumSize(50,50);
        this->oneLabel->setPixmap(QPixmap(":/images/one.png"));
        this->fourLabel=new QLabel;
        this->fourLabel->setMinimumSize(50,50);
        this->fourLabel->setPixmap(QPixmap(":/images/four.png"));
        this->midButtonLayout->addWidget(this->oneLabel);
        this->midButtonLayout->addWidget(this->fourLabel);
        this->midButtonLayout->addStretch();
        this->loginLabel=new QLabel;
        this->loginLabel->setMinimumSize(50,50);
        this->loginLabel->setPixmap(QPixmap(":/images/login.png"));
        this->midButtonLayout->addWidget(this->loginLabel);
        this->midButtonWidget->setLayout(this->midButtonLayout);

        this->midVideoWidget=new QWidget();
        this->midVideoWidget->setStyleSheet("background-color:#0F3460");
        this->initVideoDisplay();


    this->midLayout->addWidget(this->midTittleWidget,1);
    this->midLayout->addWidget(this->midVideoWidget,6);
    this->midLayout->addWidget(this->midButtonWidget,2);
    this->midWidget->setLayout(this->midLayout);

        this->rightLayout=new QVBoxLayout;
        this->rightLayout->setSpacing(5);
        this->rightTopWidget=new QWidget();
        this->rightTopWidget->setStyleSheet("background-color:#0F3460");

        this->rightMidWidget=new QWidget();
        this->rightMidWidget->setStyleSheet("background-color:#0F3460");
        QVBoxLayout *rightMidLayout = new QVBoxLayout(rightMidWidget);
        rightMidLayout->addWidget(new QLabel("告警列表", this), 0, Qt::AlignCenter);

        // 初始化告警表格
       alarmTableWidget = new QTableWidget(5, 3, this);  // 5行3列
       alarmTableWidget->setHorizontalHeaderLabels(QStringList() << "告警类型" << "告警等级" << "告警时间");
       // ==========设置告警表格列宽 ==========
       alarmTableWidget->horizontalHeader()->setStretchLastSection(false);
       alarmTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);      // 类型可拉伸
       alarmTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // 等级自适应
       alarmTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);  // 时间手动设置
       alarmTableWidget->setColumnWidth(2, 140);  // 设置时间列固定宽度
       alarmTableWidget->setStyleSheet("background-color:#0F3460; color:gray;");
       alarmTableWidget->verticalHeader()->setVisible(false);
       rightMidLayout->addWidget(alarmTableWidget);

        this->rightButtonWidget=new QWidget();
        this->rightButtonWidget->setStyleSheet("background-color:#0F3460");
        QVBoxLayout *rightBottomLayout = new QVBoxLayout(rightButtonWidget);
        rightBottomLayout->addWidget(new QLabel("告警分布", this), 0, Qt::AlignCenter);
        // 初始化告警饼图
        alarmPieSeries = new QPieSeries();
        alarmPieChart = new QChart();
        alarmPieChart->addSeries(alarmPieSeries);
        alarmPieChart->setTitle("告警类型占比|温度=红色|湿度=蓝色|光照=黄色");
        alarmPieChart->setTitleBrush(QBrush(QColor("white")));
        alarmPieChart->setBackgroundBrush(QBrush(QColor("#4A90E2")));
        alarmPieChartView = new QChartView(alarmPieChart);
        alarmPieChartView->setStyleSheet("background-color:#0F3460;");
        rightBottomLayout->addWidget(alarmPieChartView);

    this->rightLayout->addWidget(this->rightTopWidget,1);
    this->rightLayout->addWidget(this->rightMidWidget,1);
    this->rightLayout->addWidget(this->rightButtonWidget,1);
    this->rightWidget->setLayout(this->rightLayout);

    // 渲染帧率控制：每100ms从缓存渲染一帧到UI，避免过度刷新导致卡顿
    renderTimer = new QTimer(this);
    renderTimer->setInterval(100); // 10 FPS
    canRender = false;
    lastFrame = QImage(); // 初始为空
    connect(renderTimer, &QTimer::timeout, this, [this]()
    {
        if (!lastFrame.isNull() && canRender)
        {
            if (isSingleChannel)
            {
                videoLabel1->setPixmap(QPixmap::fromImage(lastFrame));
            }
            else
            {
                QSize labelSize = videoLabel1_4->size();
                QImage scaledImg = lastFrame.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                videoLabel1_4->setPixmap(QPixmap::fromImage(scaledImg));
            }
            canRender = false;
        }
    });
    renderTimer->start();

    // 初始化告警定时器
    this->alarmCheckTimer=new QTimer(this);
    this->alarmCheckTimer->setInterval(5000);//每5秒检测一次警告
    connect(this->alarmCheckTimer,&QTimer::timeout,this,&MainWindow::checkAlarms);
    this->alarmCheckTimer->start();

    alarmPageTimer = new QTimer(this);
    alarmPageTimer->setInterval(10000);  // 每10秒切换一页
    connect(alarmPageTimer, &QTimer::timeout, this, &MainWindow::onAlarmPageTimeout);
    alarmPageTimer->start();

    this->setLayout(mainLayout);
}

void MainWindow::initConect()
{
    connect(ledControlBtn,&QPushButton::clicked,this,&MainWindow::onLedControlClicked);
    connect(fanControlBtn, &QPushButton::clicked, this, &MainWindow::onFanControlClicked);
    connect(loginLabel, &QLabel::linkActivated, this, &MainWindow::onLoginLabelClicked);

    // 保留鼠标样式设置
   loginLabel->setCursor(Qt::PointingHandCursor);
   loginLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
   loginLabel->setOpenExternalLinks(false);

   //  给 loginLabel 安装事件过滤器（监听点击事件）
   loginLabel->installEventFilter(this);
}

void MainWindow::seria_port_init()
{

    this->serial=new QSerialPort(this);
    QSettings settings("../config.ini",QSettings::IniFormat);
    QString portName=settings.value("SerialConfig/SerialPort").toString();
    if(portName.isEmpty())
    {
        QList<QSerialPortInfo>ports=QSerialPortInfo::availablePorts();
        if(!ports.isEmpty())
        {
            portName=ports.first().portName();
            settings.setValue("SerialConfig/SerialPort",portName);
        }
        else
        {
            qDebug() << "未找到可用串口";
            luxValueLabel->setText("未找到串口");
            return;
        }
    }
    // 设置串口名称
   serial->setPortName(portName);

   // 配置串口参数
   serial->setBaudRate(QSerialPort::Baud115200);
   serial->setDataBits(QSerialPort::Data8);
   serial->setParity(QSerialPort::NoParity);
   serial->setStopBits(QSerialPort::OneStop);
   serial->setFlowControl(QSerialPort::NoFlowControl);

   //打开串口
   if(serial->open(QIODevice::ReadWrite))
   {
       qDebug() << "串口打开成功：" << portName;
       // 连接串口数据接收信号与槽函数
       connect(serial, &QSerialPort::readyRead, this, &MainWindow::readSerialData);//读取温湿度光照
       this->dataRequestTimer=new QTimer(this);
       connect(dataRequestTimer,&QTimer::timeout,this,&MainWindow::sendGetDataCommand);
       dataRequestTimer->start(1000);
   }
   else
   {
       qDebug() << "串口打开失败：" << serial->errorString();
       luxValueLabel->setText("串口打开失败");
   }
}

void MainWindow::initCharts()
{
    this->chart=new QChart();
    this->chart->setTitle("10秒温湿度光照曲线");
    this->chart->setTitleBrush(QBrush(Qt::black));
    chart->setBackgroundBrush(QBrush(QColor("#4A90E2")));
    //创建曲线
    this->seriesLux=new QSplineSeries();
    this->seriesTemp=new QSplineSeries();
    this->seriesHum=new QSplineSeries();

    //设置曲线属性
    this->seriesLux->setName("光照（lux）");
    this->seriesLux->setPen(QPen(Qt::yellow,2));
    this->seriesTemp->setName("温度 (°C)");
    this->seriesTemp->setPen(QPen(Qt::red, 2));
    this->seriesHum->setName("湿度 (%)");
    this->seriesHum->setPen(QPen(Qt::cyan, 2));
    //添加曲线到图表
    this->chart->addSeries(this->seriesLux);
    this->chart->addSeries(this->seriesTemp);
     this->chart->addSeries(this->seriesHum);

    //创建x轴
    this->axisX = new QDateTimeAxis();
    this->axisX->setFormat("hh:mm:ss");
    this->axisX->setTitleText("时间");
    this->axisX->setLabelsColor(Qt::white);
    this->axisX->setTitleBrush(QBrush(Qt::white));
    this->axisX->setLabelsFont(QFont("Microsoft YaHei", 8)); // 调整坐标轴标签字体
    this->axisX->setTitleFont(QFont("Microsoft YaHei", 9, QFont::Bold)); // 调整坐标轴标题字体
    this->chart->addAxis(axisX, Qt::AlignBottom);

    // 创建Y轴（优化显示）
    this->axisYLux = new QValueAxis();
    this->axisYLux->setRange(0, 200);
    this->axisYLux->setTitleText("光照（lux）");
    this->axisYLux->setTitleBrush(QBrush(Qt::yellow));
    this->axisYLux->setLabelsColor(Qt::yellow);
    this->axisYLux->setLabelsFont(QFont("Microsoft YaHei", 8));
    this->axisYLux->setTitleFont(QFont("Microsoft YaHei", 9, QFont::Bold));
    this->axisYLux->setTickCount(6); // 增加刻度数量，避免拥挤
    this->chart->addAxis(axisYLux, Qt::AlignLeft);

    this->axisYTemp = new QValueAxis();
    this->axisYTemp->setRange(10, 40);
    this->axisYTemp->setTitleText("温度 (°C)");
    this->axisYTemp->setLabelsColor(Qt::red);
    this->axisYTemp->setTitleBrush(QBrush(Qt::red));
    this->axisYTemp->setLabelsFont(QFont("Microsoft YaHei", 8));
    this->axisYTemp->setTitleFont(QFont("Microsoft YaHei", 9, QFont::Bold));
    this->axisYTemp->setTickCount(6);
    this->chart->addAxis(axisYTemp, Qt::AlignRight);

    this->axisYHum = new QValueAxis();
    this->axisYHum->setRange(0, 50);
    this->axisYHum->setTitleText("湿度 (%)");
    this->axisYHum->setLabelsColor(Qt::cyan);
    this->axisYHum->setTitleBrush(QBrush(Qt::cyan));
    this->axisYHum->setLabelsFont(QFont("Microsoft YaHei", 8));
    this->axisYHum->setTitleFont(QFont("Microsoft YaHei", 9, QFont::Bold));
    this->axisYHum->setTickCount(6);
    this->chart->addAxis(axisYHum, Qt::AlignRight);

    //关联曲线与坐标
    this->seriesLux->attachAxis(this->axisX);
    this->seriesLux->attachAxis(this->axisYLux);
    this->seriesTemp->attachAxis(this->axisX);
    this->seriesTemp->attachAxis(this->axisYTemp);
    this->seriesHum->attachAxis(this->axisX);
    this->seriesHum->attachAxis(this->axisYHum);

    //创建图表视图并添加到左侧中间区域
    this->chartView=new QChartView(chart);
    this->chartView->setRenderHint(QPainter::Antialiasing);//抗锯齿
    this->chartView->setStyleSheet("background-color:#0F3460;");  // 视图背景色
    this->leftMidWidget->layout()->addWidget(chartView);

    //初始化图表更新定时器
    this->chartUpdateTimer=new QTimer(this);
    this->chartUpdateTimer->setInterval(10000);//10000毫秒=10秒
    connect(this->chartUpdateTimer,&QTimer::timeout,this,&MainWindow::onChartUpdateTimer);
    this->chartUpdateTimer->start();

     this->updateChartData(0.0, 25.0, 50.0);
}

void MainWindow::updateChartData(double lux, double temp, double hum)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 timeStamp = currentTime.toMSecsSinceEpoch();

    // 添加新数据点
    seriesLux->append(timeStamp, lux);
    seriesTemp->append(timeStamp, temp);
    seriesHum->append(timeStamp, hum);

    // 限制数据点数量
    const int maxPoints = 2;
    if (seriesLux->count() > maxPoints) {
        seriesLux->remove(0, seriesLux->count() - maxPoints);
        seriesTemp->remove(0, seriesTemp->count() - maxPoints);
        seriesHum->remove(0, seriesHum->count() - maxPoints);
    }

    // 动态调整X轴范围（显示最近10秒数据）
    axisX->setRange(currentTime.addSecs(-10), currentTime);

    // 动态调整Y轴范围
    axisYLux->setRange(qMax(0.0, lux - 50), qMin(500.0, lux + 50));
    axisYTemp->setRange(qMax(0.0, temp - 5), qMin(50.0, temp + 5));
    axisYHum->setRange(qMax(0.0, hum - 10), qMin(100.0, hum + 10));
}

void MainWindow::readSerialData()
{
    buffer += serial->readAll();  // 读取数据到缓存区
    QStringList lines = buffer.split("\r\n");  // 按行分割
    buffer = lines.takeLast();  // 保留未完成的行

   // 解析每一行数据
   for (const QString &line : lines)
   {
       if (line.isEmpty()) continue;

       // 数据格式："Lux:123.45;Hum:67.89;Temp:23.45"
       if (line.contains("Lux:") && line.contains("Hum:") && line.contains("Temp:"))
       {
           // 提取光照值
           int luxStart = line.indexOf("Lux:") + 4;
           int luxEnd = line.indexOf(";", luxStart);
           QString luxStr = line.mid(luxStart, luxEnd - luxStart);
           luxValueLabel->setText(QString("光照：%1 lux").arg(luxStr));
           this->currentLux=luxStr.toDouble();

           // 提取湿度值
           int humStart = line.indexOf("Hum:") + 4;
           int humEnd = line.indexOf(";", humStart);
           QString humStr = line.mid(humStart, humEnd - humStart);
           humValueLabel->setText(QString("湿度：%1 %").arg(humStr));
           this->currentHum=humStr.toDouble();

           // 提取温度值
           int tempStart = line.indexOf("Temp:") + 5;
           QString tempStr = line.mid(tempStart);
           tempValueLabel->setText(QString("温度：%1 °C").arg(tempStr));
           this->currentTemp=tempStr.toDouble();
           QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
           //存储数据到数据库
           QString sql=QString("insert into environment_data"
                               "(temperature,humidity,light_intensity,collect_time) "
                               "values (%1,%2,%3, '%4')").arg(currentTemp).arg(currentHum).arg(currentLux).arg(currentTime);

           SingletonDB::getInstance()->execSql(sql);
       }
   }
}

void MainWindow::sendGetDataCommand()
{
    if(serial&&serial->isOpen())
    {
        serial->write("GETDATA\r\n");
    }
}

void MainWindow::onChartUpdateTimer()
{
    this->updateChartData(this->currentLux,this->currentTemp,this->currentHum);
}


void MainWindow::updateVideoFrame(QImage img)
{
    // 只缓存最新帧，不做直接渲染
    // 渲染由 renderTimer 在UI线程中按固定帧率执行，避免过度刷新
    lastFrame = img;  // img 已经在 CameraThread 中深拷贝过，无需再次 copy()
    canRender = true;
}



bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==this->oneLabel&&event->type()==QEvent::MouseButtonPress)
    {
        this->switchChannelMode(true);
        return  true;
    }
    else if(obj==this->fourLabel&&event->type()==QEvent::MouseButtonPress)
    {
        this->switchChannelMode(false);
        return true;
    }
    else if(obj == this->loginLabel && event->type() == QEvent::MouseButtonPress)
    {
        this->onLoginLabelClicked(); // 触发登录逻辑
        return true;
    }
    return  QWidget::eventFilter(obj,event);
}

void MainWindow::onLedControlClicked()
{
    if(!serial||!serial->isOpen())
    {
        QMessageBox::warning(this,"警告","串口未打开，无法控制设备");
        return;
    }
    this->isLedOn=!isLedOn;
    if(isLedOn)
    {
        serial->write("LEDON\r\n");
        ledControlBtn->setText("关灯");
    }
    else {
        serial->write("LEDOFF\r\n");
        ledControlBtn->setText("开灯");
    }
}

void MainWindow::onFanControlClicked()
{
    if(!serial||!serial->isOpen())
    {
        QMessageBox::warning(this,"警告","串口未打开，无法控制设备");
        return;
    }
    this->isFanOn=!isFanOn;
    if(isFanOn)
    {
        this->serial->write("FANON\r\n");
        fanControlBtn->setText("关风扇");
    }
    else {
        this->serial->write("FANOFF\r\n");
        this->fanControlBtn->setText("开风扇");
    }
}

void MainWindow::initWeekTempChart()
{
    this->weekTempChart = new QChart();

    // 计算最近7天的日期范围
    QDate startDate = QDate::currentDate().addDays(-6);
    QDate endDate = QDate::currentDate();
    QString dateRange = QString("%1 至 %2")
            .arg(startDate.toString("yy-MM-dd"))  // 两位年份，如 25-11-29
            .arg(endDate.toString("yy-MM-dd"));

    // 设置包含日期范围的标题
    this->weekTempChart->setTitle(QString("近一周最高/最低温度（%1）").arg(dateRange));
    this->weekTempChart->setTitleBrush(QBrush(Qt::white));
    this->weekTempChart->setBackgroundBrush(QBrush(QColor("#4A90E2")));

    this->weekTempSeries = new QBarSeries();
    this->weekTempSeries->setBarWidth(0.4);
    this->weekTempChart->addSeries(this->weekTempSeries);

    // X轴（星期）
    this->weekTempAxisX = new QCategoryAxis();
    this->weekTempAxisX->setLabelsColor(Qt::white);
    this->weekTempAxisX->setTitleBrush(QBrush(Qt::white));
    this->weekTempAxisX->setTitleText("星期");
    this->weekTempChart->addAxis(weekTempAxisX, Qt::AlignBottom);
    this->weekTempSeries->attachAxis(weekTempAxisX);

    // Y轴（温度）
    this->weekTempAxisY = new QValueAxis();
    this->weekTempAxisY->setRange(10, 40);
    this->weekTempAxisY->setLabelsColor(Qt::white);
    this->weekTempAxisY->setTitleBrush(QBrush(Qt::white));
    this->weekTempAxisY->setTitleText("温度 (°C)");
    this->weekTempChart->addAxis(weekTempAxisY, Qt::AlignLeft);
    this->weekTempSeries->attachAxis(weekTempAxisY);

    // 图表视图
    this->weekTempChartView = new QChartView(this->weekTempChart);
    this->weekTempChartView->setRenderHint(QPainter::Antialiasing);
    this->weekTempChartView->setStyleSheet("background-color:#0F3460;");

    // 定时更新，包含标题更新
    this->weekChartUpdateTimer = new QTimer(this);
    this->weekChartUpdateTimer->setInterval(300000); // 5分钟更新一次
    connect(this->weekChartUpdateTimer, &QTimer::timeout, this, [this]() {
        // 定时更新时同步更新标题日期
        QDate startDate = QDate::currentDate().addDays(-6);
        QDate endDate = QDate::currentDate();
        QString dateRange = QString("%1 至 %2").arg(startDate.toString("yyyy-MM-dd"), endDate.toString("yyyy-MM-dd"));
        this->weekTempChart->setTitle(QString("近一周最高/最低温度（%1）").arg(dateRange));
        this->getWeekTempData();
    });
    this->weekChartUpdateTimer->start();

    // 初始加载数据
    this->getWeekTempData();
}

QList<QPair<double, double>> MainWindow::updateWeekTempChart()
{
    QList<QPair<double,double>> weekData;
    // 获取过去七天的日期（从6天前到今天）
    for(int i = 6; i >= 0; i--)
    {
        QDate date = QDate::currentDate().addDays(-i);
        QString dateStr = date.toString("yyyy-MM-dd");

        // 查询当天最高气温
        QString highSql = QString("SELECT MAX(temperature) FROM environment_data "
                                 "WHERE date(collect_time) = '%1'").arg(dateStr);
        // 查询当天最低温度
        QString lowSql = QString("SELECT MIN(temperature) FROM environment_data "
                                "WHERE date(collect_time) = '%1'").arg(dateStr);

        char **res = nullptr;
        int row, col;
        double highTemp = 0; // 初始化为极端值
        double lowTemp = 0;   // 初始化为极端值

        // 获取最高温度
        if(SingletonDB::getInstance()->selectSql(highSql, res, row, col) == SQLITE_OK && row > 0 && col > 0 && res != nullptr)
        {
            QString highStr = res[1];
            if(!highStr.isEmpty() && highStr != "NULL") // 处理空值或NULL
                highTemp = highStr.toDouble();
            sqlite3_free_table(res);
            res = nullptr;
        }

        // 获取最低温度
        if(SingletonDB::getInstance()->selectSql(lowSql, res, row, col) == SQLITE_OK && row > 0 && col > 0 && res != nullptr)
        {
            QString lowStr = res[1];
            if(!lowStr.isEmpty() && lowStr != "NULL") // 处理空值或NULL
                lowTemp = lowStr.toDouble();
            sqlite3_free_table(res);
            res = nullptr;
        }

        // 如果没有数据，使用默认值（如当天的平均温度）
        if(highTemp == -100) highTemp = currentTemp; // 用当前温度代替
        if(lowTemp == 100) lowTemp = currentTemp - 2; // 用当前温度减2代替
        weekData.append(QPair<double, double>(highTemp, lowTemp));
    }
    return weekData;
}

QList<QPair<double, double> > MainWindow::getWeekTempData()
{
    // 1. 清除现有系列数据
    qDeleteAll(weekTempSeries->barSets());
    weekTempSeries->clear();

    // 2. 创建数据集（最高温和最低温）
    QBarSet *highTempSet = new QBarSet("最高温度 (°C)");
    QBarSet *lowTempSet = new QBarSet("最低温度 (°C)");
    highTempSet->setBrush(Qt::red);
    lowTempSet->setBrush(Qt::lightGray);
    // 设置柱子边框，增强可见性
    highTempSet->setPen(QPen(Qt::black, 1));
    lowTempSet->setPen(QPen(Qt::black, 1));

    // 3. 获取一周数据
    QList<QPair<double, double>> weekData = updateWeekTempChart();

    // 4. 设置X轴标签
    QFont axisFont("Microsoft YaHei", 7);
    weekTempAxisX->setLabelsFont(axisFont);
    const int totalDays = 7;
    for(int i = 0; i < totalDays; i++)
    {
        QDate date = QDate::currentDate().addDays(-(6 - i));
        QString dayStr = QString("周%1").arg(date.dayOfWeek() == 7 ? "日" : QString::number(date.dayOfWeek()));
        weekTempAxisX->append(dayStr, i);
    }
    weekTempAxisX->setRange(0, totalDays - 1);

    // 5. 添加数据到数据集（确保每个日期都有数据）
    for (const auto &data : weekData)
    {
        // 强制添加有效数据
        highTempSet->append(qMax(0.0, data.first));
        lowTempSet->append(qMax(0.0, data.second));
    }

    // 6. 添加数据集到系列
    weekTempSeries->append(lowTempSet);
    weekTempSeries->append(highTempSet);

    return weekData;
}

void MainWindow::initVideoDisplay()
{
    // ========== 单通道专用 Label ==========
    this->videoLabel1 = new QLabel(this->midVideoWidget);
    this->videoLabel1->setStyleSheet("background-color: #333333;");
    this->videoLabel1->setAlignment(Qt::AlignCenter);
    this->videoLabel1->setText("摄像头1");
    this->videoLabel1->setScaledContents(true);
    this->videoLabel1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ========== 四通道专用 Label ==========
    QLabel *videoLabel1_4 = new QLabel(this->midVideoWidget);
    videoLabel1_4->setStyleSheet("background-color: #333333; border:1px solid white;");
    videoLabel1_4->setAlignment(Qt::AlignCenter);
    videoLabel1_4->setText("摄像头1");
    videoLabel1_4->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    videoLabel1_4->setScaledContents(true);

    // ========== 备用通道2-4：使用QLabel ==========
    videoLabel2 = new QLabel(this->midVideoWidget);
    videoLabel2->setStyleSheet("background-color: #333333; border:1px solid white;");
    videoLabel2->setAlignment(Qt::AlignCenter);
    videoLabel2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); //可被拉伸或者压缩
    videoLabel2->setScaledContents(true);
    videoLabel2->setText("备用通道2");
    videoLabel2->setMinimumSize(200, 150); // 设置最小尺寸，确保可见

    videoLabel3 = new QLabel(this->midVideoWidget);
    videoLabel3->setStyleSheet("background-color: #333333; border:1px solid white;");
    videoLabel3->setAlignment(Qt::AlignCenter);
    videoLabel3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    videoLabel3->setScaledContents(true);
    videoLabel3->setText("备用通道3");
    videoLabel3->setMinimumSize(200, 150); // 设置最小尺寸

    videoLabel4 = new QLabel(this->midVideoWidget);
    videoLabel4->setStyleSheet("background-color: #333333; border:1px solid white;");
    videoLabel4->setAlignment(Qt::AlignCenter);
    videoLabel4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    videoLabel4->setScaledContents(true);
    videoLabel4->setText("备用通道4");
    videoLabel4->setMinimumSize(200, 150); // 设置最小尺寸

       // 检查图片是否加载成功
       QString houseImagePath = ":/images/house.png";
       QPixmap backupPixmap(houseImagePath);

       if (!backupPixmap.isNull())
       {
           qDebug() << "备用图片加载成功，尺寸：" << backupPixmap.size();

           // 缩放图片到合适大小
           int targetWidth = 300; // 设置一个合理的宽度
           int targetHeight = 200; // 设置一个合理的高度
           QPixmap scaledPixmap = backupPixmap.scaled(
               targetWidth, targetHeight,
               Qt::KeepAspectRatio,
               Qt::SmoothTransformation
           );

           qDebug() << "缩放后尺寸：" << scaledPixmap.size();

           // 设置图片
           videoLabel2->setPixmap(scaledPixmap);
           videoLabel2->setText(""); // 清除文本
           videoLabel2->setToolTip("备用通道2 - 已加载图片");

           videoLabel3->setPixmap(scaledPixmap.copy());
           videoLabel3->setText("");
           videoLabel3->setToolTip("备用通道3 - 已加载图片");

           videoLabel4->setPixmap(scaledPixmap.copy());
           videoLabel4->setText("");
           videoLabel4->setToolTip("备用通道4 - 已加载图片");

           qDebug() << "已为通道2-4设置图片";
       }
       else
       {
           qDebug() << "图片加载失败，路径：" << houseImagePath;
           // 设置背景色以便看到Label
           videoLabel2->setStyleSheet("background-color: red; border:1px solid white; color: white;");
           videoLabel3->setStyleSheet("background-color: green; border:1px solid white; color: white;");
           videoLabel4->setStyleSheet("background-color: blue; border:1px solid white; color: white;");
       }

    // ========== 四通道布局,强制平分==========
    QWidget *fourChannelWidget = new QWidget(this->midVideoWidget);
    this->videoGridLayout = new QGridLayout(fourChannelWidget);
    // 拉伸因子，确保行列平分
    videoGridLayout->setRowStretch(0, 1);
    videoGridLayout->setRowStretch(1, 1);
    videoGridLayout->setColumnStretch(0, 1);
    videoGridLayout->setColumnStretch(1, 1);
    // 添加控件
    this->videoGridLayout->addWidget(videoLabel1_4, 0, 0);
    this->videoGridLayout->addWidget(videoLabel2, 0, 1);
    this->videoGridLayout->addWidget(videoLabel3, 1, 0);
    this->videoGridLayout->addWidget(videoLabel4, 1, 1);
    videoGridLayout->setSpacing(5);
    this->videoGridLayout->setContentsMargins(1,1,1, 1);

    // ========== 单通道布局 ==========
    QWidget *singleChannelWidget = new QWidget(this->midVideoWidget);
    QVBoxLayout *singleLayout = new QVBoxLayout(singleChannelWidget);
    singleLayout->addWidget(this->videoLabel1);
    singleLayout->setContentsMargins(0, 0, 0, 0);

    // ========== 栈布局 ==========
    this->videoStackLayout = new QStackedLayout(this->midVideoWidget);
    this->videoStackLayout->addWidget(singleChannelWidget);
    this->videoStackLayout->addWidget(fourChannelWidget);

    this->videoLabel1_4 = videoLabel1_4;
    this->isSingleChannel = true;
    this->videoStackLayout->setCurrentIndex(0);

    // 事件绑定
    this->oneLabel->setCursor(Qt::PointingHandCursor);
    this->fourLabel->setCursor(Qt::PointingHandCursor);
    this->oneLabel->installEventFilter(this);
    this->fourLabel->installEventFilter(this);
}

void MainWindow::loadVideoConfig()
{
    QSettings settings("../config.ini",QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    //加载摄像头配置
    this->currentCamera=settings.value("VideoConfig/DefaultCamera").toString();
    //加载视频保存路径
    this->videoSavePath=settings.value("VideoConfig/VideoSavePath","../fileOut/Video").toString();
    QDir videoDir(videoSavePath);
    if(!videoDir.exists())
    {
        videoDir.mkpath(videoSavePath);
    }
    //加载采集间隔
    this->captureInterval=settings.value("VideoConfig/CaptureInterval",30).toInt();
}

void MainWindow::startCameraMonitoring()
{
    if (this->cameraThread)
    {
        this->cameraThread->requestInterruption();
        if (!this->cameraThread->wait(5000)) {
            qWarning() << "旧CameraThread未能正常退出，强制终止";
            this->cameraThread->terminate();
            this->cameraThread->wait(1000);
        }
        delete this->cameraThread;
        this->cameraThread = nullptr;
    }

    qDebug() << "启动摄像头线程，设备名：" << this->currentCamera;

    this->cameraThread = new CameraThread(this->currentCamera);

    // 使用 Qt::AutoConnection 即可 — 发射者在线程中，接收者在主线程，自动变为 QueuedConnection
    connect(cameraThread, &CameraThread::sendimg, this, &MainWindow::updateVideoFrame);
    connect(cameraThread, &CameraThread::errorOccurred, this, &MainWindow::handleCameraError);
    // 注意：不要将 finished 信号直接置空 cameraThread 指针
    // 析构函数负责统一回收，避免与析构流程竞态

    this->cameraThread->start();
}

void MainWindow::switchChannelMode(bool singleChannel)
{
    this->isSingleChannel = singleChannel;
    this->videoStackLayout->setCurrentIndex(singleChannel ? 0 : 1);

    if (singleChannel)
    {
        qDebug() << "切换到单通道";
        // 单通道模式下，清空备用通道的显示
        videoLabel2->clear();
        videoLabel3->clear();
        videoLabel4->clear();
        videoLabel2->setText("备用通道2");
        videoLabel3->setText("备用通道3");
        videoLabel4->setText("备用通道4");
        // 恢复单通道的样式
        videoLabel2->setStyleSheet("background-color: #333333; border:1px solid white;");
        videoLabel3->setStyleSheet("background-color: #333333; border:1px solid white;");
        videoLabel4->setStyleSheet("background-color: #333333; border:1px solid white;");
    }
    else
    {
        qDebug() << "切换到四通道";

        // 重新加载图片
        QString houseImagePath = ":/images/house.png";
        QPixmap backupPixmap(houseImagePath);

        if (!backupPixmap.isNull()) {
            qDebug() << "四通道：加载图片，尺寸：" << backupPixmap.size();

            // 根据Label的实际尺寸缩放图片
            QSize labelSize = videoLabel2->size();
            if (labelSize.width() <= 10 || labelSize.height() <= 10) {
                labelSize = QSize(300, 200);
            }

            QPixmap scaledPixmap = backupPixmap.scaled(
                labelSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );

            qDebug() << "四通道：Label尺寸：" << labelSize << "，缩放后图片尺寸：" << scaledPixmap.size();

            // 设置图片
            videoLabel2->setPixmap(scaledPixmap);
            videoLabel2->setText("");

            videoLabel3->setPixmap(scaledPixmap.copy());
            videoLabel3->setText("");

            videoLabel4->setPixmap(scaledPixmap.copy());
            videoLabel4->setText("");

            // 设置透明背景，确保图片显示
            videoLabel2->setStyleSheet("border:1px solid white; background-color: transparent;");
            videoLabel3->setStyleSheet("border:1px solid white; background-color: transparent;");
            videoLabel4->setStyleSheet("border:1px solid white; background-color: transparent;");

            // 强制更新显示
            videoLabel2->update();
            videoLabel3->update();
            videoLabel4->update();
        }
        else
        {
            qDebug() << "四通道：图片加载失败，使用默认背景";
            videoLabel2->setStyleSheet("background-color: #555555; border:1px solid white;");
            videoLabel3->setStyleSheet("background-color: #555555; border:1px solid white;");
            videoLabel4->setStyleSheet("background-color: #555555; border:1px solid white;");
            videoLabel2->setText("备用通道2\n图片加载失败");
            videoLabel3->setText("备用通道3\n图片加载失败");
            videoLabel4->setText("备用通道4\n图片加载失败");
        }
    }
}

// 添加摄像头错误处理函数
void MainWindow::handleCameraError(const QString &errorMsg)
{
    QMessageBox::critical(this, "摄像头错误", "无法打开摄像头: " + errorMsg);

    if (cameraThread)
    {
        cameraThread->requestInterruption();
        if (!cameraThread->wait(5000)) {
            cameraThread->terminate();
            cameraThread->wait(1000);
        }
        delete cameraThread;
        cameraThread = nullptr;
    }

    openSettingWindow();
}

// 添加打开设置界面的函数
void MainWindow::openSettingWindow()
{
    qDebug() << "尝试打开设置界面";
    SettingWidget *settingWin = new SettingWidget();
    connect(settingWin, SIGNAL(configSaved()), this, SLOT(onSettingSaved()));
    settingWin->show();
}

void MainWindow::onSettingSaved()
{
    loadVideoConfig(); // 加载新配置
    // 验证摄像头是否可用
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    bool cameraValid = false;
    foreach (const QCameraInfo &info, cameras)
    {
        // 匹配友好名称
        if (info.description() == currentCamera)
        {
            cameraValid = true;
            break;
        }
    }

    if (!cameraValid)
    {
        QMessageBox::warning(this, "配置无效", "所选摄像头不可用，请重新设置");
        openSettingWindow();
        return;
    }
    
    // 配置有效，启动摄像头
    QTimer::singleShot(100, this, &MainWindow::startCameraMonitoring);
}

void MainWindow::initDeviceList()
{
    //创建表格视图
    QTableView *deviceTabel=new QTableView(this->rightTopWidget);
    deviceTabel->setModel(DeviceControl::getInstance()->getModel());
    // ========== 设置表格列宽 ==========
    deviceTabel->horizontalHeader()->setStretchLastSection(false);
    deviceTabel->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);  // ID自适应（设备ID）
    deviceTabel->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);        // 名称手动设置（设备名称）
    deviceTabel->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);  // 类型自适应（设备类型）
    deviceTabel->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);       // 时间手动设置（接入时间）
    deviceTabel->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);  // 状态自适应（状态）

    // 设置各列宽度
    deviceTabel->setColumnWidth(1, 120);  // 设备名称：120像素
    deviceTabel->setColumnWidth(3, 160);  // 接入时间：160像素
    deviceTabel->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);  // 状态自适应
    deviceTabel->setStyleSheet("background-color: #4A90E2; color: #0F3460; border: none;");
    deviceTabel->horizontalHeader()->setStyleSheet("color: black; font-weight: bold;");
    //添加右侧顶部布局
    QVBoxLayout *rightTopLayout=new QVBoxLayout(this->rightTopWidget);
    QLabel *titleLabel=new QLabel("设备列表");
    titleLabel->setStyleSheet("color: pink; font-size: 18px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    rightTopLayout->addWidget(titleLabel);
    rightTopLayout->addWidget(deviceTabel);
    rightTopWidget->setLayout(rightTopLayout);
    // 加载设备数据
    DeviceControl::getInstance()->loadDevices();
}

void MainWindow::checkAlarms()
{
    QSettings settings("../config.ini",QSettings::IniFormat);
    double tempGeneral = settings.value("AlarmThreshold/TempGeneral", 30.0).toDouble();
    double tempSerious = settings.value("AlarmThreshold/TempSerious", 35.0).toDouble();
    double humGeneral = settings.value("AlarmThreshold/HumidityGeneral", 60.0).toDouble();
    double humSerious = settings.value("AlarmThreshold/HumiditySerious", 70.0).toDouble();
    double lightGeneral = settings.value("AlarmThreshold/LightGeneral", 200.0).toInt();
    double lightSerious = settings.value("AlarmThreshold/LightSerious", 100.0).toInt();

    this->checkSingleAlarm("温度", currentTemp, tempGeneral, tempSerious);
    this->checkSingleAlarm("湿度", currentHum, humGeneral, humSerious);
    this->checkSingleAlarm("光照", currentLux, lightGeneral, lightSerious);

    this->updateAlarmTable();
    this->updateAlarmPieChart();
}

void MainWindow::checkSingleAlarm(const QString &type, double value, double generalThresh, double seriousThresh)
{
    QString level;
    double threshold;

    if (type == "光照")
    {
    // 光照逻辑：≤严重阈值 → 严重警告；≤一般阈值 → 一般警告
        if (value <= seriousThresh)
        {
           level = "严重告警";
           threshold = seriousThresh;
        }
        else if (value <= generalThresh)
        {
           level = "一般告警";
           threshold = generalThresh;
         }
        else
        {
           return; // 不触发告警
         }
    }
    else
    {
    // 温湿度逻辑：≥严重阈值 → 严重警告；≥一般阈值 → 一般警告
       if (value >= seriousThresh)
       {
           level = "严重告警";
           threshold = seriousThresh;
       }
       else if (value >= generalThresh)
       {
           level = "一般告警";
           threshold = generalThresh;
       } else {
           return; // 不触发告警
       }
    }

    QDateTime now=QDateTime::currentDateTime();
    bool isDuplicate=false;
    foreach(const AlarmInfo &alarm,alarmList)
    {
        if(alarm.getAlarmType()==type&&alarm.getAlarmLevel()==level&&alarm.getAlarmTime().secsTo(now)<60)
        {
            isDuplicate=true;
            break;
        }
    }

    if(!isDuplicate)
    {
        AlarmInfo newAlarm;
        newAlarm.setAlarmId(alarmList.size()+1);
        newAlarm.setAlarmType(type);
        newAlarm.setAlarmLevel(level);
        newAlarm.setThresholdValue(threshold);
        newAlarm.setActualValue(value);
        newAlarm.setAlarmTime(now);
        newAlarm.setStatus(0);

        alarmList.append(newAlarm);
        this->saveAlarmToDatabase(newAlarm);
        this->handleDeviceControl(newAlarm);
    }
}

void MainWindow::saveAlarmToDatabase(const AlarmInfo &alarm)
{
    QString sql = QString("INSERT INTO alarm "
                        "(alarm_id, alarm_type, alarm_level, threshold_value, actual_value, alarm_time, status) "
                        "VALUES (%1, '%2', '%3', %4, %5, '%6', %7)")
                   .arg(alarm.getAlarmId())
                   .arg(alarm.getAlarmType().replace("'", "''"))
                   .arg(alarm.getAlarmLevel().replace("'", "''"))
                   .arg(alarm.getThresholdValue())
                   .arg(alarm.getActualValue())
                   .arg(alarm.getAlarmTime().toString("yyyy-MM-dd HH:mm:ss"))
                   .arg(alarm.getStatus());
    SingletonDB::getInstance()->execSql(sql);
}

void MainWindow::handleDeviceControl(const AlarmInfo &alarm)
{
    // 温度严重告警且风扇未开启
   if(alarm.getAlarmType() == "温度"&& alarm.getAlarmLevel() == "严重告警"&& !isFanOn)
   {
       if(QMessageBox::warning(this, "警告！", "温度异常，是否开风扇？",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
       {
           this->onFanControlClicked();
       }
   }
   else if(alarm.getAlarmType() == "光照" && alarm.getAlarmLevel() == "严重告警"&& !isLedOn)
   {
       if(QMessageBox::warning(this, "警告！", "光照异常，是否开灯？",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
       {
           this->onLedControlClicked();
       }
   }
}

void MainWindow::updateAlarmTable()
{
    this->alarmTableWidget->clearContents();
    QList<AlarmInfo>sortedAlarms=alarmList;
    sort(sortedAlarms.begin(),sortedAlarms.end(),[](const AlarmInfo &a,const AlarmInfo &b)
    {return a.getAlarmTime()>b.getAlarmTime();});
    int startIndex=currentAlarmPage*5;
    int displayCount=0;
    for(int i=startIndex;i<sortedAlarms.size()&&displayCount<5;i++)
    {
        const AlarmInfo &alarm=sortedAlarms[i];
        QTableWidgetItem *typeItem=new QTableWidgetItem(alarm.getAlarmType());
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
        typeItem->setBackgroundColor(QColor("#0F3460"));
        typeItem->setTextColor(QColor("white"));
        alarmTableWidget->setItem(displayCount, 0, typeItem);

        QTableWidgetItem *levelItem = new QTableWidgetItem(alarm.getAlarmLevel());
        levelItem->setFlags(levelItem->flags() & ~Qt::ItemIsEditable);
        levelItem->setBackgroundColor(QColor("#0F3460"));
        levelItem->setTextColor(alarm.getAlarmLevel() == "严重告警" ? QColor("red") : QColor("yellow"));
        alarmTableWidget->setItem(displayCount, 1, levelItem);

        QTableWidgetItem *timeItem = new QTableWidgetItem(alarm.getAlarmTime().toString("yyyy-MM-dd HH:mm:ss"));
        timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
        timeItem->setBackgroundColor(QColor("#0F3460"));
        timeItem->setTextColor(QColor("white"));
        alarmTableWidget->setItem(displayCount, 2, timeItem);

        displayCount++;
    }
}

void MainWindow::updateAlarmPieChart()
{
    alarmPieSeries->clear();

    int tempCount = 0, humCount = 0, lightCount = 0;
    foreach (const AlarmInfo &alarm, alarmList)
    {
        if (alarm.getAlarmType() == "温度") tempCount++;
        else if (alarm.getAlarmType() == "湿度") humCount++;
        else if (alarm.getAlarmType() == "光照") lightCount++;
    }

    if (tempCount > 0)
    {
        QPieSlice *tempSlice = alarmPieSeries->append(QString("温度(%1)").arg(tempCount), tempCount);
        tempSlice->setBrush(QColor("#FF5252"));
    }
    if (humCount > 0)
    {
        QPieSlice *humSlice = alarmPieSeries->append(QString("湿度(%1)").arg(humCount), humCount);
        humSlice->setBrush(QColor("#2196F3"));
    }
    if (lightCount > 0)
    {
        QPieSlice *lightSlice = alarmPieSeries->append(QString("光照(%1)").arg(lightCount), lightCount);
        lightSlice->setBrush(QColor("#FFC107"));
    }

    foreach (QPieSlice *slice, alarmPieSeries->slices())
    {
        slice->setLabel(QString("%1%").arg(QString::number(slice->percentage() * 100, 'f', 1)));
    }
}

void MainWindow::onAlarmPageTimeout()
{
    int totalPage=(alarmList.size()+4)/5;
    if(totalPage<=1) return;

    this->currentAlarmPage=(currentAlarmPage+1)%totalPage;
    this->updateAlarmTable();
}

void MainWindow::onLoginLabelClicked()
{
    if (!videoLabel1 || !cameraThread && !currentCamera.isEmpty())
       {
           QMessageBox::information(this, "提示", "界面正在初始化，请稍候");
           return;
       }
    if (UserController::getInstance()->isLoggedIn())
    {
        // 已登录，直接显示回放界面
        showPlay();
    }
    else
    {
        // 未登录，显示登录界面
        if (loginWidget)
        {
            loginWidget->deleteLater();
        }
        loginWidget = new LoginWidget();
        connect(loginWidget, &LoginWidget::loginSuccess, this, &MainWindow::onLoginSuccess);
        loginWidget->show();
     }
}

void MainWindow::onLoginSuccess()
{
    // 登录成功后直接显示回放界面
    if (loginWidget)
    {
        loginWidget->close();
        loginWidget->deleteLater();
        loginWidget = nullptr;
    }

    showPlay();
}

void MainWindow::showPlay()
{
    static bool isEntering = false;
    if (isEntering) return; // 防止重复进入
    isEntering = true;


    // 安全检查
    if (!currentCamera.isEmpty())
    {
        // 检查摄像头线程状态，但不阻止进入
        if (cameraThread && cameraThread->isRunning())
        {
            qDebug() << "摄像头线程正在运行，不影响回放界面";
        }
    }

    // 清理旧窗口（添加更安全的清理）
    if (playbackWidget)
    {
        disconnect(playbackWidget, nullptr, this, nullptr);
        playbackWidget->close();
        delete playbackWidget;
        playbackWidget = nullptr;
    }


    // 创建新的回放窗口
    playbackWidget = new VideoPlaybackWidget();

    // 连接返回主界面信号
    connect(playbackWidget, &VideoPlaybackWidget::backToMain, this, [this]() {
        // 返回时销毁当前回放窗口
        if (playbackWidget)
        {
            playbackWidget->deleteLater();
            playbackWidget = nullptr;
        }
        this->show(); // 显示主界面
    });

    // 连接logout信号
    connect(playbackWidget, &VideoPlaybackWidget::logout, this, [this]() {
        qDebug() << "收到logout信号，用户需要重新登录才能进入回放";

        // 关闭回放窗口
        if (playbackWidget)
        {
            playbackWidget->deleteLater();
            playbackWidget = nullptr;
        }

        // 显示主界面
        this->show();
        QMessageBox::information(this, "提示", "您已退出登录，下次需要重新登录");
    });

    playbackWidget->dinshiqi();
    // 显示回放窗口
    playbackWidget->show();
    this->hide();
    isEntering = false;
}

void MainWindow::showMainWindow()
{
    this->show();  // 显示主界面

}
